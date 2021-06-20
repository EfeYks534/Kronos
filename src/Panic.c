#include <Common.h>
#include <Core.h>
#include <Task.h>
#include <Device.h>
#include <Stivale2.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

struct StackFrame
{
	struct StackFrame *rbp;
	uint64_t           rip;
} PACKED;

static char panic_buffer[4096] = { 0 };

void Panic(struct Registers *r, const char *fmt, ...)
{
	asm volatile("cli");

	static int panic = 0;

	if(panic)
		Halt();

	panic = 1;

	Log("\x1B[31;1m\n\nKernel Panic: \x1B[35;1m");

	va_list ap;

	va_start(ap, fmt);

	vsnprintf(panic_buffer, 4095, fmt, ap);

	va_end(ap);

	Log("%s\n\n\x1B[36;1mBacktrace:\n", panic_buffer);

	struct StackFrame *frame = NULL;

	if(r != NULL) {
		frame = (struct StackFrame*) r->rbp;
		if(MValid(frame))
			Log("    \x1B[32m0x%xl\n", r->rip);
	} else {
		asm volatile("movq %%rbp, %0" : "=r"(frame));
	}

	size_t depth = 16;
	while(--depth && frame) {
		if(!MValid(frame)) break;
		if(!frame->rip) break;
		Log("    \x1B[32m0x%xl\n", frame->rip);
		frame = frame->rbp;
	}

	if(r != NULL) {
		Log("\x1B[90m\n");
		Log("FLAGS: %xl RIP: %xl CS: %xl SS: %xl\n", r->flags, r->rip, r->cs, r->ss);
		Log("ERROR: %xl\n\n", r->error);

		Log("RAX: %xl RBX: %xl RCX: %xl RDX: %xl\n", r->rax, r->rbx, r->rcx, r->rdx);
		Log("RDI: %xl RSI: %xl RBP: %xl RSP: %xl\n", r->rdi, r->rsi, r->rbp, r->rsp);
		Log("R8 : %xl R9 : %xl R10: %xl R11: %xl\n", r->r8,  r->r9,  r->r10, r->r11);
		Log("R12: %xl R13: %xl R14: %xl R15: %xl\n", r->r12, r->r13, r->r14, r->r15);
	}

	Log("\x1B[0m");
	Halt();
}
