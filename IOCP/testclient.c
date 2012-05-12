#include <stdio.h>
#include "common_hash_function.h"
#include "hash_map.h"

#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "KendyNet.h"
#include "Connector.h"
//HANDLE iocp;
struct connection
{
	struct Socket socket;
	WSABUF wsendbuf;
	WSABUF wrecvbuf;
	char   sendbuf[4096];
	char   recvbuf[4096];
	struct OverLapContext send_overlap;
	struct OverLapContext recv_overlap;
};


void SendFinish(struct Socket *s,struct OverLapContext *overLap,long bytestransfer,DWORD err_code)
{
	struct connection *c = (struct connection*)s;
	if(bytestransfer == 0)
	{
		printf("连接断开1:%d\n",c->socket.sock);
		closesocket(c->socket.sock);
		free(c);
	}
	else if(bytestransfer < 0)
	{
		//根据err_code判断
	}
	else
	{
		while(bytestransfer > 0)
		{
			err_code = 0;
			overLap->wbuf->len = 64;
			overLap->wbuf->buf = c->recvbuf;
			bytestransfer = WSA_Send((Socket_t)c,&c->recv_overlap,1,&err_code);
		}
		if(bytestransfer == 0 || err_code != WSA_IO_PENDING)
		{
			printf("连接断开2:%d\n",c->socket.sock);
			closesocket(c->socket.sock);
			free(c);
		}
	}
}

void RecvFinish(struct Socket *s,struct OverLapContext *overLap,long bytestransfer,DWORD err_code)
{

}

struct connection* CreateConnection(SOCKET s)
{
	struct connection *c;
	c = malloc(sizeof(*c));
	ZeroMemory(c, sizeof(*c));
	c->socket.sock = s;
	c->socket.RecvFinish = &RecvFinish;
	c->socket.SendFinish = &SendFinish;
	c->wrecvbuf.buf = c->recvbuf;
	c->wrecvbuf.len = 64;
	c->recv_overlap.buf_count = 1;
	c->recv_overlap.wbuf = &c->wrecvbuf;

	c->wsendbuf.buf = c->sendbuf;
	c->wsendbuf.len = 64;
	c->send_overlap.buf_count = 1;
	c->send_overlap.wbuf = &c->wsendbuf;
	return c;
}

static int connect_count = 0;

void on_connect_callback(SOCKET s,const char *ip,unsigned long port,void *ud)
{
	DWORD err_code = 0;
	HANDLE *iocp = (HANDLE*)ud;
	unsigned long ul = 1;
	struct connection *c;
	++connect_count;
	if(s == INVALID_SOCKET)
	{
		printf("%d,连接到:%s,%d,失败\n",s,ip,port);
	}
	else
	{
		printf("%d,连接到:%s,%d,成功\n",s,ip,port);
		ioctlsocket(s,FIONBIO,(unsigned long*)&ul);
		c = CreateConnection(s);
		Bind2Engine(*iocp,(Socket_t)c);
		WSA_Send((Socket_t)c,&c->send_overlap,0,&err_code);
	}
}

#include "buffer.h"
#include "rpacket.h"
#include "wpacket.h"

struct test_packet
{
	unsigned long  len;
	unsigned long  strlen;
	char     msg[20];
	unsigned long  val;
};


int main()
{	
	wpacket_t w = wpacket_create(12);
	rpacket_t r;
	unsigned long p;
	write_pos wpos;
	wpacket_write_long(w,1);
	wpacket_write_short(w,2);
	wpacket_write_long(w,3);
	wpos = wpacket_get_writepos(w);
	wpacket_write_long(w,4);
	wpacket_rewrite_long(&wpos,5);
	r = rpacket_create_by_wpacket(w);
	wpacket_destroy(&w);
	p = rpacket_read_long(r);
	p = rpacket_read_short(r);
	p = rpacket_read_long(r);
	p = rpacket_read_long(r);
	rpacket_destroy(&r);

/*
	struct test_packet tp;
	rpacket_t rpk;
	buffer_t b1;
	char *ptr;
	unsigned long p1;
	tp.len = 28;
	tp.strlen = 20;
	tp.val = 100;
	strcpy(tp.msg,"1234567890123456789");
	ptr = (char*)&tp;
	printf("%d\n",sizeof(tp));

	b1 = buffer_create_and_acquire(0,16);
	b1->next = buffer_create_and_acquire(0,16);
	memcpy(b1->buf,ptr,16);
	b1->size = 16;
	memcpy(b1->next->buf,ptr+16,16);
	b1->next->size = 16;

	rpk = rpacket_create(b1,0);
	ptr = rpacket_read_string(rpk);
	p1 = rpacket_read_long(rpk);

	rpacket_destroy(&rpk);
	buffer_release(&b1);
	*/
	getchar();
	/*
	buffer_t cur = 0;
	buffer_t b1 = buffer_create_and_acquire(0,16);
	buffer_t b2 = buffer_acquire(0,b1);
	buffer_release(&b1);
	buffer_release(&b2);

	b1 = buffer_create_and_acquire(0,16); //b1指向bufferA
	b1 = buffer_create_and_acquire(b1,16);//b1指向bufferB,bufferA计数减为0,被释放
	buffer_release(&b1);

	//创建3个buffer形成链表
	b1 = buffer_create_and_acquire(0,16);
	b1->next = buffer_create_and_acquire(0,16);
	b1->next->next = buffer_create_and_acquire(0,16);

	cur =b1;
	while(cur)
	{
		cur = buffer_acquire(cur,cur->next);
	}
	//遍历结束,所有buffer的计数减为0,全部被释放
	*/
/*	HANDLE iocp;
	connector_t c = NULL;
	int ret;
	int i = 0;
	InitNetSystem();
	iocp = CreateNetEngine(1);
	c =  connector_create();
	for( ; i < 20;++i)
	{
		ret = connector_connect(c,"192.168.6.13",8010,on_connect_callback,&iocp,1000*20);
		Sleep(1);
	}
	while(connect_count < 20)
		connector_run(c,0);
	while(1)
	{
		RunEngine(iocp,50);
	}
*/
	return 0;
}
/*
#include "common_hash_function.h"
int main()
{
	unsigned long ret;// = hash_integer(123455);
	//ret = (ret * 2654435769) >> (32-4);

	int i = 0;
	for(; i < 32;++i)
	{
		ret = hash_integer(i+1);
		printf("%d\n",ret = (ret * 2654435769) >> (32-5));
	}
	
	getchar();

	/*int tmp[32];
	int i = 0;
	int j = 0;//0xffffffff;
	//printf("%x\n",0xffffffff);
	tmp[0] = 0xffffffff;
	for( ; i < 31; ++i)
	{
		j = (1<<i)+j;
		tmp[i+1] = j ^ 0xffffffff;
		//printf("%x\n",j ^ 0xffffffff);
	}
	i = 32;
	for( ; i >= 0; --i)
		printf("%x\n",tmp[i]);
		*
	return 0;
}
*/