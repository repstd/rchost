#pragma once
#include "stdafx.h"
#include <osgViewer/Viewer>
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include "rcfactory.h"
#include "rcSyncImp.h"
//Parameters for parallax adjustment
typedef struct _ParallaxPara {
	//index of the camara.
	int _cam_no;
	//fixed parameter for increase/decrease the parameter
	int _para_c;
	//view distance
	int _view_dis;
} ParallaxPara;
class rcrenderer:public osgViewer::Viewer
{
public:
	rcrenderer();
	rcrenderer(osg::Node* node, rcSyncImp* syncImp);
	rcrenderer(const rcrenderer& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY);
	~rcrenderer();
	META_Object(osg, rcrenderer)
	void setupRenderer(int width, int height, const char* keystonrFilename);
	void adjustPara();
	virtual int run();
	rcSyncImp* getImp();
	ParallaxPara& getParaParameter();
private:
	ParallaxPara m_para;
	osg::ref_ptr<osg::Node> m_root;
	std::unique_ptr<rcSyncImp> m_imp;
};


