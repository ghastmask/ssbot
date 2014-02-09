/*
	List of connected hosts by cat02e@fsu.edu
*/


#ifndef HOSTLIST_H
#define HOSTLIST_H

#include "sockets.h"
struct BOT_INFO;

class hostList
{
public:
	_linkedlist <class Host> list;
	Uint16 maximumHosts;

public:
	hostList();										// Client version
	~hostList();

public:
	void doEvents();

	bool connectHost(BOT_INFO &botInfo);

	void massDisconnect();

	void massRestart();

	Uint32 getConnections();

	Host *findSpawn(char *name);
};

#endif	// HOSTLIST_H
