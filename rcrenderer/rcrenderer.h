#pragma once
#include "stdafx.h"
#include "rcfactory.h"
#include "rcSyncImp.h"
#include "rcapp.h"
#include <osgViewer/Viewer>
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/readFile>
//Parameters for parallax adjustment
typedef struct _ParallaxPara {
	//index of the camara.
	int _cam_no;
	//fixed parameter for increase/decrease the parameter
	float _para_c;
	//view distance
	float _view_dis;
	//rotate angle
	float _rotate_angle;
} ParallaxPara;
class rcrenderer:public rcApp
{
public:
	rcrenderer(int width=800, int height=600, char* keystoneFilenName=NULL);
	rcrenderer(rcSyncImp* syncImp,int width=800, int height=600, char* keystoneFilenName=NULL);
	rcrenderer(osg::Node* node, rcSyncImp* syncImp,int width=800, int height=600, char* keystoneFilenName=NULL);
	//rcrenderer(const rcrenderer& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY);
	~rcrenderer();
	virtual osgViewer::Viewer* getViewer();
	virtual int run();
	virtual void setup();
	virtual osg::Node* getRootNode();
	virtual bool getStatus();
	void setupRenderer(int width, int height, const char* keystonrFilename);
	void adjustPara();
	void setSyncImp(rcSyncImp* syncImp);
	rcSyncImp* getImp();
	ParallaxPara& getParaParameter();
private:
	int m_width;
	int m_height;
	char* m_keystoneFileName;
	bool m_isExit;
	ParallaxPara m_para;
	osg::ref_ptr<osg::MatrixTransform> m_root;
	std::unique_ptr<rcSyncImp> m_imp;
	osg::ref_ptr<osgViewer::Viewer> m_viewer;
};
class rcEventHandler:public osgGA::GUIEventHandler
{
public:
	rcEventHandler(rcrenderer*);
	virtual ~rcEventHandler();

protected:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
private:
	std::unique_ptr<rcrenderer> m_render;
};