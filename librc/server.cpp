#include "stdafx.h"
#include "server.h"
#include <iostream>

udpServer::udpServer(int port):rcsocket()
{
	initForPort(port);
}

udpServer::~udpServer() { }

int udpServer::initSocket(int port, ULONG S_addr)
{

	m_port = port;
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);

	const bool on = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));

	if (m_socket <= 0)
	{
		__STD_PRINT("Error in Creating udpServer.ErrorCode: %d\n ", GetLastError());
	}

	u_long mode = 0;
	ioctlsocket(m_socket, FIONBIO, &mode);
	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((u_short)m_port);
	m_addrSvr.sin_addr.S_un.S_addr = S_addr;

	int len = sizeof(sockaddr);

	if (bind(m_socket, (SOCKADDR *)&m_addrSvr, len) == SOCKET_ERROR) {
		//PRINT_ERR();
		__STD_PRINT("Error in Binding udpServer.ErrorCode: %d\n ", GetLastError());
	}
	return true;

}
int udpServer::initForPort(int portNumber) {
	return initSocket(portNumber, htonl(INADDR_ANY));
}

int udpServer::getPacket(sockaddr& from, void *data, int &size, int maxSize)
{

	int len = sizeof(sockaddr);
	if (m_socket)
	{
		size = recvfrom(m_socket, (char*)data, maxSize, 0, (sockaddr*)&from, &len);
#if 0
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_socket, &fdset);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		while (select(static_cast<int>(m_socket)+1, &fdset, 0L, 0L, &tv))
		{
			if (FD_ISSET(m_socket, &fdset))
			{
				size = recvfrom(m_socket, (char *)data, maxSize, 0, (sockaddr*)&from, &len);
			}
		}
#endif
		if (size == -1)
		{
			//__STD_PRINT("Error in receiving data in udpServer:%d", WSAGetLastError());
			return false;
		}
	}
	else
	{

		__STD_PRINT("Error in socket: %d\n", WSAGetLastError());
		return false;

	}
	return true;

}
int udpServer::sendPacket(sockaddr to, void *data, int size, int maxSize)
{
	int len = sizeof(to);
	int result = -1;
	if (m_socket) {
		result = sendto(m_socket, (char*)data, size, 0, (sockaddr*)&to, len);
		if (result == SOCKET_ERROR)
		{
			__STD_PRINT("Error Sending Data :%d in udpServer\n", WSAGetLastError());
			return false;
		}
	}
	else {
		__STD_PRINT(" Error In Socket:%d\n", WSAGetLastError());
		return false;

	}
	return true;

}

tcpServer::tcpServer(int port):rcsocket()
{
	initForPort(port);
}

tcpServer::~tcpServer()
{
}

int tcpServer::initSocket(int port, ULONG S_addr)
{

	m_port = port;
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	const bool on = 1;
	//setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
	if (m_socket <= 0) {
		__STD_PRINT("Error in Creating tcpServer.ErrorCode: %d\n ", GetLastError());
	}

	u_long mode = 0;
	//ioctlsocket(m_socket, FIONBIO, &mode);
	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((u_short)m_port);
	m_addrSvr.sin_addr.S_un.S_addr = S_addr;

	int len = sizeof(sockaddr);
	if (bind(m_socket, (LPSOCKADDR)&m_addrSvr, len) == SOCKET_ERROR) {
		__STD_PRINT("Error in Binding tcpServer", WSAGetLastError());
	}
	if (listen(m_socket, 10) == SOCKET_ERROR) {
		__STD_PRINT("listen error in tcpServer!", WSAGetLastError());
		return 0;
	}
	return true;
}
int tcpServer::initForPort(int portNumber) {
	return initSocket(portNumber, htonl(INADDR_ANY));
}
int tcpServer::getPacket(sockaddr& from, void *data, int &size, int maxSize) {
	int len = sizeof(sockaddr);
	if (m_socket) {
		m_sClient = accept(m_socket, (sockaddr *)&from, &len);
		if (m_sClient< 0)
			return false;
		size = recv(m_sClient, (char*)data, maxSize, 0);
	}
	else {
		__STD_PRINT("Error in tcpServer: %d\n", WSAGetLastError());
		return false;
	}
	return true;

}
int tcpServer::sendPacket(sockaddr to, void *data, int size, int maxSize)
{


	int len = sizeof(to);
	int result = -1;
	if (m_socket)
	{
		if (m_sClient== INVALID_SOCKET)
			return false;
		result = send(m_sClient, (char*)data, size, 0);
		if (result == SOCKET_ERROR)
		{
			__STD_PRINT("Error Sending Data in tcpServer: %d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		__STD_PRINT(" Error In tcpServer: %d\n", WSAGetLastError());
		return false;

	}
	return true;

}
