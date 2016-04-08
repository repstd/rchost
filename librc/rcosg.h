#pragma once
#include "rc_common.h"
#include "rcthread.h"
class rcfactory;
class rcFileMapReader;
class rcFileMapWriter;
class udpServer;
class udpClient;
class rcosg
{
public:
	rcosg();
	~rcosg();
};
class rcOsgHostClient:public THREAD
{
public:	
	void run();
protected:
	void sync();
	rcOsgHostClient(char* memShareName, int port,const char* addr);
private:
	std::unique_ptr<rcFileMapReader>	m_memshareClient;
	std::unique_ptr<udpClient>			m_cli;
	friend rcfactory;
};

class rcOsgHostServer :public THREAD
{
public:	
	void run();
protected:
	void sync();
	rcOsgHostServer(char* memShareName, int port);
private:
	std::unique_ptr<rcFileMapWriter>	m_memshareServer;
	std::unique_ptr<udpServer>			m_svr;
	friend rcfactory;
};
