#include <Common.h>
#include <DescTabs.h>

const char *exceptions[] =
{
	"Divide-by-zero error",
	"Debug interrupt triggered",
	"Non-maskable interrupt",
	"Hit breakpoint",
	"Overflowed",
	"Bound range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double faulted",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack-segment fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"x87 floating-point exception",
	"Alignment checked",
	"Machine checked",
	"SIMD floating-point exception",
	"Virtualization exception",
	"Security exception"
};

static void ExceptionHandler(struct Registers *regs)
{
	uint64_t vector = regs->vector > 20 ? 21 : regs->vector;

	uint64_t cr2 = 0;

	asm volatile("mov %%cr2, %0" : "=r"(cr2));

	if(vector == 0xE)
		Panic(regs, "Memory access violation while accessing %xl", cr2);
	Panic(regs, "%s", exceptions[vector]);
}

void ExceptionsLoad()
{
	for(int i = 0; i < 32; i++)
		IDTEntrySet(i, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, ExceptionHandler);
}
