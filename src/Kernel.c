#include <Common.h>
#include <Core.h>
#include <ACPI.h>
#include <APIC.h>
#include <Device.h>
#include <Memory.h>
#include <Stivale2.h>
#include <Peripheral.h>
#include <DescTabs.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int late_init = 0;

void KernelInit()
{
	KernelEarlyInit();

	struct SMInfo *sm_info = SysMemInfo();

	Log("\n");

	Info("Total memory   : %l KiBs\n", sm_info->pm_total   / 1024);
	Info("Usable memory  : %l KiBs\n", sm_info->pm_usable  / 1024);
	Info("Used memory    : %l KiBs\n", sm_info->pm_used    / 1024);
	Info("Virtual memory : %l KiBs\n", sm_info->vm_total * 4096 / 1024);
	Info("MAlloc memory  : %l KiBs\n\n", MAllocTotal()     / 1024);

	Info("Found %u ACPI system descriptor tables:\n", SDTCount());

	for(size_t i = 0; i < SDTCount(); i++) {
		struct SDTHeader *header = ACPIGetSDT(i);
		if(header == NULL) continue;

		Log("       + ");

		for(int j = 0; j < 4; j++)
			Log("%c", header->sign[j]);

		Log("\n");
	}
	Log("\n");

	KernelLateInit();

	struct DevTimer *timer = DevicePrimary(DEV_CATEGORY_TIMER);
	Assert(timer != NULL, "System can't run without a primary timer");

	asm volatile("sti");

	APICTimerEnable();
	late_init = 1;

	KernelDeviceInit();

	Info("Total memory   : %l KiBs\n", sm_info->pm_total   / 1024);
	Info("Usable memory  : %l KiBs\n", sm_info->pm_usable  / 1024);
	Info("Used memory    : %l KiBs\n", sm_info->pm_used    / 1024);
	Info("Virtual memory : %l KiBs\n", sm_info->vm_total * 4096 / 1024);
	Info("MAlloc memory  : %l KiBs\n\n", MAllocTotal()     / 1024);
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

			KernelInit();
		}


		int is_init = 0;

		while(1) {
			if(late_init && !is_init && ProcID() != ProcBSP()) {
				is_init = 1;

				APICTimerEnable();
			}

			if(is_init)
				asm volatile("hlt");
			else
				asm volatile("pause");
		}
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
	task->regs.rsp   = stack + 16384;
	task->regs.flags = 1ULL << 21;
	task->regs.cs    = 0x08;
	task->regs.rip   = (uintptr_t) KernelIdle;
	task->regs.rdi   = 1;

	ProcCurrent()->idle_task = task;

	TaskAdd(task);
}

void APMain()
{
	GDTInstall();
	IDTInstall();

	MSwitch(MKernel());

	struct SMInfo *sm_info = SysMemInfo();

	KernelIdle(0);

	while(1)
		Yield();
}

void VSerialInit();

void KernelMain()
{
	GDTLoad();
	IDTLoad();

	VSerialInit(); // We want to be able to debug everything pre-scheduler

	PMInit();
	VMInit();

	struct stivale2_struct_tag_smp *smp;
	smp = Stivale2GetTag(STIVALE2_STRUCT_TAG_SMP_ID);

	for(uint64_t i = 0; i < smp->cpu_count; i++) {
		struct stivale2_smp_info *core = &smp->smp_info[i];
		if(core->lapic_id == smp->bsp_lapic_id) continue;

		uint64_t stack = PMAlloc();

		core->target_stack = PhysOffseti(stack + 4096);
		core->goto_address = (uint64_t) APMain;
	}

	KernelIdle(0);

	while(1)
		Yield();
}
