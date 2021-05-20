#include <Common.h>
#include <Stivale2.h>

void *Stivale2GetTag(struct stivale2_struct *stivale2, uint64_t id)
{
	struct stivale2_tag *cur = (void*) stivale2->tags;

	while(cur != NULL) {
		if(cur->identifier == id) return cur;
		else cur = (struct stivale2_tag*) cur->next;
	}

	return NULL;
}
