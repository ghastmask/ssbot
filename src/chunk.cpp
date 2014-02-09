#include "chunk.h"

#include <string.h>

#include "algorithms.h"


//////// Chunk buffer methods ////////

void chunkBuffer::resizeMessage(Uint32 len)
{
	char *newbuff = new char[len];
	if (buffer)
	{
		memcpy(newbuff, buffer, currentLength);
		delete []buffer;
	}
	buffer = newbuff;
}

chunkBuffer::chunkBuffer()
{
	buffer = NULL;
	currentLength = 0;
	maximumLength = 1000;
}

chunkBuffer::~chunkBuffer()
{
	deleteMessage();
}

void chunkBuffer::setLimit(Uint32 max)
{
	maximumLength = max;
}

void chunkBuffer::addMessage(char *msg, Uint32 len)
{
	Uint32 newlen = currentLength + len;

	if (newlen <= maximumLength)
	{
		resizeMessage(newlen);
		memcpy(buffer + currentLength, msg, len);
		currentLength = newlen;
	}
}

void chunkBuffer::deleteMessage()
{
	if (buffer)
	{
		delete []buffer;
		buffer = NULL;
		currentLength = 0;
	}
}
