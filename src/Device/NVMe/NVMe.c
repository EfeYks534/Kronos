#include <Common.h>
#include <DescTabs.h>
#include <Peripheral.h>
#include <APIC.h>
#include <Device.h>
#include <stdlib.h>
#include <string.h>
#include "NVMe.h"



static void NVMeDestroy(struct NVMeDevice *dev)
{
	size_t to = dev->nvme_cap.to * 500;

	while(--to) {
		Lock(&dev->lock);

		if(dev->busy > 0) {
			Unlock(&dev->lock);
			Sleep(1000000);
		}

		break;
	}

	for(size_t i = 0; i < dev->queue_count; i++) {
		struct NVMeQueue *q = dev->queues[i];

		free((void*) q->sq);
		free((void*) q->cq);
		free(q->requests.items);
		free(q);

		dev->queues[i] = NULL;
	}

	for(size_t i = 0; i < dev->iv_count; i++)
		MSIDisable(dev->pci, dev->interrupts[i].msi);

	for(size_t i = 0; i < dev->ns_count; i++) {
		free(dev->namespaces[i]);

		dev->namespaces[i] = NULL;
	}

	Unlock(&dev->lock);
}


struct NVMeInterrupt *NVMeIntAlloc(struct NVMeDevice *dev)
{
	Info("NVMe: Allocating interrupt vector\n");

	if(dev->iv_count >= NVME_QUEUE_MAX) {
		Error("NVMe: Failed to allocate interrupt vector\n");
		return NULL;
	}

	struct NVMeInterrupt *iv = &dev->interrupts[dev->iv_count++];

	iv->dev = dev;

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

	iv->msi = dev->iv_count - 1;


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

		Sleep(1000000);
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


	{
		uint16_t pcmd = MMRead16(&pci->command);

		pcmd |= 1ULL << 10; // Enable interrupt disable bit
		pcmd |= 1ULL << 2;  // Enable bus master
		pcmd |= 1ULL << 1;  // Enable controller memory space

		MMWrite16(&pci->command, pcmd);
	}


	*((uint64_t*) &dev->nvme_cap) = dev->nvme->cap; // Cache capabilities register

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

		aq->requests.items = calloc(128, sizeof(struct NVMeResponse));
		aq->requests.nmemb = 128;
		aq->requests.size  = sizeof(struct NVMeRequest);


		dev->nvme->aqa = *((uint32_t*) &aqa);
		dev->nvme->asq =  (uint64_t) MPhys((void*) aq->sq);
		dev->nvme->acq =  (uint64_t) MPhys((void*) aq->cq);

		dev->queue_count++;
	}

	Info("NVMe: Initialized admin queue\n");

	NVMeEnable(dev);


	{
		struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
		struct NVMeResponse res = (struct NVMeResponse) { 0 };

		void *data = calloc(4096, 1);

		cmd.opc   = NVME_ADMIN_IDENTIFY;
		cmd.prp1  = (uint64_t) MPhys(data);
		cmd.cdw10 = 1;

		int32_t cid = NVMeSubmit(dev->queues[0], &cmd, &res);

		free(data);

		if(cid < 0) {
			Error("NVMe: Couldn't submit identify command to admin queue\n");

			NVMeDestroy(dev);
			free(dev);
			return;
		}
	}

	Info("NVMe: Creating I/O queues\n");

	size_t total = 0;

	for(int i = 0; i < 4; i++) {
		size_t r = NVMeCreateQueue(dev);

		if(r > 0)
			total++;
	}

	if(total > 0) {
		Info("NVMe: %u I/O queues were successfully created\n", total);
	} else {
		Error("NVMe: No I/O queues could be created\n");

		NVMeDestroy(dev);
		free(dev);
		return;
	}

	Info("NVMe: Validating namespace 1\n");

	struct NVMeNamespace *ns = NVMeValidateNS(dev, 1);

	if(ns != NULL) {
		Info("NVMe: Namespace 1 is active and valid.\n       + Reported LBA count: %u\n       + Reported LBA size: %u bytes\n", ns->lba_count, ns->lba_size);
	} else {
		Error("NVMe: Namespace 1 is not active or valid.\n");
	}

	dev->namespaces[dev->ns_count++] = ns;

	for(size_t i = 0; i < dev->ns_count; i++) {
		struct NVMeState *state = calloc(1, sizeof(struct NVMeState));

		state->dev = dev;
		state->ns  = i;

		struct DevStorage *vir_dev = calloc(1, sizeof(struct DevStorage));

		vir_dev->dev.type = DEV_TYPE_NVME;

		memcpy(vir_dev->dev.name, "NVMe", 4);

		vir_dev->state = state;

		vir_dev->write = NVMeVWrite;
		vir_dev->read  = NVMeVRead;

		DeviceRegister(DEV_CATEGORY_STORAGE, &vir_dev->dev);
	}
}

struct NVMeNamespace *NVMeValidateNS(struct NVMeDevice *dev, uint32_t nsid)
{
	struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
	struct NVMeResponse res = (struct NVMeResponse) { 0 };

	void *data = calloc(4096, 1);

	cmd.opc  = NVME_ADMIN_IDENTIFY;
	cmd.prp1 = (uint64_t) MPhys(data);
	cmd.nsid = nsid;

	int32_t cid = NVMeSubmit(dev->queues[0], &cmd, &res);

	if(cid < 0) {
		free(data);
		return NULL;
	}

	struct NVMeNamespace *ns = calloc(1, sizeof(struct NVMeNamespace));

	ns->ident = data;
	ns->nsid  = nsid;

	if(ns->ident->nsze == 0) {
		free(ns);
		free(data);

		return NULL;
	}

	struct NVMeLBAFormat lbaf = ns->ident->lbaf[ns->ident->flbas & 15];

	if(lbaf.lbads < 9) {
		free(ns);
		free(data);

		return NULL;
	}

	ns->lba_count = ns->ident->nsze + 1;
	ns->lba_size  = 1ULL << lbaf.lbads;

	return ns;
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
