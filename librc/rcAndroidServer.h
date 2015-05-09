#pragma once
#include "rc_common.h"
#include "rcthread.h"
class rcfactory;
class udpServer;
class udpClient;
class rcAndroidServer:public THREAD
{
public:
	~rcAndroidServer();
	void run();
private:
	rcAndroidServer(int androidPort,int hostPort);
	std::unique_ptr<udpServer> m_svr;
	std::unique_ptr<udpClient> m_clit;
	friend class rcfactory;
};