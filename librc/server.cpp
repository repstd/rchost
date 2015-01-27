#include "stdafx.h"
#include "server.h"
#include <iostream>

server::server(int port)
{
	setType(SOCK_DGRAM);
	initForPort(port);
}


server::~server()
{
}

int server::initSocket(int port, ULONG S_addr)
{

	setPort(port);
	int type = getType();

	if (type == SOCK_STREAM)
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		m_socket = socket(AF_INET, SOCK_DGRAM, 0);

	const bool on = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));

	if (m_socket <= 0)
	{
		__STD_PRINT("Error in Creating Socket.ErrorCode: %d\n ", GetLastError());
	}

	u_long mode = 0;
	ioctlsocket(m_socket, FIONBIO, &mode);
	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((u_short)m_port);
	m_addrSvr.sin_addr.S_un.S_addr = S_addr;

	int len = sizeof(sockaddr);

	if (type == SOCK_STREAM)
	{
		if (bind(m_socket, (LPSOCKADDR)&m_addrSvr, len) == SOCKET_ERROR)
		{

			__STD_PRINT("Error in Binding Socket", WSAGetLastError());
		}
		if (listen(m_socket, 10) == SOCKET_ERROR)
		{
			__STD_PRINT("listen error !", WSAGetLastError());
			return 0;
		}
	}
	else
	{
		if (bind(m_socket, (SOCKADDR *)&m_addrSvr, len) == SOCKET_ERROR)
		{
			__STD_PRINT("Error in Binding Socket.ErrorCode: %d\n ", GetLastError());
		}
	}
	return true;

}
int server::initForPort(int portNumber)
{

	return initSocket(portNumber, htonl(INADDR_ANY));

}


