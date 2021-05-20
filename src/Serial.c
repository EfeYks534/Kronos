#include <Serial.h>

void SerialInit(uint16_t port)
{
	Out8(port + 1, 0x00);

	Out8(port + 3, 1 << 7);
	Out8(port,     0x02);
	Out8(port + 1, 0x00);

	Out8(port + 3, 3);

	Out8(port + 2, 199);

	Out8(port + 4, 11);
}

void SerialWrite(uint16_t port, void *buf, int len)
{
	for(int i = 0; i < len; i++) {
		while(!SerialReady(port)) asm volatile("nop");

		Out8(port, ((uint8_t*)buf)[i]);
	}
}


