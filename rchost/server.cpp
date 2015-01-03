#include "stdafx.h"
#include "server.h"
#include <iostream>

server::server(int port):
	m_port(port)
{
	initForPort(port);
}


server::~server()
{
	if (m_netSocket)
		closesocket(m_netSocket);
}

bool server::initForPort(int portNumber)
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
		return false;
	}
#ifdef TCP_CONN
	m_netSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	m_netSocket = socket(AF_INET, SOCK_DGRAM, 0);

#endif
	const bool on = 1;
	setsockopt(m_netSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));

	if (m_netSocket <= 0)
	{
		//PRINT_ERR();
		std::cerr<< "Error in Creating Socket", WSAGetLastError();

	}
	m_port = portNumber;
	SOCKADDR_IN addr;
	u_long mode = 0;
	ioctlsocket(m_netSocket, FIONBIO, &mode);
	addr.sin_family = AF_INET;

	addr.sin_port = htons((u_short)m_port);

	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

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
	if (bind(m_netSocket, (SOCKADDR *)&addr, len) == SOCKET_ERROR)
	{
		//PRINT_ERR();
		std::cerr << "Error in Binding Socket", WSAGetLastError();
	}
#endif
	return true;
}

int server::getPort()
{
	return m_port;
}
bool server::isSocketOpen()
{
	return ([](int socket){ return socket > 0; })(m_netSocket);
}
bool server::getPacket(sockaddr& from, void *data, int &size, int maxSize)
{

	int len = sizeof(sockaddr);
	if (m_netSocket)
	{
#ifdef tcp_conn
		m_sclient = accept(m_netsocket, (sockaddr *)&from, &len);
		if (m_sclient<=0)
			return false;

		size = recv(m_sclient, (char*)data, maxsize, 0);
#else

		size = recvfrom(m_netSocket, (char*)data, maxSize, 0, (sockaddr*)&from, &len);
#endif
		if (size != -1)
		{


		}
		else
		{

			//print_err("error in receiving data", WSAGetLastError());
			std::cerr<<"error in receiving data: "<<WSAGetLastError();
			return false;
		}
	}
	else
	{

		//print_err("error in socket", WSAGetLastError());
		std::cerr<<"error in socket: "<<WSAGetLastError();
		return false;

	}
	return true;

}
bool server::sendPacket(sockaddr to, void *data, int size, int maxSize)
{


	int len = sizeof(to);
	int result = -1;
	if (m_netSocket)
	{
#ifdef TCP_CONN
		if (m_sClient == INVALID_SOCKET)
			return false;
		result = send(m_sClient, (char*)data, size, 0);

#else
		result = sendto(m_netSocket, (char*)data, size, 0, (sockaddr*)&to, len);
#endif
		if (result != SOCKET_ERROR)
		{

		}
		else
		{
			//PRINT_ERR("Error Sending Data", WSAGetLastError());
			std::cerr << "Error Sending Data "<<WSAGetLastError();
			return false;
		}
	}
	else
	{
		//PRINT_ERR("Error In Socket", WSAGetLastError());
		std::cerr<<"Error In Socket "<<WSAGetLastError();
		return false;

	}
	return true;

}
