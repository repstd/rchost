#pragma once
#include "stdafx.h"
#include "rcfactory.h"
rcfactory::rcfactory()
{
	m_mutex = std::unique_ptr<rcmutex_ext>(new rcmutex_ext);
	m_mutex->initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
}
rcfactory::~rcfactory()
{
	
}

rcfactory* rcfactory::instance()
{
	static rcfactory inst;
	return &inst;
}

tcpServer* rcfactory::createTcpServer(int port)
{
	tcpServer* svr=NULL;
	m_mutex->lock();
	svr = new tcpServer(port);
	m_mutex->unlock();
	return svr;
}

udpServer* rcfactory::createUdpServer(int port)
{
	udpServer* svr=NULL;
	m_mutex->lock();
	svr = new udpServer(port);
	m_mutex->unlock();
	return svr;
}

udpClient* rcfactory::createUdpClient(const SOCKET socket)
{
	udpClient* cli;
	m_mutex->lock();
	cli = new udpClient(socket);
	m_mutex->unlock();
	return cli;
}
udpClient* rcfactory::createUdpClient(int port,const char* serverAddr)
{
	udpClient* cli;
	m_mutex->lock();
	cli = new udpClient(port,serverAddr);
	m_mutex->unlock();
	return cli;
}
udpClient* rcfactory::createUdpClient()
{
	udpClient* cli;
	m_mutex->lock();
	cli = new udpClient();
	m_mutex->unlock();
	return cli;
}

tcpClient* rcfactory::createTcpClient(const SOCKET socket)
{
	tcpClient* cli;
	m_mutex->lock();
	cli = new tcpClient(socket);
	m_mutex->unlock();
	return cli;
}
tcpClient* rcfactory::createTcpClient(int port,const char* serverAddr)
{
	tcpClient* cli;
	m_mutex->lock();
	cli = new tcpClient(port,serverAddr);
	m_mutex->unlock();
	return cli;
}
tcpClient* rcfactory::createTcpClient()
{
	tcpClient* cli;
	m_mutex->lock();
	cli = new tcpClient();
	m_mutex->unlock();
	return cli;
}

int rcfactory::getPortStatic(int port)
{
	return m_mapUsedPort[port];
}
rcFileMapWriter* rcfactory::createFileMapWriter(char* name)
{
	rcFileMapWriter* writer = new rcFileMapWriter(name);
	return writer;
}
rcFileMapReader* rcfactory::createFileMapReader(char* name)
{
	rcFileMapReader* reader= new rcFileMapReader(name);
	return reader;
}
rcOsgHostClient* rcfactory::createOsgHostClient(char* name, int port)
{
	rcOsgHostClient* osgSyncClient = new rcOsgHostClient(name, port, NULL);
	return osgSyncClient;
}

rcOsgHostServer* rcfactory::createOsgHostServer(char* name, int port)
{
	rcOsgHostServer* osgSyncSvr = new rcOsgHostServer(name, port);
	return osgSyncSvr;
}
