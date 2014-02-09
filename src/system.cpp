#include "system.h"

#include "algorithms.h"

#include <fstream>
using namespace std;

#include "zlib.h"
#pragma comment(lib, "zlib.lib")


//////// Settings macros ////////

Uint32 getPrivateProfile32(char *section, char *key, char *def, char *path)
{
	char buffer[32];

	GetPrivateProfileString(section, key, def, buffer, 32, path);

	return getInteger(buffer, 10);
}

Uint32 getSetting32(HKEY baseKey, const char *path, const char *value)
{
	// Open the key
	HKEY key;			// Handle to a session with a registry key

	RegOpenKey((HKEY)baseKey, path, &key);

	// Get a value
	DWORD buffer;		// Results of transaction will go here
	DWORD buflen = 4;	// Length of the buffer is 4, naturally
	DWORD type;			// Type will contain type of data transfered

	RegQueryValueEx(key, value, NULL, &type, (BYTE*)&buffer, &buflen);

	// Close the key
	RegCloseKey(key);

	// Check data for validity
	if (type == REG_DWORD)
		return buffer;
	else
		return 0xffffffff;			// -1 = error
}

void setSetting32(HKEY baseKey, const char *path, const char *value, Uint32 buffer)
{
	// Open the key
	HKEY key;			// Handle to a session with a registry key

	RegCreateKey(baseKey, path, &key);

	// Get a value
	RegSetValueEx(key, value, 0, REG_DWORD, (BYTE*)&buffer, 4);

	// Close the key
	RegCloseKey(key);
}

void getServiceString(HKEY baseKey, const char *path, const char *value, char *buffer)
{
	// Open the key
	HKEY key;			// Handle to a session with a registry key

	RegOpenKey(baseKey, path, &key);

	// Get a value
	DWORD buflen = 40;
	DWORD type;			// Type will contain type of data transfered

	RegQueryValueEx(key, value, NULL, &type, (BYTE*)buffer, &buflen);

	// Close the key
	RegCloseKey(key);
}

void addNewsChecksum(Uint32 Checksum)
{
	// Extract location to write the new checksum
	Uint32 Position = getSetting32(HKEY_CURRENT_USER, "Software\\Virgin\\SubSpace\\News", "Pos");

	String s;
	if (Position < 1000)
		s += "0";
	if (Position < 100)
		s += "0";
	if (Position < 10)
		s += "0";
	s += (int)Position;

	// Write the checksum
	setSetting32(HKEY_CURRENT_USER, "Software\\Virgin\\SubSpace\\News", s.msg, Checksum);

	// Calculate new location to write the next checksum
	++Position;
	if (Position == 10000)
		Position = 0;

	// Write this location to the registry
	setSetting32(HKEY_CURRENT_USER, "Software\\Virgin\\SubSpace\\News", "Pos", Position);
}

void setWindowTitle(char *title)
{
	char fileName[532];
	Sint32 len = GetModuleFileName(GetModuleHandle(NULL), fileName, 532);

	if (!SetWindowText(FindWindow(NULL, fileName), title))
	{
		Sint32 off = -1;

		for (Sint32 i = len - 1; i > 1; --i)
		{
			switch (fileName[i])
			{
			case '/':
			case '\\':
				off = i + 1;
				i = 0;
				break;
			case '.':
				fileName[i] = '\0';
			};
		}

		if (off != -1)
		{
			SetWindowText(FindWindow(NULL, fileName + off), title);
		}
	}
}

bool readDataLines(char *name, void (*callback)(char *line))
{
	ifstream file(name);

	if (!file)
		return false;

	char c, buffer[256];
	int i = 0;
	bool skip = false;

	while ((c = file.get()) != -1)
	{
		switch (c)
		{
		case '\n':
			buffer[i] = '\0';
			if ((i > 0) && !skip) callback(buffer);
			i = 0;
			skip = false;
			break;
		default:
			if (i >= 255) break;
			if (i == 0) skip = !isAlphaNumeric(c);
			buffer[i++] = c;
		case '\r':;
		}
	}

	buffer[i] = '\0';
	if (i > 0) callback(buffer);

	return true;
}

bool _stdcall decompress_to_file(char *name, void *buffer, Uint32 buffer_length)
{
	if (buffer_length <= 0) return false;

	ofstream file(name, ios::binary);

	if (!file) return false;

	BYTE  *res	  = NULL;
	Uint32 length = buffer_length * 2,
		   status = 0;

	do
	{
		if (res)
			delete []res;

		length += buffer_length;
		res = new BYTE[length];
		if (!res) return false;

		status = uncompress(res, &length, (BYTE*)buffer, buffer_length);
	} while (status == Z_BUF_ERROR);

	if (status != Z_OK) return false;

	file.write((char*)res, length);

	delete []res;

	return true;
}
