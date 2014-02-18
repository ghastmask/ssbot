/*
	Commands by cat02e@fsu.edu
*/


#ifndef COMMAND_H
#define COMMAND_H

//#define DISABLE_COMMANDS


#include "player.h"
#include "host.h"
#include "commtypes.h"
#include <map>
#include <string>

// represents the elements in the command config.
struct Command_Config
{
	std::string name;
	std::string help;
	Operator_Level min_level;
	bool show_in_help;
};

struct Cmd
{
	Cmd(Command_Config const & cfg)
	: config_(cfg)
	{
	}
	virtual ~Cmd() { }

	virtual void execute(Host_Base & h, Command & c, Player * p) = 0;

	Command_Config const & config() { return config_; }

private:
	Command_Config config_;
};


void gotHelp(Host_Base *h, Player *p, Command *c);
void gotCommand(Host_Base *h, Player *p, char *m);
void gotRemoteHelp(Host_Base *h, char *p, Command *c, Operator_Level l);
void gotRemote(Host_Base *h, char *p, char *m);

#endif	// COMMAND_H
