/*
	SubSpace pseudo-random number generators by cat02e@fsu.edu
*/


#ifndef PRNG_H
#define PRNG_H

#include "datatypes.h"

class LCG_PRNG
{	// Internal bot generator
	Sint32 s1,	// Seeds
		   s2;	// Seeds

public:
	LCG_PRNG();	// Default initialization

	double getNextD();	// (double) Get next random number
	Sint32 getNextI();	// (long) Get next random number

	void seed(Sint32 newSeed);	// Automatic seeding
	void seed(Sint32 newSeedX,
			  Sint32 newSeedY);	// Manual seeding
};


class LFSR_PRNG
{	// Internal bot generator
	Sint32	p1, p2;				// Seed indices
	Uint32	randbuffer[11];		// Seed - matter

public:
	LFSR_PRNG();	// Default initialization

	double getNextD();	// Do (double) conversion too
	Uint32 getNextI();	// Get the next number in the series

	void seed(Uint32 newSeed);	// Re-seed the generator (rather secure)
};


struct SS_LIGHT_PRNG
{	// Assembly provided by Coconut Emulator
	// Used to generate 00 01 connection requests
	Uint32 s;					// Seed

	SS_LIGHT_PRNG();	// Default seed(0)
	SS_LIGHT_PRNG(Uint32 newSeed);	// Provide a seed

	Uint16 getNext();	// Get the next number

	void seed(Uint32 newSeed);		// Provide a seed
};


const Uint32 KSGSCD = 0x1F31D;
const Uint32 KSGSCM = 0x834E0B5F;

struct SS_HEAVY_PRNG
{	// Assembly provided by Coconut Emulator and Kavar!
	// Used to generate the keystream and transform synchronized seeds
	Uint32 s;					// Seed

	SS_HEAVY_PRNG();	// Default seed(0)
	SS_HEAVY_PRNG(Uint32 newSeed);	// Provide a seed

	Uint16 getNextE();				// Get the next number for encryption
	Uint16 getNextG();				// Get the next number for green seeds

	void seed(Uint32 newSeed);		// Provide a seed
};

#endif	// PRNG_H
