#pragma once

#include <stddef.h>
#include <stdint.h>

struct SMInfo
{
	// There is only one SMInfo that is used across the entire system,

	uint64_t    *ptab; // Paging table
	size_t   pm_total; // Total available physical memory
	size_t  pm_usable; // Total usable physical memory
	size_t    pm_used; // Total used physical memory
	size_t   vm_total; // Total virtual memory used by the higher half / 4096
	int64_t      lock; // `lock` is acquired when a page is changed in the higher half
};

struct AddressSpace
{

	// `pg_count` and `vm_total` only count the lower half's pages.
	// `lock` is acquired only when changing the lower half's pages.
	//	All operations on the higher half are held in the SMInfo struct.

	uint64_t  *ptab; // Paging table
	int64_t    lock; // Every time a page is changed, this lock is acquired
	size_t pg_count; // Total physical pages used by this address space  / 4096
	size_t vm_total; // Total virtual memory used for this address space / 4096
};

struct Page
{
	uint64_t  addr : 56; // Physical address of the page / 4096
	uint64_t flags : 8;  // Flags of this page
	size_t    refc;      // Reference count, how many times this page has been mapped
	                     // `refc` only counts pages allocated with the PMAlloc() fa-
	                     // mily of functions
};

_Static_assert(sizeof(struct SMInfo) == 48, "struct SMInfo broken");

_Static_assert(sizeof(struct AddressSpace) == 32, "struct AddressSpace broken");

_Static_assert(sizeof(struct Page) == 16, "struct Page broken");

void PMInit();

struct SMInfo *SysMemInfo();

uint64_t PMAlloc();

struct Page *PMAllocPage();

struct Page *PMPageOf(uint64_t addr);

void PMFree(uint64_t addr);

void PMFreePage(struct Page *page);
