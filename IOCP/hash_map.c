#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash_map.h"

enum
{
	ITEM_EMPTY = 0,
	ITEM_DELETE,
	ITEM_USED,
};

struct hash_item
{
	void *key;
	void *val;
	unsigned long hash_code;
	char flag;
};

struct hash_map
{
	hash_func   _hash_function;
	hash_key_eq _key_cmp_function;
	unsigned long slot_size;
	unsigned long size;
	struct hash_item  **_items;
};

hash_map_t hash_map_create(unsigned long slot_size,hash_func hash_function,hash_key_eq key_cmp_function)
{
	unsigned long capacity = sizeof(struct hash_item)*slot_size;
	hash_map_t h = (hash_map_t)malloc(sizeof(*h));
	if(!h)
		return 0;
	h->slot_size = slot_size;
	h->size = 0;
	h->_hash_function = hash_function;
	h->_key_cmp_function = key_cmp_function;
	h->_items = calloc(capacity,1);
	if(!h->_items)
	{
		free(h);
		return 0;
	}
	return h;
}

void hash_map_destroy(hash_map_t *h)
{
	free((*h)->_items);
	free(*h);
	*h = 0;
}


unsigned long _hash_map_index(unsigned long hash_code,int p)
{
	unsigned long index = hash_code * 2654435769;
	return index >> (32 - p);
}

static int _hash_map_insert(hash_map_t h,void *key,void *val,unsigned long hash_code)
{
	return 0;
}

static int _hash_map_expand(hash_map_t h)
{
	unsigned long old_slot_size = h->slot_size;
	struct hash_item **old_items = h->_items;
	unsigned long capacity;
	unsigned long i = 0;
	h->slot_size <<= 1;
	capacity = sizeof(struct hash_item)*h->slot_size;
	h->_items = calloc(capacity,1);
	if(!h->_items)
	{
		h->_items = old_items;
		h->slot_size >>= 1;
		return -1;
	}
	for(; i < old_slot_size; ++i)
	{
		struct hash_item *_item = old_items[i];
		if(_item->flag == ITEM_USED)
			_hash_map_insert(h,_item->key,_item->val,_item->hash_code);
	}
	free(old_items);
	return 0;
}

int hash_map_insert(hash_map_t h,void *key,void *val)
{
	unsigned long hash_code = h->_hash_function(key);
	if(h->slot_size < 0x80000000 && h->size >= h->slot_size - h->slot_size/4)
		//空间使用超过3/4扩展
		_hash_map_expand(h);
	if(h->size >= h->slot_size)
		return -1;
	return _hash_map_insert(h,key,val,hash_code);
}




