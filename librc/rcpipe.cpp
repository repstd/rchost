#include "stdafx.h"
#include "rcpipe.h"


rcpipe::rcpipe()
{
}


rcpipe::~rcpipe()
{
}


namedpipeServer::namedpipeServer(const char* pipeName):
NAMEDPIPE_API(pipeName)
{

	m_hPipe = createPipe(pipeName);
	m_szBUF = 512;
}
HANDLE namedpipeServer::createPipe(const char* pipeName)
{

	HANDLE hPipe = CreateNamedPipe(
		const_cast<char*>(pipeName),             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		m_szBUF,                  // output buffer size 
		m_szBUF,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		__STD_PRINT("CreateNamedPipe failed, GLE=%d.\n", GetLastError());

		return  NULL;
	}

	return hPipe;
}
//bool namedpipeServer::write(const void* msg, const DWORD sizeToWrite, DWORD& sizeWritten, const LPOVERLAPPED overlap)
//{
//
//	bool fSuccess = WriteFile(
//		m_hPipe,        // handle to pipe 
//		msg,     // buffer to write from 
//		sizeToWrite, // number of bytes to write 
//		&sizeWritten,   // number of bytes written 
//		NULL);        // not overlapped I/O 
//
//	if (!fSuccess || sizeToWrite!= sizeWritten)
//	{
//		__STD_PRINT("InstanceThread WriteFile failed, GLE=%d.\n", GetLastError());
//
//		return 0;
//	}
//
//	return 1;
//}
//bool namedpipeServer::read(void* msgBuf, const DWORD sizeAllocated, DWORD& sizeRead, const LPOVERLAPPED overlap)
//{
//	bool fSuccess = ReadFile(
//		m_hPipe,        // handle to pipe 
//		msgBuf,			// buffer to receive data 
//		sizeAllocated,	
//		&sizeRead, // number of bytes read 
//		NULL);        // not overlapped I/O 
//
//	if (!fSuccess || sizeRead== 0)
//	{
//		if (GetLastError() == ERROR_BROKEN_PIPE)
//		{
//
//			__STD_PRINT("InstanceThread: client disconnected.\n", GetLastError());
//		}
//		else
//		{
//			__STD_PRINT("InstanceThread ReadFile faile, GLE=%d.\n", GetLastError());
//		}
//		return 0;
//	}
//	return  1;
//}
int namedpipeServer::connect()
{

     int fConnected = ConnectNamedPipe(getHandle(), NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
 
	 return fConnected;
}
int namedpipeServer::disconnect()
{

	FlushFileBuffers(m_hPipe);
	DisconnectNamedPipe(m_hPipe);
	//CloseHandle(m_hPipe);
	return 1;
}

int namedpipeServer::signalClient()
{
	char msg[512];
	strcpy(msg, getPipeName());
	strcat(msg, "#");
	strcat(msg, "next_frame");
	DWORD sizeWritten=-1;
	if (connect())
	{
		write(msg, strlen(msg), sizeWritten, NULL);
		disconnect();
	}

	return sizeWritten == -1 ? false : true;
}

namedpipeClient::namedpipeClient(const char* pipeName)
:NAMEDPIPE_API(pipeName)
{
	m_szBUF = 512;
}
HANDLE namedpipeClient::createPipe(const char* pipeName)
{

	while (1)
	{
		m_hPipe = CreateFile(
			pipeName,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 

		if (m_hPipe != INVALID_HANDLE_VALUE)
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
				

			__STD_PRINT("Could not open pipe. GLE=%d\n", GetLastError());
			return NULL;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(pipeName, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return NULL;
		}
	}

	return m_hPipe;
}
int namedpipeClient::receive()
{

	if (createPipe(m_pipeName.c_str()) == NULL)
		return 0;
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	int fSuccess = SetNamedPipeHandleState(
		m_hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess)
	{
		__STD_PRINT("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
		return -1;
	}
	char buf[512];
	DWORD cbRead=0;
	do
	{
		// Read from the pipe. 

		fSuccess = ReadFile(
			m_hPipe,    // pipe handle 
			buf,    // buffer to receive reply 
			512,
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 

	} while (cbRead==0);  // repeat loop if ERROR_MORE_DATA 

	//__STD_PRINT("\"%s\"\n", buf);

	if (!fSuccess)
	{
		__STD_PRINT("ReadFile from pipe failed. GLE=%d\n", GetLastError());
		return -1;
	}


	CloseHandle(m_hPipe);

	return 1;

}
