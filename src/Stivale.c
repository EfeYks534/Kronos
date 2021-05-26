#include <Common.h>
#include <Stivale2.h>
#include <string.h>

static struct stivale2_struct *stivale2 = NULL;

void Stivale2SetStruct(struct stivale2_struct *s)
{
	stivale2 = s;
}

void *Stivale2GetTag(uint64_t id)
{
	if(stivale2 == NULL) return NULL;

	struct stivale2_tag *cur = (void*) stivale2->tags;

	while(cur != NULL) {
		if(cur->identifier == id) return PhysOffset(cur);
		else cur = (struct stivale2_tag*) cur->next;
	}

	return NULL;
}

struct stivale2_module *Stivale2GetModule(const char *name)
{
	struct stivale2_struct_tag_modules *modules;
	modules = Stivale2GetTag(STIVALE2_STRUCT_TAG_MODULES_ID);

	if(modules == NULL) return NULL;

	size_t len = strlen(name);

	if(len > STIVALE2_MODULE_STRING_SIZE - 1) return NULL;

	for(uint64_t i = 0; i < modules->module_count; i++) {
		struct stivale2_module *mod = &modules->modules[i];

		if(strncmp(name, mod->string, STIVALE2_MODULE_STRING_SIZE) == 0) {
			mod->begin = PhysOffseti(mod->begin);
			mod->end   = PhysOffseti(mod->begin);

			return mod;
		}
	}

	return NULL;
}
