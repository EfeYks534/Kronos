#include <Common.h>
#include <Core.h>
#include <Peripheral.h>
#include <DescTabs.h>
#include <Stivale2.h>
#include <Device.h>
#include <APIC.h>
#include <ACPI.h>
#include <Task.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define HPET_RATE 1000000

#define HPET_CAPID   0x000
#define HPET_CONFIG  0x010
#define HPET_INTR    0x020
#define HPET_COUNTER 0x0F0

#define HPET_TIMER_CONFIG(N) (0x100 + 0x20 * (N))
#define HPET_TIMER_COMPAR(N) (0x108 + 0x20 * (N))
#define HPET_TIMER_FSB(N)    (0x110 + 0x20 * (N))

struct HPETTimerCfg
{
	uint64_t           rsvd0 :  1;
	uint64_t    int_type_cnf :  1;
	uint64_t     int_enb_cnf :  1;
	uint64_t        type_cnf :  1;
	uint64_t     per_int_cap :  1;
	uint64_t        size_cap :  1;
	uint64_t     val_set_cnf :  1;
	uint64_t           rsvd1 :  1;
	uint64_t      mode32_cnf :  1;
	uint64_t   int_route_cnf :  5;
	uint64_t      fsb_en_cnf :  1;
	uint64_t fsb_int_del_cap :  1;
	uint64_t           rsvd2 : 16;
	uint64_t   int_route_cap : 32;
} PACKED;

struct HPETCapID
{
	uint64_t             rev_id :  8;
	uint64_t        num_tim_cap :  5;
	uint64_t     count_size_cap :  1;
	uint64_t               rsvd :  1;
	uint64_t         leg_rt_cap :  1;
	uint64_t             vendor : 16;
	uint64_t counter_clk_period : 32;
} PACKED;

struct HPETHandler
{
	struct
	{
		size_t     time : 63;
		size_t periodic :  1;
	} PACKED;

	size_t counter;

	void (*handler) (struct DevTimer*);
};

struct HPETState
{
	struct HPET *hpet;
	uint8_t     *regs;
	size_t       time;
	size_t       freq;
	size_t     period;
	size_t hand_count;

	struct HPETHandler handlers[256];
};

static void HPETTick(struct HPETState *state, size_t nsecs)
{
	if(nsecs < state->period / 1000000) nsecs = state->period / 1000000;

	nsecs = (nsecs * 1000000) / state->period;

	MMWrite64(&state->regs[HPET_TIMER_COMPAR(0)], MMRead64(&state->regs[HPET_COUNTER]) + nsecs);
}

static void HPETHandler(struct Registers *regs)
{
	struct DevTimer *timer = DeviceGet(DEV_CATEGORY_TIMER, DEV_TYPE_HPET, NULL);

	if(timer == NULL) {
		Error("HPET: Can't find timer device\n");
		APICEOI();
		return;
	}

	if(!timer->dev.enabled) {
		APICEOI();
		return;
	}

	Lock(&timer->dev.lock);

	struct HPETState *state = timer->state;


	for(int i = 0; i < state->hand_count; i++) {
		struct HPETHandler *hand = &state->handlers[i];

		if(--hand->counter == 0) {
			void (*h)(struct DevTimer*) = hand->handler;
			hand->counter = hand->time;


			if(!hand->periodic && i < (state->hand_count - 1)) {
				for(int j = i; j < state->hand_count - 2; j++)
					state->handlers[j] = state->handlers[j + 1];

				state->hand_count--;
			}

			h(timer);
		}
	}


	Unlock(&timer->dev.lock);

	APICEOI();

	HPETTick(state, HPET_RATE);

	// TODO: Schedule here
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

	MMWrite64(&state->regs[HPET_CONFIG], MMRead64(&state->regs[HPET_CONFIG]) & ~1ULL);
	MMWrite64(&state->regs[HPET_COUNTER], 0);
	MMWrite64(&state->regs[HPET_CONFIG], MMRead64(&state->regs[HPET_CONFIG]) | 1ULL);


	HPETTick(state, HPET_RATE);

	Unlock(&timer->dev.lock);
}

