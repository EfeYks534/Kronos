#include <Common.h>
#include <Device.h>
#include <Memory.h>
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

	KernelEarlyInit();

	struct SMInfo *sm_info = SysMemInfo();

	Info("Total memory  : %l MiBs\n", sm_info->pm_total  / 1024 / 1024);
	Info("Usable memory : %l MiBs\n", sm_info->pm_usable / 1024 / 1024);
	Info("Used memory   : %l MiBs\n", sm_info->pm_used   / 1024 / 1024);
}
