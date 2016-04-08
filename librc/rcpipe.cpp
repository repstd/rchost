#include "stdafx.h"
#include "rcpipe.h"

rcpipeServer::rcpipeServer(const char* pipeName, DWORD inputBufSize, DWORD outputBufSize) : NAMEDPIPE_API(pipeName), m_inBufSize(inputBufSize), m_outBufSize(outputBufSize)
{
	createPipe(pipeName);
}

rcpipeServer::~rcpipeServer() {}

HANDLE rcpipeServer::createPipe(const char* pipeName)
{

	m_hPipe = CreateNamedPipe(
		const_cast<char*>(pipeName),             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		m_inBufSize,                  // output buffer size 
		m_outBufSize,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		__STD_PRINT("CreateNamedPipe failed, GLE=%d.\n", GetLastError());

		return  NULL;
	}
	return m_hPipe;
}
int rcpipeServer::connect(const LPOVERLAPPED overlap)
{
	if (!m_hPipe)
		createPipe(getPipeName());
	int fConnected = ConnectNamedPipe(getHandle(), overlap) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	return fConnected;
}
int rcpipeServer::disconnect()
{
	FlushFileBuffers(m_hPipe);
	DisconnectNamedPipe(m_hPipe);
	CloseHandle(m_hPipe);
	return 1;
}
int rcpipeServer::writeto(const void* msg, const DWORD sizeToWrite, DWORD& sizeWritten, const LPOVERLAPPED overlap)
{
	sizeWritten = -1;
	if (connect()){
		write(msg, sizeToWrite, sizeWritten, NULL);
		//disconnect();
	}
	return sizeWritten == -1 ? false : true;
}

rcpipeClient::rcpipeClient(const char* pipeName, DWORD inputBufSize, DWORD outputBufSize) :NAMEDPIPE_API(pipeName), m_inBufSize(inputBufSize), m_outBufSize(outputBufSize) 
{ 
	createPipe(pipeName);
}
rcpipeClient::~rcpipeClient() {}
HANDLE rcpipeClient::createPipe(const char* pipeName)
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
			//return NULL;
		}
		// All pipe instances are busy, so wait for 20 seconds. 
		if (!WaitNamedPipe(pipeName, 10))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			//return NULL;
		}
	}

	return m_hPipe;
}

int rcpipeClient::connect(const LPOVERLAPPED overlap)
{
	if (!m_hPipe)
		createPipe(getPipeName());
	int fConnected = ConnectNamedPipe(getHandle(), overlap) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	return fConnected;
}
int rcpipeClient::disconnect()
{
	FlushFileBuffers(m_hPipe);
	DisconnectNamedPipe(m_hPipe);
	CloseHandle(m_hPipe);
	return 1;
}
int rcpipeClient::readfrom(void* msgBuf, const DWORD sizeAllocated, DWORD& sizeRead, const LPOVERLAPPED overlap)
{
	int fSuccess = -1;

	if (connect())
	{

		DWORD dwMode = PIPE_READMODE_MESSAGE;
		fSuccess = SetNamedPipeHandleState(
			m_hPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 
		if (!fSuccess) {
			__STD_PRINT("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
			return -1;
		}
		sizeRead = 0;
		do
		{
			fSuccess = read(msgBuf, sizeAllocated, sizeRead, overlap);
		} while (sizeRead == 0);  // repeat loop if ERROR_MORE_DATA 
		if (!fSuccess)
		{
			__STD_PRINT("ReadFile from pipe failed. GLE=%d\n", GetLastError());
			return -1;
		}
		disconnect();
	}
	return fSuccess;

}
namedpipeServer::namedpipeServer(const char* pipeName) : rcpipeServer(pipeName) { createPipe(pipeName); }

int namedpipeServer::signalClient()
{
	char msg[512];
	strcpy(msg, getPipeName());
	strcat(msg, "#");
	strcat(msg, "next_frame");
	DWORD sizeWritten = DWORD(-1);
	writeto(msg, sizeof(msg), sizeWritten, NULL);
	return sizeWritten == -1 ? false : true;
}

namedpipeClient::namedpipeClient(const char* pipeName) :rcpipeClient(pipeName){}
int namedpipeClient::receive()
{
	char msgBuf[512];
	DWORD sizeRead = 0;
	return readfrom(msgBuf, 512, sizeRead, NULL);
}
