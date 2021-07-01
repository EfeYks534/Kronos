#pragma once

#include <Common.h>
#include <stddef.h>
#include <stdint.h>

struct RSDP
{
	char         sign[8];
	uint8_t     checksum;
	char        oemid[6];
	uint8_t     revision;
	uint32_t        rsdt;

	uint32_t      length;
	uint64_t        xsdt;
	uint8_t ext_checksum;
	uint8_t  reserved[3];
} PACKED;

struct SDTHeader
{
	char     sign[4];
	uint32_t  length;
	uint8_t revision;
	uint8_t checksum;
	char    oemid[6];
	char oemtabid[8];
	uint32_t  oemrev;
	uint32_t creator;
	uint32_t    crev;
} PACKED;

struct MCFG
{
	struct SDTHeader hdr;
	uint64_t        rsvd;

	struct MCFGEntry
	{
		uint64_t     base;
		uint16_t  segment;
		uint8_t bus_start;
		uint8_t   bus_end;
		uint32_t     rsvd;
	} PACKED config_list[];
} PACKED;

struct MADT
{
	struct SDTHeader hdr;
	uint32_t  lapic_addr;
	uint32_t       flags;

	struct MADTEntry
	{
		uint8_t   type;
		uint8_t length;
	} ent[] PACKED;
} PACKED;

struct HPET
{
	struct SDTHeader hdr;

	uint8_t hw_rev;

	struct
	{
		uint8_t  cmp_count :  5;
		uint8_t count_size :  1;
		uint8_t       rsvd :  1;
		uint8_t    irq_cap :  1;
	} PACKED;

	uint16_t vendor;

	struct
	{
		uint8_t   space_id;
		uint8_t  bit_width;
		uint8_t bit_offset;
		uint8_t       rsvd;
		uint64_t      addr;
	} PACKED address;

	uint8_t hpet_number;
	uint16_t   min_tick;
	uint8_t   page_prot;
} PACKED;

size_t SDTCount();

struct SDTHeader *ACPIGetSDT(size_t i);

void *ACPIFindSDT(const char *sign);

int ACPIChecksum(void *_ptr, size_t size);
