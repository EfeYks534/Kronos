#include <Common.h>
#include <Task.h>

void Lock(int64_t *lock)
{
	int64_t e = 0;

	int r = 0;

	size_t tries = 1000000;

	r = __atomic_compare_exchange_n(lock, &e, 1, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
	e = 0;

	while(!r && --tries) {
		r = __atomic_compare_exchange_n(lock, &e, 1, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
		e = 0;

		asm volatile("pause");
	}

	if(tries == 0)
		Panic(NULL, "Deadlocked");
}

void LockYield(int64_t *lock)
{
	int64_t e = 0;

	int r = 0;

	size_t tries = 1000000;

	r = __atomic_compare_exchange_n(lock, &e, 1, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
	e = 0;

	while(!r && --tries) {
		r = __atomic_compare_exchange_n(lock, &e, 1, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
		e = 0;

		Yield();

		asm volatile("pause");
	}

	if(tries == 0)
		Panic(NULL, "Deadlocked");
}

void Unlock(int64_t *lock)
{
	__atomic_store_n(lock, 0, __ATOMIC_RELEASE);
}
