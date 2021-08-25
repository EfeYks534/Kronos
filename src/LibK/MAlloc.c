#include <Common.h>
#include <Memory.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_BASE  0xFFFF821000000000
#define HEAP_BLK   0xFFFF820000000000

struct Block
{
	struct
	{
		uint64_t  base : 58; // Block starting address is calculated by HEAP_BASE + base
		uint64_t group :  5; // Fragment size is 2^(group + 6)
		uint64_t state :  1; // Set to 1 if this block is full
	};

	uint64_t mmap;
};

_Static_assert(sizeof(struct Block) == 16, "struct Block broken");

static struct Block *blocks = (struct Block*) HEAP_BLK;

static size_t blk_count = 0;

static uint64_t blk_addr = 0;

static int64_t heap_lock = 0;

static size_t total_alloc = 0;


static struct Block *BlkAlloc(uint8_t group)
{
	struct Block *blk = &blocks[blk_count++];

	uintptr_t addr = (uintptr_t) blk & ~0xFFF;

	if(PMPageOf((uintptr_t) MPhys((void*) addr)) == NULL) {
		uintptr_t phys = PMAlloc();

		MMap((void*) addr, (void*) phys, PAGE_PRESENT | PAGE_RDWR);
	}

	blk->base  = blk_addr;
	blk->group = group;
	blk->state = 0;
	blk->mmap  = -1ULL;

	size_t n = 64ULL * (1ULL << (blk->group + 6));

	n += (ceil(blk_addr, n) * n) - blk_addr;

	blk_addr += n;

	return blk;
}

static struct Block *BlkFind(uint8_t group)
{
	for(size_t i = 0; i < blk_count; i++) {
		if(blocks[i].group == group && blocks[i].state == 0)
			return &blocks[i];
	}

	return BlkAlloc(group);
}

static void HeapMap(void *addr, size_t npages)
{
	for(size_t i = 0; i < npages; i++) {

		if(PMPageOf((uintptr_t) MPhys(addr)) == NULL) {
			uintptr_t phys = PMAlloc();

			MMap(addr, (void*) phys, PAGE_PRESENT | PAGE_RDWR);
		}

		addr = (void*) ((uintptr_t) addr + 4096);
	}
}

static void HeapUnmap(void *addr, size_t npages)
{
	for(size_t i = 0; i < npages; i++) {

		if(PMPageOf((uintptr_t) MPhys(addr)) != NULL) {
			PMFree((uintptr_t) MPhys(addr));

			MUnmap(addr);
		}

		addr = (void*) ((uintptr_t) addr + 4096);
	}
}

void *malloc(size_t n)
{
	Lock(&heap_lock);

	uint8_t group = log2(n);

	group = group < 6 ? 0 : group - 6;

	if((1ULL << (group + 6)) < n)
		group++;

	if(group > 31) {
		Unlock(&heap_lock);
		Error("malloc: Allocation size too big");
		return NULL;
	}

	struct Block *blk = BlkFind(group);

	uint64_t frag = __builtin_ffsll(blk->mmap) - 1;

	blk->mmap &= ~(1ULL << frag);

	if(blk->mmap == 0)
		blk->state = 1;
	else
		blk->state = 0;

	void *addr = (void*) (HEAP_BASE + blk->base + (frag * (1ULL << (group + 6))));

	size_t npages = ceil(1ULL << (group + 6), 4096);

	HeapMap(addr, npages);

	total_alloc += 1ULL << (group + 6);

	Unlock(&heap_lock);

	return addr;
}

void *calloc(size_t nmemb, size_t size)
{
	void *addr = malloc(nmemb * size);

	if(addr == NULL)
		return NULL;

	memset(addr, 0, nmemb * size);

	return addr;
}

void free(void *ptr)
{
	Lock(&heap_lock);

	if((uintptr_t) ptr < HEAP_BASE)
		Panic(NULL, "Can't free corrupted address %xl", ptr);

	uintptr_t addr = (uintptr_t) ptr - HEAP_BASE;

	for(size_t i = 0; i < blk_count; i++) {
		struct Block *blk = &blocks[i];

		size_t size = (1ULL << (blk->group + 6));

		if(addr >= blk->base && addr < blk->base + size * 64) {
			if((addr & (size - 1)) != 0)
				Panic(NULL, "Can't free unaligned address %xl", ptr);

			size_t bit = (addr - blk->base) / size;

			if((blk->mmap >> bit) & 1)
				Panic(NULL, "Can't free unallocated address %xl", ptr);

			blk->mmap |= (1ULL << bit);

			blk->state = 0;

			if(size >= 4096)
				HeapUnmap(ptr, size / 4096);


			Unlock(&heap_lock);
			return;
		}
	}

	Panic(NULL, "Can't free unallocated address %xl", ptr);
	Unlock(&heap_lock);
}

size_t MAllocTotal()
{
	return total_alloc;
}
