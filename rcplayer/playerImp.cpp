#include "StdAfx.h"
#include "playerImp.h"
#include <osgDB/ReaderWriter>
#include <osgDB/Registry>
void saveBMP(int width, int height, int channel, BYTE* data, char* filename, int pixelFormat = GL_BGR)
{
	osg::ref_ptr<osgDB::ReaderWriter> m_bmpWirter;
	m_bmpWirter = osgDB::Registry::instance()->getReaderWriterForExtension("bmp");
	osg::ref_ptr<osg::Image> img = new osg::Image();
	img->allocateImage(width, height, channel, pixelFormat, GL_BYTE);
	BYTE* p = img->data(0, 0);
	memcpy(p, data, width*height * channel);
	m_bmpWirter->writeImage(*img, filename);
}

vlcImp::vlcImp() :osg::ImageStream(), RCPLAYER_API(), playerImp()
{
	init();
}

vlcImp::vlcImp(const vlcImp& copy, const osg::CopyOp& op) : RCPLAYER_API(), osg::ImageStream(copy, op), playerImp()
{
	init();
	m_vlc = copy.m_vlc;
	m_vlcMedia = copy.m_vlcMedia;
	m_vlcPlayer = copy.m_vlcPlayer;
}

int vlcImp::nextFrame()
{
	return RCPLAYER_API::nextFrame();
}
void vlcImp::bindTexSrc(osg::Texture2D* texture)
{
	//Texture Binding
	texture->setImage(this);
}
void vlcImp::Start()
{
	rcplay();
}
void	vlcImp::Stop()
{
	rcstop();
}


void vlcImp::imageDirty()
{
	dirty();
}

void vlcImp::Play()
{
	__STD_PRINT("%s\n", "Play");
	rcplay();
}
void vlcImp::Pause()
{
	__STD_PRINT("%s\n", "Pause");
	rcpause();
}

void vlcImp::play()
{
	Play();
	_status = PLAYING;
}

void vlcImp::pause()
{
	Pause();
	_status = PAUSED;
}
void vlcImp::stop()
{
	Stop();
	_status = INVALID;
}
void vlcImp::updateTex()
{
	BYTE* p = data();
	unsigned int rowStepInBytes = getRowStepInBytes();
	unsigned int imageStepInBytes = getImageStepInBytes();
	for (int j = 0; j < m_height; j++)
	{
		memcpy(p + (m_height - 1 - j)*rowStepInBytes, m_frameBuf + j*rowStepInBytes, rowStepInBytes);
	}
#if 0
	static int cnt = 0;
	if (cnt++ < 100)
	{
		char buf[30];
		sprintf(buf, "%d.bmp", cnt);
		saveBMP(m_width, m_height, 4, p, buf);
	}
#endif
}

void vlcImp::init()
{
	m_vlcMedia = NULL;
	m_vlc = NULL;
	m_vlcMedia = NULL;
	m_vlcPlayer = NULL;
	m_isSleep = false;
}
int vlcImp::initPlayer(const char* const* vlc_argv, const int argc)
{
	static int isInited = false;

	if (isInited)
		return 1;
	isInited = true;

	if (!vlc_argv)
	{
		const char* vlc_args[] = {
			"--ignore-config",
			"--quiet",
			"--no-video-title-show",
			//"--network-caching=120"
		};

		m_vlc = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
	}
	else
	{
		m_vlc = libvlc_new(argc, vlc_argv);
	}

	if (!m_vlc)
	{

		__STD_PRINT("%s\n", "Failed to Init VLCInit.Exit.");
		exit(0);
	}


	//Use libvlc_media_new_from_media instead
#if 0
	m_vlcPlayer = libvlc_media_player_new(m_vlc);
	if (!m_vlcPlayer)
	{
		__STD_PRINT("%s\n", "Failed to Init VLCMedia.Exit.");
		exit(0);
	}

	libvlc_event_attach(libvlc_media_player_event_manager(m_vlcPlayer), libvlc_MediaPlayerEndReached, &vlcImp::videoEndFunc, this);
#endif
	_status = INVALID;
	return 1;
}

