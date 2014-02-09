/*
	Chunk message handler by cat02e@fsu.edu
*/


#ifndef CHUNK_H
#define CHUNK_H

#include "datatypes.h"

class chunkBuffer
{
	Uint32 maximumLength;						// Message limit (default 1KB)

	void resizeMessage(Uint32 len);				// Quick create/copy/destroy mechanism

public:
	char * buffer;								// Message contents
	Uint32 currentLength;						// Length of buffer

	chunkBuffer();								// Reset all
	~chunkBuffer();								// Delete the transmission if it exists

	void setLimit(Uint32 max);					// Set append limit (too high and you run the
												// risk of allowing Denial of Service attacks)

	void addMessage(char * msg, Uint32 len);	// Append some bytes

	void deleteMessage();						// Delete the current transmission
};

#endif	// CHUNK_H
