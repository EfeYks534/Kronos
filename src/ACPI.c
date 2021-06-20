#include <Common.h>
#include <Stivale2.h>
#include <ACPI.h>
#include <string.h>

static struct RSDP *rsdp = NULL;

static struct SDTHeader *rsdt = NULL;

static size_t sdt_count = 0;

size_t SDTCount()
{
	return sdt_count;
}

struct SDTHeader *ACPIGetSDT(size_t i)
{
	if(i >= sdt_count)
		return NULL;

	if(rsdp->revision == 2) {
		uint64_t *sdt_list = (uint64_t*) ((uintptr_t) rsdt + sizeof(struct SDTHeader));

		struct SDTHeader *hdr = PhysOffset(sdt_list[i]);

		if(!ACPIChecksum(hdr, hdr->length))
			return NULL;

		return hdr;
	} else {
		uint32_t *sdt_list = (uint32_t*) ((uintptr_t) rsdt + sizeof(struct SDTHeader));

		struct SDTHeader *hdr = PhysOffset(sdt_list[i]);

		if(!ACPIChecksum(hdr, hdr->length))
			return NULL;

		return hdr;
	}
}

static KEINIT void ACPIInit()
{
	struct stivale2_struct_tag_rsdp *rsd = NULL;
	rsd = Stivale2GetTag(STIVALE2_STRUCT_TAG_RSDP_ID);

	if(rsd == NULL)
		Panic(NULL, "Can't find RSDP");

	rsdp = PhysOffset(rsd->rsdp);


	if(rsdp->revision == 2) {
		rsdt = PhysOffset(rsdp->xsdt);
		sdt_count = (rsdt->length - sizeof(struct SDTHeader)) / 8;
	} else {
		rsdt = PhysOffset(rsdp->rsdt);
		sdt_count = (rsdt->length - sizeof(struct SDTHeader)) / 4;
	}	
}

int ACPIChecksum(void *ptr, size_t size)
{
	uint8_t sum = 0;

	for(size_t i = 0; i < size; i++)
		sum += ((uint8_t*) ptr)[i];

	return sum == 0;
}

void *ACPIFindSDT(const char *sign)
{
	if(strlen(sign) != 4)
		Panic(NULL, "ACPI system descriptor signature is not valid");

	for(size_t i = 0; i < sdt_count; i++) {
		struct SDTHeader *hdr = ACPIGetSDT(i);

		if(hdr == NULL) continue;

		if(memcmp(hdr->sign, sign, 4) == 0)
			return hdr;
	}

	return NULL;
}
