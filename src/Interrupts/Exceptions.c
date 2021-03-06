#include <DescTabs.h>
#include <Common.h>
#include <Memory.h>
#include <Core.h>

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

static void ExceptionHandler(struct Registers *regs, uint64_t arg)
{
	uint64_t vector = regs->vector > 20 ? 21 : regs->vector;
	uint64_t error  = regs->error;

	uint64_t cr2 = 0;

	asm volatile("mov %%cr2, %0" : "=r"(cr2));

	if(vector == 0xE) {
		struct Page *pg = PMPageOf((uint64_t) MPhys((void*) cr2));

		if(!(error & 1) && pg != NULL) {
			if(pg->flags != 0) {
				
			}
		}

		if(ProcCurrent()->task != NULL)
			if(ProcCurrent()->task->privl == 0)
				Panic(regs, "Memory access violation while accessing %xl in ring 3", cr2);

		Panic(regs, "Memory access violation while accessing %xl", cr2);
	}

	if(ProcCurrent()->task != NULL)
		if(ProcCurrent()->task->privl == 0)
			Panic(regs, "%s in ring 3", exceptions[vector]);
	Panic(regs, "%s", exceptions[vector]);
}

void ExceptionsLoad()
{
	for(int i = 0; i < 32; i++)
		IDTEntrySet(i, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, 0, ExceptionHandler);
}
