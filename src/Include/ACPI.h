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
	} config_list[] PACKED;
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


size_t SDTCount();

struct SDTHeader *ACPIGetSDT(size_t i);

void *ACPIFindSDT(const char *sign);

int ACPIChecksum(void *_ptr, size_t size);
