#include "stdafx.h"
#include "client.h"
#include  <iostream>
#include <assert.h>
udpClient::udpClient():rcsocket()
{
}
udpClient::udpClient(int port, const char* serverAddr):rcsocket()
{
	initForPort(port, serverAddr);
}

udpClient::~udpClient()
{

}
udpClient::udpClient(SOCKET socket) :rcsocket()
{
	m_socket = socket;
}
int udpClient::setSocketNonBlock(int flag)
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
SOCKET udpClient::getSocket()
{
	return rcsocket::getSocket();
}
int udpClient::initSocket(int port, ULONG S_addr)
{

	m_port = port;
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	const bool on = true;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
	u_long mode = 1;
	//ioctlsocket(m_socket, FIONBIO, &mode);
	if (m_socket < 0)
	{
		__STD_PRINT("Error Creating udpClient.ErrorCode:%d\n ", GetLastError());
	}
	assert(m_socket >= 0);
	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((short)m_port);
	if (S_addr == htonl(INADDR_BROADCAST))
		setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(int));
	m_addrSvr.sin_addr.S_un.S_addr = S_addr;
	return true;

}
int udpClient::initForPort(int port, const char* serverAddr)
{
	if (serverAddr == NULL)
		//return initSocket(port, inet_addr("225.225.225.225"));
		return initSocket(port, htonl(INADDR_BROADCAST));
	else
		return initSocket(port, inet_addr(serverAddr));
}

void udpClient::sendPacket(char* data, int size)
{
	sendPacket(*(sockaddr*)&m_addrSvr, data, size, _MAX_DATA_SIZE);
}

int udpClient::getPacket(sockaddr& from, void *data, int &size, int maxSize)
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
			__STD_PRINT("Error in receiving data in udpClient: %d", WSAGetLastError());
			return false;
		}
	}
	else
	{

		__STD_PRINT("Error in udpClient: %d\n", WSAGetLastError());
		return false;

	}
	return true;

}
int udpClient::sendPacket(sockaddr to, void *data, int size, int maxSize)
{
	int len = sizeof(to);
	int result = -1;
	if (m_socket)
	{
		result = sendto(m_socket, (char*)data, size, 0, (sockaddr*)&to, len);
		if (result == SOCKET_ERROR)
		{
			//PRINT_ERR("Error Sending Data", WSAGetLastError());
			__STD_PRINT("Error Sending Data in udpClient :%d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		//PRINT_ERR("Error In Socket", WSAGetLastError());
		__STD_PRINT(" Error In udpClient:%d\n", WSAGetLastError());
		return false;
	}
	return true;

}
// tcpClient

tcpClient::tcpClient():rcsocket()
{
}
tcpClient::tcpClient(int port, const char* serverAddr) : rcsocket()
{
	initForPort(port, serverAddr);
}

tcpClient::~tcpClient()
{

}
tcpClient::tcpClient(SOCKET socket) :rcsocket()
{
	m_socket = socket;
}
int tcpClient::setSocketNonBlock(int flag)
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

SOCKET tcpClient::getSocket()
{
	return rcsocket::getSocket();
}
int tcpClient::initSocket(int port, ULONG S_addr)
{

	m_port = port;
	m_socket =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket < 0) {
		__STD_PRINT("Error Creating tcpClient.ErrorCode:%d\n ", GetLastError());
	}
	assert(m_socket >= 0);
	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((u_short)m_port);
	if (connect(m_socket, (sockaddr *)&m_addrSvr, sizeof(m_addrSvr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(m_socket);
		return 0;
	}
	return true;

}
int tcpClient::initForPort(int port, const char* serverAddr)
{

	if (serverAddr == NULL) {
		__STD_PRINT("Error in tcpClient.TcpServerAddr can't be NULL.Error code: %d\n", WSAGetLastError());
		return false;
	}
		return initSocket(port, inet_addr(serverAddr));

}
void tcpClient::sendPacket(char* data, int size)
{
	sendPacket(*(sockaddr*)&m_addrSvr, data, size, _MAX_DATA_SIZE);
}
int tcpClient::getPacket(sockaddr& from, void *data, int &size, int maxSize)
{

	int len = sizeof(sockaddr);
	if (m_socket)
	{
		m_sClient = accept(m_socket, (sockaddr *)&from, &len);
		if (m_sClient<= 0)
			return false;

		size = recv(m_sClient, (char*)data, maxSize, 0);
	}
	else
	{

		__STD_PRINT("Error in tcpClient: %d\n", WSAGetLastError());
		return false;

	}
	return true;

}
int tcpClient::sendPacket(sockaddr to, void *data, int size, int maxSize)
{


	int len = sizeof(to);
	int result = -1;
	if (m_socket)
	{
		if (m_sClient== INVALID_SOCKET)
			return false;
		result = send(m_sClient, (char*)data, size, 0);
		if (result == SOCKET_ERROR) {
			//PRINT_ERR("Error Sending Data", WSAGetLastError());
			__STD_PRINT("Error Sending Data in tcpClient: %d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		//PRINT_ERR("Error In Socket", WSAGetLastError());
		__STD_PRINT(" Error In tcpClient:%d\n", WSAGetLastError());
		return false;

	}
	return true;

}
