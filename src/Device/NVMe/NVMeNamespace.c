#include <Common.h>
#include <DescTabs.h>
#include <Peripheral.h>
#include <APIC.h>
#include <Device.h>
#include <stdlib.h>
#include <string.h>
#include "NVMe.h"

size_t NVMeVWrite(struct DevStorage *stor, void *data, size_t addr, size_t size)
{
	struct NVMeDevice *dev   = ((struct NVMeState*) stor->state)->dev;
	struct NVMeNamespace *ns = dev->namespaces[((struct NVMeState*) stor->state)->ns];

	size_t lba  = addr / ns->lba_size;
	size_t blkc = ceil(addr + size, ns->lba_size) - lba;
	size_t end  = lba + blkc;

	if(lba >= ns->lba_count) return 0;

	uint8_t *block = malloc(4096);

	size_t total = 0;

	for(size_t i = lba; i < end; i++) {
		if(i >= ns->lba_count) break;

		size_t offset = (i == lba) ? (addr % ns->lba_size) : 0;
		size_t count  = min(ns->lba_size - offset, size - total);

		if(count == 0 || count > ns->lba_size) break;


		if(offset != 0 || count != ns->lba_size) {
			struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
			struct NVMeResponse res = (struct NVMeResponse) { 0 };

			cmd.opc   = NVME_NVM_READ;
			cmd.prp1  = (uint64_t) MPhys(block);
			cmd.nsid  = ns->nsid;
			cmd.cdw10 = i;
			cmd.cdw11 = i >> 32;
			cmd.cdw12 = 0;

			int32_t r = NVMeSubmit(dev->queues[1], &cmd, &res);

			if(r < 0 || res.sf.sc != 0)
				break;
		}

		memcpy(&block[offset], &((uint8_t*) data)[total], count);

		struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
		struct NVMeResponse res = (struct NVMeResponse) { 0 };

		cmd.opc   = NVME_NVM_WRITE;
		cmd.prp1  = (uint64_t) MPhys(block);
		cmd.nsid  = ns->nsid;
		cmd.cdw10 = i;
		cmd.cdw11 = i >> 32;
		cmd.cdw12 = 0;

		int32_t r = NVMeSubmit(dev->queues[1], &cmd, &res);

		if(r < 0 || res.sf.sc != 0)
			break;

		total += count;
	}

	free(block);

	return total;
}

size_t NVMeVRead(struct DevStorage *stor, void *data, size_t addr, size_t size)
{
	struct NVMeDevice *dev   = ((struct NVMeState*) stor->state)->dev;
	struct NVMeNamespace *ns = dev->namespaces[((struct NVMeState*) stor->state)->ns];

	size_t lba  = addr / ns->lba_size;
	size_t blkc = ceil(addr + size, ns->lba_size) - lba;
	size_t end  = lba + blkc;

	if(lba >= ns->lba_count) return 0;

	uint8_t *block = malloc(4096);

	size_t total = 0;

	for(size_t i = lba; i < end; i++) {
		if(i >= ns->lba_count) break;

		size_t offset = (i == lba) ? (addr % ns->lba_size) : 0;
		size_t count  = min(ns->lba_size - offset, size - total);

		if(count == 0 || count > ns->lba_size) break;


		struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
		struct NVMeResponse res = (struct NVMeResponse) { 0 };

		cmd.opc   = NVME_NVM_READ;
		cmd.prp1  = (uint64_t) MPhys(block);
		cmd.nsid  = ns->nsid;
		cmd.cdw10 = i;
		cmd.cdw11 = i >> 32;
		cmd.cdw12 = 0;

		int32_t r = NVMeSubmit(dev->queues[1], &cmd, &res);

		if(r < 0 || res.sf.sc != 0)
			break;

		memcpy(&((uint8_t*) data)[total], &block[offset], count);

		total += count;
	}

	free(block);

	return total;
}
