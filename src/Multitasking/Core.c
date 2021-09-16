#include <Stivale2.h>
#include <Common.h>
#include <Core.h>
#include <APIC.h>


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

void ProcMessage(struct IPMessage *msg, size_t count, uint32_t dest)
{
	if(dest == IPMSG_BROADCAST) {

		for(size_t i = 0; i < ProcCount(); i++)
			ProcMessage(msg, count, i);

		return;
	}

	if(dest >= ProcCount())
		Panic(NULL, "Invalid message destination");

	struct Processor *proc = &procs[dest];

	while(1) {
		Lock(&proc->msg_cntl.lock);

		if(proc->msg_cntl.busy) {
			Unlock(&proc->msg_cntl.lock);

			Sleep(10000);

			continue;
		}

		break;
	}

	proc->msg_cntl.msg_list  = msg;
	proc->msg_cntl.msg_count = count;

	proc->msg_cntl.busy = 1;

	Unlock(&proc->msg_cntl.lock);

	IPISend(0, dest);
}
