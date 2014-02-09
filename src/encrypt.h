/*
	Various SubSpace encryption schemes by cat02e@fsu.edu
*/


#ifndef ENCRYPT_H
#define ENCRYPT_H

#include "datatypes.h"
#include "prng.h"
#include <stddef.h>

char ROT13(char c);							// ROT13

struct SS_ENCR
{
	Uint32 key;								// Session key
	Uint32 sentKey;							// Client key

	SS_LIGHT_PRNG keygen;					// Key generator
	SS_HEAVY_PRNG prng;						// Keystream generator

	char keystream[520];					// Session keystream

	SS_ENCR();								// Set key = 0

	Uint32 generateKey();					// Make and set the client key

	Uint32 getSessionKey(Uint32 clientKey);	// Create a session key from a client key
	bool validateSessionKey(Uint32 key);	// Make sure the key is valid

	bool initializeEncryption(Uint32 key);	// Fill keystream, or revoke session

	void encrypt(char * msg, Uint32 len);	// Encrypt message
	void decrypt(char * msg, Uint32 len);	// Decrypt message

	void reset();							// Nullify encryption
};


void HashPassword(BYTE * in, BYTE * out);	// Billing server password hashing


void InverseHash(size_t StrLen,
				 BYTE * In,
				 BYTE * Out,
				 BYTE Key);					// Perform the inverse of HashPassword

bool InvalidPasswordCharacter(BYTE c);		// Simple rule used to make the results readable

void DecryptHashedPassword(BYTE * Password);// Put it all together

#endif	// ENCRYPT_H
