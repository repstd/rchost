#include "stdafx.h"
#include "rcSyncImp.h"

/*
*Implementation of Sync Module abstract.@yulw,2015-4-12
*/
void rcSyncImp::setType(TYPE t)
{
	m_type = t;
}
TYPE rcSyncImp::getType()
{
	return m_type;
}

/*
*Implementation of Sync Module for Masters.@yulw,2015-4-12
*/
rcOsgMasterImp::rcOsgMasterImp() : rcSyncImp()
{
	setType(_MASTER);
	m_memShareServer = std::unique_ptr<rcFileMapWriter>( rcfactory::instance()->createFileMapWriter("MemShareTempName"));
}
rcOsgMasterImp::~rcOsgMasterImp(){}

void rcOsgMasterImp::encodeMsg(osgViewer::Viewer* viewer, void* msg)
{
	SYNC_OSG_MSG* pMsg = reinterpret_cast<SYNC_OSG_MSG*>(msg);
	osg::Matrix modelview(viewer->getCamera()->getViewMatrix());
	osg::Matrix projection(viewer->getCamera()->getProjectionMatrix());
	osg::Matrix manipulatorMatrix(viewer->getCameraManipulator()->getMatrix());
	memcpy(pMsg->_matrix, manipulatorMatrix.ptr(), 16 * sizeof(double));
	memcpy(pMsg->_modelView, modelview.ptr(), 16 * sizeof(double));
	memcpy(pMsg->_projection, projection.ptr(), 16 * sizeof(double));
#if 0
	osgGA::EventQueue::Events events;
	osgViewer::ViewerBase::Contexts contexts;
	viewer->getContexts(contexts);
	for (osgViewer::ViewerBase::Contexts::iterator citr = contexts.begin(); citr != contexts.end(); ++citr)
	{
		osgGA::EventQueue::Events gw_events;

		osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(*citr);
		if (gw)
		{
			gw->checkEvents();
			gw->getEventQueue()->copyEvents(gw_events);
		}
		events.insert(events.end(), gw_events.begin(), gw_events.end());
	}
	viewer->getEventQueue()->copyEvents(events);
	//Write to msg structure
	pMsg->_eventSize= events.size();
	int offset = 0;
	int stride = sizeof(osgGA::GUIEventAdapter);
	for (osgGA::EventQueue::Events::iterator itr = events.begin(); itr != events.end(); ++itr) {
		memcpy(pMsg->_event+offset, itr->get(), stride);
		offset += stride;
	}
#endif
}
void rcOsgMasterImp::sync(osgViewer::Viewer* viewer) {
	SYNC_OSG_MSG msg;
	encodeMsg(viewer, &msg);
	m_memShareServer->write(&msg, sizeof(msg));
}
/*
*Implementation of Sync Module for Slaves.@yulw,2015-4-12
*/
rcOsgSlaveImp::rcOsgSlaveImp() :rcSyncImp()
{
	setType(_SLAVE);
	m_memShareClient = std::unique_ptr<rcFileMapReader>( rcfactory::instance()->createFileMapReader("MemShareTempName"));
}

rcOsgSlaveImp::~rcOsgSlaveImp(){}
void rcOsgSlaveImp::decodeMsg(osgViewer::Viewer* viewer, void* msg)
{
	SYNC_OSG_MSG* pMsg = reinterpret_cast<SYNC_OSG_MSG*>(msg);
	osg::Matrix modelView(pMsg->_modelView);
	osg::Matrix projection(pMsg->_projection);
	osg::Matrix mani(pMsg->_matrix);
	viewer->getCamera()->setViewMatrix(modelView);
	viewer->getCamera()->setProjectionMatrix(projection);
	viewer->getCameraManipulator()->setByMatrix(mani);
}
void rcOsgSlaveImp::sync(osgViewer::Viewer* viewer)
{
	SYNC_OSG_MSG msg;
	m_memShareClient->read(&msg, sizeof(SYNC_OSG_MSG));
	decodeMsg(viewer, &msg);
}

/*
*Implementation of Sync Module Factory.@yulw,2015-4-12
*/
rcSyncImpFactory::rcSyncImpFactory() { }
rcSyncImpFactory* rcSyncImpFactory::instance()
{
	static rcSyncImpFactory inst;
	return &inst;
}
rcSyncImp* rcSyncImpFactory::createRcSyncMasterModule()
{
	rcSyncImp* imp = new rcOsgMasterImp();
	return imp;
}
rcSyncImp* rcSyncImpFactory::createRcSyncSlaveModule()
{
	rcSyncImp* imp = new rcOsgSlaveImp();
	return imp;
}
