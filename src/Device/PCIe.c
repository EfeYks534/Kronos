#include <Common.h>
#include <Peripheral.h>
#include <ACPI.h>

static struct PCIAddress pci_dev_list[256] = { 0 };

static size_t pci_dev_count = 0;

static uintptr_t PCIBase(struct PCIAddress loc)
{
	struct MCFG *mcfg = ACPIFindSDT("MCFG");
	Assert(mcfg != NULL, "");

	size_t ent_count = (mcfg->hdr.length - sizeof(struct MCFG)) / 16;

	struct MCFGEntry *ent = NULL;

	for(size_t i = 0; i < ent_count; i++) {
		struct MCFGEntry *e = &mcfg->config_list[i];

		if(e->segment == loc.seg && loc.bus >= e->bus_start && loc.bus <= e->bus_end) {
			ent = e;
			break;
		}
	}

	Assert(ent != NULL, "");

	uintptr_t base = ent->base;
	uint64_t bus   = (loc.bus - ent->bus_start);
	uint64_t dev   = loc.dev;
	uint64_t fun   = loc.fun;

	base += (bus << 20) | (dev << 15) | (fun << 12);

	return base;
}

static void PCIEnumFunc(struct PCIAddress loc);

static void PCIEnumDev(struct PCIAddress loc)
{
	struct PCIDevice *dev = PhysOffset(PCIBase(loc));

	if(MMRead16(&dev->vendor) == 0xFFFF) return;

	PCIEnumFunc(loc);

	uint8_t dev_class    = MMRead8(&dev->class);
	uint8_t dev_subclass = MMRead8(&dev->subclass);

	if(dev_class == 0x6 && dev_subclass == 0x4)
		return;

	if(!(MMRead8(&dev->type) & 0x80)) return;


	for(uint8_t i = 1; i < 8; i++) {
		loc.fun = i;
		dev     = PhysOffset(PCIBase(loc));

		if(MMRead16(&dev->vendor) == 0xFFFF) continue;

		PCIEnumFunc(loc);
	}
}

static void PCIEnumBus(uint8_t seg, uint8_t bus)
{
	for(uint8_t i = 0; i < 32; i++)
		PCIEnumDev((struct PCIAddress) { seg, bus, i, 0});
}

static void PCIEnumFunc(struct PCIAddress loc)
{
	struct PCIDevice *dev = PhysOffset(PCIBase(loc));

	uint8_t dev_class    = MMRead8(&dev->class);
	uint8_t dev_subclass = MMRead8(&dev->subclass);

	if(dev_class == 0x6 && dev_subclass == 0x4) {
		struct PCIBridge *bridge = (struct PCIBridge*) dev;

		uint8_t sub_bus = MMRead8(&bridge->sub_bus);
		uint8_t sec_bus = MMRead8(&bridge->sec_bus);

		PCIEnumBus(loc.seg, sec_bus);
	} else {
		Info("PCIe: Found device ");
		Log("%xs:%xs\n", MMRead16(&dev->vendor), MMRead16(&dev->device));

		pci_dev_list[pci_dev_count++] = loc;
	}
}

static void PCIEnumAll()
{
	struct MCFG *mcfg = ACPIFindSDT("MCFG");
	Assert(mcfg != NULL, "");

	size_t ent_count = (mcfg->hdr.length - sizeof(struct MCFG)) / 16;

	for(size_t i = 0; i < ent_count; i++) {
		struct MCFGEntry *e = &mcfg->config_list[i];

		if(e->bus_start != 0) continue;

		struct PCIDevice *dev = PhysOffset(PCIBase((struct PCIAddress) { 0 }));

		Assert(!(MMRead8(&dev->type) & 0x80), "");

		PCIEnumBus(e->segment, 0);
	}
}

static void KLINIT PCIeInit()
{
	PCIEnumAll();
}

struct PCIAddress *PCIDeviceList()
{
	return pci_dev_list;
}

size_t PCIDeviceCount()
{
	return pci_dev_count;
}
