#include <Common.h>
#include <Peripheral.h>

void MMWrite64(void *addr, uint64_t data)
{
	*((uint64_t*) addr) = data;
}

void MMWrite32(void *addr, uint32_t data)
{
	*((uint32_t*) addr) = data;
}

void MMWrite16(void *_addr, uint16_t data)
{
	uintptr_t addr =  (uintptr_t) _addr;
	uint32_t d     = *(uint32_t*) (addr & ~3ULL);

	((uint16_t*) &d)[(addr & 2ULL) ? 1 : 0] = data;

	*((uint32_t*) _addr) = d;
}

void MMWrite8(void *_addr, uint8_t data)
{
	uintptr_t addr =  (uintptr_t) _addr;
	uint32_t d     = *(uint32_t*) (addr & ~3ULL);

	((uint8_t*) &d)[addr & 3ULL] = data;

	*((uint32_t*) _addr) = d;
}

uint64_t MMRead64(void *addr)
{
	return *((uint64_t*) addr);
}

uint32_t MMRead32(void *addr)
{
	return *((uint32_t*) addr);
}

uint16_t MMRead16(void *addr)
{
	uintptr_t aaddr = (uintptr_t) addr & ~3ULL;
	return (*((uint32_t*) aaddr) >> ((((uintptr_t) addr & 2ULL) ? 1 : 0) * 16)) & 0xFFFF;
}

uint8_t MMRead8(void *addr)
{
	uintptr_t aaddr = (uintptr_t) addr & ~3ULL;
	return (*((uint32_t*) aaddr) >> (((uintptr_t) addr & 3ULL) * 8)) & 0xFF;
}
