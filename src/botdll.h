/*
	Import bot behavior from DLL's by cat02e@fsu.edu

	Cyan~Fire fixed MinGW compilation for DLLImports::talk()
*/


#ifndef BOT_H
#define BOT_H

#include "dllcore.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef void (__stdcall *CALL_TALK)(BotEvent &event);

#define DLL_MAX_LOADED	32
#define DLL_NAMELEN		256

class DLLImports
{
	CALL_TALK DLL_TALK[DLL_MAX_LOADED];
	HMODULE DLLhMod[DLL_MAX_LOADED];
	char ModuleName[DLL_MAX_LOADED][DLL_NAMELEN];

	class Host *h;

	void talk(int slot, BotEvent event);

public:
	void talk(BotEvent event);			// Talk to the DLL
	bool importLibrary(char *file);		// Load callbacks from DLL
	void clearImports();				// Remove all references
	void clearImport(int slot);			// Remove reference
	char *getPlugin(int slot);			// Get plugin name, returns NULL if no plugin loaded

	DLLImports(Host &);					// Initialize all callbacks
	~DLLImports();						// Disable all callbacks
};

#endif	// BOT_H
