#include <Common.h>
#include <Task.h>
#include <Device.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

static inline int SerialReady(uint16_t port)
{
	return (In8(port + 5) & (1 << 5)) != 0;
}

static void SerialInit(uint16_t port)
{
	Out8(port + 1, 0x00);

	Out8(port + 3, 1 << 7);
	Out8(port,     0x02);
	Out8(port + 1, 0x00);

	Out8(port + 3, 3);

	Out8(port + 2, 199);

	Out8(port + 4, 11);
}

static void SerialWrite(uint16_t port, const void *buf, size_t len)
{
	for(size_t i = 0; i < len; i++) {
		while(!SerialReady(port)) asm volatile("nop");

		Out8(port, ((uint8_t*)buf)[i]);
	}
}

static void VSerialPut(struct DevSerial *dev, uint8_t ch)
{
	Lock(&dev->dev.lock);

	if(!dev->dev.enabled) {
		Unlock(&dev->dev.lock);
		return;
	}

	SerialWrite(dev->port, &ch, 1);
	Unlock(&dev->dev.lock);
}

static void VSerialWrite(struct DevSerial *dev, const void *buf, size_t n)
{
	Lock(&dev->dev.lock);

	if(!dev->dev.enabled) {
		Unlock(&dev->dev.lock);
		return;
	}

	SerialWrite(dev->port, buf, n);
	Unlock(&dev->dev.lock);
}

static int VSerialReady(struct DevSerial *dev)
{
	Lock(&dev->dev.lock);

	if(!dev->dev.enabled) {
		Unlock(&dev->dev.lock);
		return 0;
	}

	int r = SerialReady(dev->port);

	Unlock(&dev->dev.lock);

	return r;
}

static struct DevSerial srcom1 = { 0 };

//static void KEINIT VSerialInit()
void VSerialInit()
{
	memcpy(srcom1.dev.name, "Serial(COM1)", strlen("Serial(COM1)"));

	srcom1.dev.type    = DEV_TYPE_SERIAL;
	srcom1.dev.enabled = 1;
	srcom1.dev.lock    = 0;

	srcom1.port = SERIAL_COM1;
	SerialInit(SERIAL_COM1);

	srcom1.put   = VSerialPut;
	srcom1.write = VSerialWrite;
	srcom1.ready = VSerialReady;

	DeviceRegister(DEV_CATEGORY_SERIAL, &srcom1.dev);

	DevicePrimarySet(DEV_CATEGORY_SERIAL, &srcom1.dev);
}
