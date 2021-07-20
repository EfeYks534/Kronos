#include <Common.h>
#include <DescTabs.h>
#include <Peripheral.h>
#include <APIC.h>
#include <Device.h>
#include <stdlib.h>
#include "NVMe.h"


static void NVMeHandler(struct Registers *regs, uint64_t arg)
{
	struct NVMeInterrupt *iv = (struct NVMeInterrupt*) arg;

	APICEOI();
}



static inline uint32_t *NVMeTDBL(struct NVMeDevice *dev, size_t q)
{
	uintptr_t base = (uintptr_t) dev->nvme;

	base += 0x1000 + ((2*q) * (4ULL << dev->nvme_cap.dstrd));

	return (uint32_t*) base;
}

static inline uint32_t *NVMeHDBL(struct NVMeDevice *dev, size_t q)
{
	uintptr_t base = (uintptr_t) dev->nvme;

	base += 0x1000 + ((2*q + 1) * (4ULL << dev->nvme_cap.dstrd));

	return (uint32_t*) base;
}

static struct NVMeInterrupt *NVMeIntAlloc(struct NVMeDevice *dev)
{
	Info("NVMe: Allocating interrupt vector\n");

	if(dev->iv_count >= NVME_QUEUE_MAX) {
		Error("NVMe: Failed to allocate interrupt vector\n");
		return NULL;
	}

	struct NVMeInterrupt *iv = &dev->interrupts[dev->iv_count++];

	iv->vec = IDTEntryAlloc(IDT_ATTR_PRESENT | IDT_ATTR_INTR, (uint64_t)iv, NVMeHandler);

	if(iv->vec == 0) {
		Error("NVMe: Failed to allocate interrupt vector\n");
		return NULL;
	}

	iv->msi = MSIEnable(dev->pci, dev->iv_count - 1, iv->vec);

	if(iv->msi == 0) {
		Error("NVMe: Failed to allocate interrupt vector\n");
		return NULL;
	}


	Info("NVMe: Allocated interrupt vector\n");
	return iv;
}

static int NVMeWaitCSTS(struct NVMeDevice *dev, int expected)
{
	volatile struct NVMeCSTS csts;

	size_t to = dev->nvme_cap.to * 500;

	while(--to) {
		*((uint32_t*) &csts) = dev->nvme->csts;

		if(csts.cfs) {
			Error("NVMe: Controller in fatal state\n");
			return 0;
		}

		if(csts.rdy == expected) return 1;

		Sleep(1000000); // Sleep one millisecond
	}

	if(to == 0) {
		Error("NVMe: Controller timed out\n");
		return 0;
	}

	return 1;
}

int NVMeEnable(struct NVMeDevice *dev)
{
	Info("NVMe: Enabling controller\n");

	volatile struct NVMeCC cc;

	*((uint32_t*) &cc) = dev->nvme->cc;

	if(cc.en) {
		Info("NVMe: Enabled controller\n");
		return 1;
	}

	if(!NVMeWaitCSTS(dev, 0))
		return 0;

	cc.en = 1;
	dev->nvme->cc = *((uint32_t*) &cc);

	if(!NVMeWaitCSTS(dev, 1))
		return 0;

	Info("NVMe: Enabled controller\n");

	return 1;
}

int NVMeDisable(struct NVMeDevice *dev)
{
	Info("NVMe: Disabling controller\n");

	volatile struct NVMeCC cc;

	*((uint32_t*) &cc) = dev->nvme->cc;

	if(!cc.en) {
		Info("NVMe: Disabled controller\n");
		return 1;
	}

	if(!NVMeWaitCSTS(dev, 1))
		return 0;

	cc.en = 0;
	dev->nvme->cc = *((uint32_t*) &cc);

	if(!NVMeWaitCSTS(dev, 0))
		return 0;

	Info("NVMe: Disabled controller\n");

	return 1;
}

void NVMeInitDev(struct PCIDevice *pci)
{
	Info("NVMe: Initializing controller\n");

	struct NVMeDevice *dev = calloc(1, sizeof(struct NVMeDevice));

	dev->pci  = pci;
	dev->nvme = PCIBar(pci, 0);

	Info("NVMe: Base address %xl\n", dev->nvme);

	uint16_t cmd = MMRead16(&pci->command);

	cmd |= 1ULL << 10; // Enable interrupt disable bit
	cmd |= 1ULL << 2;  // Enable bus master
	cmd |= 1ULL << 1;  // Enable controller memory space

	MMWrite16(&pci->command, cmd);

	*((uint64_t*) &dev->nvme_cap) = dev->nvme->cap;

	if(!NVMeDisable(dev)) {
		free(dev);
		return;
	}

	if(dev->nvme_cap.mqes < 64) {
		Error("NVMe: Maximum queue entries cannot be smaller than 64\n");

		free(dev);
		return;
	}

	if(dev->nvme_cap.mpsmin != 0) {
		Error("NVMe: Minimum memory page size cannot be bigger than 4096\n");

		free(dev);
		return;
	}

	volatile struct NVMeCC cc;

	*((uint32_t*) &cc) = dev->nvme->cc;

	cc.iosqes = 6;
	cc.iocqes = 4;

	dev->nvme->cc = *((uint32_t*) &cc);


	Info("NVMe: Initializing admin queue\n");

	{
		volatile struct NVMeAQA aqa;

		*((uint32_t*) &aqa) = dev->nvme->aqa;

		aqa.asqs = 63;
		aqa.acqs = 63;

		struct NVMeQueue *aq = calloc(1, sizeof(struct NVMeQueue));
		dev->queues[0] = aq;

		aq->qsz  = 64;
		aq->sq   = calloc(4096, 1);
		aq->cq   = calloc(4096, 1);
		aq->tdbl = NVMeTDBL(dev, 0);
		aq->hdbl = NVMeHDBL(dev, 0);

		struct NVMeInterrupt *iv = NVMeIntAlloc(dev);

		if(iv == NULL) {
			free(aq);
			free(dev);
			return;
		}

		aq->iv = dev->iv_count - 1;

		dev->nvme->aqa = *((uint32_t*) &aqa);
		dev->nvme->asq =  (uint64_t) aq->sq;
		dev->nvme->acq =  (uint64_t) aq->cq;

		dev->queue_count++;
	}

	Info("NVMe: Initialized admin queue\n");

	NVMeEnable(dev);
}

static void KDINIT NVMeInitAll()
{
	size_t pci_count = PCIDeviceCount();

	struct PCIAddress *pci_list = PCIDeviceList();

	for(size_t i = 0; i < pci_count; i++) {
		struct PCIDevice *pci = PCIDeviceCache(pci_list[i]);

		if(pci->class == 0x1 && pci->subclass == 0x8 && pci->progif == 2)
			NVMeInitDev(PCIDeviceGet(pci_list[i]));
	}
}
