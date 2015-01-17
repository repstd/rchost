#pragma once
#include "osgViewer/viewer"
#include "playerImp.h"
#include "server.h"
#include "rc_common.h"
class rcviewer:public osgViewer::Viewer
{
public:
	rcviewer(){ return; }
	rcviewer(const rcviewer& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY)
	{
		return;
	}
	META_Object(osg, rcviewer)
	rcviewer(playerImp* imp);
	rcviewer(impFactory* factory,char* nameImp);

	osg::Camera* createOrdinaryCamera()
	{
		return NULL;
	}

	~rcviewer();
	void setupViewer(int width,int height,const char* keyStonehFilename);
	virtual int run();

	int handleOpenCV();
	int handleVLC();
	playerImp* getImp();
private:
	playerImp* m_pPlayerImplementation;
	osg::Texture2D* m_tex;
#ifdef _PIPE_SYNC	
	std::shared_ptr<namedpipeClient> sync_pipe_client;
#else
	std::shared_ptr<server> sync_server;
	std::string m_hostname;

#endif
};

