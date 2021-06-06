#include <Common.h>
#include <Stivale2.h>
#include <Memory.h>
#include <Task.h>
#include <stdlib.h>
#include <string.h>

static struct SMInfo sm_info = { 0 };


static struct Page **pgcache_tab = NULL; // struct Page cache

static struct Queue pgcache = { .items = NULL, .nmemb = 0, .size = 8, 0,0,0};


static uint64_t bicache_tab[2048] = { 0 }; // PM allocation bitmap index cache

static struct Stack bicache = { .items = bicache_tab, .nmemb = 2048, .size = 8, 0,0};


static uint64_t *pmmap = NULL; // PM allocation bitmap

static uint64_t pmtab[512] = { 0 }; // struct Page maps


static void BICacheFill()
{
	size_t bitmap_size = sm_info.pm_total / 4096 / 64;

	for(size_t i = 0; i < bitmap_size; i++) {
		if(pmmap[i] == 0) continue;

		int r = StackPush(&bicache, &i);
		if(!r) return;
	}
}

static uint64_t BitmapAlloc()
{
	size_t bitmap_size = sm_info.pm_total / 4096 / 64;

	if(StackEmpty(&bicache)) {
		BICacheFill();

		if(StackEmpty(&bicache))
			Panic(NULL, "Ran out of physical memory");
	}

	size_t i = 0;

	while(pmmap[i] == 0)
		StackPop(&bicache, &i); // Let's skip invalid cache entries

	if(pmmap[i] == 0)
		Panic(NULL, "Ran out of physical memory");

	size_t bit = __builtin_ffsll(pmmap[i]) - 1;

	pmmap[i] &= ~(1ULL << bit);

	sm_info.pm_usable -= 4096;
	sm_info.pm_used   += 4096;

	if(pmmap[i] != 0) // Put it back on the cache if it's still available
		StackPush(&bicache, &i);

	return (i * 64 + bit) * 4096;
}

static void BitmapFree(uint64_t phys)
{
	size_t i   = (phys / 4096) / 64;
	size_t bit = (phys / 4096) % 64;

	if((pmmap[i] >> bit) & 1)
		Panic(NULL, "Can't free unallocated page");

	pmmap[i] |= 1ULL << bit;

	sm_info.pm_usable += 4096;
	sm_info.pm_used   -= 4096;

	StackPush(&bicache, &i); // Let's put the bitmap index to the cache
}

static struct Page *PageAlloc()
{
	if(!QueueEmpty(&pgcache)) {
		// Let's get a page from the cache

		struct Page *pg = NULL;

		QueueConsume(&pgcache, &pg);

		if(pg != NULL)
			return pg;

		Panic(NULL, "Invalid page cache entry");
	}

	// Cache miss, let's fill the cache with new pages

	struct Page *pages = PhysOffset(BitmapAlloc());

	for(size_t i = 1; i < 256; i++) { // Start at 1 as we will return the 0th index
		struct Page *pg = &pages[i];

		QueueSubmit(&pgcache, &pg);
	}

	return &pages[0];
}


