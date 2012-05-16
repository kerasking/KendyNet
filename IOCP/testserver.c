#include <stdio.h>
#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "KendyNet.h"
#include "Connection.h"


DWORD packet_recv = 0;
DWORD packet_send = 0;
DWORD send_request = 0;
DWORD tick = 0;
DWORD now = 0;
unsigned long bf_count = 0;
int clientcount = 0;

#define MAX_CLIENT 250
static struct connection *clients[MAX_CLIENT];

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
			clients[i] = c;
	}
}

void send2_all_client(rpacket_t r)
{
	int i = 0;
	wpacket_t w;
	//const char *str;
	//unsigned long s;
	//s = rpacket_read_long(r);
	//str = rpacket_read_string(r);
	for(; i < MAX_CLIENT; ++i)
	{
		if(clients[i])
		{
			++send_request;
			w = wpacket_create_by_rpacket(r);
			//w = wpacket_create(64);
			//wpacket_write_long(w,s);
			//wpacket_write_string(w,str);
			connection_send(clients[i],w,1);
		}
	}
}

void on_process_packet(struct connection *c,rpacket_t r)
{
	send2_all_client(r);
	//wpacket_t w = wpacket_create_by_rpacket(r);
	//connection_send(c,w,1);
	rpacket_destroy(&r);
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
	
}

void accept_callback(SOCKET s,void *ud)
{
	DWORD err_code = 0;
	HANDLE *iocp = (HANDLE*)ud;
	struct connection *c = connection_create(s,on_process_packet);
	add_client(c);
	//++clientcount;
	printf("cli fd:%d\n",s);
	Bind2Engine(*iocp,(Socket_t)c);
	//发出第一个读请求
	connection_recv(c);
}

DWORD WINAPI Listen(void *arg)
{
	acceptor_t a = create_acceptor("192.168.6.11",8010,&accept_callback,arg);
	while(1)
		acceptor_run(a,100);
	return 0;
}
int main()
{
	DWORD dwThread;
	HANDLE iocp;
	//getchar();
	init_clients();
	InitNetSystem();
	iocp = CreateNetEngine(1);


	CreateThread(NULL,0,Listen,&iocp,0,&dwThread);
	tick = GetTickCount();
	while(1)
	{
		RunEngine(iocp,50);
		/*now = GetTickCount();
		if(now - tick > 1000 && packet_count)
		{
			printf("%d,%d\n",packet_count,now - tick);
			tick = now;
			packet_count = 0;
		}
		*/
		
	}
	return 0;
}