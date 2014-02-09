#include "checksum.h"

#include "map.h"
#include "clientprot.h"

#include <fstream>
using namespace std;


//////// Simple Transfer checksum ////////
// Assembly rips by Coconut Emulator

BYTE simpleChecksum(void *buffer, Uint32 len)
{
	BYTE c = 0;

	for (Uint32 i = 0; i < len; ++i)
	{
		c ^= ((BYTE*)buffer)[i];
	}

	return c;
}


//////// File checksum ////////
// This section made possible by Snrrrub

Uint32 getFileChecksum(void *buffer, Uint32 *dictionary, Uint32 len)
{
	Uint32 Index = 0,
		   Key   = Index - 1;

	for (Uint32 i = 0; i < len; ++i)
	{
		Index	= dictionary[(Key & 255) ^ ((BYTE*)buffer)[i]];
		Key		= (Key >> 8) ^ Index;
	}

	return ~Key;
}

Uint32 getFileChecksum(char *fileName, Uint32 *dictionary)
{
	Uint32 chk = 0xffffffff;

	ifstream file(fileName, ios::binary);
	if (file)
	{
		file.seekg (0, ios::end);
		Uint32 width = file.tellg();
		file.seekg (0, ios::beg);

		BYTE *buffer = new BYTE[width];

		file.read((char*)buffer, width);

		chk = getFileChecksum(buffer, dictionary, width);

		delete []buffer;
	}

	return chk;
}

void generate4(Uint32 *offset, Uint32 key)
{
	offset[0] = key;
	offset[1] = key ^ G4_MODIFIER;
	offset[2] = key ^= (G4_MODIFIER << 1);
	offset[3] = key ^ G4_MODIFIER;
}

void generate16(Uint32 *offset, Uint32 key)
{
	generate4(offset, key);
	generate4(offset + 4, key  ^ G16_MODIFIER);
	generate4(offset + 8, key  ^= (G16_MODIFIER << 1));
	generate4(offset + 12, key ^ G16_MODIFIER);
}

void generate64(Uint32 *offset, Uint32 key)
{
	generate16(offset, key);
	generate16(offset + 16, key ^ G64_MODIFIER);
	generate16(offset + 32, key ^= (G64_MODIFIER << 1));
	generate16(offset + 48, key ^ G64_MODIFIER);
}

void generateDictionary(Uint32 *offset, Uint32 key)
{
	generate64(offset, key);
	generate64(offset + 64, key  ^ G256_MODIFIER);
	generate64(offset + 128, key ^= (G256_MODIFIER << 1));
	generate64(offset + 192, key ^ G256_MODIFIER);
}


//////// Security checksum ////////
//  Supreme thanks go to Coconut Emulator and Snrrrub
//  for providing the assembly rips.

Uint32 generateLevelChecksum(Uint32 key, char *mapData)
{	// Snrrrub's literal version
	LONG EAX, ECX, ESI, EDX;
	LONG cnt;

	if (key & 0x80000000)
	{	// key is negative
		ECX = -((-signed(key)) & 0x1F);

		if (ECX >= 1024)
			return key;
	}
	else
	{	// key is positive
		ECX = key & 31;
	}

	EDX = key % 31;

	ESI = (ECX << 0x0A) + *(LONG*)&mapData + EDX;
	EAX = 1024 - EDX;
	cnt = (1024 + 31 - ECX) >> 5;
	mapData = *(char **)&EAX;

	LONG original_key = key;

	for (; cnt > 0; --cnt, ESI += 0x8000)
	{
		EDX = EAX + ESI;
		ECX = ESI;

		if (ESI >= EDX)
			continue;

		while (ECX < EDX)
		{
			BYTE byte = (*(*(char **)&ECX));

			if ((byte) && (byte < 0xA1 || byte == 0xAB))
				key += original_key ^ byte;

			ECX += 31;
		}
		EAX = *(LONG*)&mapData;
	}

	return key;
}

Uint32 generateParameterChecksum(Uint32 key, Uint32 *settings)
{
	Uint32 i, c = 0;

	for (i = 0; i < 357; ++i)
	{
		c += (settings[i] ^ key);
	}

	return c;
}

