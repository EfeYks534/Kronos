#include <Common.h>
#include <Core.h>
#include <Device.h>
#include <Memory.h>
#include <Stivale2.h>
#include <DescTabs.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void KernelLate()
{
	KernelLateInit();
}

void KernelIdle(uint64_t is_idle)
{
	if(is_idle) {
		if(ProcID() == ProcBSP()) {
			while(1) {
				int n = 0;

				for(int i = 0; i < ProcCount(); i++)
					if(Processors()[i].idle_task == NULL)
						n = 1;

				if(n == 0)
					break;
			}

			KernelLate();
		}

		while(1)
			Yield();
	}

	struct Task *task = calloc(1, sizeof(struct Task));

	task->tid = TIDNew();

	task->space   = MKernel();
	task->privl   = 1;
	task->invl    = 1;
	task->pause   = 1;
	task->core    = ProcID();

	uint64_t stack = (uint64_t) calloc(16384, 1);

	task->regs.ss    = 0x10;
	task->regs.rsp   = stack + 16376;
	task->regs.flags = 1ULL << 21;
	task->regs.cs    = 0x08;
	task->regs.rip   = (uintptr_t) KernelIdle;
	task->regs.rdi   = 1;

	ProcCurrent()->idle_task = task;

	TaskAdd(task);
}

void APMain()
{
	FlagsSet(FlagsGet() | (1ULL << 21));

	GDTInstall();
	IDTInstall();

	MSwitch(MKernel());

	struct SMInfo *sm_info = SysMemInfo();

	KernelIdle(0);

	while(1)
		Yield();
}

void KernelMain()
{
	FlagsSet(FlagsGet() | (1ULL << 21));

	GDTLoad();
	IDTLoad();

	PMInit();
	VMInit();	

	struct SMInfo *sm_info = SysMemInfo();

	KernelDeviceInit();
	KernelEarlyInit();

	Info("Total memory   : %l KiBs\n", sm_info->pm_total  / 1024);
	Info("Usable memory  : %l KiBs\n", sm_info->pm_usable / 1024);
	Info("Used memory    : %l KiBs\n", sm_info->pm_used   / 1024);
	Info("Virtual memory : %l KiBs\n", sm_info->vm_total  * 4096 / 1024);

	struct stivale2_struct_tag_smp *smp;
	smp = Stivale2GetTag(STIVALE2_STRUCT_TAG_SMP_ID);

	for(uint64_t i = 0; i < smp->cpu_count; i++) {
		struct stivale2_smp_info *core = &smp->smp_info[i];
		if(core->lapic_id == smp->bsp_lapic_id) continue;

		uint64_t stack = PMAlloc();

		core->target_stack = PhysOffseti(stack + 4088);
		core->goto_address = (uint64_t) APMain;
	}

	KernelIdle(0);

	while(1)
		Yield();
}
