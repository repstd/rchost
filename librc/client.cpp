#include "stdafx.h"
#include "client.h"
#include  <iostream>
#include <assert.h>
client::client() :
rcsocket()
{
}
client::client(int port, const char* serverAddr) :
rcsocket()
{
	setType(SOCK_DGRAM);
	initForPort(port, serverAddr);
}


client::~client()
{

}
client::client(SOCKET socket)
{
	m_socket = socket;
}
int client::setSocketNonBlock(int flag)
{
	/*
	*flag=0: nblock
	*/
	u_long mode = 0;
	if (flag == 1)
	{
		mode = 1;
	}
	ioctlsocket(m_socket, FIONBIO, &mode);
	return 1;
}
int client::initSocket(int port, ULONG S_addr)
{

	setPort(port);
	int type = getType();
	if (type == SOCK_STREAM)
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
	{
		m_socket = socket(AF_INET, SOCK_DGRAM, 0);
		const bool on = true;
		setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
		//u_long mode = 1;
		//ioctlsocket(m_socket, FIONBIO, &mode);
	}
	if (m_socket < 0)
	{
		__STD_PRINT("Error Creating Socket.ErrorCode:%d\n ", GetLastError());
	}
	assert(m_socket > 0);
	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((short)m_port);
	const int on = 1;
	if (type == SOCK_DGRAM)
	{
		if (S_addr == htonl(INADDR_BROADCAST))
			setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(int));
	}
	m_addrSvr.sin_addr.S_un.S_addr = S_addr;
	if (type == SOCK_STREAM)
	{
		if (connect(m_socket, (sockaddr *)&m_addrSvr, sizeof(m_addrSvr)) == SOCKET_ERROR)
		{
			printf("connect error !");
			closesocket(m_socket);
			return 0;
		}
	}
	return true;

}
int client::initForPort(int port, const char* serverAddr)
{

	if (serverAddr == NULL)
		//return initSocket(port, inet_addr("225.225.225.225"));
		return initSocket(port, htonl(INADDR_BROADCAST));
	else
		return initSocket(port, inet_addr(serverAddr));

}

void client::sendPacket(char* data, int size)
{

	rcsocket::sendPacket(*(sockaddr*)&m_addrSvr, data, size, _MAX_DATA_SIZE);
}


