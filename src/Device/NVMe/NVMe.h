#pragma once

#include <Common.h>
#include <Peripheral.h>
#include <stddef.h>
#include <stdint.h>

// #define NVME_DEBUG to enable debug messages

#define NVME_DEBUG


#ifndef NVME_DEBUG

#define Error(f, ...)
#define Info(f, ...)
#define Warn(f, ...)

#endif


#define NVME_QUEUE_MAX 16
#define NVME_NS_MAX    16

struct NVMeCap
{
	uint64_t   mqes : 16;
	uint64_t    cqr :  1;
	uint64_t    ams :  2;
	uint64_t  rsvd0 :  5;
	uint64_t     to :  8;
	uint64_t  dstrd :  4;
	uint64_t  nssrs :  1;
	uint64_t    css :  8;
	uint64_t    bps :  1;
	uint64_t   rsvd :  2;
	uint64_t mpsmin :  4;
	uint64_t mpsmax :  4;
	uint64_t   pmrs :  1;
	uint64_t   cmbs :  1;
	uint64_t  rsvd1 :  6;
} PACKED;

struct NVMeCC
{
	uint32_t     en : 1;
	uint32_t  rsvd0 : 3;
	uint32_t    css : 3;
	uint32_t    mps : 4;
	uint32_t    ams : 3;
	uint32_t    shn : 2;
	uint32_t iosqes : 4;
	uint32_t iocqes : 4;
	uint32_t  rsvd1 : 8;
} PACKED;

struct NVMeCSTS
{
	uint32_t   rdy :  1;
	uint32_t   cfs :  1;
	uint32_t  shst :  2;
	uint32_t nssro :  1;
	uint32_t    pp :  1;
	uint32_t  rsvd : 26;
} PACKED;

struct NVMeAQA
{
	uint32_t  asqs : 12;
	uint32_t rsvd0 :  4;
	uint32_t  acqs : 12;
	uint32_t rsvd1 :  4;
} PACKED;

struct NVMe
{
	uint64_t   cap;
	uint32_t    vs;
	uint32_t intms;
	uint32_t intmc;
	uint32_t    cc;
	uint32_t  rsvd;
	uint32_t  csts;
	uint32_t  nssr;
	uint32_t   aqa;
	uint64_t   asq;
	uint64_t   acq;
} PACKED;

struct NVMeCommand
{
	struct
	{
		uint32_t   opc :  8;
		uint32_t  fuse :  2;
		uint32_t rsvd0 :  4;
		uint32_t  psdt :  2;
		uint32_t   cid : 16;
	} PACKED;

	uint32_t  nsid;
	uint64_t rsvd1;
	uint64_t  mptr;

	uint64_t  prp1;
	uint64_t  prp2;

	uint32_t cdw10;
	uint32_t cdw11;
	uint32_t cdw12;
	uint32_t cdw13;
	uint32_t cdw14;
	uint32_t cdw15;
} PACKED;

struct NVMeResponse
{
	uint32_t  dw0;
	uint32_t rsvd;

	uint16_t sqhd;
	uint16_t sqid;

	uint16_t  cid;

	union
	{
		struct
		{
			uint16_t rsvd : 1;
			uint16_t   sc : 8;
			uint16_t  sct : 3;
			uint16_t  crd : 2;
			uint16_t    m : 1;
			uint16_t  dnr : 1;
		} PACKED sf;

		struct
		{
			uint16_t     p :  1;
			uint16_t rsvd1 : 15;
		} PACKED;
	} PACKED;
} PACKED;

struct NVMeQueue
{
	uint16_t qid;
	uint16_t qsz;

	struct NVMeCommand  *sq;
	struct NVMeResponse *cq;

	size_t tail;
	size_t head;

	volatile uint32_t *tdbl;
	volatile uint32_t *hdbl;

	uint16_t cmdid;
	int64_t   lock;
	size_t      iv;
};

struct NVMeNamespace
{
	uint32_t nsid;
};

struct NVMeInterrupt
{
	uint8_t vec;
	size_t  msi;

	struct NVMeDevice *dev;
};

struct NVMeDevice
{
	struct PCIDevice      *pci;
	volatile struct NVMe *nvme;
	struct NVMeCap    nvme_cap;
	size_t         queue_count;
	size_t            ns_count;
	size_t            iv_count;
	int64_t               lock;

	struct NVMeQueue         *queues[NVME_QUEUE_MAX];
	struct NVMeInterrupt  interrupts[NVME_QUEUE_MAX];
	struct NVMeNamespace *namespaces[NVME_NS_MAX];
};

struct NVMeState
{
	struct NVMeDevice *device;

	uint32_t nsid;
};

int NVMeEnable(struct NVMeDevice *dev);

int NVMeDisable(struct NVMeDevice *dev);

void NVMeInitDev(struct PCIDevice *pci);

int32_t NVMeSubmit(struct NVMeQueue *q, struct NVMeCommand *cmd);

int NVMeConsume(struct NVMeQueue *q, struct NVMeResponse *res, uint16_t cmd);
