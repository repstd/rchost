#pragma once
#include <vlc/vlc.h>
#include <osg/ImageStream>
#include "rcpipe.h"
#define RCPLAYER_MAX_ARGC 15

#define _PLAYER_LOG "./player.log"
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
	public THREAD, public RCPLAYER_API, public osg::ImageStream, public rcmutex
{
public:
	RCPLAYER(const RCPLAYER& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY);
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
	static void* lockFunc(void* data, void** p_pixels);


	static void unlockFunc(void* data, void* id, void* const* p_pixels);


	static void displayFunc(void* data, void* id);

	static void videoEndFunc(const libvlc_event_t*, void* data);
	void init();
	virtual int open(const char* filename);
	void open(const std::string& file, bool needPlay = true, unsigned int w = 512, unsigned int h = 512);
	void stop();
	int isLocked();
	void setTargetTime(ULONGLONG targetTime);
	void syncStart();

	void updateTexture();
	static void sleep()
	{
		Sleep(1);
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

	const char* getFilename();
	ULONGLONG m_targetTime;
	std::unique_ptr<namedpipeClient> client;
	std::string m_filename;
	static int m_isLocked;
	static int m_isFliped;
	BYTE* m_frameBuf;
	static int m_isSleep;
};


