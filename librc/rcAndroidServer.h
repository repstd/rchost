#pragma once
#include "rc_common.h"
#include "rcthread.h"
#include <OpenThreads\Condition>
#include <queue>
using OpenThreads::Condition;
class rcfactory;
class udpServer;
class udpClient;
class rcAndroidServer;
//@yulw,20150616, the rcTask is Mainly Designed for socket multiplex.
template<typename T>
class rcTask
{
public:
	virtual void excute() = 0;

	virtual bool isFinished() = 0;

	virtual T* getOperator() = 0;
					
};

class rcLogProducerTask: public rcTask<udpClient>
{
public:	

	rcLogProducerTask(std::shared_ptr<rcAndroidServer> androidSvr,SOCKET socket);

	virtual void excute();

	virtual bool isFinished();

	virtual udpClient* getOperator();

private :

	std::unique_ptr<udpClient> m_client;

	std::shared_ptr<rcAndroidServer>	m_androidServer;

	char m_msgBuf[_MAX_DATA_SIZE];
};

class rcLogConsumerTask :public rcTask<udpServer>
{
public:
	rcLogConsumerTask(std::shared_ptr<rcAndroidServer> androidServer,int logServerPort);

	virtual void excute();

	virtual bool isFinished();

	virtual udpServer* getOperator();

private:
	std::unique_ptr<udpServer> m_server;

	std::shared_ptr<rcAndroidServer>	m_androidServer;

	char m_msgBuf[_MAX_DATA_SIZE];
};
template <typename T>
class rcWorker :public THREAD
{
public :
	rcWorker(rcTask<T>* taskImpl) :THREAD()
	{
		m_mutex.initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
		m_taskImp = taskImpl;
	}
	virtual void run() {
		while (!getImp()->isFinished()) {
			m_mutex.lock();

			getImp()->excute();

			m_mutex.unlock();
		}
	}
	rcTask<T>* getImp() {
		return m_taskImp;
	}

private:
	rcTask<T>* m_taskImp;
	rcmutex_ext m_mutex;
};
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
class rcAndroidServer:
	public THREAD,public std::enable_shared_from_this<rcAndroidServer>
{
	friend class rcfactory;
public:
	~rcAndroidServer();

	virtual void run();
	
	virtual int cancel();

	virtual int start();

	int getLogCacheCapacity();

	void setLogCacheCapacity(int size);
		
	bool isNewLogsAvaliable();

	std::string getLog();
		
	void writeLog(std::string log);

private:
		
	rcAndroidServer(int androidPort,int hostPort);

	std::unique_ptr<udpServer> m_svr;

	std::unique_ptr<udpClient> m_clit;

	//used to collect msg sent to m_clit
	std::vector<rcWorker<udpClient>*> m_producerPool;

	std::unique_ptr<rcWorker<udpClient> > m_logProducer;

	std::unique_ptr<rcWorker<udpServer> > m_logConsumer;

		

	std::queue<std::string> m_recentLogsCache;

	int m_logCacheSize;

	rcmutex_ext m_mutex;

	bool m_isNotified;

	long long m_logCnt;
};
