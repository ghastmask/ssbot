#include "encrypt.h"

#include "checksum.h"
#include "algorithms.h"

#include <string.h>


//////// Misc. encryption ////////

char ROT13(char c)
{
	if (c >= 'a' && c <= 'z')
	{
		c += 13;
		if (c > 'z') c -= 26;
	}
	else if (c >= 'A' && c <= 'Z')
	{
		c += 13;
		if (c > 'Z') c -= 26;
	}

	return c;
}


//////// SubSpace packet encryption ////////
// Courtesy of Coconut Emulator

SS_ENCR::SS_ENCR()
{
	key = 0;
	sentKey = 0;
}

Uint32 SS_ENCR::generateKey()
{
	if (sentKey)
		return sentKey;

	Uint32 edx = getTime() * 0xCCCCCCCD;

	Uint32 res = (keygen.getNext() << 16) + (edx >> 3) + keygen.getNext();

	res = (res ^ edx) - edx;

	if (res <= 0x7fffffff) res = ~res + 1;

	return (sentKey = res);
}

Uint32 SS_ENCR::getSessionKey(Uint32 clientKey)
{
	return ((~clientKey) + 1);	// Two's complement, same as arithmetic minus on unsigned data
}

bool SS_ENCR::validateSessionKey(Uint32 serverKey)
{
	return ((serverKey == sentKey) || (serverKey == getSessionKey(sentKey)));
}

bool SS_ENCR::initializeEncryption(Uint32 serverKey)
{
	if (!validateSessionKey(serverKey)) return false;

	if (sentKey == serverKey)
	{
		key = 0;

		memset(keystream, 0, 520);
	}
	else
	{
		key = serverKey;

		Uint16 *stream = (Uint16*)keystream;

		prng.seed(serverKey);

		for (Uint32 i = 0; i < 260; ++i)
		{
			stream[i] = prng.getNextE();
		}
	}

	return true;
}

void SS_ENCR::encrypt(char *msg, Uint32 len)
{
	if (!key) return;

	Uint32 ksi = 0,
		   i = 1,
		   IV = key;

	if (*msg == 0)
	{
		if (len <= 2) return;

		++i;
	}

	while (i + 4 <= len)
	{
		*(Uint32*)&msg[i] = IV = (getLong(msg, i) ^ getLong(keystream, ksi) ^ IV);

		i += 4;
		ksi += 4;
	}

	Uint32 diff = len - i;

	if (diff)
	{
		Uint32 buffer = 0;
		memcpy(&buffer, msg + i, diff);
		buffer ^= getLong(keystream, ksi) ^ IV;
		memcpy(msg + i, &buffer, diff);
	}
}

void SS_ENCR::decrypt(char *msg, Uint32 len)
{
	if (!key) return;

	Uint32 ksi = 0,
		   i = 1,
		   IV = key,
		   EDX;

	if (*msg == 0)
	{
		if (len <= 2) return;

		++i;
	}

	while (i + 4 <= len)
	{
		EDX = getLong(msg, i);
		*(Uint32*)&msg[i] = getLong(keystream, ksi) ^ IV ^ EDX;
		IV = EDX;

		i += 4;
		ksi += 4;
	}

	Uint32 diff = len - i;

	if (diff)
	{
		Uint32 buffer = 0;
		memcpy(&buffer, msg + i, diff);
		buffer ^= getLong(keystream, ksi) ^ IV;
		memcpy(msg + i, &buffer, diff);
	}
}

void SS_ENCR::reset()
{
	key = 0;
	sentKey = 0;
}


//////// Billing password encryption ////////
// Also from Coconut Emulator

void hashPassword(BYTE *in, BYTE *out)
{
	Uint32 L, StrLen = STRLEN((char*)in);
	BYTE Factor = simpleChecksum(in, StrLen);
	BYTE Char;

	for (L = 0; L < StrLen; ++L)
	{
		Char = in[L] ^ Factor;
		Factor = (Factor ^ (Char << (Char & 3))) & 255;
		if (Char == 0)
			Char = 0xED;
		out[L] = Char;
	}

	out[L] = 0;
}


//////// Billing password decryption ////////

void inverseHash(BYTE *In, BYTE *Out, BYTE Key)
{
	size_t StrLen = STRLEN((char*)In);

	for (Uint32 L = 0; L < StrLen; ++L)
	{
		BYTE Char = In[L];
		if (Char == 0xED)
			Char = 0;
		Out[L] = Char ^ (BYTE)Key;
		Key = (Key ^ (Char << (Char & 3))) & 255;
	}
}

void decryptHashedPassword(BYTE *Password)
{
	Uint32 StrLen = STRLEN((char*)Password);

	BYTE *Buffer = new BYTE[StrLen + 1];
	Buffer[StrLen] = 0;

	// Passwords of EVEN length are very easy to crack.
	if (StrLen & 1)
	{
		// Guess and check to find one of the solutions
		for (int i = 0; i < 256; ++i)
		{
			// Generate a possible solution
			inverseHash(Password, Buffer, i);

			// Compare resultant hash with given hash
			BYTE Char, Key = i;
			bool OK = true;

			for (Uint32 L = 0; L < StrLen; ++L)
			{
				Char = Buffer[L];

				if (!isPrintable(Char))
				{
					OK = false;

					break;
				}

				Char ^= Key;
				Key = (Key ^ (Char << (Char & 3))) & 255;
				if (Char == 0)
					Char = 0xED;

				if (Char != Password[L])
				{
					OK = false;

					break;
				}
			}

			if (OK)
				break;
		}
	}
	else
	{
		// Generate password checksum
		inverseHash(Password, Buffer, 0);

		// Generate actual password
		inverseHash(Password, Buffer, simpleChecksum(Buffer, StrLen));
	}

	memcpy(Password, Buffer, StrLen);

	delete Buffer;
	Buffer = NULL;
}
