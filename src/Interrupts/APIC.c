#include <Peripheral.h>
#include <DescTabs.h>
#include <Common.h>
#include <Memory.h>
#include <Device.h>
#include <Task.h>
#include <ACPI.h>
#include <APIC.h>
#include <Core.h>

struct MADTLAPICOverride
{
	uint8_t   type;
	uint8_t length;
	uint16_t  rsvd;
	uint64_t  addr;
} PACKED;


static uint8_t *lapic_addr = NULL;

static size_t spur_count = 0;


static void SpurHandler(struct Registers *regs, uint64_t arg)
{
	spur_count++;
	APICEOI();
}

static void TimerHandler(struct Registers *regs, uint64_t arg)
{
	APICEOI(); // This ordering will bite me in the ass later

	Schedule(regs);
}

static void IPIHandler(struct Registers *regs, uint64_t arg)
{
	struct IPMessageCntl *cntl = &ProcCurrent()->msg_cntl;

	if(!cntl->busy || cntl->msg_count == 0 || cntl->msg_list == NULL)
		Panic(NULL, "IPI with no pending messages");


	for(size_t i = 0; i < cntl->msg_count; i++) {
		struct IPMessage *msg = &cntl->msg_list[i];

		switch(msg->type)
		{
		case IPMSG_PING: {
			msg->res  = IPMSG_SUCCESS;
			msg->done = 1;
			break;
		  }
		default:
			Panic(NULL, "Can't recognize message");
		}
	}


	cntl->msg_list  = NULL;
	cntl->msg_count = 0;

	APICEOI();
	cntl->busy = 0;
}

static void KLINIT APICInit()
{
	struct MADT *madt = ACPIFindSDT("APIC");
	Assert(madt != NULL, "");

	// Disable the 8259 PIC
	Out8(0xA1, 0xFF);
	Out8(0x21, 0xFF);

	lapic_addr = PhysOffset(madt->lapic_addr);

	struct MADTEntry *ent = madt->ent;

	size_t len = sizeof(struct MADT);

	while(len < madt->hdr.length) {
		switch(ent->type)
		{
		case 5: {
			struct MADTLAPICOverride *lapic = (void*) ent;

			lapic_addr = PhysOffset(lapic->addr);
			break;
		  }
		}

		len += ent->length;
		ent  = (struct MADTEntry*) ((uintptr_t) madt + len);
	}

	Info("LAPIC: Base address identified as %xl\n", MPhys(lapic_addr));


	IDTEntrySet(0xFF, IDT_ATTR_PRESENT | IDT_ATTR_INTR, 0, SpurHandler);
	IDTEntrySet(0xFE, IDT_ATTR_PRESENT | IDT_ATTR_INTR, 0, TimerHandler);
	IDTEntrySet(0xFD, IDT_ATTR_PRESENT | IDT_ATTR_INTR, 0, IPIHandler);

	uint32_t spur = MMRead32(&lapic_addr[LAPIC_SPUR]);
	MMWrite32(&lapic_addr[LAPIC_SPUR], spur | (1ULL << 8) | 0xFF);
}

void APICTimerEnable()
{
	uint32_t spur = MMRead32(&lapic_addr[LAPIC_SPUR]);
	if(((spur >> 8) & 1) == 0)
		MMWrite32(&lapic_addr[LAPIC_SPUR], spur | (1ULL << 8) | 0xFF);


	MMWrite32(&lapic_addr[LAPIC_DIVCFG], 3); // Divide by 16
	MMWrite32(&lapic_addr[LAPIC_ICOUNT], 0xFFFFFFFF);

	Sleep(10000000); // Sleep 10 milliseconds

	size_t period = 0xFFFFFFFF - MMRead32(&lapic_addr[LAPIC_CCOUNT]);

	MMWrite32(&lapic_addr[LAPIC_LVT],    LAPIC_INTN | (1ULL << 17));
	MMWrite32(&lapic_addr[LAPIC_ICOUNT], period);
}

void APICEOI()
{
	MMWrite32(&lapic_addr[LAPIC_EOI], 0);
}

void ICRSend(struct ICR *icr)
{
	uint32_t *icr32 = (uint32_t*) icr;

	MMWrite32(&lapic_addr[LAPIC_ICR_HIGH], icr32[1]);
	MMWrite32(&lapic_addr[LAPIC_ICR_LOW],  icr32[0]);
}

void IPISend(uint8_t type, uint32_t dest)
{
	struct ICR icr = (struct ICR) { 0 };

	icr.vector = LAPIC_INTIPI;
	icr.level  = 1;
	icr.destt  = type;
	icr.dest   = dest;

	ICRSend(&icr);
}
