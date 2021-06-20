#include <Common.h>
#include <Memory.h>
#include <ACPI.h>
#include <APIC.h>

struct MADTLAPICOverride
{
	uint8_t   type;
	uint8_t length;
	uint16_t  rsvd;
	uint64_t  addr;
} PACKED;


static uint32_t *lapic_addr = NULL;

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
}

void ICRSend(struct ICR *icr);

void IPISend(uint8_t type, uint32_t dest);
