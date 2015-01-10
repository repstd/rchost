#pragma once
#include <vlc/vlc.h>
#include <osg/ImageStream>
#include "rcpipe.h"
#define RCPLAYER_MAX_ARGC 15
class RCPLAYER_API
{
public:
	RCPLAYER_API();
	~RCPLAYER_API();

protected:
	virtual int initPlayer(const char* const* vlc_argv = 0, const int argc = 0) = 0;
	//virtual int initPlayer(const char* vlc_argv[]) = 0;
	virtual int open(const char* filename) = 0;
	virtual int rcplay();
	virtual int rcpause();
	virtual int rcstop();
	virtual	int nextFrame();
	virtual int setResolution(int w, int h);
	virtual int faster()
	{
		return 1;
	}
	virtual int slower()
	{
		return 1;
	}
	const libvlc_media_player_t* getMediaPlayer() const;
	virtual float getPosition();
	virtual int setPosition(float pos);
	virtual int isPlaying();
	virtual libvlc_state_t getState();
	int m_width, m_height;
	libvlc_instance_t*		m_vlc;
	libvlc_media_t*			m_vlcMedia;
	libvlc_media_player_t*	m_vlcPlayer;


};

class RCPLAYER :
	public THREAD, public RCPLAYER_API, public osg::ImageStream
{
public:
	RCPLAYER(const RCPLAYER& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY) :
		RCPLAYER_API(),
		osg::ImageStream(copy, op),
		THREAD()
	{
			m_vlc = copy.m_vlc;
			m_vlcMedia = copy.m_vlcMedia;
			m_vlcPlayer = copy.m_vlcPlayer;
		}

	static RCPLAYER* instance();
	virtual int initPlayer(const char* const* vlc_argv = 0, const int argc = 0);
	//virtual int initPlayer(const char* vlc_argv[]);
	META_Object(osg, RCPLAYER)

protected:

	RCPLAYER();

	~RCPLAYER()
	{

		if (_status != INVALID)
		{
			libvlc_media_player_stop(m_vlcPlayer);
			libvlc_media_player_release(m_vlcPlayer);
		}

		libvlc_release(m_vlc);

	}
public:
	static void* lockFunc(void* data, void** p_pixels)
	{

		m_isLocked = true;
		RCPLAYER* stream = (RCPLAYER*)data;
		*p_pixels = (void*)stream->data();
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

	static void unlockFunc(void* data, void* id, void* const* p_pixels)
	{
		RCPLAYER* stream = (RCPLAYER*)data;
		stream->dirty();
		m_isLocked = false;
	}

	static void displayFunc(void* data, void* id)
	{
	}

	static void videoEndFunc(const libvlc_event_t*, void* data)
	{

		RCPLAYER* stream = (RCPLAYER*)data;
		stream->setPosition(0.1);
		stream->play();
		stream->play();
	}

	virtual int open(const char* filename);
	void open(const std::string& file, bool needPlay = true, unsigned int w = 512, unsigned int h = 512);
	void stop()
	{
		rcstop();
	}
	int isLocked()
	{
		return m_isLocked == true;
	}
	
	virtual void play();
	virtual void pause();
	virtual void rewind();
	virtual void quit(bool waitForThreadToExit = true);
	virtual void setReferenceTime(double time);
	virtual double getReferenceTime() const;
	virtual void setTimeMultiplier(double m);
	virtual double getTimeMultiplier() const;
	virtual void setVolume(float vol);
	virtual float getVolume() const;
	virtual void run();
	const char* getFilename()
	{
		return m_filename.c_str();
	}
	std::auto_ptr<namedpipeClient> client;
	std::string m_filename;
	static int m_isLocked;
};


