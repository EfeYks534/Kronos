#include <Common.h>
#include <Core.h>
#include <Stivale2.h>
#include <Device.h>
#include <ACPI.h>
#include <Task.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define HPET_CAPID   0x000
#define HPET_CONFIG  0x010
#define HPET_INTR    0x020
#define HPET_COUNTER 0x0F0

#define HPET_TIMER_CONFIG(N) (0x100 + 0x20 * (N))
#define HPET_TIMER_COMPAR(N) (0x108 + 0x20 * (N))
#define HPET_TIMER_FSB(N)    (0x110 + 0x20 * (N))

struct HPETHandler
{
	size_t time;

	void (*handler) (struct DevTimer*);
};

struct HPETState
{
	struct HPET *hpet;
	size_t       time;
	size_t       freq;
	uint64_t     hmap;

	struct HPETHandler handlers[64];
};


static void HPETInit(struct DevTimer *timer)
{
	struct HPETState *state = calloc(1, sizeof(struct HPETState));

	timer->state = state;

	state->hpet = ACPIFindSDT("HPET");
	state->hmap = -1ULL;

	Assert(state->hpet != NULL, "");

	Assert(state->hpet->address.space_id == 0, "");


	
}

void HPETReset(struct DevTimer *timer)
{
	Lock(&timer->dev.lock);

	if(!timer->dev.enabled) {
		Unlock(&timer->dev.lock);
		return;
	}

	struct HPETState *state = timer->state;

	state->time = 0;

	Unlock(&timer->dev.lock);
}

size_t HPETTime(struct DevTimer *timer)
{
	Lock(&timer->dev.lock);

	struct HPETState *state = timer->state;

	size_t time = state->time;

	Unlock(&timer->dev.lock);

	return time;
}

void HPETHandlerRegister(struct DevTimer *timer, void (*h)(struct DevTimer*), size_t n)
{
	Lock(&timer->dev.lock);

	if(!timer->dev.enabled) {
		Unlock(&timer->dev.lock);
		return;
	}

	struct HPETState *state = timer->state;

	if(state->hmap == 0) {
		Unlock(&timer->dev.lock);
		return;
	}

	size_t bit = __builtin_ffsll(state->hmap) - 1;

	state->hmap &= ~(1ULL << bit);

	state->handlers[bit] = (struct HPETHandler) { n, h };

	Unlock(&timer->dev.lock);
}

void HPETHandlerUnregister(struct DevTimer *timer, void (*h)(struct DevTimer*))
{
	Lock(&timer->dev.lock);

	if(!timer->dev.enabled) {
		Unlock(&timer->dev.lock);
		return;
	}

	struct HPETState *state = timer->state;

	if(state->hmap == -1ULL) {
		Unlock(&timer->dev.lock);
		return;
	}

	for(int i = 0; i < 64; i++) {
		if((state->hmap >> i) & 1) continue;

		if(state->handlers[i].handler == h) {
			state->hmap |= 1ULL << i;
			break;
		}
	}

	Unlock(&timer->dev.lock);
}
