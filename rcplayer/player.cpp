#include "StdAfx.h"
#include "player.h"

int RCPLAYER::m_isLocked = -1;
int RCPLAYER::m_isFliped = 0;
int RCPLAYER::m_isSleep = 0;
RCPLAYER_API::RCPLAYER_API()
{

}


void RCPLAYER_API::release()
{
	if (m_vlc)
		libvlc_release(m_vlc);
	if (m_vlcMedia)
		libvlc_media_release(m_vlcMedia);
	if (m_vlcPlayer)
		libvlc_media_player_release(m_vlcPlayer);
}
RCPLAYER_API::~RCPLAYER_API()
{

}
libvlc_state_t RCPLAYER_API::getState()
{
	return libvlc_media_player_get_state(m_vlcPlayer);
}
int RCPLAYER_API::rcplay()
{
	//libvlc_state_t state = getState();
	//switch (state)
	//{
	//case libvlc_Paused:
	//	libvlc_media_player_play(m_vlcPlayer);
	//	break;
	//case libvlc_Playing:
	//	libvlc_media_player_set_pause(m_vlcPlayer, true);
	//	break;
	//case libvlc_Ended:
	//	libvlc_media_player_play(m_vlcPlayer);
	//	break;
	//default:
	//	return 0;
	//}
	libvlc_state_t state;
	libvlc_media_player_play(m_vlcPlayer);
	while (1)
	{
		state = libvlc_media_player_get_state(m_vlcPlayer);
#ifdef _VLC_IMPLEMENTATION
		__STD_PRINT("%s\n", "waiting");
#endif
		if (state == libvlc_Playing)
			break;
	}
	return 1;
}

int RCPLAYER_API::rcpause()
{
	libvlc_state_t state;
rcpause_redo:	
	while (1)
	{
#ifdef _VLC_IMPLEMENTATION
		__STD_PRINT("%s\n", "RCPAUSE");
#endif
		if (libvlc_media_player_can_pause(m_vlcPlayer))
		{
			break;
		}
	}
	libvlc_media_player_set_pause(m_vlcPlayer,true);
	//libvlc_media_player_pause(m_vlcPlayer);
	state = libvlc_media_player_get_state(m_vlcPlayer);
	if (state == libvlc_Paused)
		goto rcpause_end;
	else
		goto rcpause_redo;
rcpause_end:
	return 1;
}
int RCPLAYER_API::rcstop()
{
	libvlc_state_t state = getState();
	switch (state)
	{
	case libvlc_Playing:

	case libvlc_Paused:
		libvlc_media_player_stop(m_vlcPlayer);
		break;
	default:
		return 0;
	}
	return 1;
}
int RCPLAYER_API::nextFrame()
{
	libvlc_media_player_next_frame(m_vlcPlayer);
	return 1;
}
float RCPLAYER_API::getPosition()
{
	return libvlc_media_player_get_position(m_vlcPlayer);
}
int RCPLAYER_API::setPosition(float pos)
{
	libvlc_media_player_set_position(m_vlcPlayer, pos);
	return 1;

}
libvlc_media_player_t* RCPLAYER_API::getMediaPlayer() const
{
	return m_vlcPlayer;
}

int RCPLAYER_API::isPlaying()
{
	return (getState() == libvlc_Playing);
}


int RCPLAYER_API::setResolution(int w, int h)
{
	m_width = w;
	m_height = h;
	return 1;
}


RCPLAYER* RCPLAYER::m_inst = new RCPLAYER;

RCPLAYER* RCPLAYER::instance()
{
	if (!m_inst)
	{
		m_inst = new RCPLAYER;
	}
	return m_inst;
}

RCPLAYER::RCPLAYER() : osg::ImageStream(), THREAD(), rcmutex()
{
	init();
}

RCPLAYER::RCPLAYER(const RCPLAYER& copy, const osg::CopyOp& op) : RCPLAYER_API(), osg::ImageStream(copy, op), THREAD(), rcmutex()
{
	init();
	m_vlc = copy.m_vlc;
	m_vlcMedia = copy.m_vlcMedia;
	m_vlcPlayer = copy.m_vlcPlayer;

}
void RCPLAYER::init()
{
	m_vlcMedia = NULL;
	m_vlc = NULL;
	m_vlcMedia = NULL;
	m_vlcPlayer = NULL;
	m_isSleep = false;
	initMutex(new MUTEX(MUTEX::MUTEX_NORMAL));
}
int RCPLAYER::initPlayer(const char* const* vlc_argv, const int argc)
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
	m_vlcPlayer = libvlc_media_player_new(m_vlc);
	if (!m_vlcPlayer)
	{
		__STD_PRINT("%s\n", "Failed to Init VLCMedia.Exit.");
		exit(0);
	}

	libvlc_event_attach(libvlc_media_player_event_manager(m_vlcPlayer), libvlc_MediaPlayerEndReached, &RCPLAYER::videoEndFunc, this);

	_status = INVALID;

	return 1;
}

