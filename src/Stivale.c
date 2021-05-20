#include <Common.h>
#include <Stivale2.h>

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
		if(cur->identifier == id) return cur;
		else cur = (struct stivale2_tag*) cur->next;
	}

	return NULL;
}
