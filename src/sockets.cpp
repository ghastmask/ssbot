#include "sockets.h"

#include <stdio.h>

#include "algorithms.h"

#pragma comment(lib, "ws2_32.lib")


//////// Winsock2 interface ////////

void beginWinsock()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == SOCKET_ERROR)
	{
		printf("WSAStartup(0x202, &wsaData) error : %s [%i]\n", WSAGetErrorString(WSAGetLastError()), WSAGetLastError());
	}
}

void endWinsock()
{
	WSACleanup();
}


//////// UDP Sockets ////////

UDPSocket::UDPSocket()
{
	sid = (Uint32)SOCKET_ERROR;
}

UDPSocket::~UDPSocket()
{
	if (sid != SOCKET_ERROR)
	{
		WSACloseEvent(event);
		closesocket(sid);
	}
}

SOCKET UDPSocket::create(Uint16 port)
{
	if (sid != SOCKET_ERROR)
	{
		WSACloseEvent(event);
		closesocket(sid);
	}

	sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sid == SOCKET_ERROR)
	{
		printf("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) error : %s [%i]\n", WSAGetErrorString(WSAGetLastError()), WSAGetLastError());
		return (SOCKET)SOCKET_ERROR;
	}

	INADDR sa(0, port);

	if (bind(sid, sa.getAddress(), sizeof(INADDR)) == SOCKET_ERROR)
	{
		printf("bind(%i) error\n", port);
		return (SOCKET)SOCKET_ERROR;
	}

	event = WSACreateEvent();
	WSAEventSelect(sid, event, FD_READ);

	return sid;
}

void UDPSocket::set(Uint32 ip, Uint16 port)
{
	remote.set(ip, port);
}

void UDPSocket::set(INADDR host)
{
	remote = host;
}

bool UDPSocket::send(BYTE *msg, int len)
{
	return sendto(sid, (char*)msg, len, 0, remote.getAddress(), sizeof(INADDR)) != SOCKET_ERROR;
}

bool UDPSocket::send(char *msg, int len)
{
	return sendto(sid, msg, len, 0, remote.getAddress(), sizeof(INADDR)) != SOCKET_ERROR;
}

UDPPacket *UDPSocket::poll()
{
	if (WSAWaitForMultipleEvents(1, &event, false, 0, true) == WSA_WAIT_EVENT_0)
	{
		int len, FromLen = sizeof(INADDR);
		INADDR src;

		len = recvfrom(sid, packet.msg, PACKET_MAX_LENGTH, 0, src.getAddress(), &FromLen);

		if (len <= 0)
			return NULL;

		packet.src = src;
		packet.len = len;

		return &packet;
	}

	return NULL;
}


//////// Addressing ////////

INADDR::INADDR(Uint32 nip, Uint16 nport)
{
	family	= AF_INET;
	ip		= nip;
	port	= htons(nport);
}

INADDR::INADDR()
{
	family	= AF_INET;
	ip		= 0;
	port	= 0;
}

bool INADDR::operator==(INADDR &other)
{
	return (other.ip == ip) && (other.port == port);
}

void INADDR::operator=(INADDR &other)
{
	ip	= other.ip;
	port= other.port;
}

char *INADDR::getString()
{
	in_addr in;
	in.S_un.S_addr = ip;

	return inet_ntoa(in);
}

int INADDR::getPort()
{
	return HTONS(port);
}

sockaddr *INADDR::getAddress()
{
	return (sockaddr*)this;
}

void INADDR::set(Uint32 nip, Uint16 nport)
{
	ip	= nip;
	port= HTONS(nport);
}


//////// Error codes ////////

