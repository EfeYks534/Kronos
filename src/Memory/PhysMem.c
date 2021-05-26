#include <Common.h>
#include <Memory.h>
#include <Stivale2.h>
#include <stdlib.h>
#include <string.h>

static struct SMInfo sm_info = { 0 };


static uint64_t *pmmap = NULL; // Physical memory bitmap


static struct Page *pgcache_tab[2048] = { 0 };

static struct Queue pgcache = { .items = pgcache_tab, .nmemb = 2048, .size = 8, 0,0,0};


static uint64_t pmtab[512] = { 0 };


static void BitmapFree(uint64_t phys)
{
	size_t i   = (phys / 4096) / 64;
	size_t bit = (phys / 4096) % 64;

	if((pmmap[i] >> bit) & 1)
		Panic(NULL, "Can't free unallocated page");

	pmmap[i] |= 1ULL << bit;

	sm_info.pm_usable += 4096;
	sm_info.pm_used   -= 4096;
}

static uint64_t BitmapAlloc()
{
	size_t len = sm_info.pm_total / 4096 / 64;

	for(size_t i = 0; i < len; i++)  {
		if(pmmap[i] == 0) continue;

		size_t bit = __builtin_ffsll(pmmap[i]) - 1;

		pmmap[i] &= ~(1ULL << bit);

		sm_info.pm_usable -= 4096;
		sm_info.pm_used   += 4096;

		return (i * 64 + bit) * 4096;
	}

	Panic(NULL, "Can't allocate physical page");
	return 0;
}


static struct Page *PageAlloc()
{
	if(!QueueEmpty(&pgcache)) {

		// Get a page from the cache

		struct Page *page = NULL;

		int res = QueueConsume(&pgcache, &page);

		if(page != NULL && res)
			return page;

		Panic(NULL, "Page cache invalid");
	}

	// Cache miss, let's create pages ourselves

	struct Page *pages = PhysOffset(BitmapAlloc());

	for(size_t i = 1; i < 256; i++) {
		struct Page *item = &pages[i];
		int r = QueueSubmit(&pgcache, &item);

		if(!r)
			Panic(NULL, "Page cache invalid");
	}

	return &pages[0];
}

struct SMInfo *SysMemInfo()
{
	return &sm_info;
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

	// Allocate bitmap

	size_t bitmap_size = sm_info.pm_total / 4096 / 8;

	sm_info.pm_used   += bitmap_size;
	sm_info.pm_usable -= bitmap_size;

	for(size_t i = 0; i < memmap->entries; i++) {
		struct stivale2_mmap_entry *ent = &memmap->memmap[i];

		if(ent->type == STIVALE2_MMAP_USABLE && ent->length >= bitmap_size) {

			pmmap = PhysOffset(ent->base);

			ent->length -= bitmap_size;
			ent->base   += bitmap_size;
			break;
		}
	}

	if(pmmap == NULL)
		Panic(NULL, "Couldn't allocate physical memory bitmap");

	memset(pmmap, 0, bitmap_size);

	// Mark usable entries as free

	for(size_t i = 0; i < memmap->entries; i++) {
		struct stivale2_mmap_entry *ent = &memmap->memmap[i];

		if(ent->type != STIVALE2_MMAP_USABLE) continue;

		size_t base = ent->base / 4096 / 64;

		for(size_t j = 0; j < ent->length / 4096; j++)
			pmmap[base + j / 64] |= 1ULL << (j % 64);
	}
}

uint64_t PMAlloc()
{
	return PMAllocPage()->addr;
}

static uint64_t *PMNextLevel(uint64_t *cur, uint64_t ent)
{
	cur = (uint64_t*) ((uintptr_t) cur & ~1);

	if((cur[ent] & 1) == 0) {
		uint64_t addr = BitmapAlloc();

		cur[ent] = addr | 1;

		memset((void*) addr, 0, 4096);

		return (uint64_t*) addr;
	}

	return (uint64_t*) (cur[ent] & ~1);
}

struct Page *PMAllocPage()
{
	struct Page *page = PageAlloc();

	page->addr = BitmapAlloc();
	page->refc = 1;

	uint64_t addr = page->addr;


	size_t lvl4 = (addr >> 39) & 0x1FF;
	size_t lvl3 = (addr >> 30) & 0x1FF;
	size_t lvl2 = (addr >> 21) & 0x1FF;
	size_t lvl1 = (addr >> 12) & 0x1FF;

	uint64_t *pm3 = PMNextLevel(pmtab, lvl4);
	uint64_t *pm2 = PMNextLevel(pm3, lvl3);
	uint64_t *pm1 = PMNextLevel(pm2, lvl2);

	pm1[lvl1] = (uintptr_t) page | 1;

	return page;
}

struct Page *PMPageOf(uint64_t addr)
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

void PMFree(uint64_t addr)
{
	struct Page *pg = PMPageOf(addr);

	if(pg == NULL)
		Panic(NULL, "Can't free unallocated page");

	PMFreePage(pg);
}

void PMFreePage(struct Page *page)
{
	if(--page->refc == 0) {
		BitmapFree(page->addr);

		page->addr = 0;
		page->refc = 0;

		if(!QueueFull(&pgcache)) // Let's put it back on the cache
			QueueSubmit(&pgcache, &page);
	}
}
