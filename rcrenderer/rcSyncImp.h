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
class rcSyncImp
{
public:
	rcSyncImp(){}
	virtual ~rcSyncImp(){}
	virtual void sync(osgViewer::Viewer* viewer)=0;
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
protected:
	rcOsgMasterImp();
private:	
	std::unique_ptr<rcFileMapWriter> m_memShareServer;
};

class rcOsgSlaveImp :public rcSyncImp
{
	friend rcSyncImpFactory;
public:
	virtual ~rcOsgSlaveImp();
	void decodeMsg(osgViewer::Viewer* viewer, void* msg);
	void sync(osgViewer::Viewer* viewer);
protected:
	rcOsgSlaveImp();
private:
	std::unique_ptr<rcFileMapReader> m_memShareClient;
};

class rcSyncImpFactory
{
public:
	static rcSyncImpFactory* instance();
	rcSyncImp* createRcSyncMasterModule();
	rcSyncImp* createRcSyncSlaveModule();
private:
	rcSyncImpFactory();
};