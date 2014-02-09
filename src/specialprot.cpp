#include "clientprot.h"
#include "specialprot.h"


//////// Special handlers ////////

void __stdcall handleSpecialHeader(hostMessage *m)
{	DEFLATE_CLASS

	if (len >= 2)
	{
		h->gotSpecialMessage(m);
	}
	else
	{
		h->logEvent("WARNING: Malformed special header ignored.");
	}
}

void __stdcall handleSpecialUnknown(hostMessage *m)
{	DEFLATE_CLASS

	h->logEvent("WARNING: Unknown special message %i(%i)", msg[1], len);
	h->logIncoming(msg, len);
}


//////// Core protocol handshake ////////

void __stdcall handleEncryptRequest(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 8)
	{
		h->gotEncryptRequest(getLong(msg, 2), getShort(msg, 6));
	}
	else
	{
		h->logEvent("WARNING: Malformed encryption request ignored.");
	}
}

void __stdcall handleEncryptResponse(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 6)
	{
		h->gotEncryptResponse(getLong(msg, 2));
	}
	else if (len >= 7)
	{
		h->gotEncryptResponse(getLong(msg, 2), getByte(msg, 6));
	}
	else
	{
		h->logEvent("WARNING: Malformed encryption response ignored.");
	}
}

void __stdcall handleDisconnect(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 2)
	{
		h->gotDisconnect();
	}
	else
	{
		h->logEvent("WARNING: Malformed disconnection ignored.");
	}
}


//////// Reliable messaging ////////

void __stdcall handleReliable(hostMessage *m)
{	DEFLATE_CLASS

	if (len >= 7)
	{
		h->gotReliable(getLong(msg, 2), msg + 6, len - 6);
	}
	else
	{
		h->logEvent("WARNING: Malformed reliable header ignored.");
	}
}

void __stdcall handleACK(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 6)
	{
		h->gotACK(getLong(msg, 2));
	}
	else
	{
		h->logEvent("WARNING: Malformed acknowledgement ignored.");
	}
}


//////// Time synchronization ////////

void __stdcall handleSyncRequest(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 6)
	{
		h->gotSyncRequest(getLong(msg, 2));
	}
	else if (len == 14)
	{
		h->gotSyncRequest(getLong(msg, 2), getLong(msg, 6), getLong(msg, 10));
	}
	else
	{
		h->logEvent("WARNING: Malformed synchronization request ignored.");
	}
}

void __stdcall handleSyncResponse(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 10)
	{
		h->gotSyncResponse(getLong(msg, 2), getLong(msg, 6));
	}
	else
	{
		h->logEvent("WARNING: Malformed synchronization response ignored.");
	}
}


//////// Oversized packets ////////

void __stdcall handleChunkBody(hostMessage *m)
{	DEFLATE_CLASS

	if (len >= 3)
	{
		h->gotChunkBody(msg + 2, len - 2);
	}
	else
	{
		h->logEvent("WARNING: Malformed chunk body ignored.");
	}
}

void __stdcall handleChunkTail(hostMessage *m)
{	DEFLATE_CLASS

	if (len >= 3)
	{
		h->gotChunkTail(msg + 2, len - 2);
	}
	else
	{
		h->logEvent("WARNING: Malformed chunk tail ignored.");
	}
}


//////// File transfer ////////

void __stdcall handleBigChunk(hostMessage *m)
{	DEFLATE_CLASS

	if (len >= 7)
	{
		h->gotBigChunk(getLong(msg, 2), msg + 6, len - 6);
	}
	else
	{
		h->logEvent("WARNING: Malformed big chunk ignored.");
	}
}

void __stdcall handleCancelDownload(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 2)
	{
		h->gotCancelDownload();
	}
	else
	{
		h->logEvent("WARNING: Malformed cancel download ignored.");
	}
}

void __stdcall handleCancelDownloadAck(hostMessage *m)
{	DEFLATE_CLASS

	if (len == 2)
	{
		h->gotCancelDownloadAck();
	}
	else
	{
		h->logEvent("WARNING: Malformed cancel download acknowledgement ignored.");
	}
}

void __stdcall handleCluster(hostMessage *m)
{	DEFLATE_CLASS

	if (len >= 6)
	{	// Require at least two messages to be clustered
		h->gotCluster(msg + 2, len - 2);
	}
	else
	{
		h->logEvent("WARNING: Malformed cluster ignored (one or no messages).");
	}
}
