#pragma once
#include "rcapp.h"
#include <osg/Node>
#include <osgEarth/MapNode>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/ExampleResources>
#include <osgEarthUtil/Controls>
#include <osgEarthSymbology/Color>
#include <osgEarthDrivers/tms/TMSOptions>
#include <osgGA/GUIEventHandler>
#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/MatrixTransform>
#include "BingOptions"
#include <osgEarth/TileSource>
#include <osgEarth/Registry>
#include <osgEarth/URI>
#include <osgEarth/StringUtils>
#include <osgEarth/Random>
#include <osgEarth/ImageUtils>
#include <osgEarth/Containers>
#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/GeometryRasterizer>
#include <osgDB/FileNameUtils>
#include <osgText/Font>
class rcBingTiledMap:public rcScene
{
public:
	rcBingTiledMap(rcApp* app,osg::ArgumentParser& arg);
	~rcBingTiledMap();
	virtual void active();
	virtual void setupScene();
	virtual rcApp* getApp();
	virtual osg::Node* getSceneNode();
private:
	std::unique_ptr<rcApp> m_renderer;
	osg::ref_ptr<osgEarth::MapNode> m_map;
	osg::ArgumentParser& m_arg;
};
