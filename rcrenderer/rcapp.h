#pragma once
#include "stdafx.h"
#include <osgViewer/Viewer>
#include <osg/Node>
/*
*@yulw,2015-4-14,Interface for a synchronized osgRender.
*/
class rcApp
{
public:
	virtual osgViewer::Viewer* getViewer()=0;
	virtual int run() = 0;
	virtual void setup()=0;
	virtual osg::Node* getRootNode()=0;
	virtual bool getStatus() = 0;
};

/*
*@yulw,2015-4-14,Interface for various scenes to be rendered.
*/
class rcScene
{
public:
	virtual void active()=0;
	virtual void setupScene()=0;
	virtual rcApp* getApp()=0;
	virtual osg::Node* getSceneNode() = 0;
};
