#include "clientprot.h"
#include "botinfo.h"
#include "system.h"
#include "algorithms.h"


//////// Bot descriptor ////////

BOT_INFO::BOT_INFO()
{
	setLogin("Bobo the Bot", "+++++", "");
	setZone(resolveHostname("127.0.0.1"), 2000);
	setArena("0", SHIP_Spectator, 1024, 768, false);
	resetSystemInfo();
	setReg("Catid.bot", "cat02e@fsu.edu", "CatVille", "CatState", SEX_Male, 17, true, true, true);
	setDatabase(NULL, 3);
	setSpawn("default.dll");
}

BOT_INFO::BOT_INFO(BOT_INFO &a)
{
	set(a);
}

void BOT_INFO::operator=(BOT_INFO &a)
{
	set(a);
}

void BOT_INFO::set(BOT_INFO &a)
{
	setLogin(a.name, a.password, a.staffpw);
	setZone(a.ip, a.port);
	setArena(a.initialArena, a.initialShip, a.xres, a.yres, a.allowAudio);
	setBan(a.machineID, a.timeZoneBias, a.permissionID, a.processor, a.regName, a.regOrg);
	setReg(a.realName, a.email, a.city, a.state, a.sex, a.age, a.playAtHome, a.playAtWork, a.playAtSchool);
	setDatabase(a.db, a.maxSpawns);
	setSpawn(a.dllName);
}

void BOT_INFO::resetSystemInfo()
{
	// CPU type
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	processor = si.dwProcessorType;

	// Windows registration name
	{
		char buffer[40];

		// Get platform
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		Uint32 PlatformId = osvi.dwPlatformId;

		// Prepare for registry access
		HKEY key;			// Handle to a session with a registry key
		Uint32 buflen;		// Length of the buffer
		Uint32 type;		// Type will contain type of data transfered

		if (PlatformId != VER_PLATFORM_WIN32_NT)
		{
			// Look up Windows 9x or Windows 3.1 version information
			RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", (HKEY*)&key);
		}
		else
		{
			// Look up Windows NT version information
			RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", (HKEY*)&key);
		}

		buflen = 40;
		RegQueryValueEx(key, "RegisteredOwner", NULL, &type, (BYTE*)&buffer, &buflen);
		strncpy(regName, buffer, 40);

		buflen = 40;
		RegQueryValueEx(key, "RegisteredOrganization", NULL, &type, (BYTE*)&buffer, &buflen);
		strncpy(regOrg, buffer, 40);

		RegCloseKey(key);
	}

	// Timezone Bias
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);
	timeZoneBias = (SHORT)tzi.Bias;

	// Permission ID
	permissionID = getSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE", "D2");

	// Install some SubSpace registry keys
	if (permissionID == -1)
	{
		do
		{
			permissionID = (GetTickCount() ^ 0xAAAAAAAA) * 0x5f346d + 0x5abcdef;
		}
		while (!permissionID || permissionID == 1 || permissionID == -1);

		setSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE", "D2", permissionID);
	}

	// Machine ID
	GetVolumeInformation("C:\\", NULL, 0, &machineID, NULL, NULL, NULL, 0);

	if (!machineID || machineID == 1 || machineID == -1)
	{
		machineID = getSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductCode");

		if (!machineID || machineID == 1 || machineID == -1)
		{
			machineID = permissionID;
			setSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", "ProductCode", machineID);
		}
	}

	if (machineID > 0x7fffffff) machineID += 0x7fffffff;

	setSetting32(HKEY_LOCAL_MACHINE, "SOFTWARE", "D1", machineID);
}

void BOT_INFO::setSpawn(char *ddllName)
{
	strncpy(dllName, ddllName, 256);
}

void BOT_INFO::setParams(char *pParams)
{
	strncpy(params, pParams, 512);
}

void BOT_INFO::setLogin(char *nname, char *ppassword, char *sstaffpw)
{
	strncpy(name, nname, 64);
	strncpy(password, ppassword, 64);
	strncpy(staffpw, sstaffpw, 64);
}

void BOT_INFO::setZone(Uint32 iip, Uint16 pport)
{
	ip = iip;
	port = pport;
}

void BOT_INFO::setDatabase(BOT_DATABASE *ddb, Uint32 mmaxSpawns)
{
	db = ddb;
	maxSpawns = mmaxSpawns;
}

void BOT_INFO::setArena(char *iinitialArena, Ship_Types iinitialShip, Uint16 xxres, Uint16 yyres, bool aallowAudio)
{
	strncpy(initialArena, iinitialArena, 64);
	initialShip = iinitialShip;

	xres = xxres;
	yres = yyres;

	allowAudio = aallowAudio;
}

void BOT_INFO::setBan(Uint32 mmachineID, Uint16 ttimeZoneBias, Uint32 ppermissionID, Uint32 pprocessor, char *rregName, char *rregOrg)
{
	machineID = mmachineID;
	timeZoneBias = ttimeZoneBias;
	permissionID = ppermissionID;
	processor = pprocessor;

	strncpy(regName, rregName, 64);
	strncpy(regOrg, rregOrg, 64);
}

void BOT_INFO::maskBan()
{
	++machineID;

	++permissionID;

	if (processor != 586) processor = 586;

	Uint32 i;

	for (i = 0; i < STRLEN(regName); ++i)
	{
		regName[i] = ROT13(regName[i]);
	}

	for (i = 0; i < STRLEN(regOrg); ++i)
	{
		regOrg[i] = ROT13(regOrg[i]);
	}
}

void BOT_INFO::setReg(char *rrealName, char *eemail, char *ccity, char *sstate, RegForm_Sex ssex, BYTE aage, bool pplayAtHome, bool pplayAtWork, bool pplayAtSchool)
{
	strncpy(realName, rrealName, 64);
	strncpy(email, eemail, 64);
	strncpy(city, ccity, 64);
	strncpy(state, sstate, 64);

	sex = ssex;
	age = aage;
	playAtHome = pplayAtHome;
	playAtWork = pplayAtWork;
	playAtSchool = pplayAtSchool;
}
