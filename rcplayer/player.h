#pragma once
#include <vlc/vlc.h>
#include <osg/ImageStream>
#include "rcpipe.h"
class RCPLAYER_API
{
public:
	RCPLAYER_API();
	~RCPLAYER_API();

protected:
	virtual int initPlayer(const char* const* vlc_argv = 0) = 0;
	virtual int open(const char* filename) = 0;
	virtual int rcplay() ;
	virtual int rcpause();
	virtual int rcstop() ;
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

class RCPLAYER :public THREAD,public RCPLAYER_API,public osg::ImageStream
{
public:
	RCPLAYER(const RCPLAYER& copy, const osg::CopyOp& op = osg::CopyOp::SHALLOW_COPY): 
		RCPLAYER_API(),
		osg::ImageStream(copy, op),
		THREAD()
	{
			m_vlc = copy.m_vlc;
			m_vlcMedia = copy.m_vlcMedia;
			m_vlcPlayer = copy.m_vlcPlayer;
	}

	META_Object(osg, RCPLAYER)

	RCPLAYER(const char* const* vlc_argv = 0);

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
		RCPLAYER* stream = (RCPLAYER*)data;
		*p_pixels = (void*)stream->data();
		return NULL;
	}

	static void unlockFunc(void* data, void* id, void* const* p_pixels)
	{
		RCPLAYER* stream = (RCPLAYER*)data;
		stream->dirty();
	}

	static void displayFunc(void* data, void* id)
	{
	}

	static void videoEndFunc(const libvlc_event_t*, void* data)
	{
		
		RCPLAYER* stream = (RCPLAYER*)data;
		stream->_status = INVALID;
	}
	virtual int open(const char* filename);
	virtual int initPlayer(const char* const* vlc_argv = 0);
	void open(const std::string& file, bool needPlay = true, unsigned int w = 512, unsigned int h = 512);
	void stop()
	{
		rcstop();
	}
	virtual void play();
	virtual void pause();
	virtual void rewind();
	virtual void quit(bool waitForThreadToExit = true);
	virtual void setReferenceTime(double time);
	virtual double getReferenceTime() const;
	virtual void setTimeMultiplier(double m) ;
	virtual double getTimeMultiplier() const ;
	virtual void setVolume(float vol) ;
	virtual float getVolume() const ;
	virtual void run();

	std::auto_ptr<namedpipeClient> client;
};

