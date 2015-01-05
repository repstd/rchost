#include "stdafx.h"
#include "client.h"
#include  <iostream>
#include <assert.h>
#include "rc_common.h"
#define PRINT_ERR(tag,errCode) \
{\
	char err[256]; \
	sprintf(err, "%s  Error Code: %d\n", tag, errCode, WSAGetLastError()); \
	__LOG(err); \
}

client::client()
{
}


client::~client()
{
	close();
}

int client::initForPort(int port, const char* serverAddr)
{

	m_port = port;
	WORD wVersionRequested;

	WSADATA wsaData;

	int err;
	wVersionRequested = MAKEWORD(2, 2);



	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {


		return false;

	}

	if (LOBYTE(wsaData.wVersion) != 2 ||

		HIBYTE(wsaData.wVersion) != 2) {

		/* Tell the user that we could not find a usable */

		/* WinSock DLL.                                  */

		WSACleanup();

		return false;

	}

#ifdef TCP_CONN
	m_socket =socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	const bool on = true;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
	u_long mode = 1;
	ioctlsocket(m_socket, FIONBIO, &mode);
#endif
	if (m_socket < 0)
	{
		std::cerr << "Error Creating Socket.ErrorCode: " << GetLastError() << std::endl;
	}
	assert(m_socket >= 0);

	m_addrSvr.sin_family = AF_INET;
	m_addrSvr.sin_port = htons((short)m_port);
#ifdef TCP_CONN
	assert(serverAddr!=NULL);
#else
	if (serverAddr != NULL)
		m_addrSvr.sin_addr.S_un.S_addr = inet_addr(serverAddr);
	else
	{
		setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(int));
		m_addrSvr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);

	}
#endif

#ifdef TCP_CONN
	if (connect(m_socket, (sockaddr *)&m_addrSvr, sizeof(m_addrSvr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(m_socket);
		return 0;
	}
#endif

	return true;
}

void client::sendPacket(char* data, int size)
{

	int len = sizeof(sockaddr);
	int result = -1;
	if (m_socket)
	{

#ifdef TCP_CONN

		result = send(m_socket, data, size, 0);

#else
		result = sendto(m_socket, data, size, 0, (sockaddr*)&m_addrSvr, len);
		if (result <= 0)
		{
			std::cerr << "Error in sending data" << std::endl;
		}
#endif
	}
}


bool client::getPacket(sockaddr& from, void *data, int &size, int maxSize)
{
	int len = sizeof(sockaddr);
	if (m_socket)
	{
#ifdef TCP_CONN
		size = recv(m_socket, (char*)data, maxSize, 0);
#else
		size = recvfrom(m_socket, (char*)data, maxSize, 0, (sockaddr*)&from, &len);

#endif
		if (size==-1)
		{
			//PRINT_ERR("Error in Receiving Data", WSAGetLastError());
			std::cerr << "Error in Receiving Data " << WSAGetLastError();
		}
	}
	else
	{

		//PRINT_ERR("Error in Socket", WSAGetLastError());
		std::cerr << "Error in Socket" << WSAGetLastError();
	}
	return true;
}
void client::close()
{

	closesocket(m_socket);
	WSACleanup();
}
