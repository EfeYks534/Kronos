#include "stdlib.h"
#include "string.h"

int QueueSubmit(struct Queue *q, void *item)
{
	Lock(&q->lock);

	if(QueueFull(q)) {
		Unlock(&q->lock);
		return 0;
	}

	uint8_t *qitem = (uint8_t*) ((uintptr_t) q->items + q->size * q->tail);

	memcpy(qitem, item, q->size);

	q->tail = (q->tail + 1) % q->nmemb;

	Unlock(&q->lock);

	return 1;
}

int QueueConsume(struct Queue *q, void *item)
{
	Lock(&q->lock);

	if(QueueEmpty(q)) {
		Unlock(&q->lock);
		return 0;
	}

	uint8_t *qitem = (uint8_t*) ((uintptr_t) q->items + q->size * q->head);

	memcpy(item, qitem, q->size);

	q->head = (q->head + 1) % q->nmemb;

	Unlock(&q->lock);

	return 1;
}
