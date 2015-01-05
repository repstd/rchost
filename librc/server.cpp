#include "stdafx.h"
#include "server.h"
#include <iostream>

server::server(int port)
{
	initForPort(port);
}


server::~server()
{
}

int server::initSocket(int port, ULONG S_addr)
{

	m_port = port;
#ifdef TCP_CONN
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);

#endif
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

#ifdef TCP_CONN
	if (bind(m_netSocket, (LPSOCKADDR)&addr, len) == SOCKET_ERROR)
	{

		PRINT_ERR("Error in Binding Socket", WSAGetLastError());
	}
	if (listen(m_netSocket, 10) == SOCKET_ERROR)
	{
		PRINT_ERR("listen error !", WSAGetLastError());
		return 0;
	}
#else
	if (bind(m_socket, (SOCKADDR *)&m_addrSvr, len) == SOCKET_ERROR)
	{
		//PRINT_ERR();
		__STD_PRINT("Error in Binding Socket.ErrorCode: %d\n ", GetLastError());
	}
#endif
	return true;

}
bool server::initForPort(int portNumber)
{

	return initSocket(portNumber, htonl(INADDR_ANY));

}


