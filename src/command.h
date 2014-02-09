/*
	Commands by cat02e@fsu.edu
*/


#ifndef COMMAND_H
#define COMMAND_H

//#define DISABLE_COMMANDS

class Host;
#include "player.h"
#include "commtypes.h"


void gotHelp(Host *h, Player *p, Command *c);
void gotCommand(Host *h, Player *p, char *m);
void gotRemoteHelp(Host *h, char *p, Command *c, Operator_Level l);
void gotRemote(Host *h, char *p, char *m);

#endif	// COMMAND_H
