#pragma once

#include <Task.h>
#include <Memory.h>
#include <stddef.h>
#include <stdint.h>

struct Processor
{
	struct AddressSpace *space;
	struct Task          *task;
	struct Task     *idle_task;
};

struct Processor *Processors();

struct Processor *ProcCurrent();

uint64_t ProcID();

uint64_t ProcBSP();

size_t ProcCount();



