/*
	System interface by cat02e@fsu.edu
*/


#ifndef SYSTEM_H
#define SYSTEM_H

#include "datatypes.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>

Uint32 getPrivateProfile32(char *section, char *key, char *def, char *path);

Uint32 getSetting32(HKEY baseKey, const char *path, const char *value);

void setSetting32(HKEY baseKey, const char *path, const char *value, Uint32 buffer);

void getServiceString(HKEY baseKey, const char *path, const char *value, char *buffer);

// Add news checksum to SubSpace news checksum archive in registry
void addNewsChecksum(Uint32 Checksum);

// Works only once! Changes DOS prompt window title
void setWindowTitle(char *title);

// Extract data lines from a mixed-format database file
bool readDataLines(char *file, void (*callback)(char *line));

// Decompress buffer to a file on disk
bool _stdcall decompress_to_file(char *name, void *buffer, Uint32 len);

#endif	// SYSTEM_H
