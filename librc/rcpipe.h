#pragma once
#include "stdafx.h"
#include "rc_common.h"
#include "rcthread.h"
typedef HANDLE RCPIPEHANDLE;
class NAMEDPIPE_API
{

public:
	NAMEDPIPE_API(const char* pipeName)
	{
		m_pipeName = pipeName;
		return;
	}
	virtual HANDLE createPipe(const char* pipeName) = 0;
	virtual  int write(const void* msg, const DWORD sizeToWrite, DWORD& sizeWritten, const LPOVERLAPPED overlap)
	{
		int fSuccess = WriteFile(
			m_hPipe,        // handle to pipe 
			msg,     // buffer to write from 
			sizeToWrite, // number of bytes to write 
			&sizeWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!fSuccess || sizeToWrite != sizeWritten)
		{
			__STD_PRINT("InstanceThread WriteFile failed, GLE=%d.\n", GetLastError());

			return 0;
		}

		return 1;

	}
	virtual int read(void* msgBuf, const DWORD sizeAllocated, DWORD& sizeRead, const LPOVERLAPPED overlap)
	{
		int fSuccess = ReadFile(
			m_hPipe,			// handle to pipe 
			msgBuf,				// buffer to receive data 
			sizeAllocated,
			&sizeRead,			// number of bytes read 
			overlap);			// not overlapped I/O 

		if (!fSuccess || sizeRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{

				__STD_PRINT("InstanceThread: client disconnected.\n", GetLastError());
			}
			else
			{
				__STD_PRINT("InstanceThread ReadFile faile, GLE=%d.\n", GetLastError());
			}
			return 0;
		}
		return  1;
	}
	const HANDLE getHandle() const
	{
		return m_hPipe;
	}
	const char* getPipeName() const
	{
		return m_pipeName.c_str();
	}
	const void closeHandle() const
	{
		CloseHandle(m_hPipe);
	}
protected:
	HANDLE		m_hPipe;
	std::string m_pipeName;
};
class namedpipeServer :public NAMEDPIPE_API
{

public:

	namedpipeServer(const char* pipeName);

	virtual HANDLE createPipe(const char* pipeName);
	int connect();
	int disconnect();
	int signalClient();
private:
	int m_szBUF;
};

class namedpipeClient:public NAMEDPIPE_API
{
public:
	namedpipeClient(const char* pipeName);
		
	virtual HANDLE createPipe(const char* pipeName);
	int receive();
private:
	int m_szBUF;

};
class rcpipe
{
public:
	rcpipe();
	virtual ~rcpipe();
};

