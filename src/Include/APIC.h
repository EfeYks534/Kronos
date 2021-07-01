#pragma once

#include <Common.h>
#include <stddef.h>
#include <stdint.h>

#define LAPIC_EOI 0xB0
#define LAPIC_SPUR 0xF0

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

void ICRSend(struct ICR *icr);

void IPISend(uint8_t type, uint32_t dest);
