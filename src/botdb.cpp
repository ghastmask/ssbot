#include "botdb.h"

#include "algorithms.h"
#include "system.h"
#include "clientprot.h"
#include "command.h"

using namespace std;


//////// Operators ////////

opEntry::opEntry(char *nname, char *ppass, Operator_Level aaccess)
:	name(nname),
	pass(ppass)
{
	setAccess(aaccess);

	loginCount = 0;
	failedAttempts = 0;
}

void opEntry::setPassword(char *ppass)
{
	pass = ppass;
}

void opEntry::setName(char *nname)
{
	name = nname;
}

void opEntry::setAccess(Operator_Level aaccess)
{
	access = aaccess;
}

Operator_Level opEntry::getAccess()
{
	return access;
}

void opEntry::addCounter()
{
	++loginCount;
}

void opEntry::addFailure()
{
	++failedAttempts;
}

Uint32 opEntry::getOverallCount()
{
	return loginCount;
}

Uint32 opEntry::getFailureCount()
{
	return failedAttempts;
}

char *opEntry::getName()
{
	return name.msg;
}

bool opEntry::validateName(char *nname)
{
	return (name == nname);
}

bool opEntry::validatePass(char *ppass)
{
	if (!pass.msg[0]) return true;	// NULL passwords are unassigned

	return (pass == ppass);
}


//////// Aliases ////////

cmdAlias::cmdAlias(char *ccmd, char *aalias)
:	cmd(ccmd),
	alias(aalias)
{
}

bool cmdAlias::isCmd(char *ccmd)
{
	return (cmd == ccmd);
}

bool cmdAlias::isAlias(char *aalias)
{
	return (alias == aalias);
}

bool cmdAlias::test(char *&ccmd)
{
	if (alias == ccmd)
	{
		ccmd = cmd.msg;

		return true;
	}

	return false;
}

String &cmdAlias::getAlias()
{
	return alias;
}

String &cmdAlias::getCommand()
{
	return cmd;
}


//////// Aliases ////////

void BOT_DATABASE::aliasCommand(char *&command)
{
	_listnode <cmdAlias> *parse = aliasList.head;

	while (parse)
	{
		cmdAlias *alias = parse->item;
		parse = parse->next;

		if (alias->test(command)) return;
	}
}

void BOT_DATABASE::addAlias(char *command, char *alias)
{
	cmdAlias *a = new cmdAlias(command, alias);
	if (a)
	{
		aliasList.append(a);

		aliasesUpdated = true;
	}
}

bool BOT_DATABASE::killAlias(char *alias)
{
	_listnode <cmdAlias> *parse = aliasList.head;

	while (parse)
	{
		if (parse->item->isAlias(alias))
		{
			aliasList.kill(parse);

			aliasesUpdated = true;

			return true;
		}

		parse = parse->next;
	}

	return false;
}

cmdAlias *BOT_DATABASE::findAlias(char *alias)
{
	_listnode <cmdAlias> *parse = aliasList.head;

	while (parse)
	{
		cmdAlias *a = parse->item;

		if (a->isAlias(alias))
			return a;

		parse = parse->next;
	}

	return NULL;
}

String BOT_DATABASE::getAliasList(char *command)
{
	_listnode <cmdAlias> *parse = aliasList.head;

	String s;

	Uint32 count = 0;

	while (parse)
	{
		cmdAlias *alias = parse->item;
		parse = parse->next;

		if (alias->isCmd(command))
		{
			if (count > 4)
			{
				s += "...";

				break;
			}
			else
			{
				if (count++) s += " ";

				s += alias->getAlias();
			}
		}
	}

	return s;
}


//////// Operators ////////

opEntry *BOT_DATABASE::findOperator(char *name)
{
	_listnode <opEntry> *parse = opList.head;

	while (parse)
	{
		opEntry *op = parse->item;

		if (op->validateName(name))
		{
			return op;
		}

		parse = parse->next;
	}

	return NULL;
}

opEntry *BOT_DATABASE::addOperator(char *name, char *pass, Operator_Level access)
{
	opEntry *op = new opEntry(name, pass, access);

	if (op)
	{
		_listnode <opEntry> *parse = opList.head;

		while (parse)
		{
			opEntry *nop = parse->item;

			if (nop->getAccess() < access)
			{
				opList.insertBefore(parse, op);

				operatorsUpdated = true;

				return op;
			}

			parse = parse->next;
		}

		opList.append(op);
	}

	operatorsUpdated = true;

	return op;
}

opEntry *BOT_DATABASE::addOperator(char *name, Operator_Level level)
{
	opEntry *op = addOperator(name, "", level);

	return op;
}

