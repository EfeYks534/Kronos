#include <Common.h>
#include <Core.h>
#include <Task.h>

extern int is_panicking;

void Lock(int64_t *lock)
{
	if(is_panicking) return;

	int ifl = (FlagsGet() >> 9) & 1;

	asm volatile("cli");

	uint32_t *status = (uint32_t*) lock;

	uint32_t e = 0;

	int r = 0;

	size_t tries = 1000000;

	do
	{
		r = __atomic_compare_exchange_n(status, &e, 1, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
		e = 0;

		asm volatile("pause");
	} while(!r && --tries);

	if(tries == 0) {
		*lock = 0;
		Panic(NULL, "Deadlocked");
	}

	uint32_t *higher = (uint32_t*) ((uintptr_t) lock + 4);

	*higher = ifl;
}

void Unlock(int64_t *lock)
{
	if(is_panicking) return;

	uint32_t *higher = (uint32_t*) ((uintptr_t) lock + 4);

	if(*higher)
		asm volatile("sti");

	__atomic_store_n(lock, 0, __ATOMIC_RELEASE);

}
