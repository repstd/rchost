#include "stdafx.h"
#include "rcrenderer.h"
rcrenderer::rcrenderer() :osgViewer::Viewer() { }

rcrenderer::rcrenderer(const rcrenderer& copy, const osg::CopyOp& op)
: osgViewer::Viewer(copy, op) 
{ }
rcrenderer::~rcrenderer() { }

rcrenderer::rcrenderer(osg::Node* node,rcSyncImp* syncImp) : osgViewer::Viewer() {
	if (node!=NULL)
		m_root = node;
	if (syncImp != NULL)
		m_imp = std::unique_ptr<rcSyncImp>(syncImp);
}
void rcrenderer::setupRenderer(int width, int height, const char* keystonrFilename)
{
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->useCursor = false;
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (gc.valid()) {
		osg::notify(osg::INFO) << "  GraphicsWindow has been created successfully." << std::endl;
		gc->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.6f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else {
		osg::notify(osg::NOTICE) << "  GraphicsWindow has not been created successfully." << std::endl;
	}

	getCamera()->setViewport(0, 0, traits->width, traits->height);
	getCamera()->setGraphicsContext(gc.get());

	osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform();
	mt->addChild(m_root.get());
	osg::DisplaySettings* ds = osg::DisplaySettings::instance();
	/*	ds->setStereo(true);
	ds->setStereoMode (osg::DisplaySettings::ANAGLYPHIC);*/
	getCamera()->setDisplaySettings(ds);

	ds->setKeystoneHint(TRUE);
	setSceneData(mt.get());
	assignStereoOrKeystoneToCamera(getCamera(), ds);
	double fovy, aspectRatio, zNear, zFar;
	getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
	double newAspectRatio = double(traits->width) / double(traits->height);
	double aspectRatioChange = newAspectRatio / aspectRatio;
	if (aspectRatioChange != 1.0) {
		getCamera()->getProjectionMatrix() *= osg::Matrix::scale(1.0 / aspectRatioChange, 1.0, 1.0);
	}

	float camera_fov = -1.0f;
	if (camera_fov>0.0f)
	{
		double fovy, aspectRatio, zNear, zFar;
		getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

		double original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
		std::cout << "setting lens perspective : original " << original_fov << "  " << fovy << std::endl;

		fovy = atan(tan(osg::DegreesToRadians(camera_fov)*0.5) / aspectRatio)*2.0;
		getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

		getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
		original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
		std::cout << "setting lens perspective : new " << original_fov << "  " << fovy << std::endl;
	}
}
rcSyncImp* rcrenderer::getImp() {
	return m_imp.get();
}
int rcrenderer::run()
{
	realize();
	rcSyncImp* syncModule = getImp();
	while (!done())
	{
		advance();
		syncModule->sync(this);
		adjustPara();
		frame();
		eventTraversal();
		updateTraversal();
		renderingTraversals();
	}
	return 0;
}
void rcrenderer::adjustPara()
{
	ParallaxPara& p = getParaParameter();
	//Specified algorithm to adjust the parallax.
	//TODO:
}
ParallaxPara& rcrenderer::getParaParameter() {
	return m_para;
}
