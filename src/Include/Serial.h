#pragma once

#include <Common.h>
#include <stddef.h>
#include <stdint.h>

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

void SerialInit(uint16_t port);

void SerialWrite(uint16_t port, void *buf, int len);

static inline int SerialReady(uint16_t port)
{
	return (In8(port + 5) & (1 << 5)) != 0;
}
