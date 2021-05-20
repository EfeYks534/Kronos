#include <Common.h>
#include <Serial.h>
#include <Stivale2.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static char log_buffer[4096] = { 0 };

static char panic_buffer[4096] = { 0 };

struct StackFrame
{
	struct StackFrame *rbp;
	uint64_t           rip;
} PACKED;

void Panic(struct Registers *r, const char *fmt, ...)
{
	Log("\x1B[31;1m\n\nKernel Panic: \x1B[35;1m");

	va_list ap;

	va_start(ap, fmt);

	vsnprintf(panic_buffer, 4095, fmt, ap);

	va_end(ap);

	Log("%s\n\n\x1B[36;1mBacktrace:\n", panic_buffer);

	struct StackFrame *frame = NULL;

	if(r != NULL)
		frame = (struct StackFrame*) r->rbp;
	else
		asm volatile("movq %%rbp, %0" : "=r"(frame):);


	size_t depth = 16;
	while(--depth && frame) {
		if(!frame->rip) break;
		Log("    \x1B[32m0x%xl\n", frame->rip);
		frame = frame->rbp;
	}

	if(r != NULL) {
		Log("\x1B[37m");
		Log("FLAGS: %xl RIP: %xl CS: %xl SS: %xl\n\n", r->flags, r->rip, r->cs, r->ss);

		Log("RAX: %xl RBX: %xl RCX: %xl RDX: %xl\n", r->rax, r->rbx, r->rcx, r->rdx);
		Log("RDI: %xl RSI: %xl RBP: %xl RSP: %xl\n", r->rdi, r->rsi, r->rbp, r->rsp);
		Log("R8 : %xl R9 : %xl R10: %xl R11: %xl\n", r->r8,  r->r9,  r->r10, r->r11);
		Log("R12: %xl R13: %xl R14: %xl R15: %xl\n", r->r12, r->r13, r->r14, r->r15);
	}

	Log("\x1B[0m");
	Halt();
}

void Log(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4095, fmt, ap);

	va_end(ap);

	SerialWrite(SERIAL_COM1, log_buffer, len);

	struct stivale2_struct_tag_terminal *term;
	term = Stivale2GetTag(STIVALE2_STRUCT_TAG_TERMINAL_ID);
	if(term != NULL) {
		void (*term_write)(const char *str, size_t len) = (void*) term->term_write;

		term_write(log_buffer, len);
	}
}

void Out8(uint16_t port, uint8_t data)
{
	asm volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

void Out16(uint16_t port, uint16_t data)
{
	asm volatile("outw %0, %1" ::"a"(data), "Nd"(port));
}

void Out32(uint16_t port, uint32_t data)
{
	asm volatile("outl %0, %1" ::"a"(data), "Nd"(port));
}

uint8_t In8(uint16_t port)
{
	uint8_t data;
	asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
	return data;
}

uint16_t In16(uint16_t port)
{
	uint16_t data;
	asm volatile("inw %1, %0" : "=a"(data) : "Nd"(port));
	return data;
}

uint32_t In32(uint16_t port)
{
	uint32_t data;
	asm volatile("inl %1, %0" : "=a"(data) : "Nd"(port));
	return data;
}

void IOWait()
{
	Out8(0x80, 0);
}
