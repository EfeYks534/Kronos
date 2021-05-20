#include <Common.h>
#include <Stivale2.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t stack[16384] = { 0 }; // 16K is enough for our stack

static struct stivale2_header_tag_terminal term_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
		.next       = 0
	},
	.flags = 0
};

static struct stivale2_header_tag_smp smp_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_SMP_ID,
		.next       = (uintptr_t) &term_tag,
	},
	.flags = 0
};

static struct stivale2_header_tag_framebuffer fb_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
		.next       = (uintptr_t) &smp_tag,
	},

	.framebuffer_width  = 0,
	.framebuffer_height = 0,
	.framebuffer_bpp    = 32
};

__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
	.entry_point = 0,
	.stack       = (uintptr_t) stack + sizeof(stack) - 1,
	.flags       = 2,
	.tags        = (uintptr_t) &fb_tag
};

void KernelBoot(struct stivale2_struct *stivale)
{
	Stivale2SetStruct(stivale);

	struct stivale2_struct_tag_smp *smp;
	smp = Stivale2GetTag(STIVALE2_STRUCT_TAG_SMP_ID);

	if(smp == NULL)
		Panic(NULL, "Shitos4 doesn't support non-SMP systems");

	

	Halt();
}
