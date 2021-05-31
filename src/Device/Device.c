#include <Common.h>
#include <Device.h>
#include <string.h>

static const char *category_names[6] =
{
	"(null)",
	"Graphics",
	"Storage",
	"Audio",
	"Data",
	"Terminals",
};

static const char *type_names[][6] =
{
	{"."},
	{"."}, // Graphics
	{"."}, // Storage
	{"."}, // Audio
	{"Serial"}, // Data IO
	{"Graphics Terminal"}, // Terminal
};

struct DeviceCategory
{
	size_t        device_count;
	uint64_t            bitmap;
	uint64_t           primary;
	struct Device *devices[64];
};

struct DeviceCategory dev_list[64] = { 0 };

static uint64_t DevAlloc(uint64_t category)
{
	struct DeviceCategory *devs = &dev_list[category];

	if(devs->device_count == 0)
		devs->bitmap = -1ULL;

	if(devs->bitmap == 0)
		Panic(NULL, "Couldn't allocate new device");

	uint64_t idx = __builtin_ffsll(devs->bitmap) - 1;

	devs->bitmap &= ~(1ULL << idx);

	return idx;
}

void DeviceRegister(uint64_t category, struct Device *dev)
{
	dev_list[category].devices[DevAlloc(category)] = dev;

	const char *type = type_names[category][dev->type];
	const char *cat  = category_names[category];

	Info("Device '%s' registered as %s in %s\n", dev->name, type, cat);
}

void DeviceUnregister(uint64_t category, struct Device *dev)
{
	struct DeviceCategory *cat = &dev_list[category];
	struct Device **devs       = cat->devices;

	for(size_t i = 0; i < 64; i++) {
		if(devs[i] == NULL) continue;

		if(devs[i] == dev) {
			devs[i] = NULL;

			if(cat->primary == i)
				cat->primary = -1ULL;

			cat->bitmap |= 1ULL << i;
			return;
		}
	}
}


void *DevicePrimary(uint64_t category)
{
	struct DeviceCategory *devs = &dev_list[category];

	if(devs->primary == -1ULL)
		return NULL;
	else
		return devs->devices[devs->primary];
}

void DevicePrimarySet(uint64_t category, struct Device *dev)
{
	struct DeviceCategory *cat = &dev_list[category];

	for(size_t i = 0; i < 64; i++) {
		if(cat->devices[i] == NULL) continue;

		if(cat->devices[i] == dev) {
			cat->primary = i;
			return;
		}
	}
}


void *DeviceGet(uint64_t category, uint64_t type, const char *name)
{
	struct DeviceCategory *cat = &dev_list[category];

	for(size_t i = 0; i < 64; i++) {
		if(cat->devices[i] == NULL) continue;

		if(cat->devices[i]->type == type) {
			struct Device *dev = cat->devices[i];

			if(strncmp(dev->name, name, 64) == 0)
				return dev;
		}
	}

	return NULL;
}

size_t DeviceCollect(uint64_t category, struct Device **list)
{
	size_t nmemb = 0;

	struct Device **devs = dev_list[category].devices;

	for(size_t i = 0; i < 64; i++) {
		if(devs[i] != NULL)
			list[nmemb++] = devs[i];
	}

	return nmemb;
}
