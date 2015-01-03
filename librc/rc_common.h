#pragma once
#include "assert.h"
#include <memory>
#include <vector>

#define _MSG_ARGC 20
#define _MSG_BUF_SIZE 300
#define _MAX_DATA_SIZE (sizeof(_MSG))


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

	char _operation;
	char _prog;
	char _filename[30];
	int	 _size;
	int  _argc;
	char _pBuf[_MSG_BUF_SIZE];

	int  _pSize[_MSG_ARGC];
};
void writeArgs(_MSG* msg, const char* arg);
void readArgs(_MSG* msg, int i, char* arg);
void clear(_MSG* msg);