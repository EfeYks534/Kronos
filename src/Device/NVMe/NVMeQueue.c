#include <Common.h>
#include <DescTabs.h>
#include <Peripheral.h>
#include <APIC.h>
#include <Device.h>
#include <stdlib.h>
#include "NVMe.h"



size_t NVMeCreateQueue(struct NVMeDevice *dev)
{
	struct NVMeQueue *q = calloc(1, sizeof(struct NVMeQueue));

	q->qid = dev->queue_count++;

	dev->queues[q->qid] = q;

	q->qsz  = 64;
	q->qsz  = 64;
	q->sq   = calloc(4096, 1);
	q->cq   = calloc(4096, 1);
	q->tdbl = NVMeTDBL(dev, q->qid);
	q->hdbl = NVMeHDBL(dev, q->qid);

	struct NVMeInterrupt *iv = NVMeIntAlloc(dev);

	if(iv == NULL) {
		free((void*) q->sq);
		free((void*) q->cq);
		free(q);
		return 0;
	}

	q->iv = dev->iv_count - 1;

	q->requests.items = calloc(128, sizeof(struct NVMeResponse));
	q->requests.nmemb = 128;
	q->requests.size  = sizeof(struct NVMeRequest);

	{
		struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
		struct NVMeResponse res = (struct NVMeResponse) { 0 };

		cmd.opc   = NVME_ADMIN_NEWIOCQ;
		cmd.prp1  = (uint64_t) MPhys((void*) q->cq);
		cmd.cdw10 = q->qid | (63ULL << 16);
		cmd.cdw11 = 3 | (iv->msi << 16);

		int32_t r = NVMeSubmit(dev->queues[0], &cmd, &res);

		if(r == -1 || res.sf.sc != 0) {
			free((void*) q->sq);
			free((void*) q->cq);
			free(q);
			return 0;
		}
	}

	{
		struct NVMeCommand cmd  = (struct NVMeCommand)  { 0 };
		struct NVMeResponse res = (struct NVMeResponse) { 0 };

		cmd.opc   = NVME_ADMIN_NEWIOSQ;
		cmd.prp1  = (uint64_t) MPhys((void*) q->sq);
		cmd.cdw10 = q->qid | (63ULL << 16);
		cmd.cdw11 = 1 | q->qid << 16;

		int32_t r = NVMeSubmit(dev->queues[0], &cmd, &res);

		if(r == -1 || res.sf.sc != 0) {
			free((void*) q->sq);
			free((void*) q->cq);
			free(q);
			return 0;
		}
	}

	return q->qid;
}

void NVMeHandler(struct Registers *regs, uint64_t arg)
{
	struct NVMeInterrupt *iv = (struct NVMeInterrupt*) arg;

	Lock(&iv->dev->lock);

	iv->dev->busy++;

	Unlock(&iv->dev->lock);

	for(size_t i = 0; i < iv->dev->queue_count; i++) {
		struct NVMeQueue *q = iv->dev->queues[i];

		if(q->iv != iv->msi) continue;
		if(q->head == q->tail) continue;

		Lock(&q->lock);

		struct NVMeResponse res;
		struct NVMeRequest req;

		while(q->head != q->tail) {
			res = q->cq[q->head];
			if(res.p == q->phase) break;

			size_t rc = q->requests.tail - q->requests.head;


			for(size_t j = 0; j < rc; j++) {
				if(!QueueConsume(&q->requests, &req))
					break;

				if(req.cid == res.cid) {
					*req.res   = res;
					*req.phase = 1;
				} else {
					QueueSubmit(&q->requests, &req);
				}
			}

			q->head++;

			if(q->head >= q->qsz) {
				q->head  = 0;
				q->phase = !q->phase;
			}
		}

		*q->hdbl = q->head;

		Unlock(&q->lock);
	}

	Lock(&iv->dev->lock);

	iv->dev->busy--;

	Unlock(&iv->dev->lock);


	APICEOI();
}

int32_t NVMeThrow(struct NVMeQueue *q, struct NVMeCommand *cmd)
{
	size_t timeout = 1000000;

	while(--timeout) {
		Lock(&q->lock);

		if(!(q->head == ((q->tail + 1) % q->qsz)))
			break;

		Unlock(&q->lock);

		if((FlagsGet() >> 9) & 1)
			asm volatile("hlt");
	}

	if(timeout == 0) {
		Error("NVMe: NVMeThrow timed out\n");

		Unlock(&q->lock);
		return -1;
	}


	cmd->cid = q->cmdid++;

	q->sq[q->tail] = *cmd;

	q->tail = (q->tail + 1) % q->qsz;

	*q->tdbl = q->tail;


	Unlock(&q->lock);


	return cmd->cid;
}

int32_t NVMeSubmit(struct NVMeQueue *q, struct NVMeCommand *cmd, struct NVMeResponse *res)
{
	if(!((FlagsGet() >> 9) & 1))
		Warn("NVMe: NVMeSubmit called with IFL 0\n");

	size_t timeout = 1000000;

	while(--timeout) {
		Lock(&q->lock);

		if(!(q->head == ((q->tail + 1) % q->qsz)))
			break;


		Unlock(&q->lock);

		asm volatile("hlt");
	}

	if(timeout == 0) {
		Error("NVMe: NVMeSubmit timed out\n");

		Unlock(&q->lock);
		return -1;
	}


	cmd->cid = q->cmdid++;

	q->sq[q->tail] = *cmd;

	q->tail = (q->tail + 1) % q->qsz;


	uint8_t ph = 0;

	struct NVMeRequest req;
	req.cid   = cmd->cid;
	req.phase = &ph;
	req.res   = res;

	int r = QueueSubmit(&q->requests, &req);

	if(!r) {
		Error("NVMe: NVMeSubmit can't push requests\n");
		Unlock(&q->lock);
		return -1;
	}


	*q->tdbl = q->tail;
	Unlock(&q->lock);

	while(!ph)
		asm volatile("hlt");

	return req.cid;
}
