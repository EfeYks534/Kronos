#pragma once

#include <Common.h>
#include <stddef.h>
#include <stdint.h>

#define PCICAP_HOTPLUG 0x0C
#define PCICAP_MSI     0x05
#define PCICAP_MSIX    0x11

struct PCIAddress
{
	uintptr_t seg;
	uintptr_t bus;
	uintptr_t dev;
	uintptr_t fun;
};

struct PCIDevice
{
	uint16_t  vendor;
	uint16_t  device;
	uint16_t command;
	uint16_t  status;
	uint8_t revision;
	uint8_t   progif;
	uint8_t subclass;
	uint8_t    class;
	uint8_t   caches;
	uint8_t   timerl;
	uint8_t     type;
	uint8_t     bist;
	uint32_t  bar[6];

	uint32_t cis_ptr;
	uint16_t svendor;
	uint16_t     sid;
	uint32_t    erom;
	uint8_t      cap;
	uint8_t  rsvd[7];
	uint8_t     intr;
	uint8_t intr_pin;
	uint8_t min_gran;
	uint8_t  max_lat;
} PACKED;

struct PCIBridge
{
	uint16_t  vendor;
	uint16_t  device;
	uint16_t command;
	uint16_t  status;
	uint8_t revision;
	uint8_t   progif;
	uint8_t subclass;
	uint8_t    class;
	uint8_t   caches;
	uint8_t   timerl;
	uint8_t     type;
	uint8_t     bist;
	uint32_t  bar[2];

	uint8_t  pri_bus;
	uint8_t  sec_bus;
	uint8_t  sub_bus;
	uint8_t  stimerl;
} PACKED;


void MMWrite64(void *addr, uint64_t data);

void MMWrite32(void *addr, uint32_t data);

void MMWrite16(void *addr, uint16_t data);

void MMWrite8(void *addr, uint8_t data);

uint64_t MMRead64(void *addr);

uint32_t MMRead32(void *addr);

uint16_t MMRead16(void *addr);

uint8_t MMRead8(void *addr);


struct PCIDevice *PCIDeviceGet(struct PCIAddress loc);

struct PCIAddress *PCIDeviceList();

size_t PCIDeviceCount();

struct PCIDevice *PCIDeviceCache(struct PCIAddress loc);

void *PCIBar(struct PCIDevice *dev, size_t bar);


int MSIEnable(struct PCIDevice *dev, size_t msi, size_t vector);

void MSIDisable(struct PCIDevice *dev, size_t msi);
