#include "StdAfx.h"
#include "player.h"

int RCPLAYER::m_isLocked = -1;
RCPLAYER_API::RCPLAYER_API()
{

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
	libvlc_state_t state = getState();
	switch (state)
	{
	case libvlc_Paused:
		libvlc_media_player_play(m_vlcPlayer);
		break;
	case libvlc_Playing:
		libvlc_media_player_set_pause(m_vlcPlayer, true);
		break;
	case libvlc_Ended:
		libvlc_media_player_play(m_vlcPlayer);
		break;
	default:
		return 0;
	}
	return 1;
}

int RCPLAYER_API::rcpause()
{
	libvlc_media_player_set_pause(m_vlcPlayer, true);
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
const libvlc_media_player_t* RCPLAYER_API::getMediaPlayer() const
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



RCPLAYER* RCPLAYER::instance()
{
	static RCPLAYER player;
	return &player;
}
RCPLAYER::RCPLAYER() :
osg::ImageStream(),
THREAD()
{
	m_vlcMedia = 0;

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

	m_vlcPlayer = libvlc_media_player_new(m_vlc);

	libvlc_event_attach(libvlc_media_player_event_manager(m_vlcPlayer), libvlc_MediaPlayerEndReached, &RCPLAYER::videoEndFunc, this);
	_status = INVALID;
	return 1;
}

int RCPLAYER::open(const char* filename)
{

	m_vlcMedia = libvlc_media_new_path(m_vlc, filename);
	libvlc_media_player_set_media(m_vlcPlayer, m_vlcMedia);
	libvlc_video_set_callbacks(m_vlcPlayer, &RCPLAYER::lockFunc, &RCPLAYER::unlockFunc, &RCPLAYER::displayFunc, this);
	libvlc_video_set_format(m_vlcPlayer, "RGBA", m_width, m_height, m_width * 4);
	allocateImage(m_width, m_height, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	m_filename = filename;

	__STD_PRINT("Opened %s Successfully.\n", filename);

	return 1;
}

void RCPLAYER::open(const std::string& file, bool needPlay, unsigned int w, unsigned int h)
{

	setResolution(w, h);
	open(file.c_str());
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
		if (sleepTime < 0)
			sleepTime = 0;
		if (sleepTime > 30 * 1000.0)
			sleepTime = 30 * 1000.0;
		Sleep(sleepTime);
	}

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

	if (_status != PLAYING)
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
	client = std::auto_ptr<namedpipeClient>(new namedpipeClient(_RC_PIPE_NAME));
	//play();
	int cnt = 0;

	while (getState() != libvlc_Ended)
	{

		if (client->receive() && isLocked() == false)
		{
			//nextFrame();
			play();
			play();

		}

	}
}















