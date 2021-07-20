#include <Common.h>
#include <DescTabs.h>
#include <Peripheral.h>
#include <APIC.h>
#include <Device.h>
#include <stdlib.h>
#include "NVMe.h"

int32_t NVMeSubmit(struct NVMeQueue *q, struct NVMeCommand *cmd)
{
	size_t timeout = 1000000;

	while(--timeout) {
		Lock(&q->lock);

		if(!(q->head == ((q->tail + 1) % q->sz)))
			break;

		Unlock(&q->lock);

		if((FlagsGet() >> 9) & 1)
			asm volatile("hlt");
		else
			Yield();
	}

	if(timeout == 0) {
		Unlock(&q->lock);
		return -1;
	}


	cmd->cid = q->cmdid++;

	q->sq[q->tail] = *cmd;

	q->tail = (q->tail + 1) % q->sz;

	*q->tdbl = q->tail;


	Unlock(&q->lock);


	return cmd->cid;
}

void NVMeConsume(struct NVMeQueue *q, struct NVMeResponse *res, uint16_t cmd);
