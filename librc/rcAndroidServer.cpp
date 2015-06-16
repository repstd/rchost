#include "StdAfx.h"
#include "rcAndroidServer.h"
#include "rcfactory.h"
#include <iostream>
#include "rcthread.h"
rcAndroidServer::rcAndroidServer(int androidPort, int hostPort) :THREAD()
{
	//std::cout << "rcAndroidServer inited for " << androidPort << "\t" << hostPort << std::endl;
	m_svr.reset(rcfactory::instance()->createUdpServer(androidPort));
	m_clit.reset(rcfactory::instance()->createUdpClient(hostPort, NULL));

	m_mutex.initMutex(new MUTEX(MUTEX::MUTEX_RECURSIVE));

	for (int i = 0; i < 20; i++) {
		m_producerPool.push_back(new rcWorker<udpClient>(new rcLogProducerTask(std::shared_ptr<rcAndroidServer>(this), m_clit->getSocket())));
		//m_producerPool[i]=rcWorker<udpClient>(new rcLogProducerTask(std::shared_ptr<rcAndroidServer>(this), m_clit->getSocket()));
	}
	//m_logProducer.reset(new rcWorker<udpClient>(new rcLogProducerTask(std::shared_ptr<rcAndroidServer>(this), m_clit->getSocket())));

	m_logConsumer.reset(new rcWorker<udpServer>(new rcLogConsumerTask(std::shared_ptr<rcAndroidServer>(this), 20716)));

	setLogCacheCapacity(100);

	m_logCnt = 0;

	m_isNotified = false;
}

rcAndroidServer::~rcAndroidServer() {
	m_svr.reset();
	m_clit.reset();
}

void rcAndroidServer::run()
{
	sockaddr from;
	char data[_MAX_DATA_SIZE];
	int size = -1;
	_MSG msg;
	while (m_svr->isSocketOpen()){
		memset(data, 0, _MAX_DATA_SIZE);
		m_svr->getPacket(from, data, size, _MAX_DATA_SIZE);
		if (size != -1 && m_clit->isSocketOpen()) {
			printf("MessageFromAndroid:%s\n", data);
			parseMsg(data, msg);
			m_clit->sendPacket((char*)(&msg), sizeof(msg));
		}
	}
}
int rcAndroidServer::cancel()
{
	m_logProducer->cancel();
	m_logConsumer->cancel();
	return THREAD::cancel();
}

int rcAndroidServer::start() {
	//m_logProducer->start();

	for (int i = 0; i < 20; i++) {
		//m_producerPool.push_back(rcWorker<udpClient>(new rcLogProducerTask(std::shared_ptr<rcAndroidServer>(this), m_clit->getSocket())));
		std::cout << m_producerPool[i]->getThreadId() << "#started." << std::endl;
		m_producerPool[i]->start();
	}
	m_logConsumer->start();
	return THREAD::start();
}

int rcAndroidServer::getLogCacheCapacity()
{
	int result = -1;
	m_mutex.lock();

	result = m_logCacheSize;

	m_mutex.unlock();
	return result;
}

void rcAndroidServer::setLogCacheCapacity(int size)
{
	m_mutex.lock();

	m_logCacheSize = size;

	m_mutex.unlock();
}

bool rcAndroidServer::isNewLogsAvaliable() {
	if (m_recentLogsCache.size() == 0)
		return false;
	else
		return true;
}

std::string rcAndroidServer::getLog()
{
	m_mutex.lock();
	//m_alarm->wait(m_mutex.getMutex());
	std::cout << "awake.." << std::endl;
	std::string result = "";
	if (isNewLogsAvaliable()) {
		result = m_recentLogsCache.front();
		m_recentLogsCache.pop();
	}
	m_mutex.unlock();
	return result;
}

void rcAndroidServer::writeLog(std::string log) {
	m_mutex.lock();
	if (m_recentLogsCache.size() == getLogCacheCapacity())
		m_recentLogsCache.pop();
	m_recentLogsCache.push(log);
	std::cout << m_recentLogsCache.size() << std::endl;
	//m_alarm->signal();
	m_mutex.unlock();
}
//Producer
rcLogProducerTask::rcLogProducerTask(std::shared_ptr<rcAndroidServer> androidSvr, SOCKET socket) :rcTask<udpClient>() {
	m_client.reset(rcfactory::instance()->createUdpClient(socket));
	m_androidServer = androidSvr;
}

void rcLogProducerTask::excute() {
	memset(m_msgBuf, 0, _MAX_DATA_SIZE);
	sockaddr from;
	int sizeRcv = -1;
	if (m_client->isSocketOpen())
	{
		m_client->getPacket(from, m_msgBuf, sizeRcv, _MAX_DATA_SIZE);
		if (sizeRcv != -1) {
			//__STD_PRINT("%s\n", m_msgBuf);
			m_androidServer->writeLog(std::string(m_msgBuf));
		}
	}
}

bool rcLogProducerTask::isFinished() {
	return !m_client.get() || !m_client->isSocketOpen();
}

udpClient* rcLogProducerTask::getOperator() {
	return m_client.get();
}

//Consumer
rcLogConsumerTask::rcLogConsumerTask(std::shared_ptr<rcAndroidServer> androidSvr, int logServerPort) :rcTask<udpServer>() {
	m_server.reset(rcfactory::instance()->createUdpServer(logServerPort));
	m_androidServer = androidSvr;
}

void rcLogConsumerTask::excute() {
	memset(m_msgBuf, 0, _MAX_DATA_SIZE);
	sockaddr from;
	int sizeRcv = -1;
	if (m_server->isSocketOpen())
	{
		m_server->getPacket(from, m_msgBuf, sizeRcv, _MAX_DATA_SIZE);
		if (sizeRcv != -1) {
			while (m_androidServer->isNewLogsAvaliable()) {
				std::string log = m_androidServer->getLog();
				m_server->sendPacket(from, const_cast<char*>(log.c_str()), log.length()*sizeof(char), _MAX_DATA_SIZE);
				std::cout<<"sent log#"<<log.c_str() << std::endl;

			}
		}
	}
}

bool rcLogConsumerTask::isFinished() {
	return !m_server.get() || !m_server->isSocketOpen();
}

udpServer* rcLogConsumerTask::getOperator() {
	return m_server.get();
}