int vlcImp::open(const char* filename)
{

	FILE* tryOpen = fopen(filename, "r");
	if (tryOpen == NULL)
	{
		__STD_PRINT("%s\n", "Failed to Open File.Exit.");
		exit(0);
	}
	else
	{
		fclose(tryOpen);
	}
	if (strstr(filename, "avi") == NULL && strstr(filename, "bmp") == NULL)
	{
		release();
		exit(0);
	}

	m_vlcMedia = libvlc_media_new_path(m_vlc, filename);
	if (!m_vlcMedia)
	{
		__STD_PRINT("%s\n", "Failed to Init Media.Exit.");
		exit(0);
	}

	m_vlcMedia = libvlc_media_new_path(m_vlc, filename);
	if (!m_vlcMedia)
	{
		__STD_PRINT("%s\n", "Failed to Open File.Exit.");
		exit(0);
	}

	m_vlcPlayer = libvlc_media_player_new_from_media(m_vlcMedia);

	if (!m_vlcPlayer)
	{
		__STD_PRINT("%s\n", "Failed to Init VLCMedia.Exit.");
		exit(0);
	}

	libvlc_event_attach(libvlc_media_player_event_manager(m_vlcPlayer), libvlc_MediaPlayerEndReached, &vlcImp::videoEndFunc, this);

#if 0
	libvlc_media_player_set_media(m_vlcPlayer, m_vlcMedia);
#endif

	libvlc_video_set_format(m_vlcPlayer, "RGBA", m_width, m_height, m_width * 4);

	allocateImage(m_width, m_height, 4, GL_RGBA, GL_UNSIGNED_BYTE);
	m_frameBuf = (BYTE*)calloc(1, getImageSizeInBytes());

	libvlc_video_set_callbacks(m_vlcPlayer, &vlcImp::lockFunc, &vlcImp::unlockFunc, &vlcImp::displayFunc, m_frameBuf);
	m_filename = filename;
	//libvlc_media_player_set_rate(m_vlcPlayer, 30.0);
	__STD_PRINT("Opened %s Successfully.\n", filename);

	return 1;

}
void vlcImp::syncStart()
{

	if (m_targetTime)
	{
		SYSTEMTIME curSysTime;
		GetLocalTime(&curSysTime);
		FILETIME curFileTime;
		SystemTimeToFileTime(&curSysTime, &curFileTime);
		ULARGE_INTEGER slave;
		slave.HighPart = curFileTime.dwHighDateTime;
		slave.LowPart = curFileTime.dwLowDateTime;

		ULONGLONG sleepTime = (m_targetTime - slave.QuadPart) / 10000.0;

		__STD_PRINT("Now sleep for %I64u ms\n", sleepTime);
		__LOG_FORMAT(_PLAYER_LOG, "Now sleep for %I64u ms\n", sleepTime);

		if (sleepTime < 0)
			sleepTime = 0;
		if (sleepTime > 30 * 1000.0)
			sleepTime = 30 * 1000.0;
		Sleep(sleepTime);

	}

}
void vlcImp::open(const std::string& file, bool needPlay, unsigned int w, unsigned int h)
{

	setResolution(w, h);

	open(file.c_str());

	if (needPlay)
		play();

}


void vlcImp::rewind()
{
	libvlc_media_player_stop(m_vlcPlayer);
	libvlc_media_player_set_time(m_vlcPlayer, 0);
	libvlc_media_player_next_frame(m_vlcPlayer);
	_status = INVALID;
}

void vlcImp::quit(bool waitForThreadToExit)
{
	libvlc_media_player_stop(m_vlcPlayer);

	libvlc_media_player_release(m_vlcPlayer);

	_status = INVALID;
}

void vlcImp::setReferenceTime(double time)
{
	libvlc_media_player_set_time(m_vlcPlayer, (int)time);
}
double vlcImp::getReferenceTime() const
{
	return (int)libvlc_media_player_get_time(m_vlcPlayer);
}

