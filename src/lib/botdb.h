/*
	Various global databases by cat02e@fsu.edu
*/


#ifndef BOTDB_H
#define BOTDB_H

#include "hostlist.h"
#include "player.h"
#include "clientprot.h"
#include "botinfo.h"

#include <list>
#include <memory>

class opEntry
{
	Uint32 loginCount;		// Overall count of logins, this instantiation
	Uint32 failedAttempts;	// Count of failed logins, this instantiation
	Operator_Level access;
	String name, pass;

	friend struct BOT_DATABASE;

public:
	opEntry(char *nname, char *ppass, Operator_Level aaccess);

	void setPassword(char *ppass);
	void setName(char *nname);
	void setAccess(Operator_Level aaccess);

	Operator_Level getAccess() const;

	void addCounter();
	void addFailure();
	Uint32 getOverallCount();
	Uint32 getFailureCount();
	const char *getName() const;

	bool validateName(const char *nname);
	bool validatePass(char *ppass);
};

class cmdAlias
{
	String cmd, alias;

public:
	cmdAlias(char *ccmd, char *aalias);

	bool isCmd(char *ccmd) const;
	bool isAlias(char *aalias) const;

	bool test(char *&ccmd) const;	// change command to alias on match

	String const & getAlias() const;
	String const & getCommand() const;
};

struct BOT_DATABASE
{
	// Database
	char path[532];

	BOT_DATABASE();

	Uint32 lastSave;			// in hundredths of a second

	// Parameters
	BOT_INFO botInfo;

	// Spawns
	hostList spawns;

	void loadSpawns();

	// Operators
	std::list <opEntry> opList;
	bool operatorsUpdated;

	opEntry *findOperator(const char *name);
	opEntry *addOperator(char *name, Operator_Level level);
	opEntry *addOperator(char *name, char *pass, Operator_Level level);
	bool removeOperator(const char *name);

	void loadOperators();
	void saveOperators();

	//OmegaFirebolt added loadOperators2
	void loadOperators2();

	// Aliasing
	std::list <cmdAlias> aliasList;
	bool aliasesUpdated;

	void aliasCommand(char *&command);
	void addAlias(char *command, char *alias);
	bool killAlias(char *alias);

	cmdAlias *findAlias(char *alias);

	void loadAliases();
	void saveAliases();

	String getAliasList(char *command);

	// INI
	char regName[40];
	char regEMail[40];
	char regState[40];
	BYTE regAge;
	Uint32 loginIP;
	Uint16 loginPort;
	bool recordLogins;
	Uint16 resX;
	Uint16 resY;
	bool chatterLog;
	bool encryptMode;
	char windowCaption[64];
	bool maskBan;
	bool remoteInterpreter;
	bool remoteOperator;
	bool playerVoices;
	Uint32 saveInterval;			// in hundredths of a second
	char spawnsFile[64];
	char commandsFile[64];
	char operatorsFile[64];
	Uint32 maxSpawns;
	bool noTerminal;
	bool forceContinuum;	// hidden option
	bool runSilent;			// hidden option
	bool disablePub;
	bool noisySpectator;
	Uint32 maxTries;
	char initialChatChannels[64];

	void reloadINI(bool doLogin);	// doLogin: Ignore [Login] section to avoid DoS attempts
};

#endif	// BOTDB_H