char *WSAGetErrorString(int code)
{
	switch (code)
	{
		case WSANOTINITIALISED:			return "WSANOTINITIALISED";
		case WSAENETDOWN:				return "WSAENETDOWN";
		case WSAEINPROGRESS:			return "WSAEINPROGRESS";
		case WSA_NOT_ENOUGH_MEMORY:		return "WSA_NOT_ENOUGH_MEMORY";
		case WSA_INVALID_HANDLE:		return "WSA_INVALID_HANDLE";
		case WSA_INVALID_PARAMETER:		return "WSA_INVALID_PARAMETER";
		case WSAEFAULT:					return "WSAEFAULT";
		case WSAEINTR:					return "WSAEINTR";
		case WSAEINVAL:					return "WSAEINVAL";
		case WSAEISCONN:				return "WSAEISCONN";
		case WSAENETRESET:				return "WSAENETRESET";
		case WSAENOTSOCK:				return "WSAENOTSOCK";
		case WSAEOPNOTSUPP:				return "WSAEOPNOTSUPP";
		case WSAESOCKTNOSUPPORT:		return "WSAESOCKTNOSUPPORT";
		case WSAESHUTDOWN:				return "WSAESHUTDOWN";
		case WSAEWOULDBLOCK:			return "WSAEWOULDBLOCK";
		case WSAEMSGSIZE:				return "WSAEMSGSIZE";
		case WSAETIMEDOUT:				return "WSAETIMEDOUT";
		case WSAECONNRESET:				return "WSAECONNRESET";
		case WSAENOTCONN:				return "WSAENOTCONN";
		case WSAEDISCON:				return "WSAEDISCON";
		case WSA_IO_PENDING:			return "WSA_IO_PENDING";
		case WSA_OPERATION_ABORTED:		return "WSA_OPERATION_ABORTED";
		default:						return "Unknown error";
	};
}


//////// Address resolution ////////

Uint16 HTONS(Uint16 hostshort)
{
	return (hostshort >> 8) | (hostshort << 8);
}

Uint32 resolveHostname(char *name)
{
	Uint32 ip = inet_addr(name);

	if (ip == SOCKET_ERROR)
	{
		hostent *host = gethostbyname(name);

		if (host)
			ip = *((Uint32*)*(host->h_addr_list));
	}

	return ip;
}

Uint32 getnetworkip()
{
    char hostname[80];

	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
	{
		printf("getnetworkip()::gethostname() failed\n");

		return resolveHostname("127.0.0.1");
	}

	printf("Hostname: %s\n", hostname);

    hostent *host = gethostbyname(hostname);
	if (!host)
	{
		printf("getnetworkip()::gethostbyname() failed\n");

		return resolveHostname("127.0.0.1");
	}

	Uint32 ip		= resolveHostname("127.0.0.1"),
		   routable = 0xffffffff;

    for (Uint32 i = 0; (host->h_addr_list[i] != 0); ++i)
	{
		in_addr addr;
		memcpy(&addr, host->h_addr_list[i], sizeof(in_addr));

		ip = addr.S_un.S_addr;
		if ((ip & 0x000000FF) == 0x0000007f)
		{	// 127.*.*.*
			printf("Non-routable: %s\n", inet_ntoa(addr));

			continue;
		}
		if ((ip & 0x000000FF) == 0x0000000a)
		{	// 10.*.*.*
			printf("Non-routable: %s\n", inet_ntoa(addr));

			continue;
		}
		if ((ip & 0x0000FFFF) == 0x0000a8c0)
		{	// 192.168.*.*
			printf("Non-routable: %s\n", inet_ntoa(addr));

			continue;
		}
		if ((ip & 0x000000FF) == 0x000000ac)
		{	// 172.16.*.* - 172.31.*.*
			Uint32 node = (ip & 0x0000FF00) >> 8;

			if (node >= 16 && node <= 31)
			{
				printf("Non-routable: %s\n", inet_ntoa(addr));

				continue;
			}
		}

		printf("Routable: %s\n", inet_ntoa(addr));

		routable = ip;
    }

	if (routable == -1)	routable = ip;

	return routable;
}
