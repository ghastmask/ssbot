/*
	Advanced coding macros by cat02e@fsu.edu
*/


#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "datatypes.h"


Uint32 SQRT(Uint32 factor);	// Fast integer square root


Uint32 distance(Uint32 a, Uint32 b);	// 1D distance
Sint32 distance(Sint32 a, Sint32 b);	// 1D distance


Sint32 sgn(Sint32 a);	// Return the sign


Uint32 getTime();	// Return time since boot in hundredths of a second


char tolower(char c);			// Change upper-case to lower-case letters
void tolower(char *buffer);		// Use tolower on a whole string

char toupper(char c);			// Change lower-case to upper-case letters
void toupper(char *buffer);		// Use toupper on a whole string


bool isPrintable(char c);		// Determine if you can see this character
bool isAlphaNumeric(char c);	// Determine if this character is alpha-numeric

bool isNumeric(char *buffer);	// Determine if this string is entirely numerical


// This f(n) is weird.  Ex:
//	char out[3][32];
//	split(':', "test:test:test", (char*)out, 3, 32);
// ^^ note how you *have* to cast the variable 'out'
bool split(char d, char *in, char *out, Uint32 lx, Uint32 ly);	// Parse string with delimiter (inline: read notes in math.h)


void trimString(char d, String &s);	// Remove all instances of (char) d


String getString(Uint32 number, Uint32 base, Uint16 leading, bool sign);	// Convert binary numbers to ASCIIZ number representations

int getInteger(char *number, int base);										// Convert ASCIIZ numbers to binary number representations


Uint32 trim(Uint32 large, Uint32 little);	// Useful for clipping calcs


Uint32 limit(Uint32 a, Uint32 max);	// Useful to avoid buffer overflows


void swap(void *&a, void *&b);	// Quick swap using XOR
void swap(Uint32 &a, Uint32 &b);	// Quick swap using XOR
void swap(Uint16 &a, Uint16 &b);	// Quick swap using XOR


Uint32 IMULHIDWORD(Uint32 A,
				   Uint32 B);	// Sort of like a modulus operator for multiplication


void IDIVCOMP(Uint32 value,
			  Uint32 width,
			  Uint32 &x,
			  Uint32 &y);	// Do signed division and modulus in the same step


Sint32 neg_quadratic(Sint32 a, Sint32 b, Sint32 c);	// Quadratic formula
Sint32 pos_quadratic(Sint32 a, Sint32 b, Sint32 c);	// Quadratic formula


Uint32 distance(Uint32 x1,
				Uint32 y1,
				Uint32 x2,
				Uint32 y2);	// d = sqrt((x1-x2)^2 + (y1-y2)^2)

Uint32 distancesqr(Uint32 x1,
				   Uint32 y1,
				   Uint32 x2,
				   Uint32 y2);	// dsqr = (x1-x2)^2 + (y1-y2)^2


Uint32 ROL(Uint32 x, Uint32 r);	// Emulate ROL opcode
Uint32 ROR(Uint32 x, Uint32 r);	// Emulate ROR opcode
Uint32 ROT(Uint32 x, Sint32 r);	// Negative numbers -> ROR, Positives -> ROL

#endif	// ALGORITHMS_H
