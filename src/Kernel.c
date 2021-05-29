#include <Common.h>
#include <Memory.h>
#include <DescTabs.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void KernelMain()
{
	Info("Loading GDT: ");
	GDTLoad();
	Log("done\n");

	Info("Loading IDT: ");
	IDTLoad();
	Log("done\n");

	Info("Initializing PMM\n");
	PMInit();
	Info("Initialized PMM\n");

	struct SMInfo *sm_info = SysMemInfo();

	Info("Total memory : %l MiBs\n", sm_info->pm_total  / 1024 / 1024);
	Info("Total usable : %l MiBs\n", sm_info->pm_usable / 1024 / 1024);
	Info("Total used   : %l MiBs\n", sm_info->pm_used   / 1024 / 1024);

	for(int i = 0; i < 524288; i++)
		PMAlloc();

	Info("Total memory : %l MiBs\n", sm_info->pm_total  / 1024 / 1024);
	Info("Total usable : %l MiBs\n", sm_info->pm_usable / 1024 / 1024);
	Info("Total used   : %l MiBs\n", sm_info->pm_used   / 1024 / 1024);
}