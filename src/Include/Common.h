#include <stddef.h>
#include <stdint.h>

#ifndef NULL

#define NULL ((void*) 0 )

#endif

#define PACKED __attribute__((packed))

static inline void Halt()
{
	while(1)
		asm volatile("hlt");
}
