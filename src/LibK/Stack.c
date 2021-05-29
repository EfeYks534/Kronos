#include "stdlib.h"
#include "string.h"

int StackPush(struct Stack *s, void *item)
{
	Lock(&s->lock);

	if(s->count == s->nmemb) {
		Unlock(&s->lock);
		return 0;
	}

	void *target = (void*) ((uintptr_t) s->items + s->count * s->size);

	memcpy(target, item, s->size);

	s->count++;

	Unlock(&s->lock);

	return 1;
}

int StackPop(struct Stack *s, void *item)
{
	Lock(&s->lock);

	if(s->count == 0) {
		Unlock(&s->lock);
		return 0;
	}

	s->count--;

	void *target = (void*) ((uintptr_t) s->items + s->count * s->size);

	memcpy(item, target, s->size);

	Unlock(&s->lock);

	return 1;
}
