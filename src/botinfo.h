/*
	Bot descriptor by cat02e@fsu.edu
*/


#ifndef BOTINFO_H
#define BOTINFO_H

#include "datatypes.h"
struct BOT_DATABASE;

struct BOT_INFO
{
	char params[512];

	void setParams(char *pParams);

	// Login
	char name[64];
	char password[64];
	char staffpw[64];
	Uint32 ip;
	Uint16 port;

	// Spawn
	char dllName[256];

	// Database
	BOT_DATABASE *db;
	Uint32 maxSpawns;

	// Ban
	Uint32 machineID;
	Uint16 timeZoneBias;
	Uint32 permissionID;

	// Registration form
	char realName[64];
	char email[64];
	char city[64];
	char state[64];
	enum RegForm_Sex sex;
	BYTE age;
	bool playAtHome;
	bool playAtWork;
	bool playAtSchool;
	Uint32 processor;
	char regName[64];
	char regOrg[64];

	// Arena
	enum Ship_Types initialShip;
	char initialArena[64];
	Uint16 xres;
	Uint16 yres;
	bool allowAudio;

	BOT_INFO();
	BOT_INFO(BOT_INFO &a);
	void operator=(BOT_INFO &a);

	void set(BOT_INFO &a);

	void resetSystemInfo();

	void setLogin(char *nname,
				  char *ppassword,
				  char *sstaffpw);

	void setZone(Uint32 iip,
				 Uint16 pport);

	void setSpawn(char *ddllName);

	void setDatabase(BOT_DATABASE *ddb,
					 Uint32 mmaxSpawns);

	void setArena(char *iinitialArena,
				  Ship_Types iinitialShip,
				  Uint16 xxres,
				  Uint16 yyres,
				  bool aallowAudio);

	void setBan(Uint32 mmachineID,
				Uint16 ttimeZoneBias,
				Uint32 ppermissionID,
				Uint32 pprocessor,
				char *rregName,
				char *rregOrg);

	void maskBan();

	void setReg(char *rrealName,
				char *eemail,
				char *ccity,
				char *sstate,
				RegForm_Sex ssex,
				BYTE aage,
				bool pplayAtHome,
				bool pplayAtWork,
				bool pplayAtSchool);
};

#endif	// BOTINFO_H