void vlcImp::setTimeMultiplier(double m)
{
	libvlc_video_set_scale(m_vlcPlayer, m);
}
double vlcImp::getTimeMultiplier() const
{
	return libvlc_video_get_scale(m_vlcPlayer);
}

void vlcImp::setVolume(float vol)
{
	libvlc_audio_set_volume(m_vlcPlayer, (int)vol);
}
float vlcImp::getVolume() const
{
	return (int)libvlc_audio_get_volume(m_vlcPlayer);
}

const char* vlcImp::getFilename()
{
	return m_filename.c_str();
}
void vlcImp::updateTexture()
{
	//Here we need to flip the frame vertically.
	BYTE* p = data();
	unsigned int rowStepInBytes = getRowStepInBytes();
	unsigned int imageStepInBytes = getImageStepInBytes();
	for (int j = 0; j < m_height; j++)
	{
		memcpy(p + (m_height - 1 - j)*rowStepInBytes, m_frameBuf + j*rowStepInBytes, rowStepInBytes);
	}
}


int vlcImp::isLocked()
{
	return m_isLocked == true;
}

void vlcImp::setTargetTime(ULONGLONG targetTime)
{
	m_targetTime = targetTime;
}



void* vlcImp::lockFunc(void* data, void** p_pixels)
{
#if 0 
	char temp[30];
	SYSTEMTIME time;
	GetLocalTime(&time);
	_STD_ENCODE_TIMESTAMP(temp, time);
	__STD_PRINT("Lock: %s\n",temp);
#endif

	BYTE* buf = (BYTE*)data;
	*p_pixels = buf;
	if (1)
	{
#ifdef _FPS_STAT
		static DWORD frameCnt = 0;
		static float fps = 0.0;
		static DWORD last = GetTickCount();
		static DWORD elapse_time = 0;
		++frameCnt;
		DWORD now = GetTickCount();
		elapse_time = now - last;
		if (now - last >= 1000)
		{
			fps = frameCnt;
			__STD_PRINT("FPS: %.0f\n", fps);
			frameCnt = 0;
			last = now;
		}
#endif
	}

	return NULL;
}

void vlcImp::unlockFunc(void* data, void* id, void* const* p_pixels)
{
	BYTE* buf = (BYTE*)data;
	//vlcImp* stream = (vlcImp*)data;
#if 0 
	char temp[30];
	SYSTEMTIME time;
	GetLocalTime(&time);
	_STD_ENCODE_TIMESTAMP(temp, time);
	__STD_PRINT("UnLock: %s\n", temp);
#endif
}

void vlcImp::displayFunc(void* data, void* id)
{

	vlcImp* stream = (vlcImp*)data;
}

void vlcImp::videoEndFunc(const libvlc_event_t*, void* data)
{

	vlcImp* stream = (vlcImp*)data;
	stream->setPosition(0.1);
	stream->play();
}

int cvImp::open(std::string filename)
{
	FILE* tryOpen = fopen(filename.c_str(), "r");
	if (tryOpen == NULL)
	{
		__STD_PRINT("%s\n", "Failed to Open File.Exit.");
		exit(0);
	}
	else
	{
		fclose(tryOpen);
	}
	m_videoDevice.open(filename);
	if (!m_videoDevice.isOpened())
	{
		__STD_PRINT("%s\n", "opencv video capture: failed to open the media.")
			exit(0);
	}
	m_filename = filename;
	m_srcWidth = m_videoDevice.get(CV_CAP_PROP_FRAME_WIDTH);
	m_srcHeight = m_videoDevice.get(CV_CAP_PROP_FRAME_HEIGHT);
	m_srcChannels = 4;
	m_srcFrameCnts = m_videoDevice.get(CV_CAP_PROP_FRAME_COUNT);
	if (strstr(filename.c_str(), "bmp") ||
		strstr(filename.c_str(), "jpeg") ||
		strstr(filename.c_str(), "jpg") ||
		strstr(filename.c_str(), "png"))
		m_type = IMAGE;
	else if (strstr(filename.c_str(), "avi"))
		m_type = VIDEO;
	else
	{
		__STD_PRINT("%s\n", "unknown media format.");
		exit(0);
	}
	allocateImage(m_srcWidth, m_srcHeight, 3, GL_BGR, GL_UNSIGNED_BYTE);
	setInternalTextureFormat(GL_BGR);
	return 1;
}

