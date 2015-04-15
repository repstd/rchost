#pragma once
#include "stdafx.h"
#include <osgViewer/Viewer>
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/readFile>
#include "rcfactory.h"
#include "rcSyncImp.h"
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
class rcrenderer:public osgViewer::Viewer
{
public:
	rcrenderer();
	rcrenderer(osg::Node* node, rcSyncImp* syncImp);
	rcrenderer(const rcrenderer& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY);
	~rcrenderer();
	META_Object(osg, rcrenderer)
	virtual int run();
	void setupRenderer(int width, int height, const char* keystonrFilename);
	void adjustPara();
	rcSyncImp* getImp();
	ParallaxPara& getParaParameter();
	osg::ref_ptr<osg::Node> getNode();
	bool getStatus();
private:
	bool m_isExit;
	ParallaxPara m_para;
	osg::ref_ptr<osg::Group> m_root;
	std::unique_ptr<rcSyncImp> m_imp;
};
class rcEventHandler:public osgGA::GUIEventHandler
{
public:
	rcEventHandler();
	virtual ~rcEventHandler();

protected:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);
};