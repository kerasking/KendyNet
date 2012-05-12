#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"

static buffer_t buffer_create(unsigned long capacity)
{
	unsigned long size = sizeof(struct buffer) + capacity;
	buffer_t b = calloc(size,1);
	if(b)
	{
		b->ref_count = 0;
		b->size = 0;
		b->capacity = capacity;
	}
	return b;
}

static void     buffer_destroy(buffer_t *b)
{
	printf("buffer destroy\n");
	if((*b)->next)
		buffer_release(&(*b)->next);
	free(*b);
	*b = 0;
}

buffer_t buffer_create_and_acquire(buffer_t b,unsigned long capacity)
{
	buffer_t nb = buffer_create(capacity);
	return buffer_acquire(b,nb);
}

buffer_t buffer_acquire(buffer_t b1,buffer_t b2)
{
	if(b1 == b2)
		return b1;	
	if(b2)
		++b2->ref_count;
	if(b1)
		buffer_release(&b1);

	return b2;
}

void buffer_release(buffer_t *b)
{
	if(*b)
	{
		if(--(*b)->ref_count <= 0)
			buffer_destroy(b);
		*b = 0;
	}
}