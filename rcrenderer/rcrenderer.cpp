#include "stdafx.h"
#include "rcrenderer.h"
#include <vector>
#include <string>
#include <stdlib.h>
#include <string>
#include <exception>
rcrenderer::~rcrenderer() {
	m_isExit = true;
	m_viewer.release();
}

rcrenderer::rcrenderer(int width, int height, char* keystoneFileName) :
rcApp(), m_width(width), m_height(height), m_keystoneFileName(keystoneFileName), m_isExit(false)
{
	m_viewer = new osgViewer::Viewer();
	m_root = new osg::MatrixTransform;
}
rcrenderer::rcrenderer(rcSyncImp* syncImp, int width, int height, char* keystoneFileName) :
rcApp(), m_width(width), m_height(height), m_keystoneFileName(keystoneFileName), m_isExit(false)
{
	m_viewer = new osgViewer::Viewer();
	m_root = new osg::MatrixTransform;
	if (syncImp != NULL)
		m_imp = std::unique_ptr<rcSyncImp>(syncImp);
}
rcrenderer::rcrenderer(osg::Node* node, rcSyncImp* syncImp, int width, int height, char* keystoneFileName) :
rcApp(), m_width(width), m_height(height), m_keystoneFileName(keystoneFileName), m_isExit(false)
{
	m_viewer = new osgViewer::Viewer();
	m_root = new osg::MatrixTransform;
	if (node != NULL)
		m_root->addChild(node);
	if (syncImp != NULL)
		m_imp = std::unique_ptr<rcSyncImp>(syncImp);
}
osgViewer::Viewer* rcrenderer::getViewer()
{
	return m_viewer.get();
}
void rcrenderer::setup()
{
	setupRenderer(m_width, m_height, m_keystoneFileName);
	getRootNode()->setName("RcRootNode");
}
osg::Node* rcrenderer::getRootNode() {
	return m_root.get();
}
bool rcrenderer::getStatus() {
	return m_isExit;
}
#define _RC_RENDER_TEST
void rcrenderer::setupRenderer(int width, int height, const char* keystoneFilename)
{
#ifdef _RC_RENDER_TEST
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
	traits->x = 50;
	traits->y = 50;
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
	m_viewer->getCamera()->setViewport(0, 0, traits->width, traits->height);
	m_viewer->getCamera()->setGraphicsContext(gc.get());
	osg::DisplaySettings* ds = osg::DisplaySettings::instance();

	std::vector<std::string> keystones;
	keystones.push_back(keystoneFilename);
	ds->setKeystoneFileNames(keystones);
	m_viewer->setSceneData(m_root.get());
	osg::Camera* maniCamera = dynamic_cast<osg::Camera*>(m_viewer->getCameraManipulator());
	//apply keystone calibration
	if (keystoneFilename)
	{
		osg::ref_ptr<osgViewer::Keystone> keystone = osgDB::readFile<osgViewer::Keystone>(keystoneFilename);
		ds->setKeystoneHint(true);
		bool keystonesLoaded = true;
		ds->getKeystones().push_back(keystone.get());
		if (maniCamera)
			m_viewer->assignStereoOrKeystoneToCamera(maniCamera, ds);
		else
			m_viewer->assignStereoOrKeystoneToCamera(m_viewer->getCamera(), ds);
	}

	double fovy, aspectRatio, zNear, zFar;
	m_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
	double newAspectRatio = double(traits->width) / double(traits->height);
	double aspectRatioChange = newAspectRatio / aspectRatio;
	if (aspectRatioChange != 1.0) {
		m_viewer->getCamera()->getProjectionMatrix() *= osg::Matrix::scale(1.0 / aspectRatioChange, 1.0, 1.0);
		float camera_fov = -1.0f;
		if (camera_fov > 0.0f)
		{
			double fovy, aspectRatio, zNear, zFar;
			m_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

			double original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;

			fovy = atan(tan(osg::DegreesToRadians(camera_fov)*0.5) / aspectRatio)*2.0;
			m_viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);

			m_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, zNear, zFar);
			original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
		}
	}
	m_viewer->addEventHandler(new rcEventHandler(this));
}
void rcrenderer::setSyncImp(rcSyncImp* syncImp)
{
	m_imp.reset(syncImp);
}
std::ostream& operator<<(std::ostream& out, osg::Matrixd& mat)
{
	double* p = mat.ptr();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			out << p[4 * i + j] << " ";
		out << std::endl;
	}
	return out;
}
class TraverseCallback : public osg::NodeCallback
{
public:
	TraverseCallback(rcrenderer* render)
	{
		m_render.reset(render);
	}
	~TraverseCallback()
	{
		m_render.reset();
	}
	void adjustPara(osg::MatrixTransform* transform)
	{
		ParallaxPara& p = m_render->getParaParameter();
		//osg::Matrix rotChange= osg::Matrix::rotate(osg::DegreesToRadians(p._rotate_angle), 0.0f, 0.0f, 1.0f);
		const osg::BoundingSphere& bs = m_render->getRootNode()->getBound();
		osg::Matrix mat = transform->getMatrix();
		osg::Vec3d trans, scale;
		osg::Matrixd change;
		osg::Quat rot, so;
		mat.decompose(trans, rot, scale, so);
		trans += osg::Vec3d(p._cam_no*p._para_c, 0, 0);
		change.makeTranslate(trans);
		mat = mat*change;

		double viewDistance = (mat.getTrans() - bs.center()).length();
		osg::Matrixd para
			(
			1.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			p._cam_no*p._para_c / viewDistance, 0, 1.0, 0.0,
			0.0, 0.0, 0.0, 1.0
			);
		mat = para*mat;
		transform->setMatrix(mat);
	}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osg::MatrixTransform* trans =
			static_cast<osg::MatrixTransform*>(node);
		std::cout << node->getName() << std::endl;
		TYPE type = m_render->getImp()->getType();
		if (trans&&type == _SLAVE) {
			//adjustPara(trans);
		}
		traverse(node, nv);
	}
	std::unique_ptr<rcrenderer> m_render;
};
class rcUpdate :public osg::NodeVisitor
{
public:
	rcUpdate(rcrenderer* render)
	{
		m_render = render;
		setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
	}
	~rcUpdate() {
		m_render = nullptr;
	}
	void apply(osg::Node& node)
	{
		if (strstr(node.getName().c_str(), "rcTileMapScene"))
			node.setUpdateCallback(new TraverseCallback(m_render));
		traverse(node);
	}
private:
	rcrenderer* m_render;
};
rcSyncImp* rcrenderer::getImp() {
	return m_imp.get();
}
int rcrenderer::run()
{
	osg::ref_ptr<osgViewer::Viewer> viewer = getViewer();
	viewer->realize();
	rcSyncImp* syncModule = getImp();
	//rcUpdate rcUpdateVisitor(this);
	//getRootNode()->accept(rcUpdateVisitor);
	while (!viewer->done())
	{
		viewer->advance();
		syncModule->sync(this);
		adjustPara();
		viewer->frame();
		viewer->eventTraversal();
		viewer->updateTraversal();
		viewer->renderingTraversals();
	}
	m_isExit = true;
	syncModule->sync(this);
	return 0;
}