size_t HPETTime(struct DevTimer *timer)
{
	Lock(&timer->dev.lock);

	if(!timer->dev.enabled) {
		Unlock(&timer->dev.lock);
		return 0;
	}

	struct HPETState *state = timer->state;

	state->time = (MMRead64(&state->regs[HPET_COUNTER]) * state->period) / 1000000;

	size_t time = state->time;

	Unlock(&timer->dev.lock);

	return time;
}

size_t HPETRate(struct DevTimer *timer)
{
	return HPET_RATE;
}

void HPETOneShot(struct DevTimer *timer, void (*hand)(struct DevTimer*), size_t ticks)
{
	Lock(&timer->dev.lock);

	if(!timer->dev.enabled) {
		Unlock(&timer->dev.lock);
		return;
	}

	struct HPETState *state = timer->state;

	if(state->hand_count >= 256) {
		return;
	}

	struct HPETHandler handler = (struct HPETHandler) { 0 };

	handler.time     = ticks;
	handler.counter  = ticks;
	handler.periodic = 0;
	handler.handler  = hand;

	state->handlers[state->hand_count++] = handler;

	Unlock(&timer->dev.lock);

	return;
}

void HPETPeriodic(struct DevTimer *timer, void (*hand)(struct DevTimer*), size_t ticks)
{
	Lock(&timer->dev.lock);

	if(!timer->dev.enabled) {
		Unlock(&timer->dev.lock);
		return;
	}

	struct HPETState *state = timer->state;

	if(state->hand_count >= 256) {
		return;
	}

	struct HPETHandler handler = (struct HPETHandler) { 0 };

	handler.time     = ticks;
	handler.counter  = ticks;
	handler.periodic = 1;
	handler.handler  = hand;

	state->handlers[state->hand_count++] = handler;

	Unlock(&timer->dev.lock);

	return;
}


static KLINIT void HPETInit()
{
	struct DevTimer *timer = calloc(1, sizeof(struct DevTimer));

	timer->dev.type    = DEV_TYPE_HPET;
	timer->dev.enabled = 1;

	memcpy(timer->dev.name, "HPET", 5);

	timer->reset    = HPETReset;
	timer->time     = HPETTime;
	timer->one_shot = HPETOneShot;
	timer->periodic = HPETPeriodic;

	struct HPETState *state = calloc(1, sizeof(struct HPETState));

	timer->state = state;

	state->hpet = ACPIFindSDT("HPET");

	// Sanity check the HPET table

	if(state->hpet == NULL)
		return;

	if(state->hpet->address.space_id != 0)
		return;

	if(state->hpet->address.addr == 0)
		return;

	state->regs = PhysOffset(state->hpet->address.addr);


	struct HPETCapID cap = (struct HPETCapID) { 0 };

	*((uint64_t*) &cap) = MMRead64(&state->regs[HPET_CAPID]);

	if(cap.rev_id == 0)
		return;

	state->freq   = 0x38D7EA4C68000 / cap.counter_clk_period;
	state->period = cap.counter_clk_period;

	struct HPETTimerCfg tn_cfg = (struct HPETTimerCfg) { 0 } ;

	*((uint64_t*) &tn_cfg) = MMRead64(&state->regs[HPET_TIMER_CONFIG(0)]);

	if(tn_cfg.fsb_int_del_cap != 1) return;

	tn_cfg.int_enb_cnf  = 1;
	tn_cfg.fsb_en_cnf   = 1;
	tn_cfg.mode32_cnf   = 0;
	tn_cfg.int_type_cnf = 0;


	uint64_t vec = IDTEntryAlloc(IDT_ATTR_PRESENT | IDT_ATTR_TRAP, HPETHandler);

	uint64_t tn_fsb = 0;

	tn_fsb  = (0xFEE00000 | (ProcBSP() << 12)) << 32;
	tn_fsb |= vec;

	MMWrite64(&state->regs[HPET_TIMER_CONFIG(0)], *((uint64_t*) &tn_cfg));

	MMWrite64(&state->regs[HPET_TIMER_FSB(0)], tn_fsb);

	MMWrite64(&state->regs[HPET_CONFIG], MMRead64(&state->regs[HPET_CONFIG]) | 1);

	DeviceRegister(DEV_CATEGORY_TIMER, &timer->dev);
	DevicePrimarySet(DEV_CATEGORY_TIMER, &timer->dev);

	HPETTick(state, HPET_RATE);
}
