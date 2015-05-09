#include "StdAfx.h"
#include "rcAndroidServer.h"
#include "rcfactory.h"
rcAndroidServer::rcAndroidServer(int androidPort,int hostPort) :THREAD() 
{
	m_svr.reset(rcfactory::instance()->createUdpServer(androidPort));
	m_clit.reset(rcfactory::instance()->createUdpClient(hostPort,NULL));
}

rcAndroidServer::~rcAndroidServer() {
	m_svr.reset();
	m_clit.reset();
}

void rcAndroidServer::run() 
{
	sockaddr from;
	char data[_MAX_DATA_SIZE];
	int size=-1;
	_MSG msg;
	while (m_svr->isSocketOpen()) {
		memset(data, 0, _MAX_DATA_SIZE);
		m_svr->getPacket(from, data, size, _MAX_DATA_SIZE);
		if (size != -1 && m_clit->isSocketOpen()) {
			printf("MessageFromAndroid:%s\n", data);
			parseMsg(data, msg);
			m_clit->sendPacket((char*)(&msg), sizeof(msg));
		}
	}
}

