#include <Common.h>

typedef void(*kinit_obj)();

extern kinit_obj keinit_begin;
extern kinit_obj keinit_end;
extern kinit_obj klinit_begin;
extern kinit_obj klinit_end;


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
