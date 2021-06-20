#pragma once

#include <stddef.h>
#include <stdint.h>

#define DEV_CATEGORY_GRAPHICS 0x1 // Graphics related devices
#define DEV_CATEGORY_STORAGE  0x2 // Storage devices
#define DEV_CATEGORY_AUDIO    0x3 // Audio devices
#define DEV_CATEGORY_SERIAL   0x4 // Serial devices
#define DEV_CATEGORY_TERM     0x5 // Terminals

#define DEV_TYPE_GRTERM 0x0 // VGA terminal

#define DEV_TYPE_SERIAL 0x0 // Serial device

struct Device
{
	uint64_t     type;
	char     name[64]; // NULL-terminated name
	uint8_t   enabled; // 1 if enabled, 0 if not
	int64_t      lock; // Device lock
};

struct DevTerminal
{
	struct Device dev;

	uint32_t   *fb;
	size_t  column;
	size_t    line;
	void    *state;

	void  (*write) (struct DevTerminal*, const char*);
};

struct DevSerial
{
	struct Device dev;

	uint16_t port;

	void    (*put) (struct DevSerial*, uint8_t);
	void  (*write) (struct DevSerial*, const void*, size_t);
	int   (*ready) (struct DevSerial*);
};

void DeviceRegister(uint64_t category, struct Device *dev);

void DeviceUnregister(uint64_t category, struct Device *dev);


void *DevicePrimary(uint64_t category);

void DevicePrimarySet(uint64_t category, struct Device *dev);


void *DeviceGet(uint64_t category, uint64_t type, const char *name);

size_t DeviceCollect(uint64_t category, struct Device **list);



