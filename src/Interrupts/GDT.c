#include <Common.h>
#include <DescTabs.h>
#include <stdlib.h>
#include <string.h>

static struct GDTEntry gdt_entries[16] = { 0 };

static struct DescPointer gdt_ptr = { 0 };

static size_t gdt_entry_count = 0;


void GDTEntryNew(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
	struct GDTEntry *ent = &gdt_entries[gdt_entry_count++];

	ent->base_low   = (base & 0x0000FFFF);
	ent->base_mid   = (base & 0x00FF0000) >> 16;
	ent->base_upper = (base & 0xFF000000) >> 24;

	ent->access = access;
	ent->flags  = flags;

	ent->limit_low   = (limit & 0x0000FFFF);
	ent->limit_upper = (limit & 0x000F0000) >> 16;

	gdt_ptr.addr = (uintptr_t) gdt_entries;
	gdt_ptr.size = sizeof(struct GDTEntry) * gdt_entry_count - 1;
}

void GDTClean()
{
	gdt_entry_count = 0;
	memset(gdt_entries, 0, sizeof(gdt_entries));
	memset(&gdt_ptr, 0, sizeof(gdt_ptr));
}

void GDTFlush(void *ptr);

void GDTInstall()
{
	GDTFlush(&gdt_ptr);
}

void GDTLoad()
{
	GDTClean();

	GDTEntryNew(0, 0, 0, 0);

	uint8_t acc = GDT_ATTR_CODEDATA | GDT_ATTR_EXEC | GDT_ATTR_PRESENT | GDT_ATTR_RDWR;

	GDTEntryNew(0, 0xFFFFFFFF, acc, GDT_FATTR_GRAN4K | GDT_FATTR_S64B);

	acc &= ~(GDT_ATTR_EXEC);

	GDTEntryNew(0, 0xFFFFFFFF, acc, GDT_FATTR_GRAN4K | GDT_FATTR_S32B);

	GDTInstall();
}
