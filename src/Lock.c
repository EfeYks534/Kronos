#include <Common.h>
#include <Core.h>
#include <Task.h>

static int saved_ifl[256] = { 0 };

extern int is_panicking;

void Lock(int64_t *lock)
{
	if(is_panicking) return;

	saved_ifl[ProcID()] = (FlagsGet() >> 9) & 1;

	uint8_t type = (*(uint16_t*) lock >> 8) & 0xFF;

	if(type == 0)
		asm volatile("cli");


	uint8_t e = 0;

	int r = 0;

	size_t tries = 1000000;

	uint8_t *status = (uint8_t*) lock;


	do
	{
		r = __atomic_compare_exchange_n(status, &e, 1, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
		e = 0;

		if(type == 1)
			Yield();

		asm volatile("pause");
	} while(!r && --tries);

	if(tries == 0) {
		*status = 0;
		Panic(NULL, "Deadlocked");
	}
}

void Unlock(int64_t *lock)
{
	if(is_panicking) return;

	uint8_t *status = (uint8_t*) lock;

	__atomic_store_n(status, 0, __ATOMIC_RELEASE);

	uint8_t type = (*(uint16_t*) lock >> 8) & 0xFF;

	if(type == 0 && saved_ifl[ProcID()])
		asm volatile("sti");
}
