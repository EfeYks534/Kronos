#pragma once

#include <DescTabs.h>
#include <Memory.h>
#include <stddef.h>
#include <stdint.h>
#include <Task.h>


#define IPMSG_BROADCAST 0xFFFFFFFF

#define IPMSG_PING 0x0
#define IPMSG_INVL 0x1

#define IPMSG_SUCCESS 0x1
#define IPMSG_FAIL    0x0


struct IPMessage
{
	uint64_t type;

	uint64_t mqw1;
	uint64_t mqw2;

	struct
	{
		uint64_t res  : 63;
		uint64_t done :  1;
	} PACKED;
} PACKED;

struct IPMessageCntl
{
	struct IPMessage *msg_list;
	size_t           msg_count;

	uint32_t busy;
	int64_t  lock;
};

struct Processor
{
	struct AddressSpace    *space;
	struct Task             *task;
	struct Task        *idle_task;
	struct TSS                tss;

	struct IPMessageCntl msg_cntl;
};

struct Processor *Processors();

struct Processor *ProcCurrent();

uint64_t ProcID();

uint64_t ProcBSP();

size_t ProcCount();

void ProcMessage(struct IPMessage *msg, size_t count, uint32_t dest);
