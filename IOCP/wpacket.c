#include "wpacket.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int is_pow_of_2(unsigned long size)
{
	return !(size&(size-1));
}

static unsigned char GetK(unsigned long size)
{
	unsigned char k = 0;
	if(!is_pow_of_2(size))
	{
		size = (size << 1);
	}
	//除最高为1位以外,其它位全部清0
	while(size > 1)
	{
		k++;
		size = size >> 1;
	}
	return k;
}

wpacket_t wpacket_create(unsigned long size)
{
	unsigned char k = GetK(size);
	wpacket_t w;
	size = 1 << k;
	w = calloc(sizeof(*w),1);
	w->factor = k;
	w->wpos = sizeof(w->len);
	w->buf = buffer_create_and_acquire(0,size);
	w->writebuf = buffer_acquire(0,w->buf);
	w->len = (unsigned long*)w->buf->buf;
	*(w->len) = 0;
	w->buf->size = sizeof(w->len);
	return w;
}

write_pos wpacket_get_writepos(wpacket_t w)
{
	write_pos wp = {w->writebuf,w->wpos};
	return wp;
}

void wpacket_destroy(wpacket_t *w)
{
	buffer_release(&(*w)->buf);
	buffer_release(&(*w)->writebuf);
	free(*w);
	*w = 0;
}

static void wpacket_expand(wpacket_t w)
{
	unsigned long size;
	w->factor <<= 1;
	size = 1 << w->factor;
	w->writebuf->next = buffer_create_and_acquire(0,size);
    w->writebuf = buffer_acquire(w->writebuf,w->writebuf->next); 
	w->wpos = 0;
}

static void wpacket_write(wpacket_t w,char *addr,unsigned long size)
{
	char *ptr = addr;
	unsigned long copy_size;
	while(size)
	{
		copy_size = w->buf->capacity - w->wpos;
		if(copy_size == 0)
		{
			wpacket_expand(w);//空间不足,扩展
			copy_size = w->buf->capacity - w->wpos;
		}
		copy_size = copy_size > size ? size:copy_size;
		memcpy(w->writebuf->buf + w->wpos,ptr,copy_size);
		w->writebuf->size += copy_size;
		(*w->len) += copy_size;
		w->wpos += copy_size;
		ptr += copy_size;
		size -= copy_size;
	}
}


void wpacket_write_char(wpacket_t w,unsigned char value)
{
	wpacket_write(w,(char*)&value,sizeof(value));
}

void wpacket_write_short(wpacket_t w,unsigned short value)
{
	wpacket_write(w,(char*)&value,sizeof(value));
}

void wpacket_write_long(wpacket_t w,unsigned long value)
{
	wpacket_write(w,(char*)&value,sizeof(value));
}

void wpacket_write_double(wpacket_t w ,double value)
{
	wpacket_write(w,(char*)&value,sizeof(value));
}

static void wpacket_rewrite(write_pos *wp,char *addr,unsigned long size)
{
	char *ptr = addr;
	unsigned long copy_size;
	unsigned long pos = wp->wpos;
	while(size)
	{
		copy_size = wp->buf->capacity - pos;
		copy_size = copy_size > size ? size:copy_size;
		memcpy(wp->buf->buf + pos,ptr,copy_size);
		ptr += copy_size;
		size -= copy_size;
		pos += copy_size;
		if(size && pos >= wp->buf->capacity)
		{
			assert(wp->buf->next);
			wp->buf = wp->buf->next;
			pos = 0;
		}

	}
}

void wpacket_rewrite_char(write_pos *wp,unsigned char value)
{
	wpacket_rewrite(wp,&value,sizeof(value));
}

void wpacket_rewrite_short(write_pos *wp,unsigned short value)
{
	wpacket_rewrite(wp,(char*)&value,sizeof(value));
}

void wpacket_rewrite_long(write_pos *wp,unsigned long value)
{
	wpacket_rewrite(wp,(char*)&value,sizeof(value));
}

void wpacket_rewrite_double(write_pos *wp,double value)
{
	wpacket_rewrite(wp,(char*)&value,sizeof(value));
}

void wpacket_write_string(wpacket_t w ,const char *value)
{

}

void wpacket_write_binary(wpacket_t w,const void *value,unsigned long size)
{

}

