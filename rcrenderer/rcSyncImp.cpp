#include "stdafx.h"
#include "rcSyncImp.h"
#include "rcrenderer.h"
rcMemShareReader::rcMemShareReader(char* filemappingName) :rcMsgerImp() {
	m_memShareReader = std::unique_ptr<rcFileMapReader>(rcfactory::instance()->createFileMapReader(filemappingName));
}

rcMemShareReader::~rcMemShareReader() {
	m_memShareReader->close();
	m_memShareReader.release();
}

void rcMemShareReader::read(void* msg,int sizeToRead)
{
	if (isValid())
		m_memShareReader->read(msg, sizeToRead);
}
bool rcMemShareReader::isValid() {
	return m_memShareReader->isValid();
}

rcSocketReader::rcSocketReader(int port) :rcMsgerImp() {
	m_socketReader = std::unique_ptr<udpServer>(rcfactory::instance()->createUdpServer(port));
}

rcSocketReader::~rcSocketReader() {

}

void rcSocketReader::read(void* msg,int maxSize) {
	sockaddr from;
	int sizeRead;
	if (isValid())
		m_socketReader->getPacket(from, msg, sizeRead, maxSize);
}
bool rcSocketReader::isValid() 
{
	return m_socketReader->isSocketOpen() ? true : false;
}


rcMemShareWriter::rcMemShareWriter(char* filemappingName) :rcMsgerImp() {
	m_memShareWriter = std::unique_ptr<rcFileMapWriter>(rcfactory::instance()->createFileMapWriter(filemappingName));
}

rcMemShareWriter::~rcMemShareWriter() {
	m_memShareWriter->close();
	m_memShareWriter.release();
}

void rcMemShareWriter::write(void* msg,int sizeToRead)
{
	if (isValid())
		m_memShareWriter->write(msg, sizeToRead);
}

bool rcMemShareWriter::isValid() {
	return m_memShareWriter->isValid();
}

rcSocketWriter::rcSocketWriter(int port) :rcMsgerImp() 
{
	m_socketWriter= std::unique_ptr<udpClient>(rcfactory::instance()->createUdpClient(port,NULL));
}

rcSocketWriter::~rcSocketWriter() {

}

void rcSocketWriter::write(void* msg,int maxSize) {
	sockaddr from;
	int sizeRead;
	if (isValid())
		m_socketWriter->sendPacket((char*)msg, maxSize);
}
bool rcSocketWriter::isValid() 
{
	return m_socketWriter->isSocketOpen() ? true : false;
}

/*
*Implementation of Sync Module abstract.@yulw,2015-4-12
*/
void rcSyncImp::setType(TYPE t)
{
	m_type = t;
}
;
TYPE rcSyncImp::getType()
{
	return m_type;
}

/*
*Implementation of Sync Module for Masters.@yulw,2015-4-12
*/
rcOsgMasterImp::rcOsgMasterImp(rcMsgerImp* imp) : rcSyncImp()
{
	setType(_MASTER);
	m_imp = std::unique_ptr<rcMsgerImp>(imp);
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
	int cnt = 0;
	int stride = sizeof(osgGA::GUIEventAdapter);
	char* temp = new char[events.size()*stride];
	osg::ref_ptr<osgGA::GUIEventAdapter> ele;
	for (osgGA::EventQueue::Events::iterator itr = events.begin(); itr != events.end(); ++itr) {
		if (!itr->get())
			break;
		ele = reinterpret_cast<osgGA::GUIEventAdapter*>(itr->get());
		/*
		*@yulw,2015-4-15. 
		*To decrease the amount of useless information transmitted among the master and slaves, 
		*only the Keyboard events are captured and sent  via the shared memory and socket.
		*/
		if (ele->getEventType() == osgGA::GUIEventAdapter::KEYDOWN||
			ele->getEventType() == osgGA::GUIEventAdapter::KEYUP) 
		{
			memcpy(temp+cnt*stride, itr->get(), stride);
			cnt++;
		}
	}
	pMsg->_eventSize = cnt;
	memcpy(pMsg->_event, temp, cnt*stride);
	delete temp;
}
void rcOsgMasterImp::sync(osgViewer::Viewer* viewer) {
	/*@yulw,2014-4-14. 
	*We should skip the sync process before the FileMapping is Ready in case of crash.
	*/
	rcMsgerImp* imp = getImp();
	if (!imp->isValid())
		return;
	SYNC_OSG_MSG msg;
	msg._isExit = dynamic_cast<rcrenderer*>(viewer)->getStatus();
	encodeMsg(viewer, &msg);
	imp->write(&msg, sizeof(msg));
}

rcMsgerImp* rcOsgMasterImp::getImp()
{
	return m_imp.get();
}
/*
*Implementation of Sync Module for Slaves.@yulw,2015-4-12
*/
rcOsgSlaveImp::rcOsgSlaveImp(rcMsgerImp* imp) :rcSyncImp()
{
	setType(_SLAVE);
	//m_memShareClient = std::unique_ptr<rcFileMapReader>(rcfactory::instance()->createFileMapReader(fileMappingName));
	m_imp = std::unique_ptr<rcMsgerImp>(imp);
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
	int eventSz = pMsg->_eventSize;
	osgGA::EventQueue::Events evts;
	osg::ref_ptr<osgGA::GUIEventAdapter> ele;
	int stride = sizeof(osgGA::GUIEventAdapter);
	for (int i = 0; i < eventSz; i++) {
		ele = reinterpret_cast<osgGA::GUIEventAdapter*>(pMsg->_event + i*stride);
		if (!ele.valid()) {
			__STD_PRINT("%s\n", "Failed to Retrieve Events");
			break;
		}
		evts.push_back(ele.get());
	}
	viewer->getEventQueue()->clear();
	viewer->getEventQueue()->appendEvents(evts);
}

void rcOsgSlaveImp::sync(osgViewer::Viewer* viewer)
{
	/*@yulw,2014-4-14. 
	*We should skip the sync process before the FileMapping is Ready in case of crash.
	*/
	rcMsgerImp* imp = getImp();
	if (!imp->isValid())
		return;
	SYNC_OSG_MSG msg;
	imp->read(&msg, sizeof(SYNC_OSG_MSG));
	if (msg._isExit)
		exit(0);

	decodeMsg(viewer, &msg);
}

rcMsgerImp* rcOsgSlaveImp::getImp() {
	return m_imp.get();
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
rcSyncImp* rcSyncImpFactory::createRcSyncMasterModule(char* name) 
{
	rcOsgMasterImp* imp = new rcOsgMasterImp(new rcMemShareWriter(name));
	return imp;
}

rcSyncImp* rcSyncImpFactory::createRcSyncMasterModule(int port) {

	rcOsgMasterImp* imp = new rcOsgMasterImp(new rcSocketWriter(port));
	return imp;

}
rcSyncImp* rcSyncImpFactory::createRcSyncMasterModule() {
	return createRcSyncMasterModule("MemShareTemp");
}

rcSyncImp* rcSyncImpFactory::createRcSyncSlaveModule(char* name)
{
	rcOsgSlaveImp* imp = new rcOsgSlaveImp(new rcMemShareReader(name));
	return imp;
}

rcSyncImp* rcSyncImpFactory::createRcSyncSlaveModule(int port)
{
	rcOsgSlaveImp* imp = new rcOsgSlaveImp(new rcSocketReader(port));
	return imp;
}
rcSyncImp* rcSyncImpFactory::createRcSyncSlaveModule()
{
	return createRcSyncSlaveModule("MemShareTemp");
}
