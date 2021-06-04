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

void APMain()
{
	GDTInstall();
	IDTInstall();

	MSwitch(MKernel());

	struct SMInfo *sm_info = SysMemInfo();

	Info("Total memory   : %l KiBs\n", sm_info->pm_total  / 1024);
	Info("Usable memory  : %l KiBs\n", sm_info->pm_usable / 1024);
	Info("Used memory    : %l KiBs\n", sm_info->pm_used   / 1024);
	Info("Virtual memory : %l KiBs\n", sm_info->vm_total  * 4096 / 1024);

	Halt();
}

void KernelMain()
{
	GDTLoad();
	IDTLoad();

	struct SMInfo *sm_info = SysMemInfo();

	KernelEarlyInit();

	PMInit();
	VMInit();


	Info("Total memory   : %l KiBs\n", sm_info->pm_total  / 1024);
	Info("Usable memory  : %l KiBs\n", sm_info->pm_usable / 1024);
	Info("Used memory    : %l KiBs\n", sm_info->pm_used   / 1024);
	Info("Virtual memory : %l KiBs\n", sm_info->vm_total  * 4096 / 1024);

	struct stivale2_struct_tag_smp *smp;
	smp = Stivale2GetTag(STIVALE2_STRUCT_TAG_SMP_ID);
}