bool BOT_DATABASE::removeOperator(char *name)
{
	opEntry *op = findOperator(name);
	if (op == NULL) return false;

	opList.kill(op);

	operatorsUpdated = true;
	return true;
}


//////// INI ////////

BOT_DATABASE::BOT_DATABASE()
{
	// Retrieve path
	GetCurrentDirectory(520, path);
	strcat(path, "\\");
	strcat(path, INI_NAME);

	lastSave = getTime();
	saveInterval = 30000;

	aliasesUpdated = false;
	operatorsUpdated = false;
}

void BOT_DATABASE::reloadINI(bool doLogin)
{
	// [Login]
	char buffer[32];
	Uint32 len			= GetPrivateProfileString("Login", "Zone", "127.0.0.1:2000", buffer, 32, path);
	buffer[31]			= '\0';

	String port, ip;
	port = buffer;
	ip = port.split(':');

	if (doLogin)
		loginIP			= resolveHostname(ip.msg);
	loginPort			= port.toInteger();
	recordLogins		= (getPrivateProfile32("Login", "RecordLogins", "1", path) != 0);
	resX				= (Uint16)getPrivateProfile32("Login", "ResX", "1280", path);
	resY				= (Uint16)getPrivateProfile32("Login", "ResY", "1024", path);

	// [Database]
	GetPrivateProfileString("Database", "Spawns", "Spawns.txt", spawnsFile, 64, path);
	spawnsFile[63]		= '\0';
	GetPrivateProfileString("Database", "Commands", "Commands.txt", commandsFile, 64, path);
	commandsFile[63]	= '\0';
	GetPrivateProfileString("Database", "Operators", "Operators.txt", operatorsFile, 64, path);
	operatorsFile[63]	= '\0';
	saveInterval		= getPrivateProfile32("Database", "SaveInterval", "300", path) * 100;

	// [Misc]
	GetPrivateProfileString("Misc", "WindowCaption", "My first bot terminal", windowCaption, 64, path);
	windowCaption[63]	= '\0';
	maxSpawns			= getPrivateProfile32("Misc", "MaxSpawns", "3", path);
	maskBan				= (getPrivateProfile32("Misc", "MaskBan", "0", path) != 0);
	playerVoices		= (getPrivateProfile32("Misc", "PlayerVoices", "0", path) != 0);
	forceContinuum		= (getPrivateProfile32("Misc", "ForceContinuum", "0", path) != 0);
	runSilent			= (getPrivateProfile32("Misc", "RunSilent", "0", path) != 0);
	noTerminal			= (getPrivateProfile32("Misc", "NoTerminal", "0", path) != 0);
	disablePub			= (getPrivateProfile32("Misc", "DisablePubCommands", "0", path) != 0);
	noisySpectator		= (getPrivateProfile32("Misc", "NoisySpectator", "0", path) != 0);
	maxTries			= getPrivateProfile32("Misc", "MaxConnectionTries", "3", path);
	GetPrivateProfileString("Misc", "InitialChatChannels", "", initialChatChannels, 64, path);
	initialChatChannels[63] = '\0';

	// [Security]
	encryptMode			= (getPrivateProfile32("Security", "EncryptMode", "1", path) != 0);
	remoteInterpreter	= (getPrivateProfile32("Security", "RemoteInterpreter", "1", path) != 0);
	remoteOperator		= (getPrivateProfile32("Security", "RemoteOperator", "1", path) != 0);
	chatterLog			= (getPrivateProfile32("Security", "ChatterLog", "0", path) != 0);

	// [Registration]
	GetPrivateProfileString("Registration", "Name", "Catid", regName, 40, path);
	regName[39]			= '\0';
	GetPrivateProfileString("Registration", "EMail", "cat02e@fsu.edu", regEMail, 40, path);
	regEMail[39]		= '\0';
	GetPrivateProfileString("Registration", "State", "California", regState, 40, path);
	regState[39]		= '\0';
	regAge				= (BYTE)getPrivateProfile32("Registration", "Age", "17", path);

	// Save INI settings
	botInfo.setDatabase(this, maxSpawns);
	botInfo.setLogin("UNKNOWN BOT ERROR", "AHH CRAP", "");
	botInfo.setZone(loginIP, loginPort);
	botInfo.setArena("0", SHIP_Spectator, resX, resY, playerVoices);
	botInfo.setReg(regName, regEMail, "", regState, SEX_Male, regAge, false, false, false);
	if (maskBan) botInfo.maskBan();
}

