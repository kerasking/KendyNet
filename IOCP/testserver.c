#include <stdio.h>
#include <winsock2.h>
#include <WinBase.h>
#include <Winerror.h>
#include "KendyNet.h"
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

DWORD packet_count = 0;
DWORD tick = 0;
DWORD now = 0;
int clientcount = 0;
/*
void RecvFinish(struct Socket *s,struct OverLapContext *overLap,long bytestransfer)
{
	struct connection *c = (struct connection*)s;
	now = GetTickCount();
	if(now - tick > 1000)
	{
		printf("%d,%d\n",packet_count,now - tick);
		tick = now;
		packet_count = 0;
	}
	if(bytestransfer < 0)
	{
		closesocket(c->socket.sock);
		free(c);
	}
	else
	{
		
		DWORD lastErrno = 0;
		if(bytestransfer == overLap->wbuf->len)
		{
			++packet_count;
			c->recv_overlap.wbuf->len = 128;
			c->recv_overlap.wbuf->buf = c->recvbuf;
			WSA_Recv((Socket_t)c,&c->recv_overlap,1);
		}
		else
		{
			overLap->wbuf->len -= bytestransfer;
			overLap->wbuf->buf += bytestransfer;
			WSA_Recv((Socket_t)c,overLap,1);
		}
	}

}
*/
void RecvFinish(struct Socket *s,struct OverLapContext *overLap,long bytestransfer,DWORD err_code)
{
	struct connection *c = (struct connection*)s;
	if(bytestransfer == 0)
	{
		printf("连接断开1:%d\n",c->socket.sock);
		closesocket(c->socket.sock);
		free(c);
		--clientcount;
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
			overLap->wbuf->len -= bytestransfer;
			if(overLap->wbuf->len == 0)
			{
				++packet_count;
				overLap->wbuf->len = 64;
			}
			overLap->wbuf->buf = c->recvbuf;
			bytestransfer = WSA_Recv((Socket_t)c,&c->recv_overlap,1,&err_code);
		}
		if(bytestransfer == 0 || err_code != WSA_IO_PENDING)
		{
			printf("连接断开2:%d\n",c->socket.sock);
			closesocket(c->socket.sock);
			free(c);
			--clientcount;
		}
	}
}

void SendFinish(struct Socket *s,struct OverLapContext *overLap,long bytestransfer,DWORD err_code)
{
	struct connection *c = (struct connection*)s;
	if(bytestransfer < 0)
	{
		closesocket(c->socket.sock);
		free(c);
	}
	else
	{
		if(overLap->wbuf->len == bytestransfer)
		{
			c->recv_overlap.wbuf->len = 4096;
			c->recv_overlap.wbuf->buf = c->recvbuf;
			WSA_Recv((Socket_t)c,&c->recv_overlap,1,&err_code);
		}
		else
		{
			
			overLap->wbuf->len -= bytestransfer;
			overLap->wbuf->buf += bytestransfer;
			WSA_Send(s,&c->send_overlap,1,&err_code);
		}
	}
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
	c->wsendbuf.len = 0;
	c->send_overlap.buf_count = 1;
	c->send_overlap.wbuf = &c->wsendbuf;
	return c;
}

void accept_callback(SOCKET s,void *ud)
{
	DWORD err_code = 0;
	HANDLE *iocp = (HANDLE*)ud;
	struct connection *c = CreateConnection(s);
	++clientcount;
	printf("clientcount:%d\n",clientcount);
	Bind2Engine(*iocp,(Socket_t)c);
	//发出第一个读请求
	WSA_Recv((Socket_t)c,&c->recv_overlap,0,&err_code);
}

DWORD WINAPI Listen(void *arg)
{
	acceptor_t a = create_acceptor("192.168.6.13",8010,&accept_callback,arg);
	while(1)
		acceptor_run(a,100);
	return 0;
}
int main()
{
	DWORD dwThread;
	HANDLE iocp;
	//getchar();
	InitNetSystem();
	iocp = CreateNetEngine(1);


	CreateThread(NULL,0,Listen,&iocp,0,&dwThread);
	tick = GetTickCount();
	while(1)
	{
		RunEngine(iocp,50);
		now = GetTickCount();
		if(now - tick > 1000 && packet_count)
		{
			printf("%d,%d\n",packet_count,now - tick);
			tick = now;
			packet_count = 0;
		}
	}
	return 0;
}