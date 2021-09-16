#include <Common.h>
#include <Core.h>
#include <Stivale2.h>
#include <Memory.h>
#include <Task.h>
#include <stdlib.h>
#include <string.h>

static struct SMInfo *sm_info = NULL;

static struct AddressSpace dummy_space = (struct AddressSpace) { 0 };


struct AddressSpace *MAddrSpaceAlloc()
{
	struct AddressSpace *space = calloc(1, sizeof(struct AddressSpace));
	space->ptab = (uint64_t*) PMAlloc();
	memset(PhysOffset(space->ptab), 0, 2048);

	uint64_t *ph  = PhysOffset(space->ptab);
	uint64_t *kph = PhysOffset(sm_info->ptab);
	
	for(int i = 256; i < 512; i++)
		ph[i] = kph[i];

	return space;
}

void VMInit()
{
	sm_info = SysMemInfo();

	// Allocate the paging table

	sm_info->ptab = (uint64_t*) PMAlloc();
	memset(PhysOffset(sm_info->ptab), 0, 4096);

	dummy_space.ptab = sm_info->ptab;
	ProcCurrent()->space = &dummy_space;


	uint64_t *ph = PhysOffset(sm_info->ptab);
	for(int i = 256; i < 512; i++)
		ph[i] = PMAlloc() | PAGE_PRESENT | PAGE_RDWR;


	for(uintptr_t phys = 0; phys < sm_info->pm_total; phys += 2097152)
		MMapHuge(PhysOffset(phys), (void*) phys, PAGE_PRESENT | PAGE_RDWR);

	for(uintptr_t phys = 0; phys < 0x80000000; phys += 2097152)
		MMapHuge((void*) (KERNEL_OFFSET + phys), (void*) phys, PAGE_PRESENT | PAGE_RDWR);

	MSwitchKernel();
}

void MSwitch(struct AddressSpace *space)
{
	uintptr_t ptab = (uintptr_t) space->ptab;

	ProcCurrent()->space = space;

	asm volatile("mov %0, %%cr3" :: "a"(ptab) :"memory");
}

void MSwitchKernel()
{
	uintptr_t ptab = (uintptr_t) sm_info->ptab;

	ProcCurrent()->space = &dummy_space;

	asm volatile("mov %0, %%cr3" :: "a"(ptab) :"memory");
}

struct AddressSpace *MActive()
{
	return ProcCurrent()->space;
}

static uint64_t *VMNextLevel(uint64_t *cur, uint64_t ent, uint16_t flags)
{
	cur = PhysOffset(cur);

	if((cur[ent] & PAGE_PRESENT) == 0) {
		uint64_t addr = PMAllocPageLockless()->addr;

		memset(PhysOffset(addr), 0, 4096);

		cur[ent] = addr | flags | PAGE_PRESENT | PAGE_RDWR;

		return PhysOffset(addr);
	}

	return PhysOffset(cur[ent] & ~0xFFF);
}

static void Invalidate(void *virt)
{
	uint64_t cr3 = 0;
	asm volatile("mov %%cr3, %0" : "=a"(cr3));

	if(cr3 == (uint64_t) MActive()->ptab)
		asm volatile("invlpg %0" :: "m"(virt) : "memory");
}

void MMap(void *virt, void *phys, uint16_t flags)
{
	int high = (uintptr_t) virt >= PHYS_OFFSET;

	if(high)
		Lock(&sm_info->lock);
	else
		Lock(&MActive()->lock);


	uint64_t *ptab = PhysOffset(high ? sm_info->ptab : MActive()->ptab);

	uintptr_t addr = (uintptr_t) virt;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pml3 = VMNextLevel(ptab, lvl4, flags);
	uint64_t *pml2 = VMNextLevel(pml3, lvl3, flags);
	uint64_t *pml1 = VMNextLevel(pml2, lvl2, flags);

	pml1[lvl1] = (uintptr_t) phys | flags;

	Invalidate(virt);

	if(high)
		sm_info->vm_total++;
	else
		MActive()->vm_total++;


	if(high)
		Unlock(&sm_info->lock);
	else
		Unlock(&MActive()->lock);
}

