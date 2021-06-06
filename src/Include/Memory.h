#pragma once

#include <stddef.h>
#include <stdint.h>

#define PAGE_PRESENT (1ULL)
#define PAGE_RDWR    (1ULL << 1)
#define PAGE_USER    (1ULL << 2)
#define PAGE_WRITET  (1ULL << 3)
#define PAGE_NCACHE  (1ULL << 4)
#define PAGE_PCD     (1ULL << 5)
#define PAGE_GLOBAL  (1ULL << 8)

struct SMInfo
{
	// There is only one SMInfo that is used across the entire system,

	uint64_t    *ptab; // Paging table, physical address
	size_t   pm_total; // Total available physical memory
	size_t  pm_usable; // Total usable physical memory
	size_t    pm_used; // Total used physical memory
	size_t   vm_total; // Total virtual memory used by the higher half / 4096
	int64_t      lock; // `lock` is acquired when any field in here changes
};

struct AddressSpace
{

	// `pg_count` and `vm_total` only count the lower half's pages.
	// `lock` is acquired only when changing the lower half's pages.
	//	All operations on the higher half are held in the SMInfo struct.

	uint64_t  *ptab; // Paging table, physical address
	int64_t    lock; // Every time a page is changed, this lock is acquired
	size_t pg_count; // Total physical pages used by this address space  / 4096
	size_t vm_total; // Total virtual memory used for this address space / 4096
};

struct Page
{
	uint64_t  addr : 56; // Physical address of the page
	uint64_t flags : 8;  // Flags of this page
	size_t    refc;      // Reference count, how many times this page has been mapped
	                     // `refc` only counts pages allocated with the PMAlloc() fa-
	                     // mily of functions
};

_Static_assert(sizeof(struct SMInfo) == 48, "struct SMInfo broken");

_Static_assert(sizeof(struct AddressSpace) == 32, "struct AddressSpace broken");

_Static_assert(sizeof(struct Page) == 16, "struct Page broken");


void VMInit();

void MSwitch(struct AddressSpace *space);

struct AddressSpace *MKernel();

struct AddressSpace *MActive();

void MMap(void *virt, void *phys, uint16_t flags);

void MUnmap(void *virt);

void MFlag(void *virt, uint16_t flags);

void *MPhys(void *virt);

int MValid(void *virt);


void PMInit();

struct SMInfo *SysMemInfo();

uint64_t PMAlloc();

struct Page *PMAllocPageLockless();

struct Page *PMAllocPage();

struct Page *PMPageOfLockless(uint64_t addr);

struct Page *PMPageOf(uint64_t addr);

void PMFree(uint64_t addr);

void PMFreePageLockless(struct Page *page);

void PMFreePage(struct Page *page);