Uint32 generateEXEChecksum(Uint32 key)
{	// File-less version; developed one lazy Monday..
	Uint32 part, csum = 0;

	part = 0xc98ed41f;
	part += 0x3e1bc | key;
	part ^= 0x42435942 ^ key;
	part += 0x1d895300 | key;
	part ^= 0x6b5c4032 ^ key;
	part += 0x467e44 | key;
	part ^= 0x516c7eda ^ key;
	part += 0x8b0c708b | key;
	part ^= 0x6b3e3429 ^ key;
	part += 0x560674c9 | key;
	part ^= 0xf4e6b721 ^ key;
	part += 0xe90cc483 | key;
	part ^= 0x80ece15a ^ key;
	part += 0x728bce33 | key;
	part ^= 0x1fc5d1e6 ^ key;
	part += 0x8b0c518b | key;
	part ^= 0x24f1a96e ^ key;
	part += 0x30ae0c1 | key;
	part ^= 0x8858741b ^ key;
	csum += part;

	part = 0x9c15857d;
	part += 0x424448b | key;
	part ^= 0xcd0455ee ^ key;
	part += 0x727 | key;
	part ^= 0x8d7f29cd ^ key;
	csum += part;

	part = 0x824b9278;
	part += 0x6590 | key;
	part ^= 0x8e16169a ^ key;
	part += 0x8b524914 | key;
	part ^= 0x82dce03a ^ key;
	part += 0xfa83d733 | key;
	part ^= 0xb0955349 ^ key;
	part += 0xe8000003 | key;
	part ^= 0x7cfe3604 ^ key;
	csum += part;

	part = 0xe3f8d2af;
	part += 0x2de85024 | key;
	part ^= 0xbed0296b ^ key;
	part += 0x587501f8 | key;
	part ^= 0xada70f65 ^ key;
	csum += part;

	part = 0xcb54d8a0;
	part += 0xf000001 | key;
	part ^= 0x330f19ff ^ key;
	part += 0x909090c3 | key;
	part ^= 0xd20f9f9f ^ key;
	part += 0x53004add | key;
	part ^= 0x5d81256b ^ key;
	part += 0x8b004b65 | key;
	part ^= 0xa5312749 ^ key;
	part += 0xb8004b67 | key;
	part ^= 0x8adf8fb1 ^ key;
	part += 0x8901e283 | key;
	part ^= 0x8ec94507 ^ key;
	part += 0x89d23300 | key;
	part ^= 0x1ff8e1dc ^ key;
	part += 0x108a004a | key;
	part ^= 0xc73d6304 ^ key;
	part += 0x43d2d3 | key;
	part ^= 0x6f78e4ff ^ key;
	csum += part;

	part = 0x45c23f9;
	part += 0x47d86097 | key;
	part ^= 0x7cb588bd ^ key;
	part += 0x9286 | key;
	part ^= 0x21d700f8 ^ key;
	part += 0xdf8e0fd9 | key;
	part ^= 0x42796c9e ^ key;
	part += 0x8b000003 | key;
	part ^= 0x3ad32a21 ^ key;
	csum += part;

	part = 0xb229a3d0;
	part += 0x47d708 | key;
	part ^= 0x10b0a91 ^ key;
	csum += part;

	part = 0x466e55a7;
	part += 0xc7880d8b | key;
	part ^= 0x44ce7067 ^ key;
	part += 0xe4 | key;
	part ^= 0x923a6d44 ^ key;
	part += 0x640047d6 | key;
	part ^= 0xa62d606c ^ key;
	part += 0x2bd1f7ae | key;
	part ^= 0x2f5621fb ^ key;
	part += 0x8b0f74ff | key;
	part ^= 0x2928b332;
	csum += part;

	part = 0x62cf369a;
	csum += part;

	return csum;
}


/*
Uint32 generatePartialEXE(Uint32 key, ifstream &file, Uint32 file_offset, Uint32 len)
{
	char buffer[0x0000D5A0];	// largest amount read

	file.seekg(file_offset, ios_base::beg);
	file.read(buffer, len);

	Uint32 data,
		   chk = 0,
		   offset = 0;

	while (len > 4)
	{
		data = getLong(buffer, offset);

		if ((len & 0x7FF) == 0)
			chk += (data | key);
		else
			chk ^= (data ^ key);

		len -= 3;
		offset += 3;
	}

	return chk;
}

Uint32 generateEXEChecksum(Uint32 key, char *fileName)
{
	ifstream file(fileName, ios::binary);
	if (!file) return -1;

	Uint32 chk = 0;

	chk += generatePartialEXE(key, file, 0x00006940, 0x0000D5A0);
	chk += generatePartialEXE(key, file, 0x00000400, 0x00002480);
	chk += generatePartialEXE(key, file, 0x0001CAE0, 0x00006F70);
	chk += generatePartialEXE(key, file, 0x00042E70, 0x000021E0);
	chk += generatePartialEXE(key, file, 0x000305A0, 0x0000D3C0);
	chk += generatePartialEXE(key, file, 0x0002A5B0, 0x00005FE0);
	chk += generatePartialEXE(key, file, 0x00003960, 0x00001C90);
	chk += generatePartialEXE(key, file, 0x00013EF0, 0x00008BE0);
	chk += generatePartialEXE(key, file, 0x00005750, 0x000000E0);

	return chk;
}
*/
