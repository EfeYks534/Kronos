#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef NULL

#define NULL ((void*)0)

#endif

#define Halt() { while(1) asm volatile("hlt"); }

#define PACKED __attribute__((packed))

#define PHYS_OFFSET   0xFFFF800000000000

#define KERNEL_OFFSET 0xFFFFFFFF80000000

#define PhysOffset(x) ((void*) PhysOffseti((uintptr_t) (x)))

#define KEINIT __attribute__((constructor(0), used, noinline))

#define KLINIT __attribute__((constructor(1), used, noinline))

#define KDINIT __attribute__((constructor(2), used, noinline))

#define Assert(expr, str) _Assert((expr), #expr, str, __FILE__, __LINE__)

struct Registers
{
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t  r9;
	uint64_t  r8;
	uint64_t rbp;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;

	uint64_t vector;
	uint64_t  error;

	uint64_t   rip;
	uint64_t    cs;
	uint64_t flags;
	uint64_t   rsp;
	uint64_t    ss;
} PACKED;

struct CPUID
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
};


void Sleep(size_t nsecs);

void _Assert(int expr, const char *sexpr, const char *str, char *file, int line);

struct CPUID CPUID(uint32_t func);

uint64_t FlagsGet();

void FlagsSet(uint64_t flags);

void KernelEarlyInit();

void KernelLateInit();

void KernelDeviceInit();

void KernelMain();

void Panic(struct Registers *regs, const char *fmt, ...);

void Log(const char *fmt, ...);

void Info(const char *fmt, ...);

void Warn(const char *fmt, ...);

void Error(const char *fmt, ...);

void Out8(uint16_t  port, uint8_t  data);
void Out16(uint16_t port, uint16_t data);
void Out32(uint16_t port, uint32_t data);

uint8_t  In8(uint16_t  port);
uint16_t In16(uint16_t port);
uint32_t In32(uint16_t port);

void IOWait();

static inline uintptr_t PhysOffseti(uintptr_t addr)
{
	return addr < PHYS_OFFSET ? addr + PHYS_OFFSET : addr;
}

static inline size_t ceil(size_t num, size_t div)
{
	return num / div + (num % div == 0 ? 0 : 1);
}

static inline size_t log2(size_t num)
{
	return num == 0 ? 0 : 63 - __builtin_clzll(num);
}

static inline size_t min(size_t a, size_t b)
{
	return a < b ? a : b;
}

static inline size_t max(size_t a, size_t b)
{
	return a > b ? a : b;
}
