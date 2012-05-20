#include "wpacket.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "rpacket.h"

static struct link_list *g_wpacket_pool;

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


void init_wpacket_pool(unsigned long pool_size)
{
	unsigned long i = 0;
	wpacket_t w;// = calloc(1,sizeof(*w));
	g_wpacket_pool = LIST_CREATE();
	for( ; i < pool_size; ++i)
	{
		w = calloc(1,sizeof(*w));
		LIST_PUSH_BACK(g_wpacket_pool,w);
	}
}

unsigned long wpacket_pool_size()
{
	return list_size(g_wpacket_pool);
}

//static wpacket_t wpacket_get()
//{
//	wpacket_t w = LIST_POP(wpacket_t,g_wpacket_pool);
//	return w;
//}

//static void wpacket_put(wpacket_t w)
//{
//	LIST_PUSH_BACK(g_wpacket_pool,w);
//}

wpacket_t wpacket_create(unsigned long size)
{
	unsigned char k = GetK(size);
	wpacket_t w;
	size = 1 << k;
	w = LIST_POP(wpacket_t,g_wpacket_pool);//calloc(1,sizeof(*w));
	if(!w)
	{
		printf("缓存不够了\n");
		getchar();
		exit(0);
	}
	
	//w = calloc(1,sizeof(*w));
	w->factor = k;
	w->wpos = sizeof(w->len);
	w->buf = buffer_create_and_acquire(0,size);
	w->writebuf = buffer_acquire(0,w->buf);
	w->len = (unsigned long*)w->buf->buf;
	*(w->len) = 0;
	w->buf->size = sizeof(w->len);
	w->begin_pos = 0;
	w->data_size = sizeof(*(w->len));
	return w;
}

wpacket_t wpacket_create_by_rpacket(struct rpacket *r)
{
	wpacket_t w = LIST_POP(wpacket_t,g_wpacket_pool);//calloc(1,sizeof(*w));
	if(!w)
	{
		printf("缓存不够了\n");
		getchar();
		exit(0);
	}
	
	//wpacket_t w = calloc(1,sizeof(*w));
	w->factor = 0;
	w->writebuf = 0;
	w->begin_pos = r->begin_pos;
	w->buf = buffer_acquire(0,r->buf);
	w->len = 0;//触发拷贝之前len没有作用
	w->wpos = 0;
	w->data_size = r->len + sizeof(r->len);
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
	LIST_PUSH_BACK(g_wpacket_pool,*w);
	//free(*w);
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


static void wpacket_copy(wpacket_t w,buffer_t buf)
{
	char *ptr = buf->buf;
	buffer_t tmp_buf = w->buf;
	unsigned long copy_size;
	while(tmp_buf)
	{
		copy_size = tmp_buf->size - w->wpos;
		memcpy(ptr,tmp_buf->buf,copy_size);
		ptr += copy_size;
		w->wpos = 0;
		tmp_buf = tmp_buf->next;
	}
}

static void wpacket_write(wpacket_t w,char *addr,unsigned long size)
{
	char *ptr = addr;
	unsigned long copy_size;
	buffer_t tmp;
	unsigned char k;
	if(!w->writebuf)
	{
		/*wpacket是由rpacket构造的，这里执行写时拷贝，
		* 执行完后wpacket和构造时传入的rpacket不再共享buffer
		*/
		k = GetK(*w->len);
		w->factor = k;
		tmp = buffer_create_and_acquire(0,1 << k);
		wpacket_copy(w,tmp);
		w->begin_pos = 0;
		w->len = (unsigned long*)tmp->buf;
		w->wpos = sizeof(*w->len);
		w->buf = buffer_acquire(w->buf,tmp);
		w->writebuf = buffer_acquire(w->writebuf,w->buf);
	}
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
		w->data_size += copy_size;
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
	wpacket_write_binary(w,value,strlen(value)+1);
}

void wpacket_write_binary(wpacket_t w,const void *value,unsigned long size)
{
	assert(value);
	wpacket_write_long(w,size);
	wpacket_write(w,(char*)value,size);
}

