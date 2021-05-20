#pragma once

#include <Task.h>
#include <stddef.h>
#include <stdint.h>

struct Queue
{
	void   *items;
	size_t  nmemb;
	size_t   size;
	uint64_t head;
	uint64_t tail;
	int64_t  lock;
};

static inline int QueueFull(struct Queue *q)
{
	Lock(&q->lock);
	int r = q->head == (q->tail + 1) % q->nmemb;
	Unlock(&q->lock);

	return r;
}

static inline int QueueEmpty(struct Queue *q)
{
	Lock(&q->lock);
	int r = q->head == q->tail;
	Unlock(&q->lock);

	return r;
}

int QueueSubmit(struct Queue *q, void *item);

int QueueConsume(struct Queue *q, void *item);
