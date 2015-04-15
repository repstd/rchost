#include "stdafx.h"
#include "rcrenderer.h"
#include <vector>
#include <string>


rcrenderer::rcrenderer() :osgViewer::Viewer() { }

rcrenderer::rcrenderer(const rcrenderer& copy, const osg::CopyOp& op)
: osgViewer::Viewer(copy, op) {
	m_root = new osg::Group;
	m_isExit = false;
}
rcrenderer::~rcrenderer() {
	if (m_root.valid())
		m_root.release();
	m_isExit = true;
}

rcrenderer::rcrenderer(osg::Node* node, rcSyncImp* syncImp) : osgViewer::Viewer() {
	m_root = new osg::Group;
	if (node != NULL)
		m_root->addChild(node);
	if (syncImp != NULL)
		m_imp = std::unique_ptr<rcSyncImp>(syncImp);
	m_isExit = false;
}
void rcrenderer::setupRenderer(int width, int height, const char* keystoneFilename)
{
#ifdef _RC_RENDER_TEST
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->useCursor = true;
#else
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 0;
	traits->y = 0;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->useCursor = false;
#endif
	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (gc.valid()) {
		gc->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.6f, 1.0f));
		gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	getCamera()->setViewport(0, 0, traits->width, traits->height);
	getCamera()->setGraphicsContext(gc.get());
	osg::DisplaySettings* ds = osg::DisplaySettings::instance();

	std::vector<std::string> keystones;
	keystones.push_back(keystoneFilename);
	ds->setKeystoneFileNames(keystones);
	setSceneData(m_root.get());
	osg::Camera* maniCamera = dynamic_cast<osg::Camera*>(getCameraManipulator());
	osg::ref_ptr<osgViewer::Keystone> keystone = osgDB::readFile<osgViewer::Keystone>(keystoneFilename);
	ds->setKeystoneHint(true);
	bool keystonesLoaded = true;
	ds->getKeystones().push_back(keystone.get());
	if (maniCamera)
		assignStereoOrKeystoneToCamera(maniCamera, ds);
	else
		assignStereoOrKeystoneToCamera(getCamera(), ds);

	double fovy, aspectRatio, zNear, zFar;
	getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
	double newAspectRatio = double(traits->width) / double(traits->height);
	double aspectRatioChange = newAspectRatio / aspectRatio;
	if (aspectRatioChange != 1.0) {
		getCamera()->getProjectionMatrix() *= osg::Matrix::scale(1.0 / aspectRatioChange, 1.0, 1.0);
		float camera_fov = -1.0f;
		if (camera_fov > 0.0f)
		{
			double fovy, aspectRatio, zNear, zFar;
			getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

			double original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;

			fovy = atan(tan(osg::DegreesToRadians(camera_fov)*0.5) / aspectRatio)*2.0;
			getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

			getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
			original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
		}
	}
}
rcSyncImp* rcrenderer::getImp() {
	return m_imp.get();
}

bool rcrenderer::getStatus()
{
	return m_isExit;
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
	m_isExit = true;
	syncModule->sync(this);
	return 0;
}

osg::ref_ptr<osg::Node> rcrenderer::getNode()
{
	return m_root.get();
}
void rcrenderer::adjustPara()
{
	ParallaxPara& p = getParaParameter();
	//Specified algorithm to adjust the parallax.
	//TODO:
	const osg::BoundingSphere& bs = getNode()->getBound();
	osg::Matrix manipulator(getCameraManipulator()->getMatrix());
	osg::Matrix modelView(getCamera()->getViewMatrix());
	osg::Matrix projection(getCamera()->getProjectionMatrix());
	osg::Matrix rot = osg::Matrix::rotate(osg::DegreesToRadians(p._rotate_angle), 0.0f, 0.0f, 1.0f);

	osg::Matrix newView = rot*modelView;
	newView.makeTranslate(p._cam_no*p._para_c, 0, 0);
	newView = modelView*newView;

	double viewDistance = (newView.getTrans() - bs.center()).length();
	osg::Matrixd para 
		(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		p._cam_no*p._para_c / viewDistance, 0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);

	if (getImp()->getType() == _SLAVE) {
		//osg::Matrix invMat = osg::Matrix::inverse(manipulator);
		getCameraManipulator()->setByMatrix(manipulator*rot);
		//getCamera()->setViewMatrix(newView);
	}
	getCamera()->setProjectionMatrix(para*projection);
}
ParallaxPara& rcrenderer::getParaParameter() {
	return m_para;
}

rcEventHandler::rcEventHandler() :osgGA::GUIEventHandler()
{

}
rcEventHandler::~rcEventHandler() { }
bool rcEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	osg::ref_ptr<osgViewer::Viewer> pViewer = dynamic_cast<osgViewer::Viewer*>(&us);
	if (!pViewer.valid())
		return false;
	osg::ref_ptr<rcrenderer> pRender = reinterpret_cast<rcrenderer*>(pViewer.get());
	if (!pRender.valid())
		return false;
	ParallaxPara& p = pRender->getParaParameter();
	switch (ea.getEventType())
	{
	case osgGA::GUIEventAdapter::KEYDOWN:
	{
			switch (ea.getKey())
			{
			case osgGA::GUIEventAdapter::KEY_Left:
				p._para_c += 0.5;
				break;
			case osgGA::GUIEventAdapter::KEY_Right:
				p._para_c -= 0.5;
				break;
			case osgGA::GUIEventAdapter::KEY_Up:
				p._rotate_angle += 0.1;
				break;
			case osgGA::GUIEventAdapter::KEY_Down:
				p._rotate_angle -= 0.1;
				break;
			default:
				break;
			}
	}
		break;
	default:
		break;
	}
	return osgGA::GUIEventHandler::handle(ea, us);
}
