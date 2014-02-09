/*
	Simple socket wrapper by cat02e@fsu.edu
	------------------------------------------
	UDPSocket supports polling and writing.
	Before you can use these features, you must
	first call beginWinsock().
*/


#ifndef SOCKETS_H
#define SOCKETS_H

#include <winsock2.h>

#include "datatypes.h"


#define PACKET_MAX_LENGTH 520


void beginWinsock();					// Register process with winsock2

void endWinsock();						// Unregister process from winsock2


struct INADDR
{
	unsigned family		: 16;			// Socket family (Always INET_ADDR)
	unsigned port		: 16;			// Socket subclassing
	unsigned ip			: 32;			// Host address
	unsigned padding0	: 32;			// Unused space
	unsigned padding1	: 32;			// Unused space

	INADDR(Uint32 ip, Uint16 port);		// Fill address
	INADDR();							// Fill address

	bool operator==(INADDR &other);		// Validate source

	void operator=(INADDR &other);		// Copy source

	char *getString();					// Dotted quad representation
	int getPort();						// Return host-order
	sockaddr *getAddress();				// Passable address
	void set(Uint32 ip, Uint16 port);	// Re-fill address
};


struct UDPPacket
{
	INADDR	src;						// Sender address
	char msg[PACKET_MAX_LENGTH];		// Message contents
	Uint32	len;						// Message length
};


class UDPSocket
{
	SOCKET sid;							// Socket identifier
	HANDLE event;						// Read event polling
	INADDR remote;						// Remote address

	UDPPacket packet;					// Speedup, not thread-safe

public:
	UDPSocket();						// Start up
	~UDPSocket();						// Clean up

	SOCKET create(WORD port);			// Bind to a port

	void set(DWORD ip, WORD port);		// Set host
	void set(INADDR host);				// Set host

	bool send(BYTE *msg, int len);		// Send a datagram
	bool send(char *msg, int len);		// Send a datagram

	UDPPacket *poll();					// Recv a datagram
};


char *WSAGetErrorString(int code);		// Get a string-representation of an error code


Uint32 resolveHostname(char *name);	// Resolve Internet address from dotted quad representations


Uint16 HTONS(Uint16 hostshort);		// Reverse byte order


Uint32 getnetworkip();					// Resolve the first routable Internet IP


#endif	// SOCKETS_H
