//TODO: setup paths correctly
#include "../lib/botdb.h"
#include "../lib/system.h"
#include "../lib/settings.h"
//#include "basewin.h"

#include <conio.h>	// kbhit
#include <stdio.h>

int _cdecl main(int argc, char *argv[])
{
	/*
		argv[0] Commandline to exe
		argv[1] First argument
	*/

	// Get API hooks
	beginWinsock();

	// Create bot database
	BOT_DATABASE botDatabase;

	// Load INI
	botDatabase.reloadINI(true);

	// Load operators
	botDatabase.loadOperators();

	// Load aliases
	botDatabase.loadAliases();

	printf("\n%s\nPress any key to close this window when done.\n\n", VERSION_STRING);

	// Load spawns
	botDatabase.loadSpawns();

	// Put up the window title
	setWindowTitle(botDatabase.windowCaption);

	// Message pump
	while (!kbhit())
	{
		// Poll for events
		botDatabase.spawns.doEvents();

		// Clean up when no bots remain
		if (botDatabase.spawns.getConnections() == 0)
		{
			Sleep(5000);
			break;
		}

		// No CPU hogging
		Sleep(5);
	}

	// Leave
	botDatabase.spawns.massDisconnect();

	// Clean up
	endWinsock();

	// Prepare for next session
	botDatabase.saveAliases();
	botDatabase.saveOperators();

	return 0;
}