void BOT_DATABASE::saveOperators()
{
	if (!operatorsUpdated) return;

	ofstream file(operatorsFile);
	if (!file)
	{
		printf("WARNING: Unable to overwrite %s for Operators database save\n", operatorsFile);

		return;
	}

	printf("Saving operators database...");

	String s;
	_listnode <opEntry> *parse = opList.head;

	while (parse)
	{
		opEntry *op = parse->item;
		parse = parse->next;

		file << Sint32(op->access) << ":" << op->name.msg << ":" << op->pass.msg << "\r\n";
	}

	operatorsUpdated = false;

	printf("Completed.\n");
}

void BOT_DATABASE::saveAliases()
{
	if (!aliasesUpdated) return;

	ofstream file(commandsFile);
	if (!file)
	{
		printf("WARNING: Unable to overwrite %s for Commands database save\n", commandsFile);

		return;
	}

	printf("Saving aliases database...");

	String s;
	_listnode <cmdAlias> *parse = aliasList.head;

	while (parse)
	{
		cmdAlias *cmd = parse->item;
		parse = parse->next;

		file << cmd->getAlias().msg << ":" << cmd->getCommand().msg << "\r\n";
	}

	aliasesUpdated = false;

	printf("Completed.\n");
}

BOT_DATABASE *db;

void gotOperator(char *line);
void gotAlias(char *line);
void gotSpawn(char *line);
//OmegaFirebolt added gotOperator2
void gotOperator2(char *line);

void BOT_DATABASE::loadOperators()
{
	db = this;
	readDataLines(operatorsFile, &gotOperator);
	operatorsUpdated = false;
	db = NULL;
}

void BOT_DATABASE::loadAliases()
{
	db = this;
	readDataLines(commandsFile, &gotAlias);
	aliasesUpdated = false;
	db = NULL;
}

void BOT_DATABASE::loadSpawns()
{
	db = this;
	readDataLines(spawnsFile, &gotSpawn);
	db = NULL;
}

//OmegaFirebolt added loadOperators2
void BOT_DATABASE::loadOperators2()
{
	db = this;
	readDataLines(operatorsFile, &gotOperator2);
	operatorsUpdated = false;
	db = NULL;
}

void gotOperator(char *line)
{
	String s = line;

	Operator_Level level;
	String name, pass;

	level = (Operator_Level)s.split(':').toInteger();
	name = s.split(':');
	pass = s.split(':');

	if ((level >= OP_Limited && level <= OP_God) &&
		(!invalidName(name.msg))						)
	{
		db->addOperator(name.msg, pass.msg, level);

		printf("Added %s-level operator: %s (%s)\n", getLevelString(level), name.msg, pass.msg);
	}
	else
		printf("Invalid %s-level operator ignored: %s (%s)\n", getLevelString(level), name.msg, pass.msg);
}

//OmegaFirebolt added gotOperator2 [modified from the original submission]
void gotOperator2(char *line)
{
	String s = line;

	Operator_Level level;
	String name, pass;

	level = (Operator_Level)s.split(':').toInteger();
	name = s.split(':');
	pass = s.split(':');

	if ((level >= OP_Limited && level <= OP_God) &&
		(!invalidName(name.msg))						)
	{
		opEntry *op = db->findOperator(name.msg);	
		if (!op)
		{
			db->addOperator(name.msg, pass.msg, level);

			printf("Added %s-level operator: %s (%s)\n", getLevelString(level), name.msg, pass.msg);
		}
		else
		{
			op->setAccess(level);
			op->setPassword(pass.msg);
		}
	}
	else
		printf("Invalid %s-level operator ignored: %s (%s)\n", getLevelString(level), name.msg, pass.msg);
}

void gotAlias(char *line)
{
	String s = line;

	String alias, command;

	alias = s.split(':');
	command = s.split(':');

	tolower(alias.msg);
	tolower(command.msg);

	db->addAlias(command.msg, alias.msg);
}

void gotSpawn(char *line)
{
	String s = line;

	String name, pass, arena, dll, staff, params;

	name = s.split(':');
	pass = s.split(':');
	arena = s.split(':');
	dll = s.split(':');
	staff = s.split(':');
	params = s;

	db->botInfo.setLogin(name.msg, pass.msg, staff.msg);
	db->botInfo.setSpawn(dll.msg);
	db->botInfo.setArena(arena.msg, SHIP_Spectator, db->resX, db->resY, db->playerVoices);
	db->botInfo.setParams(params.msg);

	if (!db->spawns.connectHost(db->botInfo))
	{
		printf("Spawn with the same name already exists: %s\n", name.msg);
	}
}
