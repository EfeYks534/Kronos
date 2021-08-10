#include <Common.h>
#include <Core.h>
#include <Peripheral.h>

struct MSICap
{
	uint16_t   mid;
	uint16_t    mc;
	uint32_t    ma;
	uint32_t   mua;
	uint16_t    md;
	uint32_t mmask;
	uint32_t mpend;
} PACKED;

struct MSIXCap
{
	uint16_t mxid;
	uint16_t  mxc;
	uint32_t mtab;
	uint32_t mpba;
} PACKED;

struct MSIXTab
{
	uint64_t ma;
	uint32_t md;
	uint32_t vc;
} PACKED;

int MSIEnable(struct PCIDevice *dev, size_t slot, size_t vector)
{
	uint16_t status = MMRead16(&dev->status);
	uint16_t vendor = MMRead16(&dev->vendor);
	uint16_t device = MMRead16(&dev->device);

	if(((status >> 4) & 1) == 0) {
		Error("MSI: Device %xs:%xs incapable of MSIs\n", vendor, device);
		return 0;
	}


	uint8_t cptr = MMRead8(&dev->cap);

	while(cptr) {
		uint16_t cap = MMRead16((void*) ((size_t) dev + cptr));

		if((cap & 0xFF) == 0x5) {

			struct MSICap *msi = (struct MSICap*) ((size_t) &dev + cptr);

			uint16_t mc = MMRead16(&msi->mc);

			mc &= ~0x70; // Disable MME

			mc |= 1;


			MMWrite32(&msi->ma, 0xFEE00000 | ProcBSP());

			if((mc >> 7) & 1) // 64 bit enabled
				MMWrite16(&msi->md, vector);
			else
				MMWrite16(&msi->mua, vector);

			MMWrite16(&msi->mc, mc);

			Info("MSI: Enabled MSIs for device %xs:%xs\n", vendor, device);

			return 1;
		}
		if((cap & 0xFF) == 0x11) {

			struct MSIXCap *msi = (struct MSIXCap*) ((size_t) dev + cptr);

			uint16_t mxc  = MMRead16(&msi->mxc);
			uint32_t mtab = MMRead32(&msi->mtab);

			mxc |=   1ULL << 15;  // Enable MSI-X
			mxc &= ~(1ULL << 14); // Disable function mask bit


			uint16_t ts = (mxc & 2047) + 1; // Get the table size

			if(slot >= ts) {
				Error("MSI: Device %xs:%xs out of MSI-X slots\n", vendor, device);
				return 0;
			}

			struct MSIXTab *tab = (struct MSIXTab*) PCIBar(dev, mtab & 7);
			tab = (struct MSIXTab*) ((uintptr_t) tab + (mtab & ~2047ULL));

			MMWrite64(&tab[slot].ma, 0xFEE00000 | ProcBSP());
			MMWrite32(&tab[slot].md, vector);
			MMWrite32(&tab[slot].vc, 0);

			MMWrite16(&msi->mxc, mxc);

			Info("MSI: Enabled MSI-X for device %xs:%xs\n", vendor, device);
			return 1;
		}

		cptr = cap >> 8;
	}

	Error("MSI: Device %xs:%xs incapable of MSIs\n", vendor, device);
	return 0;
}

int MSIDisable(struct PCIDevice *dev, size_t slot)
{
	uint16_t status = MMRead16(&dev->status);
	uint16_t vendor = MMRead16(&dev->vendor);
	uint16_t device = MMRead16(&dev->device);

	if(((status >> 4) & 1) == 0) {
		Error("MSI: Device %xs:%xs incapable of MSIs\n", vendor, device);
		return 0;
	}


	uint8_t cptr = MMRead8(&dev->cap);

	while(cptr) {
		uint16_t cap = MMRead16((void*) ((size_t) dev + cptr));

		if((cap & 0xFF) == 0x5) {

			if(slot != 0)
				Warn("MSI: Trying to disable MSI %u on device with MME disabled\n", slot);

			struct MSICap *msi = (struct MSICap*) ((size_t) &dev + cptr);

			uint16_t mc = MMRead16(&msi->mc);

			mc &= ~1; // Clear enable bit

			MMWrite16(&msi->mc, mc);

			Info("MSI: Disabled MSIs for device %xs:%xs\n", vendor, device);

			return 1;
		}
		if((cap & 0xFF) == 0x11) {

			struct MSIXCap *msi = (struct MSIXCap*) ((size_t) dev + cptr);

			uint16_t mxc  = MMRead16(&msi->mxc);
			uint32_t mtab = MMRead32(&msi->mtab);

			uint16_t ts = (mxc & 2047) + 1; // Get the table size

			if(slot >= ts) {
				Error("MSI-X: Device %xs:%xs doesn't have MSI-X vector %u\n", slot);
				return 0;
			}

			struct MSIXTab *tab = (struct MSIXTab*) PCIBar(dev, mtab & 7);
			tab = (struct MSIXTab*) ((uintptr_t) tab + (mtab & ~2047ULL));

			MMWrite64(&tab[slot].ma, 0);
			MMWrite32(&tab[slot].md, 0);
			MMWrite32(&tab[slot].vc, 1);

			Info("MSI-X: Disabled vector %u for device %xs:%xs\n", msi, vendor, device);
			return 1;
		}

		cptr = cap >> 8;
	}

	Error("MSI: Device %xs:%xs incapable of MSI(-X)\n", vendor, device);
	return 0;
}

