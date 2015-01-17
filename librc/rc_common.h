#pragma once
#include "assert.h"
#include <memory>
#include <vector>

#define _MSG_ARGC 20
#define _MSG_BUF_SIZE 300
#define _MAX_DATA_SIZE (sizeof(_MSG))


#define __LOG(filename,info)\
{ \
	FILE* logfout = fopen(filename, "ab+"); \
	fwrite(info, strlen(info), 1, logfout); \
	fclose(logfout);\
}
#define __LOG_FORMAT(filename,fmt,data)\
{\
	FILE* logfout = fopen(filename, "ab+"); \
	fprintf(logfout, fmt, data);\
	fclose(logfout);\
}

#define __LOG_INIT(filename)\
{\
	FILE* logfout = fopen(filename, "w"); \
	fclose(logfout); \
}
#define _PRINT(fmt,data,...) std::printf(fmt,data);

#define __STD_PRINT(fmt,data,...) _PRINT(fmt,data)  
//#define __DEBUG_PRINT(fmt,data,...) __STD_PRINT(fmt,data)

#define __DEBUG_PRINT(fmt,data,...) 
#define _FMT_TIME "%02d-%02d-%02d %02d:%02d:%02d:%02d\n"
#define _STD_PRINT_TIME(systime) std::printf(_FMT_TIME,systime.wYear,systime.wMonth,systime.wDay,systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
#define _STD_ENCODE_TIMESTAMP(buf,systime) std::sprintf(buf,_FMT_TIME,systime.wYear,systime.wMonth,systime.wDay,systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
#define __LOG__FORMAT_TIME(filename,systime) __LOG_FORMAT(filename,_FMT_TIME,systime.wYear,systime.wMonth,systime.wDayOfWeek,systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);

#define _FMT_TIME_PLAIN "%02d %02d %02d %02d %2d %02d %02d %02d\n"
#define _STD_PRINT_TIME_PLAIN(systime) std::printf(_FMT_TIME,systime.wYear,systime.wMonth,systime.wDayOfWeek,systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
#define _RC_PIPE_NAME "\\\\.\\pipe\\rcpipe" 
#define _RC_PIPE_BROADCAST_PORT 8000

enum EVENT
{
	_OPEN,
	_CLOSE
};

struct _MSG
{
	_MSG()
	{
		_size = 0;
		_argc = 0;
		memset(_pBuf, 0, _MSG_BUF_SIZE*sizeof(char));
		memset(_pSize, 0, _MSG_ARGC*sizeof(int));
	}
	bool operator==(_MSG& other)
	{
		if (_operation == other._operation)
			return true;
		if (strcmp(_filename, other._filename) == 0)
			return true;
		//TODO: Add more restrictions	
				
		return false;
	}
	char _operation;
	char _prog;
	char _filename[30];
	int	 _size;
	int  _argc;
	WORD _elapseTime;
	FILETIME _time;
	char _pBuf[_MSG_BUF_SIZE];
	int  _pSize[_MSG_ARGC];
};

struct cmp
{

	bool operator()(const std::string a, const std::string b)
	{


		if (strcmp(a.c_str(), b.c_str()))
			return true;
		else
			return false;
	}

};

class HOST_CONFIG_API
{
public:
	//use pathes stored in a configuration file.
	virtual DWORD loadConfig(const char* confg) = 0;
	virtual DWORD updateConfig()
	{
		return 0;
	}
	HOST_CONFIG_API()
	{
		return;
	}
	~HOST_CONFIG_API()
	{
		return;
	}
};

void writeArgs(_MSG* msg, const char* arg);
void readArgs(_MSG* msg, int i, char* arg);
void clear(_MSG* msg);