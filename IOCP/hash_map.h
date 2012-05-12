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
#ifndef _HASH_MAP_H
#define _HASH_MAP_H

typedef struct hash_map* hash_map_t;

typedef unsigned long (*hash_func)(void*);
typedef unsigned long (*hash_key_eq)(void*,void*);

hash_map_t hash_map_create(unsigned long slot_size,hash_func,hash_key_eq);
void       hash_map_destroy(hash_map_t*);
int        hash_map_insert(hash_map_t,void *key,void *val);
int        hash_map_remove(hash_map_t,void* key);
void      *hash_map_find(hash_map_t,void* key); 

#endif