void PMInit()
{
	struct stivale2_struct_tag_memmap *memmap;
	memmap = Stivale2GetTag(STIVALE2_STRUCT_TAG_MEMMAP_ID);

	if(memmap == NULL)
		Panic(NULL, "Couldn't load Stivale 2 memory map");

	// Initialize the SMInfo fields

	for(size_t i = 0; i < memmap->entries; i++) {
		struct stivale2_mmap_entry *ent = &memmap->memmap[i];
		sm_info.pm_total += ent->length;

		if(ent->type >= 0x1000 && ent->type <= 0x1002)
			sm_info.pm_used += ent->length;

		if(ent->type == STIVALE2_MMAP_USABLE)
			sm_info.pm_usable += ent->length;
	}

	// Allocate the allocation bitmap and the struct Page cache

	size_t bitmap_size = sm_info.pm_total / 4096 / 8;

	sm_info.pm_used   += bitmap_size;
	sm_info.pm_usable -= bitmap_size;

	size_t spage_size = sm_info.pm_total / 4096 * 8;

	sm_info.pm_used   += spage_size;
	sm_info.pm_usable -= spage_size;

	pgcache.nmemb = sm_info.pm_total / 4096;

	for(size_t i = 0; i < memmap->entries; i++) {
		struct stivale2_mmap_entry *ent = &memmap->memmap[i];

		if(ent->type == STIVALE2_MMAP_USABLE && ent->length >= bitmap_size) {

			pmmap = PhysOffset(ent->base);

			ent->length -= bitmap_size;
			ent->base   += bitmap_size;
			break;
		}
	}

	for(size_t i = 0; i < memmap->entries; i++) {
		struct stivale2_mmap_entry *ent = &memmap->memmap[i];

		if(ent->type == STIVALE2_MMAP_USABLE && ent->length >= spage_size) {
			pgcache_tab = PhysOffset(ent->base);

			ent->length -= spage_size;
			ent->base   += spage_size;
			break;
		}
	}

	if(pmmap == NULL || pgcache_tab == NULL)
		Panic(NULL, "Couldn't allocate enough memory for PMM");

	pgcache.items = pgcache_tab;

	memset(pmmap, 0, bitmap_size);

	

	// Let's mark the usable entries as free


	for(size_t i = 0; i < memmap->entries; i++) {
		struct stivale2_mmap_entry *ent = &memmap->memmap[i];

		if(ent->type != STIVALE2_MMAP_USABLE) continue;

		size_t base = ent->base / 4096 / 64;

		for(size_t j = 0; j < ent->length / 4096; j++)
			pmmap[base + j / 64] |= 1ULL << (j % 64);
	}
}

struct SMInfo *SysMemInfo()
{
	return &sm_info;
}

static uint64_t *PMNextLevel(uint64_t *cur, uint64_t ent)
{
	cur = (uint64_t*) ((uintptr_t) cur & ~1);

	if((cur[ent] & 1) == 0) {
		uint64_t *addr = PhysOffset(BitmapAlloc());

		cur[ent] = (uintptr_t) addr | 1;

		memset(addr, 0, 4096);

		return addr;
	}

	return (uint64_t*) (cur[ent] & ~1);
}

struct Page *PMAllocPageLockless()
{
	struct Page *pg = PageAlloc();

	pg->addr = BitmapAlloc();
	pg->refc = 1;

	uint64_t addr = pg->addr;

	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pm3 = PMNextLevel(pmtab, lvl4);
	uint64_t *pm2 = PMNextLevel(pm3, lvl3);
	uint64_t *pm1 = PMNextLevel(pm2, lvl2);

	pm1[lvl1] = (uintptr_t) pg | 1;
	return pg;
}

struct Page *PMAllocPage()
{
	Lock(&sm_info.lock);

	struct Page *ret = PMAllocPageLockless();

	Unlock(&sm_info.lock);

	return ret;
}

uint64_t PMAlloc()
{
	Lock(&sm_info.lock);

	uint64_t addr = PMAllocPageLockless()->addr;

	Unlock(&sm_info.lock);

	return addr;
}

struct Page *PMPageOfLockless(uint64_t addr)
{
	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pm3 = PMNextLevel(pmtab, lvl4);
	uint64_t *pm2 = PMNextLevel(pm3, lvl3);
	uint64_t *pm1 = PMNextLevel(pm2, lvl2);

	if(pm1[lvl1] & 1)
		return (struct Page*) (pm1[lvl1] & ~1);
	return NULL;
}

struct Page *PMPageOf(uint64_t addr)
{
	Lock(&sm_info.lock);

	struct Page *ret = PMPageOfLockless(addr);

	Unlock(&sm_info.lock);

	return ret;
}

void PMFreePageLockless(struct Page *page)
{
	if(--page->refc == 0) {
		BitmapFree(page->addr);

		page->addr = 0;
		page->refc = 0;

		QueueSubmit(&pgcache, &page); // Let's put it back on the cache
	}
}

void PMFreePage(struct Page *page)
{
	Lock(&sm_info.lock);

	PMFreePageLockless(page);

	Unlock(&sm_info.lock);
}


void PMFree(uint64_t addr)
{
	Lock(&sm_info.lock);

	struct Page *pg = PMPageOfLockless(addr);

	if(pg == NULL)
		Panic(NULL, "Can't free unallocated page");

	PMFreePageLockless(pg);

	Unlock(&sm_info.lock);
}
