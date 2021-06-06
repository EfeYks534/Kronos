#include <Task.h>

void Lock(int64_t *lock)
{
	if(*lock)
		Panic(NULL, "Nested lock");

	*lock = 1;

	return;
}

void Unlock(int64_t *lock)
{
	*lock = 0;
	return;
}
