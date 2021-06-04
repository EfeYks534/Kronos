#pragma once

#include <Memory.h>
#include <stddef.h>
#include <stdint.h>

struct Task
{
	uint64_t              tid; // Task ID
	char             name[64]; // Task name (null-terminated string)
	size_t              timer; // Keeps track of time in nanoseconds
	size_t            quantum; // Nanoseconds until next task switch, counts down
	size_t            tickets; // How many lottery tickets this task has

	struct AddressSpace space; // Tasks can't share address spaces

	uint8_t         regs[512] __attribute__((aligned(16))); // General purpose registers
	uint8_t     ext_regs[512] __attribute__((aligned(16))); // Other registers
};

void Lock(int64_t *lock);

void Unlock(int64_t *lock);
