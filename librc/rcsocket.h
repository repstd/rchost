#pragma once

class rcsocket
{
public:
	rcsocket();
	~rcsocket();
	int getPort();
	int isSocketOpen();
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
protected:
	virtual int initSocket(int port, ULONG S_addr) = 0;
	int initContext();
	int cleanContext();

	SOCKET m_socket;
	SOCKADDR_IN	m_addrSvr;
	int m_port;
};