void rcrenderer::adjustPara()
{
	osg::ref_ptr<osgViewer::Viewer> viewer = getViewer();
	ParallaxPara& p = getParaParameter();
	//Specified algorithm to adjust the parallax.
	//TODO:
	const osg::BoundingSphere& bs = getRootNode()->getBound();
	osg::Matrix manipulator(viewer->getCameraManipulator()->getMatrix());
	osg::Matrix modelView(viewer->getCamera()->getViewMatrix());
	osg::Matrix projection(viewer->getCamera()->getProjectionMatrix());
	osg::Matrix rot = osg::Matrix::rotate(osg::DegreesToRadians(p._rotate_angle), 0.0f, 0.0f, 1.0f);
	osg::Matrix newView;
	newView.makeTranslate(p._cam_no*p._para_c, 0, 0);
	newView = rot*manipulator*newView;
	double viewDistance = (newView.getTrans() - bs.center()).length();
	osg::Matrixd para
		(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		p._cam_no*p._para_c / viewDistance, 0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);

	if (getImp()->getType() == _SLAVE) {
		viewer->getCameraManipulator()->setByMatrix(newView);
		viewer->getCamera()->setProjectionMatrix(para*projection);
	}
}

ParallaxPara& rcrenderer::getParaParameter() {
	return m_para;
}

rcEventHandler::rcEventHandler(rcrenderer* render) :osgGA::GUIEventHandler()
{
	m_render.reset(render);
}
rcEventHandler::~rcEventHandler()
{
	if (m_render.get())
		m_render.reset();
}
bool rcEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	if (!m_render.get())
		return false;
	ParallaxPara& p = m_render->getParaParameter();
	switch (ea.getEventType())
	{
	case osgGA::GUIEventAdapter::KEYDOWN:
	{
											switch (ea.getKey())
											{
											case osgGA::GUIEventAdapter::KEY_L:
												p._para_c += 1.0;
												break;
											case osgGA::GUIEventAdapter::KEY_J:
												p._para_c -= 1.0;
												break;
											case osgGA::GUIEventAdapter::KEY_I:
												p._rotate_angle += 1.0;
												break;
											case osgGA::GUIEventAdapter::KEY_K:
												p._rotate_angle -= 1.0;
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
