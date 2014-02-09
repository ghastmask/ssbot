/*
	Special protocol security services by cat02e@fsu.edu
	-------------------------------------------------------
	This library implements all the SubSpace core protocol
	features in complete safety from exploitation, use only
	in combination with <host.h>
*/


#ifndef SPECIALPROT_H
#define SPECIALPROT_H

#include "host.h"

#ifndef DEFLATE_CLASS
#define DEFLATE_CLASS		\
	Host *	h	= m->src;	\
	char *	msg	= m->msg;	\
	Uint32	len	= m->len;
#endif

void __stdcall handleSpecialUnknown		(hostMessage *msg);
void __stdcall handleSpecialHeader		(hostMessage *msg);	// 00 "Request some of the core protocol"
void __stdcall handleEncryptRequest		(hostMessage *msg);	// 01 <Client key(4)> <Protocol version(2)>
void __stdcall handleEncryptResponse	(hostMessage *msg);	// 02 <Session encryption key(4)> [Continuum mudge(1)]
void __stdcall handleDisconnect			(hostMessage *msg);	// 07 "Session termination"
void __stdcall handleReliable			(hostMessage *msg);	// 03 <ACK_ID(4)> <Message(...)>
void __stdcall handleACK				(hostMessage *msg);	// 04 <ACK_ID(4)>
void __stdcall handleSyncRequest		(hostMessage *msg);	// 05 <Client time1(4)> [Total packets sent(4)] [Total packets recv'd(4)]
void __stdcall handleSyncResponse		(hostMessage *msg);	// 06 <Server time(4)> <Client time1(4)>
void __stdcall handleChunkBody			(hostMessage *msg);	// 08 <Message chunk(...)>
void __stdcall handleChunkTail			(hostMessage *msg);	// 09 <Message chunk(...)> "Process accumulated buffer as a message"
void __stdcall handleBigChunk			(hostMessage *msg);	// 0A <Total length(4)> <Message chunk(...)> "Buffer contains up to 20 MB"
void __stdcall handleCancelDownload		(hostMessage *msg);	// 0B "Nevermind about the 00 0a transfer stuff i was sending you" (Snrrrub)
void __stdcall handleCancelDownloadAck	(hostMessage *msg);	// 0C "Cancel received successfully" (Snrrrub)
// 00 0d no operation server->client
void __stdcall handleCluster			(hostMessage *msg);	// 0E <Message length(1)> <Message(...)> "Carpool into one packet"

#endif	// SPECIALPROT_H