int RCPLAYER::open(const char* filename)
{

	FILE* tryOpen = fopen(filename,"r");
	if (tryOpen == NULL)
	{
		__STD_PRINT("%s\n", "Failed to Open File.Exit.");
		exit(0);
	}
	else
	{
		fclose(tryOpen);
	}
	m_vlcMedia = libvlc_media_new_path(m_vlc, filename);
	if (!m_vlcMedia)
	{
		__STD_PRINT("%s\n", "Failed to Init Media.Exit.");
		exit(0);
	}

	libvlc_media_player_set_media(m_vlcPlayer, m_vlcMedia);
	libvlc_video_set_format(m_vlcPlayer, "RGBA", m_width, m_height, m_width * 4);
	allocateImage(m_width, m_height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	m_frameBuf = (BYTE*)calloc(1, getImageSizeInBytes());

	libvlc_video_set_callbacks(m_vlcPlayer, &RCPLAYER::lockFunc, &RCPLAYER::unlockFunc, &RCPLAYER::displayFunc, m_frameBuf);
	
	m_filename = filename;
	__STD_PRINT("Opened %s Successfully.\n", filename);

	return 1;

}
void RCPLAYER::syncStart()
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
void RCPLAYER::open(const std::string& file, bool needPlay, unsigned int w, unsigned int h)
{

	setResolution(w, h);

	open(file.c_str());


	if (needPlay)
		play();

}

void RCPLAYER::play()
{
	//if (_status == PAUSED)
	//{
	//	libvlc_media_player_set_pause(m_vlcPlayer, false);

	//}
	//else if (_status != PLAYING)
	//{
	//	libvlc_media_player_play(m_vlcPlayer);
	//}
	if (_status!=PLAYING)
	{
		libvlc_media_player_play(m_vlcPlayer);
		_status = PLAYING;
	}
	else
	{
		libvlc_media_player_set_pause(m_vlcPlayer, true);
		_status = PAUSED;
	}

}


void RCPLAYER::pause()
{
	rcpause();

	_status = PAUSED;
}

void RCPLAYER::rewind()
{
	libvlc_media_player_stop(m_vlcPlayer);
	libvlc_media_player_set_time(m_vlcPlayer, 0);
	libvlc_media_player_next_frame(m_vlcPlayer);
	_status = INVALID;
}

void RCPLAYER::quit(bool waitForThreadToExit)
{
	libvlc_media_player_stop(m_vlcPlayer);

	libvlc_media_player_release(m_vlcPlayer);

	_status = INVALID;
}

void RCPLAYER::setReferenceTime(double time)
{
	libvlc_media_player_set_time(m_vlcPlayer, (int)time);
}
double RCPLAYER::getReferenceTime() const
{
	return (int)libvlc_media_player_get_time(m_vlcPlayer);
}

void RCPLAYER::setTimeMultiplier(double m)
{
	libvlc_video_set_scale(m_vlcPlayer, m);
}
double RCPLAYER::getTimeMultiplier() const
{
	return libvlc_video_get_scale(m_vlcPlayer);
}

void RCPLAYER::setVolume(float vol)
{
	libvlc_audio_set_volume(m_vlcPlayer, (int)vol);
}
float RCPLAYER::getVolume() const
{
	return (int)libvlc_audio_get_volume(m_vlcPlayer);
}
void RCPLAYER::run()
{
	client = std::shared_ptr<namedpipeClient>(new namedpipeClient(_RC_PIPE_NAME));
	int cnt = 0;
	//play();
	while (m_vlc&&client.get())
	{
		//pause();
		//m_isSleep = 1;
		if(client->receive()==1)
		{
			//play();
			//m_isSleep = 0;
			//__STD_PRINT("%s\n", "next");
			//nextFrame();
			play();
			play();
		}
	}
	__STD_PRINT("%s\n", "PipeClient Exit Successfully.");
}

const char* RCPLAYER::getFilename()
{
	return m_filename.c_str();
}
void RCPLAYER::updateTexture()
{
	//Here we need to flip the frame vertically.
	lock();
	BYTE* p = data();
	unsigned int rowStepInBytes = getRowStepInBytes();
	unsigned int imageStepInBytes = getImageStepInBytes();

	for (int j = 0; j < m_height; j++)
	{
		memcpy(p + (m_height - 1 - j)*rowStepInBytes, m_frameBuf + j*rowStepInBytes, rowStepInBytes);
	}
	unlock();

}

void RCPLAYER::stop()
{
	rcstop();
}

int RCPLAYER::isLocked()
{
	return m_isLocked == true;
}

void RCPLAYER::setTargetTime(ULONGLONG targetTime)
{
	m_targetTime = targetTime;
}



void* RCPLAYER::lockFunc(void* data, void** p_pixels)
{

	m_isLocked = true;
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

void RCPLAYER::unlockFunc(void* data, void* id, void* const* p_pixels)
{
	BYTE* buf = (BYTE*)data;
	//RCPLAYER* stream = (RCPLAYER*)data;
	while (m_isSleep==1)
	{
		sleep();
	}
}

void RCPLAYER::displayFunc(void* data, void* id)
{

	RCPLAYER* stream = (RCPLAYER*)data;
}

void RCPLAYER::videoEndFunc(const libvlc_event_t*, void* data)
{

	RCPLAYER* stream = (RCPLAYER*)data;
	stream->setPosition(0.1);
	stream->play();
}





