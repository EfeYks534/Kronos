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

#define DEV_TYPE_NVME   0x0 // NVMe I/O controller

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

	void      (*reset) (struct DevTimer*);
	size_t     (*time) (struct DevTimer*);
	size_t     (*rate) (struct DevTimer*);
	size_t (*one_shot) (struct DevTimer*, void (*)(struct DevTimer*), size_t);
	size_t (*periodic) (struct DevTimer*, void (*)(struct DevTimer*), size_t);
	void     (*cancel) (struct DevTimer*, size_t);

	// `time()` returns the amount of  nanoseconds since this timer
	// has  been last  reset. `reset()` resets the timer back to 0.
	// `rate()` returns the rate at which this timer ticks. `one_sh
	// ot()` registers a timer  that will run once and never again.
	// `periodic()` registers a timer  that will run every n ticks.

	// Both `one_shot()` and `periodic()` take ticks as an argument,
	// not nanoseconds. The tick rate can be found in `rate()` which
	// returns the rate in nanosecond. `one_shot()` and `periodic()`
	// both return a registration id, they can be cancelled by `canc
	// el()`

	// Every timer must have at least 100000 ns precision on `time(),`
};

struct DevStorage
{
	struct Device dev;

	void *state;

	size_t (*write) (struct DevStorage*, void*, size_t, size_t);
	size_t  (*read) (struct DevStorage*, void*, size_t, size_t);
};


void DeviceRegister(uint64_t category, struct Device *dev);

void DeviceUnregister(uint64_t category, struct Device *dev);


void *DevicePrimary(uint64_t category);

void DevicePrimarySet(uint64_t category, struct Device *dev);


void *DeviceGet(uint64_t category, uint64_t type, const char *name);

size_t DeviceCollect(uint64_t category, struct Device **list);



