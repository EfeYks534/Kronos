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


struct Stack
{
	void  *items;
	size_t nmemb;
	size_t  size;
	size_t count;
	int64_t lock;
};

static inline int StackFull(struct Stack *s)
{
	Lock(&s->lock);
	int r = s->count == s->nmemb;
	Unlock(&s->lock);

	return r;
}

static inline int StackEmpty(struct Stack *s)
{
	Lock(&s->lock);
	int r = s->count == 0;
	Unlock(&s->lock);

	return r;
}

int StackPush(struct Stack *s, void *item);

int StackPop(struct Stack *s, void *item);


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


void *malloc(size_t n);

void *calloc(size_t nmemb, size_t size);

void free(void *ptr);

size_t MAllocTotal();
