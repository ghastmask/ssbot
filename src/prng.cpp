#include "prng.h"

#include "algorithms.h"


//////// Linear Congruential Generator ////////

LCG_PRNG::LCG_PRNG()
{
	s1 = 99999;
	s2 = 55555;
}

Sint32 LCG_PRNG::getNextI()
{
	Uint32 x, y;

	IDIVCOMP(s1, 53668, x, y);
	s1 = (x * 40014) - (y * 12211);
	if (s1 < 0) s1 += 0x7FFFFFAB;

	IDIVCOMP(s1, 52774, x, y);
	s2 = (x * 40692) - (y * 3791);
	if (s2 < 0) s2 += 0x7FFFFF07;

	Uint32 z = (s1 - 0x7FFFFFAB) + s2;
	if (z <= 0) z += 0x7FFFFFAA;

	return z;
}

double LCG_PRNG::getNextD()
{
	return getNextI() / double(0xFFFFFFFF);
}

void LCG_PRNG::seed(Sint32 newSeed)
{
	s1 = 55555 ^ newSeed;
	s2 = newSeed;
}

void LCG_PRNG::seed(Sint32 newSeedX, Sint32 newSeedY)
{
	s1 = newSeedX;
	s2 = newSeedY;
}


//////// Linear Feedback Shift Register Generator ////////

LFSR_PRNG::LFSR_PRNG()
{
	seed(55555);
}

void LFSR_PRNG::seed(Uint32 newSeed)
{
	if (newSeed == 0) --newSeed;	// Set seed to 0xFFFFFFFF
	Sint32 i;

	for (i = 0; i < 11; ++i)
	{	// Loses bits pretty easily, which is good if
		// you want to keep the original seed secret.
		newSeed ^= newSeed << 13;
		newSeed ^= newSeed >> 17;
		newSeed ^= newSeed << 5;
		randbuffer[i] = newSeed;
	}

	p1 = 0;
	p2 = 7;

	for (i = 0; i < 9; ++i)	// Mask original seed further
		getNextI();			// Skip the (double) conversion
}

Uint32 LFSR_PRNG::getNextI()
{
	Uint32 x = (randbuffer[p1] = ROR(randbuffer[p1] + randbuffer[p2], 13));

	if (--p1 < 0) p1 = 10;	// Spin the counters
	if (--p2 < 0) p2 = 10;

	return x;
}

double LFSR_PRNG::getNextD()
{	// Using (GetNextI() / MAX) shouldn't make a difference
	Uint32 x = getNextI();

	union
	{
		double	randp1;
		Uint32	randbits[2];
	};

	randbits[0] = (x << 20);
	randbits[1] = (x >> 12) | 0x3FF00000;

	return randp1 - 1.0;
}


//////// Simple SubSpace LCG ////////

SS_LIGHT_PRNG::SS_LIGHT_PRNG()
{
	seed(0);
}

SS_LIGHT_PRNG::SS_LIGHT_PRNG(Uint32 newSeed)
{
	seed(newSeed);
}

void SS_LIGHT_PRNG::seed(Uint32 newSeed)
{
	s = newSeed;
}

Uint16 SS_LIGHT_PRNG::getNext()
{
	s *= 0x343FD;	// Has some nice properties, but
	s += 0x269EC3;	//  ultimately fails to pass any
					//  randomality test.

	return Uint16((s >> 10) & 0x7FFF);	// Return the high word
}


//////// SubSpace Remainder Congruential Generator ////////

SS_HEAVY_PRNG::SS_HEAVY_PRNG()
{
	seed(0);
}

SS_HEAVY_PRNG::SS_HEAVY_PRNG(Uint32 newSeed)
{
	seed(newSeed);
}

void SS_HEAVY_PRNG::seed(Uint32 newSeed)
{
	s = newSeed;
}

Uint16 SS_HEAVY_PRNG::getNextE()
{	// Original C++ implementation contributed by UDP
	Uint32 old_seed = s;

	s = (Sint32)(((Sint64)old_seed * KSGSCM) >> 48);
	s = s + (s >> 31);
	s = ((old_seed % KSGSCD) * 16807) - (s * 2836) + 123;
	if ((Sint32)s <= 0) s += 0x7fffffff;

	return Uint16(s);
}

Uint16 SS_HEAVY_PRNG::getNextG()
{
	Uint32 TSeed = IMULHIDWORD(s, KSGSCM) + s;
	TSeed = ((long)TSeed >> 16) + (TSeed >> 31);
	s = ((s % KSGSCD) * 16807) - (TSeed * 2836) + 123;
	if ((Sint32)s <= 0) s += 0x7fffffff;

	return Uint16(s);
}
