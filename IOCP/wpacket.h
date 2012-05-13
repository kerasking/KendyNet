/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _WPACKET_H
#define _WPACKET_H
#include "buffer.h"
typedef struct wpacket
{
	unsigned long *len;
	buffer_t buf;
	buffer_t writebuf;
	unsigned long wpos;
	unsigned char factor;
	unsigned long begin_pos;
}*wpacket_t;
struct rpacket;


typedef struct
{
	buffer_t buf;
	unsigned long wpos;
}write_pos;

wpacket_t wpacket_create(unsigned long size);
wpacket_t wpacket_create_by_rpacket(struct rpacket*);//通过rpacket构造
void wpacket_destroy(wpacket_t*);

write_pos wpacket_get_writepos(wpacket_t);

void wpacket_write_char(wpacket_t,unsigned char);
void wpacket_write_short(wpacket_t,unsigned short);
void wpacket_write_long(wpacket_t,unsigned long);
void wpacket_write_double(wpacket_t,double);

void wpacket_rewrite_char(write_pos*,unsigned char);
void wpacket_rewrite_short(write_pos*,unsigned short);
void wpacket_rewrite_long(write_pos*,unsigned long);
void wpacket_rewrite_double(write_pos*,double);

//不提供对非定长数据的rewrite
void wpacket_write_string(wpacket_t,const char*);
void wpacket_write_binary(wpacket_t,const void*,unsigned long);
#endif