#include <Common.h>
#include <Core.h>
#include <Stivale2.h>

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

size_t ProcCount()
{
	struct stivale2_struct_tag_smp *smp;
	smp = Stivale2GetTag(STIVALE2_STRUCT_TAG_SMP_ID);

	return smp->cpu_count;
}

uint64_t ProcBSP()
{
	struct stivale2_struct_tag_smp *smp;
	smp = Stivale2GetTag(STIVALE2_STRUCT_TAG_SMP_ID);

	return smp->bsp_lapic_id;
}
