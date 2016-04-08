#pragma once
//An abstract for client/servers.
class rcsocket
{
public:
	rcsocket();
	virtual ~rcsocket();
	int getPort();
	int isSocketOpen();
protected:
	virtual int getPacket(sockaddr& from, void *data, int &size, int maxSize);
	virtual int sendPacket(sockaddr to, void *data, int size, int maxSize);
	virtual int initSocket(int port, ULONG S_addr) = 0;
	int initContext();
	int cleanContext();
	SOCKET getSocket();
	SOCKET m_socket;
	SOCKADDR_IN	m_addrSvr;
	int m_port;
};
