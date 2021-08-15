#pragma once

#include <Common.h>
#include <stddef.h>
#include <stdint.h>

#define LAPIC_EOI    0x0B0
#define LAPIC_SPUR   0x0F0
#define LAPIC_LVT    0x320
#define LAPIC_LINT0  0x350
#define LAPIC_LINT1  0x360
#define LAPIC_ICOUNT 0x380
#define LAPIC_CCOUNT 0x390
#define LAPIC_DIVCFG 0x3E0

#define LAPIC_INTN   0xFE

struct ICR
{
	struct
	{
		uint32_t vector :  8;
		uint32_t   delv :  3;
		uint32_t  destm :  1;
		uint32_t   pend :  1;
		uint32_t  rsvd0 :  1;
		uint32_t  level :  1;
		uint32_t   trig :  1;
		uint32_t  rsvd1 :  2;
		uint32_t  destt :  2;
		uint32_t  rsvd2 : 12;
	} PACKED;

	struct
	{
		uint32_t rsvd3 : 24;
		uint32_t  dest :  8;
	} PACKED;
} PACKED;

void APICEOI();

void APICTimerEnable();

void ICRSend(struct ICR *icr);

void IPISend(uint8_t type, uint32_t dest);

