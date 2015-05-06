#pragma once
#include "stdafx.h"
#include <osgViewer/Viewer>
#include <osg/Group>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/readFile>
#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
#include "rcapp.h"
class rcModelScene:public rcScene
{
public:
	rcModelScene(rcApp* app,osg::ArgumentParser& arg);
	~rcModelScene();
	virtual void active();
	virtual void setupScene();
	virtual rcApp* getApp();
	virtual osg::Node* getSceneNode();
private:
	std::shared_ptr<rcApp> m_renderer;
	osg::ref_ptr<osg::Node> m_model;
};

