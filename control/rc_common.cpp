#include "stdafx.h"
#include "rc_common.h"

#pragma once
#ifdef  _CLIENT_H

void writeArgs(_MSG* msg, const char* arg)
{
	assert(msg != NULL);
	char* temp = const_cast<char*>(arg);
	int curSize = strlen(temp) + 1;
	assert(msg->_pBuf + _size + curSize != NULL);
	memcpy(msg->_pBuf + msg->_size, temp, curSize*sizeof(char));
	msg->_size += curSize;
	msg->_pSize[++(msg->_argc)] = msg->_size;

}
void readArgs(_MSG* msg, int i, char* arg)
{
	assert(msg != NULL);
	assert(i < msg->_argc);
	//int curSize = msg->_pSize[i + 1] - msg->_pSize[i];
	assert(arg + curSize != NULL);
	memcpy(arg, msg->_pBuf + msg->_pSize[i], msg->_pSize[i + 1] - msg->_pSize[i]);

}
void clear(_MSG* msg)
{
	assert(msg != NULL);
	memset(msg->_pBuf, 0, msg->_size);
	memset(msg->_pSize, 0, msg->_argc);
	msg->_size = 0;
	msg->_argc = 0;
}
#endif