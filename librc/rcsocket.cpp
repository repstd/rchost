#include "stdafx.h"
#include "rcsocket.h"
#include <iostream>

rcsocket::rcsocket()
{

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
int rcsocket::isSocketOpen()
{
		if (m_socket>=0)
			return true;
		return false;
}
int rcsocket::getPacket(sockaddr& from, void *data, int &size, int maxSize)
{

	int len = sizeof(sockaddr);
	if (m_socket)
	{
#ifdef tcp_conn
		m_sclient = accept(m_netsocket, (sockaddr *)&from, &len);
		if (m_sclient <= 0)
			return false;

		size = recv(m_sclient, (char*)data, maxsize, 0);
#else
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
#endif
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
	if (m_socket)
	{
#ifdef TCP_CONN
		if (m_sClient == INVALID_SOCKET)
			return false;
		result = send(m_sClient, (char*)data, size, 0);

#else
		result = sendto(m_socket, (char*)data, size, 0, (sockaddr*)&to, len);
#endif
		if (result == SOCKET_ERROR)
		{
			//PRINT_ERR("Error Sending Data", WSAGetLastError());
			__STD_PRINT("Error Sending Data :%d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		//PRINT_ERR("Error In Socket", WSAGetLastError());
		__STD_PRINT(" Error In Socket:%d\n", WSAGetLastError());
		return false;

	}
	return true;

}
SOCKET rcsocket::getSocket()
{
	return m_socket;
}