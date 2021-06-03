#pragma once

#include <Memory.h>
#include <stddef.h>
#include <stdint.h>

struct Processor
{
	struct AddressSpace *space;
};

struct Processor *Processors();

struct Processor *ProcCurrent();

uint64_t ProcID();
