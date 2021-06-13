#include <Common.h>
#include <Task.h>
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

static int64_t print_lock = 0;

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
	Lock(&print_lock);

	Log("\x1B[32;1m[INFO]\x1B[0m ");

	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4096, fmt, ap);

	va_end(ap);

	PrintBuffer(log_buffer, len);

	Unlock(&print_lock);
}

void Warn(const char *fmt, ...)
{
	Lock(&print_lock);

	Log("\x1B[33;1m[WARNING]\x1B[0m ");

	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4096, fmt, ap);

	va_end(ap);

	PrintBuffer(log_buffer, len);
	Unlock(&print_lock);
}

void Error(const char *fmt, ...)
{
	Lock(&print_lock);

	Log("\x1B[31;1m[ERROR]\x1B[0m ");

	va_list ap;

	va_start(ap, fmt);

	int len = vsnprintf(log_buffer, 4096, fmt, ap);

	va_end(ap);

	PrintBuffer(log_buffer, len);

	Unlock(&print_lock);
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
	             "popq %0" : "=a"(flags) :: "memory");

	return flags;
}

void FlagsSet(uint64_t flags)
{
	asm volatile("pushq %0\n"
	             "popfq" : "=a"(flags) :: "memory");
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
