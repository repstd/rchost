#pragma once
#include "stdafx.h"
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/MatrixTransform>
#include "rcSyncImp.h"
#include "rcfactory.h"
enum TYPE
{
	_MASTER=0,
	_SLAVE
};
class rcSyncImpFactory;

class rcMsgerImp
{
public:
	rcMsgerImp(){}
	virtual ~rcMsgerImp(){}
	virtual void read(void* msg, int sizeToRead){}
	virtual void write(void* msg, int sizeToWrite){}
	virtual bool isValid()=0;
};
class rcMemShareReader:public rcMsgerImp
{
public:
	rcMemShareReader(char* filemappingName);
	~rcMemShareReader();

	virtual void read(void* msg, int sizeToRead);
	virtual bool isValid();
private:
	std::unique_ptr<rcFileMapReader> m_memShareReader;
};
class rcSocketReader:public rcMsgerImp
{
public:
	rcSocketReader(int port);
	virtual ~rcSocketReader();
	virtual void read(void* msg, int maxSize);
	virtual bool isValid();
private:
	std::unique_ptr<udpServer> m_socketReader;
};
class rcMemShareWriter:public rcMsgerImp
{
public:
	rcMemShareWriter(char* filemappingName);
	~rcMemShareWriter();

	virtual void write(void* msg, int sizeToRead);
	virtual bool isValid();
private:
	std::unique_ptr<rcFileMapWriter> m_memShareWriter;
};
class rcSocketWriter:public rcMsgerImp
{
public:
	rcSocketWriter(int port);
	virtual ~rcSocketWriter();
	virtual void write(void* msg, int maxSize);
	virtual bool isValid();
private:
	std::unique_ptr<udpClient> m_socketWriter;
};



class rcSyncImp
{
public:
	rcSyncImp(){}
	virtual ~rcSyncImp(){}
	virtual void sync(osgViewer::Viewer* viewer) = 0;
	void setType(TYPE t);
	TYPE getType();
	TYPE m_type;
};
class rcOsgMasterImp :public rcSyncImp
{
	friend rcSyncImpFactory;
public:
	virtual ~rcOsgMasterImp();
	void encodeMsg(osgViewer::Viewer* viewer, void* msg);
	void sync(osgViewer::Viewer* viewer);
	rcMsgerImp* getImp();
protected:
	rcOsgMasterImp(rcMsgerImp* imp);
private:
	std::unique_ptr<rcMsgerImp> m_imp;
};
class rcOsgSlaveImp :public rcSyncImp
{
	friend rcSyncImpFactory;
public:
	virtual ~rcOsgSlaveImp();
	void decodeMsg(osgViewer::Viewer* viewer, void* msg);
	virtual void sync(osgViewer::Viewer* viewer);
	rcMsgerImp* getImp();
protected:
	rcOsgSlaveImp(rcMsgerImp* imp);
	/*
	*@yulw,2015-4-14.An alternative to receive message from the master directly via the sockets.
	*/
private:
	std::unique_ptr<rcMsgerImp> m_imp;
};
class rcSyncImpFactory
{
public:
	static rcSyncImpFactory* instance();
	rcSyncImp* createRcSyncMasterModule();
	rcSyncImp* createRcSyncMasterModule(char* name);
	rcSyncImp* createRcSyncMasterModule(int port);
	rcSyncImp* createRcSyncSlaveModule();
	rcSyncImp* createRcSyncSlaveModule(char* name);
	rcSyncImp* createRcSyncSlaveModule(int port);
private:
	rcSyncImpFactory();
};