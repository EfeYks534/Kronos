#include <Common.h>

typedef void(*kinit_obj)();

extern kinit_obj kdinit_begin;
extern kinit_obj kdinit_end;
extern kinit_obj keinit_begin;
extern kinit_obj keinit_end;
extern kinit_obj klinit_begin;
extern kinit_obj klinit_end;

void KernelDeviceInit()
{
	for(kinit_obj *obj = &kdinit_begin; obj < &kdinit_end; obj++) {
		(*obj)();
	}
}

void KernelEarlyInit()
{
	for(kinit_obj *obj = &keinit_begin; obj < &keinit_end; obj++) {
		(*obj)();
	}
}

void KernelLateInit()
{
	for(kinit_obj *obj = &klinit_begin; obj < &klinit_end; obj++) {
		(*obj)();
	}
}
