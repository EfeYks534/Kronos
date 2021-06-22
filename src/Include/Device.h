#pragma once

#include <stddef.h>
#include <stdint.h>

#define DEV_CATEGORY_GRAPHICS 0x1 // Graphics related devices
#define DEV_CATEGORY_STORAGE  0x2 // Storage devices
#define DEV_CATEGORY_AUDIO    0x3 // Audio devices
#define DEV_CATEGORY_SERIAL   0x4 // Serial devices
#define DEV_CATEGORY_TERM     0x5 // Terminals
#define DEV_CATEGORY_TIMER    0x6 // Timers

#define DEV_TYPE_GRTERM 0x0 // VGA terminal

#define DEV_TYPE_SERIAL 0x0 // Serial device

#define DEV_TYPE_HPET   0x0 // HPET timer

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

	// `write()` writes a  null-terminated string to this  terminal
	// `column` and `line` are the  position  of the cursor on this
	// terminal. `fb` is the buffer the terminal writes to. `state`
	// is an internal state  used by the terminal that is device s-
	// pecific.
};

struct DevSerial
{
	struct Device dev;

	uint16_t port;

	void    (*put) (struct DevSerial*, uint8_t);
	void  (*write) (struct DevSerial*, const void*, size_t);
	int   (*ready) (struct DevSerial*);

	// `put()` writes a single unsigned byte to the serial, `write()`
	// writes %3 bytes to the serial. `ready()` returns `1` if the s-
	// erial is ready for a write operation.
};

struct DevTimer
{
	struct Device dev;

	void *state;

	void              (*reset) (struct DevTimer*);
	size_t             (*time) (struct DevTimer*);
	void   (*handler_register) (struct DevTimer*, void (*)(struct DevTimer*), size_t);
	void (*handler_unregister) (struct DevTimer*, void (*)(struct DevTimer*));

	// `time()` returns the amount of  nanoseconds since this timer
	// has  been last reset. `handler_register()` registers a hand-
	// ler to the timer  which runs every %3 nanoseconds. `handler_
	// unregister()` unregisters an existing handler. `reset()` re-
	// sets the timer to start counting from 0. The timer won't co-
	// unt when it's disabled.
};

void DeviceRegister(uint64_t category, struct Device *dev);

void DeviceUnregister(uint64_t category, struct Device *dev);


void *DevicePrimary(uint64_t category);

void DevicePrimarySet(uint64_t category, struct Device *dev);


void *DeviceGet(uint64_t category, uint64_t type, const char *name);

size_t DeviceCollect(uint64_t category, struct Device **list);



