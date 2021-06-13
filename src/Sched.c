#include <Common.h>
#include <Core.h>
#include <Memory.h>
#include <Task.h>
#include <Stivale2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void _Yield();

void TaskSwitch(struct Registers *regs, uint8_t *ext_regs);

struct Bucket
{
	struct Task *tasks[32];
	size_t           count;
};


static struct Bucket task_list[512] = { 0 };

static int64_t tasks_lock = 0;

static size_t task_count = 0;

static size_t ticket_count = 0;


struct Task *TaskGet(uint64_t tid)
{
	Lock(&tasks_lock);

	struct Bucket *bucket = &task_list[TIDHash(tid)];

	if(bucket->count == 0) return NULL;

	for(size_t i = 0; i < bucket->count; i++) {
		if(bucket->tasks[i] == NULL) continue;

		if(bucket->tasks[i]->tid == tid) {
			Unlock(&tasks_lock);
			return bucket->tasks[i];
		}
	}

	Unlock(&tasks_lock);

	return NULL;
}

void TaskAdd(struct Task *task)
{
	Lock(&tasks_lock);

	task_count++;

	ticket_count += task->tickets;

	uint64_t tid = task->tid;

	struct Bucket *bucket = &task_list[TIDHash(tid)];

	for(size_t i = 0; i < bucket->count; i++) {
		if(bucket->tasks[i] == NULL) {
			bucket->tasks[i] = task;

			Unlock(&tasks_lock);
			return;
		}

		if(bucket->tasks[i]->tid == tid)
			Panic(NULL, "Tried to queue existing task %u", task->tid);
	}

	bucket->tasks[bucket->count++]= task;

	Unlock(&tasks_lock);
}

static size_t Random()
{
	static size_t  seed = 123456789;

	if(seed == 123456789) {
		struct stivale2_struct_tag_epoch *epoch;
		epoch = Stivale2GetTag(STIVALE2_STRUCT_TAG_EPOCH_ID);

		seed += epoch == NULL ? 0 : epoch->epoch;
	}

	seed = (6364136223846793005 * seed + 1442695040888963407);

	size_t shift = (seed >> 32) % 64;

	size_t rand = (seed >> shift) | (seed << (63 - shift));

	return rand;
}

static void Switch(struct Task *task, struct Registers *regs)
{
	struct Task *current = ProcCurrent()->task;

	if(current != NULL) {
		current->running = 0;
		memcpy(&current->regs, regs, sizeof(struct Registers));
	}

	ProcCurrent()->task = task;

	task->running = 1;

	task->quantum = 5000000 / (task_count == 0 ? 1 : task_count);


	MSwitch(task->space);

	Unlock(&tasks_lock);
	TaskSwitch(&task->regs, task->ext_regs);
}

void Schedule(struct Registers *regs)
{
	Lock(&tasks_lock);

	if(task_count > 0) {

		struct Task *task = NULL;

		size_t golden_ticket = Random() % (ticket_count == 0 ? 1 : ticket_count);

		size_t tickets = 0;

		for(int i = 0; i < 512; i++) {
			struct Bucket *bucket = &task_list[i];

			for(size_t j = 0; j < bucket->count; j++) {
				task = bucket->tasks[j];

				if(task == NULL) continue;
				if((task->dead && !task->invl) || task->pause || task->running) continue;

				if(task->core != ProcID()) continue;

				tickets += task->tickets;

				if(tickets < golden_ticket) continue;

				Switch(task, regs);
			}
		}
	}

	Switch(ProcCurrent()->idle_task, regs);
}

uint64_t TIDNew()
{
	static _Atomic uint64_t tid_last = 0;

	return tid_last++;
}

void Yield()
{
	int ifl = (FlagsGet() >> 9) & 1;

	asm volatile("cli");


	if(ProcCurrent()->idle_task != NULL)
		_Yield();


	if(ifl)
		asm volatile("sti");
}

struct Task *TaskSpawn(void (*ent)(), uint64_t arg, size_t tickets)
{
	struct Task *task = calloc(1, sizeof(struct Task));

	static _Atomic size_t next_core = 0;

	task->tid     = TIDNew();
	task->tickets = tickets;
	task->privl   = 1;
	task->space   = MKernel();
	task->core    = next_core++ % ProcCount();

	uint64_t stack = (uint64_t) calloc(16384, 1);

	task->regs.ss    = 0x10;
	task->regs.rsp   = stack + 16384;
	task->regs.flags = 1ULL << 21;
	task->regs.cs    = 0x08;
	task->regs.rip   = (uintptr_t) ent;
	task->regs.rdi   = arg;

	TaskAdd(task);

	return task;
}
