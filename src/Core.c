#include <Common.h>
#include <Core.h>

static struct Processor procs[256] = { 0 };

struct Processor *Processors()
{
	return procs;
}

uint64_t ProcID()
{
	return (CPUID(1).b & 0xFF000000) >> 24;
}

struct Processor *ProcCurrent()
{
	struct Processor *proc = &procs[ProcID()];
	return proc;
}
