
#include "stdafx.h"
#include "osg/ImageStream"
#include "osg/Texture2D"
#include "player.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
class playerImp
{
public:
	playerImp(){ return; }
	~playerImp(){ return; }
	virtual int nextFrame() = 0;
	virtual void syncFrame(ULONGLONG& current, const LONGLONG target){ return; };
	virtual void bindTexSrc(osg::Texture2D*) { return; }
	virtual void bindTexSrc(BYTE*) { return; }
	virtual void updateTex() = 0;
	virtual void Start() = 0;
	virtual void Pause() = 0;
	virtual void Stop() = 0;
	virtual void imageDirty() { return; }
	virtual void setName(const char* name) { m_nameImp = name; }
private:
	std::string m_nameImp;
};


class vlcImp : public RCPLAYER_API, public playerImp, public osg::ImageStream
{
	friend class impFactory;
protected:
	vlcImp(const vlcImp& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY);
	META_Object(osg, vlcImp)
	vlcImp();
public:
	//API for playerImp
	virtual int nextFrame();
	virtual void bindTexSrc(osg::Texture2D* texture);
	virtual void Start();
	virtual void Stop();
	virtual void Pause();
	virtual void Play();
	virtual void updateTex();
	virtual void imageDirty();
	//API for RCPLAYER and ImageStream
	virtual int initPlayer(const char* const* vlc_argv = 0, const int argc = 0);
	virtual int open(const char* filename);
	void open(const std::string& file, bool needPlay = true, unsigned int w = 512, unsigned int h = 512);
	static void* lockFunc(void* data, void** p_pixels);
	static void unlockFunc(void* data, void* id, void* const* p_pixels);
	static void displayFunc(void* data, void* id);
	static void videoEndFunc(const libvlc_event_t*, void* data);
	void init();
	int isLocked();
	void setTargetTime(ULONGLONG targetTime);
	void syncStart();
	void updateTexture();
	static void sleep() { Sleep(1); }
	void setSleep(int status) { m_isSleep = status; }
	virtual void play();
	virtual void pause();
	virtual void stop();
	virtual void rewind();
	virtual void quit(bool waitForThreadToExit = true);
	virtual void setReferenceTime(double time);
	virtual double getReferenceTime() const;
	virtual void setTimeMultiplier(double m);
	virtual double getTimeMultiplier() const;
	virtual void setVolume(float vol);
	virtual float getVolume() const;
	const char* getFilename();

private:
	ULONGLONG m_targetTime;
	std::string m_filename;
	int m_isLocked;
	int m_isFliped;
	BYTE* m_frameBuf;
	int m_isSleep;
};
class cvImp :public playerImp, public osg::ImageStream
{
	friend class impFactory;

	enum DATA_FORMT
	{
		IMAGE,
		VIDEO
	};
public:

	int open(std::string filename);
	void syncStart();
	int getFrameIndex();
	void setTargetTime(ULONGLONG targetTime);
	int getTotalFrameCnts();
	virtual int nextFrame();
	virtual void syncFrame(ULONGLONG& current, const LONGLONG target);
	virtual	void bindTexSrc(osg::Texture2D* texImg);
	virtual	void bindTexSrc(BYTE* buf) { return; }
	virtual	void updateTex();
	virtual	void Start();
	virtual	void Pause();
	virtual	void Stop();
	virtual void imageDirty();
protected:
	cvImp(const cvImp& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY)
	{

	}
	META_Object(osg, cvImp)
	cvImp() : playerImp(), osg::ImageStream()
	{
		m_frameIndex = 0;
	}
private:
	int m_srcWidth, m_srcHeight, m_srcChannels;
	int m_dstWidth, m_dstHeight, m_dstChannels;
	DATA_FORMT m_type;
	std::string m_filename;
	cv::Mat  m_frame;
	cv::VideoCapture m_videoDevice;
	int m_srcFrameCnts;
	int m_frameIndex;
	ULONGLONG m_targetTime;
};

class impFactory
{
public:
	static impFactory* instance()
	{
		static impFactory factoryInst;
		return &factoryInst;
	}
	playerImp* createVLCImp()
	{

		std::unique_ptr<vlcImp> imp(new vlcImp);
		return imp.release();
	}
	playerImp* createOpenCVImp()
	{
		std::unique_ptr<cvImp> imp(new cvImp);
		return imp.release();
	}
protected:
	impFactory()
	{

	}
	~impFactory()
	{

	}
};