void MMapHuge(void *virt, void *phys, uint16_t flags)
{
	int high = (uintptr_t) virt >= PHYS_OFFSET;

	if(high)
		Lock(&sm_info->lock);
	else
		Lock(&MActive()->lock);


	uint64_t *ptab = PhysOffset(high ? sm_info->ptab : MActive()->ptab);

	uintptr_t addr = (uintptr_t) virt;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;

	uint64_t *pml3 = VMNextLevel(ptab, lvl4, flags);
	uint64_t *pml2 = VMNextLevel(pml3, lvl3, flags);
	uint64_t *pml1 = VMNextLevel(pml2, lvl2, flags);

	pml2[lvl2] = (uintptr_t) phys | flags | PAGE_HUGE;

	Invalidate(virt);

	if(high)
		sm_info->vm_total++;
	else
		MActive()->vm_total++;


	if(high)
		Unlock(&sm_info->lock);
	else
		Unlock(&MActive()->lock);
}

void MUnmap(void *virt)
{
	int high = (uintptr_t) virt >= PHYS_OFFSET;

	if(high)
		Lock(&sm_info->lock);
	else
		Lock(&MActive()->lock);


	uint64_t *ptab = PhysOffset(high ? sm_info->ptab : MActive()->ptab);

	uintptr_t addr = (uintptr_t) virt;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pml3 = VMNextLevel(ptab, lvl4, 0);
	if(pml3 == NULL) goto ret;
	uint64_t *pml2 = VMNextLevel(pml3, lvl3, 0);
	if(pml2 == NULL) goto ret;
	uint64_t *pml1 = VMNextLevel(pml2, lvl2, 0);
	if(pml1 == NULL) goto ret;

	pml1[lvl1] = 0;

	Invalidate(virt);

	if(high && sm_info->vm_total > 0)
		sm_info->vm_total--;
	else if(!high && MActive()->vm_total > 0)
		MActive()->vm_total--;

ret:
	if(high)
		Unlock(&sm_info->lock);
	else
		Unlock(&MActive()->lock);
}

void MFlag(void *virt, uint16_t flags)
{
	int high = (uintptr_t) virt >= PHYS_OFFSET;

	if(high)
		Lock(&sm_info->lock);
	else
		Lock(&MActive()->lock);


	uint64_t *ptab = PhysOffset(high ? sm_info->ptab : MActive()->ptab);

	uintptr_t addr = (uintptr_t) virt;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pml3 = VMNextLevel(ptab, lvl4, flags);
	uint64_t *pml2 = VMNextLevel(pml3, lvl3, flags);
	uint64_t *pml1 = VMNextLevel(pml2, lvl2, flags);

	pml1[lvl1] = (pml1[lvl1] & ~0xFFF) | flags;

	Invalidate(virt);

	if(high)
		Unlock(&sm_info->lock);
	else
		Unlock(&MActive()->lock);
}

int MValid(void *virt)
{
	int high = (uintptr_t) virt >= PHYS_OFFSET;

	if(high)
		Lock(&sm_info->lock);
	else
		Lock(&MActive()->lock);


	uint64_t *ptab = PhysOffset(high ? sm_info->ptab : MActive()->ptab);

	uintptr_t addr = (uintptr_t) virt;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pml3 = VMNextLevel(ptab, lvl4, 0);
	uint64_t *pml2 = VMNextLevel(pml3, lvl3, 0);
	uint64_t *pml1 = VMNextLevel(pml2, lvl2, 0);

	int ret = 0;

	if(pml2[lvl2] & PAGE_HUGE)
		ret = (pml2[lvl2] & PAGE_PRESENT) != 0;
	else
		ret = (pml1[lvl1] & PAGE_PRESENT) != 0;

	if(high)
		Unlock(&sm_info->lock);
	else
		Unlock(&MActive()->lock);

	return ret;
}

void *MPhys(void *virt)
{
	int high = (uintptr_t) virt >= PHYS_OFFSET;

	if(high)
		Lock(&sm_info->lock);
	else
		Lock(&MActive()->lock);


	uint64_t *ptab = PhysOffset(high ? sm_info->ptab : MActive()->ptab);

	uintptr_t addr = (uintptr_t) virt;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pml3 = VMNextLevel(ptab, lvl4, 0);
	uint64_t *pml2 = VMNextLevel(pml3, lvl3, 0);
	uint64_t *pml1 = VMNextLevel(pml2, lvl2, 0);

	void *ret = NULL;

	if(pml2[lvl2] & PAGE_HUGE)
		ret = (void*) (pml2[lvl2] & ~0xFFF);
	else
		ret = (void*) (pml1[lvl1] & ~0xFFF);

	if(high)
		Unlock(&sm_info->lock);
	else
		Unlock(&MActive()->lock);

	return ret;
}
