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

void KernelMain()
{
	GDTLoad();
	IDTLoad();

	PMInit();
	VMInit();

	KernelEarlyInit();

	struct SMInfo *sm_info = SysMemInfo();

	Info("Total memory   : %l MiBs\n", sm_info->pm_total  / 1024 / 1024);
	Info("Usable memory  : %l MiBs\n", sm_info->pm_usable / 1024 / 1024);
	Info("Used memory    : %l MiBs\n", sm_info->pm_used   / 1024 / 1024);

	Info("Virtual memory : %l KiBs\n", sm_info->vm_total * 4096 / 1024);

}
