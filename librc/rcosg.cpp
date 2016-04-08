#include "stdafx.h"
#include "rcosg.h"
#include "rcfactory.h"


rcosg::rcosg()
{
}


rcosg::~rcosg()
{
}
rcOsgHostClient::rcOsgHostClient(char* memShareName, int port, const char* addr) :THREAD()
{
	m_memshareClient = std::unique_ptr<rcFileMapReader>(rcfactory::instance()->createFileMapReader(memShareName));
	m_cli = std::unique_ptr<udpClient>(rcfactory::instance()->createUdpClient(port, addr));
}
void rcOsgHostClient::sync()
{
	char data[_MAX_OSG_DATA_SIZE];
	memset(data, 0, _MAX_OSG_DATA_SIZE);
	m_memshareClient->read(data, sizeof(SYNC_OSG_MSG));
	m_cli->sendPacket(data, sizeof(SYNC_OSG_MSG));
	
}
void rcOsgHostClient::run()
{
	while (m_cli->isSocketOpen()) {
		sync();
	}
}
rcOsgHostServer::rcOsgHostServer(char* memShareName, int port) :THREAD()
{
	m_memshareServer = std::unique_ptr<rcFileMapWriter>(rcfactory::instance()->createFileMapWriter(memShareName));
	m_svr = std::unique_ptr<udpServer>(rcfactory::instance()->createUdpServer(port));
}
void rcOsgHostServer::sync()
{
	char data[_MAX_OSG_DATA_SIZE];
	memset(data, 0, _MAX_OSG_DATA_SIZE);
	sockaddr from;
	int sizeRcv = -1;
	m_svr->getPacket(from, data, sizeRcv, sizeof(SYNC_OSG_MSG));
	if (sizeRcv != -1)
		m_memshareServer->write(data, sizeof(SYNC_OSG_MSG));

}
void rcOsgHostServer::run()
{
	while (m_svr->isSocketOpen()) {
		sync();
	}
}

