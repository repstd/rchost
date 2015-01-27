#pragma once

class rcsocket
{
public:
	rcsocket();
	~rcsocket();
	int getPort();
	int isSocketOpen();
	int getType();
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
protected:
	virtual int initSocket(int port, ULONG S_addr) = 0;
	void setType(int type);
	void setPort(int port);
	int initContext();
	int cleanContext();
	int m_socketType;
	SOCKET m_socket;
	SOCKET m_clientSocket;
	SOCKADDR_IN	m_addrSvr;
	int m_port;
};

