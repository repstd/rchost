#include "stdafx.h"
#include "rcsocket.h"
#include <iostream>

rcsocket::rcsocket()
{
	//By default,we use UDP.
	setType(SOCK_DGRAM);
	initContext();
}


rcsocket::~rcsocket()
{
	cleanContext();
}
int rcsocket::initContext()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0)
		return false;
	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return 0;
	}
	return 1;
}
int rcsocket::cleanContext()
{

	if (m_socket)
		closesocket(m_socket);
	WSACleanup();
	return true;
}

int rcsocket::getPort()
{
	return m_port;
}
void rcsocket::setPort(int port)
{
	m_port = port;
}
void rcsocket::setType(int type)
{
	m_socketType = type;
}
int rcsocket::getType()
{
	return m_socketType;
}
int rcsocket::isSocketOpen()
{
	if (m_socket)
		return true;
	return false;
}
int rcsocket::getPacket(sockaddr& from, void *data, int &size, int maxSize)
{

	int len = sizeof(sockaddr);
	int type = getType();
	if (isSocketOpen())
	{
		if (type == SOCK_STREAM)
		{
			m_clientSocket = accept(m_socket, (sockaddr *)&from, &len);
			if (m_clientSocket <= 0)
				return false;
			size = recv(m_clientSocket, (char*)data, maxSize, 0);
		}
		else
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
				__STD_PRINT("Error in receiving data:%d", WSAGetLastError());
				return false;
			}
		}
	}
	else
	{

		__STD_PRINT("Error in socket: %d\n", WSAGetLastError());
		return false;

	}
	return true;

}
int rcsocket::sendPacket(sockaddr to, void *data, int size, int maxSize)
{

	int len = sizeof(to);
	int result = -1;
	int type = getType();
	if (isSocketOpen())
	{
		if (type == SOCK_STREAM)
		{
			if (m_clientSocket == INVALID_SOCKET)
				return false;
			result = send(m_clientSocket, (char*)data, size, 0);
		}
		else
			result = sendto(m_socket, (char*)data, size, 0, (sockaddr*)&to, len);
		if (result == SOCKET_ERROR)
		{
			__STD_PRINT("Error Sending Data :%d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		__STD_PRINT(" Error In Socket:%d\n", WSAGetLastError());
		return false;
	}
	return true;

}
