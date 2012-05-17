#include <stdio.h>
#include "common_hash_function.h"
#include "hash_map.h"

#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "KendyNet.h"
#include "Connector.h"
#include "Connection.h"

static int connect_count = 0;
DWORD packet_recv = 0;
DWORD packet_send = 0;
DWORD send_request = 0;
DWORD tick = 0;
DWORD now = 0;
unsigned long bf_count = 0;
#define MAX_CLIENT 350
static struct connection *clients[MAX_CLIENT];
DWORD last_recv = 0;

void init_clients()
{
	int i = 0;
	for(; i < MAX_CLIENT;++i)
		clients[i] = 0;
}

void add_client(struct connection *c)
{
	int i = 0;
	for(; i < MAX_CLIENT; ++i)
	{
		if(clients[i] == 0)
		{
			clients[i] = c;
			break;
		}
	}
}

void remove_client(struct connection *c)
{
	int i = 0;
	for(; i < MAX_CLIENT; ++i)
	{
		if(clients[i] == c)
		{
			clients[i] = 0;
			break;
		}
	}
}

void on_process_packet(struct connection *c,rpacket_t r)
{
	now = GetTickCount();
	++packet_recv;
	if(now - tick > 1000)
	{
		printf("packet_recv:%u,packet_send:%u,send_request:%u,interval:%u,bf_count:%u\n",packet_recv,packet_send,send_request,now - tick,bf_count);
		tick = now;
		packet_recv = 0;
		packet_send = 0;
		send_request = 0;
	}
	rpacket_destroy(&r);
}

void on_connect_callback(SOCKET s,const char *ip,unsigned long port,void *ud)
{
	DWORD err_code = 0;
	HANDLE *iocp = (HANDLE*)ud;
	unsigned long ul = 1;
	BOOL                         optval=1;
	struct connection *c;
	wpacket_t wpk;
	++connect_count;
	if(s == INVALID_SOCKET)
	{
		printf("%d,连接到:%s,%d,失败\n",s,ip,port);
	}
	else
	{
		printf("%d,连接到:%s,%d,成功\n",s,ip,port);
		ioctlsocket(s,FIONBIO,(unsigned long*)&ul);
		//setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char*)&optval,sizeof(optval));         //不采用延时算法 

		c = connection_create(s,on_process_packet,remove_client);
		add_client(c);
		Bind2Engine(*iocp,(Socket_t)c);
		wpk = wpacket_create(64);
		wpacket_write_long(wpk,(unsigned long)s);
		wpacket_write_string(wpk,"hello kenny");
		connection_send(c,wpk,0);
		connection_recv(c);
	}
}

void test1()
{
	wpacket_t w = wpacket_create(12);
	rpacket_t r,r1;
	wpacket_t w1 = 0;
	wpacket_t w2;
	const char *str;
	wpacket_write_string(w,"hello kenny");
	r = rpacket_create_by_wpacket(w);
	wpacket_destroy(&w);
	str = rpacket_read_string(r);
	printf("str=%s\n",str);
	w1 = wpacket_create_by_rpacket(r);
	w2 = wpacket_create_by_rpacket(r);
	r1 = rpacket_create_by_wpacket(w1);
	str = rpacket_read_string(r1);
	printf("str=%s\n",str);
	rpacket_destroy(&r);
	rpacket_destroy(&r1);
	wpacket_destroy(&w1);
	wpacket_destroy(&w2);
}

void test2()
{
	wpacket_t w = wpacket_create(12);
	rpacket_t r;
	write_pos wp;
	wpacket_write_long(w,1);
	wp = wpacket_get_writepos(w);
	wpacket_write_short(w,2);
	wpacket_write_string(w,"hello kenny");
	wpacket_rewrite_short(&wp,4);
    r = rpacket_create_by_wpacket(w);
	printf("%u\n",rpacket_read_long(r));
	printf("%u\n",rpacket_read_short(r));
	printf("%s\n",rpacket_read_string(r));
	rpacket_destroy(&r);
	wpacket_destroy(&w);
}

void testNet()
{
	HANDLE iocp;
	connector_t c = NULL;
	int ret;
	int i = 0;
	wpacket_t wpk;
	getchar();
	InitNetSystem();
	init_clients();
	iocp = CreateNetEngine(1);
	c =  connector_create();
	for( ; i < MAX_CLIENT;++i)
	{
		ret = connector_connect(c,"192.168.6.11",8010,on_connect_callback,&iocp,1000*20);
		Sleep(1);
	}
	while(connect_count < 1)
		connector_run(c,0);
	while(1)
	{
		RunEngine(iocp,50);
		for(i = 0; i < MAX_CLIENT; ++i)
		{
			if(clients[i])
			{
				wpk = wpacket_create(64);
				wpacket_write_long(wpk,clients[i]->socket.sock);
				wpacket_write_string(wpk,"hello kenny");
				connection_send(clients[i],wpk,0);
			}
		}
	}

}

int main()
{	
	//test1();
	//test2();
	testNet();
	getchar();
	return 0;
}
