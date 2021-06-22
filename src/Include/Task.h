#pragma once

#include <Common.h>
#include <Memory.h>
#include <stddef.h>
#include <stdint.h>

#define TICKET_IDEAL 100

struct Task
{
	uint64_t               tid; // Task ID
	char              name[64]; // Task name (null-terminated string)
	size_t               timer; // Keeps track of time in nanoseconds
	size_t             quantum; // Nanoseconds until next task switch, counts down
	size_t             tickets; // How many lottery tickets this task has

	struct
	{
		uint64_t   privl : 1; // Task privilege level, 0 for user and 1 for supervisor.

		uint64_t    invl : 1; // Invulnerability flag, the   task cannot be  killed if
		                      // this flag is set.

		uint64_t    dead : 1; // The task wont be scheduled if  the `dead` flag is set
		                      // If the `invl` flag isn't  set it will be  sent to the
		                      // dead task queue where it will  be killed by the  task
		                      // killer. If the `invl` flag is set the task will still
		                      // be scheduled.

		uint64_t   pause : 1; // The task wont be scheduled if this  flag is set, how-
		                      // ever, the task wont be killed and can be unpaused.

		uint64_t running : 1; // Set if the task is currently running

		uint64_t    core : 8; // The core this task is running in
	};

	struct AddressSpace *space; // This task's address space. Compare with MKernel()
	                            // before trying to `free` it.

	struct Registers      regs __attribute__((aligned(16))); // General purpose regs
	uint8_t     ext_regs[2048] __attribute__((aligned(16))); // Other registers
};

struct Task *TaskGet(uint64_t tid);

void TaskAdd(struct Task *task); // Adds this task to the task list

struct Task *TaskSpawn(void (*ent)(), uint64_t arg, size_t tickets); // Spawn task

void TaskKill(struct Task *task);

void Exit(uint64_t code);

void Schedule(struct Registers *regs);

void Yield();

void Lock(int64_t *lock);

void Unlock(int64_t *lock);

uint64_t TIDNew();

static inline uint64_t TIDHash(uint64_t tid)
{
	return tid % 512;
}
