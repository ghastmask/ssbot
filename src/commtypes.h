/*
	Command parsers by cat02e@fsu.edu
*/


#ifndef COMMTYPES_H
#define COMMTYPES_H

#include "datatypes.h"

bool invalidName(char *name);
bool invalidArena(char *name);

bool validRemotePrivate(char *command);
char *getRemoteCommand(char *text);
char *getRemoteName(char *text);

bool validRemoteChat(char *command);
char *getChatCommand(char *text);
char *getChatName(char *text);

struct _switch
{					//            |----|
	char type;		// !command -a_=blah-s blah
	char *param;	// !command -at=____-s blah

	_switch(char t, char *p);
	~_switch();
};

struct Command
{
	_linkedlist <_switch> slist;

	char *cmd;		// !_______ -at=blah-s blah
	char *final;	// !command -at=blah-s ____

	Command(char *msg);				// Parse
	~Command();						// Clean up

	bool check(char *msg);			// Check against cmd
	bool checkParam(char *msg);		// Check against final

	void addParam(char *msg);		// Add a switch
	_switch *getParam(char type);	// Check against switches
};

#endif	// COMMTYPES_H
