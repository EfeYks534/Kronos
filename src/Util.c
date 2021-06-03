#include <Common.h>
#include <Device.h>
#include <Stivale2.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static void PrintBuffer(void *buf, size_t len)
{
	struct DevSerial *sr;
	sr = DeviceGet(DEV_CATEGORY_DATA, DEV_TYPE_DTSERIAL, "Serial(COM1)");

	struct DevTerminal *term;
	term = DeviceGet(DEV_CATEGORY_TERM, DEV_TYPE_GRTERM, "Terminal");

	uint8_t *b = buf;
	b[len] = 0;

	if(sr != NULL)
		sr->write(sr, buf, len);

	if(term != NULL)
		term->write(term, buf);
}


static char log_buffer[4096] = { 0 };

static char panic_buffer[4096] = { 0 };

struct StackFrame
{
	struct StackFrame *rbp;
	uint64_t           rip;
} PACKED;

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
		Log("    \x1B[32m0x%xl\n", r->rip);
	} else {
		asm volatile("movq %%rbp, %0" : "=r"(frame));
	}


	size_t depth = 16;
	while(--depth && frame) {
		if(!frame->rip) break;
		Log("    \x1B[32m0x%xl\n", frame->rip);
		frame = frame->rbp;
	}

	if(r != NULL) {
		Log("\x1B[90m\n");
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

	PrintBuffer(log_buffer, len);
}

void Info(const char *fmt, ...)
{
	Log("\x1B[32;1m[INFO]\x1B[0m ");

	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4096, fmt, ap);

	va_end(ap);

	PrintBuffer(log_buffer, len);
}

void Warn(const char *fmt, ...)
{
	Log("\x1B[33;1m[WARNING]\x1B[0m ");

	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4096, fmt, ap);

	va_end(ap);

	PrintBuffer(log_buffer, len);
}

void Error(const char *fmt, ...)
{
	Log("\x1B[31;1m[ERROR]\x1B[0m ");

	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4096, fmt, ap);

	va_end(ap);

	PrintBuffer(log_buffer, len);
}


struct CPUID CPUID(uint32_t func)
{
	struct CPUID id = (struct CPUID) { 0 };

	asm volatile("cpuid" : "=a"(id.a), "=b"(id.b), "=c"(id.c), "=d"(id.d) : "a"(func), "c"(0));

	return id;
}

uint64_t FlagsGet()
{
	uint64_t flags = 0;

	asm volatile("pushfq\n"
	             "pop %0" : "=a"(flags) :: "memory");

	return flags;
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
