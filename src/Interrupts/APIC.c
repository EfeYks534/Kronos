#include <Peripheral.h>
#include <DescTabs.h>
#include <Common.h>
#include <Memory.h>
#include <Device.h>
#include <Task.h>
#include <ACPI.h>
#include <APIC.h>

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

	uint32_t spur = MMRead32(&lapic_addr[LAPIC_SPUR]);

	MMWrite32(&lapic_addr[LAPIC_SPUR], spur | (1ULL << 8) | 0xFF);
}

void APICTimerEnable()
{
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

void ICRSend(struct ICR *icr);

void IPISend(uint8_t type, uint32_t dest);
