#pragma once
#include "rc_common.h"
#include "rcthread.h"
class rcfactory;
class udpServer;
class udpClient;
/*
*@yulw,2015-5-8
*AndroidSupport is added using rcAndroidServer.
*m_svr is used to listen the androidPort and get message from the AndroidApp,and then
*forward it to other host nodes using  m_clit.
*Para:
*@androidPort: the same as the port used in Android.
*				The value is 7000 by default,which is also refered  in rc_common.h. 
*@hostPort:		port of the host,which is used as to broadcast msg to slaves.
*/
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