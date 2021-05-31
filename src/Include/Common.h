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

#define PhysOffset(x) ((void*) PhysOffseti((uintptr_t) x))

#define KEINIT __attribute__((constructor(0), used, noinline))

#define KLINIT __attribute__((constructor(1), used, noinline))

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

void KernelEarlyInit();

void KernelLateInit();

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