void cvImp::setTargetTime(ULONGLONG targetTime)
{
	m_targetTime = targetTime;
}
int cvImp::nextFrame()
{
	//Loop
	if (m_frameIndex >= m_srcFrameCnts)
	{
		m_videoDevice.open(m_filename);
		m_frameIndex = 0;
	}
	/*
	*Only update video and keep the images the same.
	*/
	if (m_type == VIDEO)
		m_videoDevice >> m_frame;
	else if (m_frameIndex == 0)
		m_videoDevice >> m_frame;

	m_frameIndex++;
	return 1;
}
void cvImp::syncStart()
{

	if (m_targetTime)
	{
		SYSTEMTIME curSysTime;
		GetLocalTime(&curSysTime);
		FILETIME curFileTime;
		SystemTimeToFileTime(&curSysTime, &curFileTime);
		ULARGE_INTEGER slave;
		slave.HighPart = curFileTime.dwHighDateTime;
		slave.LowPart = curFileTime.dwLowDateTime;

		ULONGLONG sleepTime = (m_targetTime - slave.QuadPart) / 10000.0;

		__STD_PRINT("Now sleep for %I64u ms\n", sleepTime);
		__LOG_FORMAT(_PLAYER_LOG, "Now sleep for %I64u ms\n", sleepTime);

		if (sleepTime < 0)
			sleepTime = 0;
		if (sleepTime > 30 * 1000.0)
			sleepTime = 30 * 1000.0;
		Sleep(sleepTime);

	}
}
int cvImp::getFrameIndex()
{
	return m_frameIndex;
}

int cvImp::getTotalFrameCnts()
{
	return m_srcFrameCnts;
}
void cvImp::bindTexSrc(osg::Texture2D* texImg)
{
	//if (!m_image.valid())
	//{
	//	__STD_PRINT("%s\n", "image invalid");
	//	exit(0);
	//}
	m_dstWidth = texImg->getTextureWidth();
#if 0
	if (m_dstWidth != m_srcWidth)
	{
		__STD_PRINT("%s\n", "Image width not match.");
		exit(0);
	}
#endif
	m_dstHeight = texImg->getTextureHeight();
#if 0
	if (m_dstHeight != m_srcHeight)
	{
		__STD_PRINT("%s\n", "Image height not match.");
		exit(0);
	}
#endif
	/*
	*We set the format of the texture to "RGBA" by default.
	*/
	m_dstChannels = 4;
#if 0
	if (m_dstChannels != m_srcChannels)
	{
		__STD_PRINT("%s\n", "Image channel not match.");
		exit(0);
	}
#endif
	texImg->setImage(this);

}
void cvImp::updateTex()
{
	BYTE* dst = data();
	BYTE* src = m_frame.data;
	assert(src.data = NULL);

	int step = m_frame.channels()*m_srcWidth;
	for (int i = 0; i < m_frame.rows; i++)
		memcpy(dst + i*step, m_frame.ptr(m_srcHeight - 1 - i), step);
	//memcpy(dst, src, getImageSizeInBytes());

}
void cvImp::Start()
{
	__STD_PRINT("%s\n", "opencv video capture:start.\tNot implemented.");
}


void cvImp::Pause()
{
	__STD_PRINT("%s\n", "opencv video capture:pause.\tNot implemented.");
}

void cvImp::Stop()
{
	__STD_PRINT("%s\n", "opencv video capture:stop.\tNot implemented.");
}

/*
*Desigened for osg::Image,which will use this function call to update the frame buffer inside it.
*So,as you can see,this is just a wrapper for that.
*/
void cvImp::imageDirty()
{
	if (m_type == VIDEO)
		dirty();
}

