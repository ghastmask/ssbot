#include "algorithms.h"
#include <cstring>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//////// Integer square root ////////
// Jim Ulery's revision of Mark Borgerding's square root algorithm

Uint32 SQRT(Uint32 val)
{
	Uint32 temp,
		   g = 0,
		   b = 0x8000,
		   bshft = 15;

	do
	{
		if (val >= (temp = (((g << 1) + b) << bshft--)))
		{
			g += b;
			val -= temp;
		}
	} while (b >>= 1);

	return g;
}

Sint32 neg_quadratic(Sint32 a, Sint32 b, Sint32 c)
{
	return (-b - SQRT(b * b - a * c * 4)) / (2 * a);
}

Sint32 pos_quadratic(Sint32 a, Sint32 b, Sint32 c)
{
	return (-b + SQRT(b * b - a * c * 4)) / (2 * a);
}


//////// Time ////////

Uint32 getTime()
{
	return GetTickCount() / 10;
}


//////// Characters ////////


bool isNumeric(char *buffer)
{
	if (!*buffer) return false;

	char c = *buffer++;

	if (c == '-') c = *buffer++;

	do
	{
		if (c < '0' || c > '9') return false;
	} while (c = *buffer++);

	return true;
}

char tolower(char c)
{
	if ((c >= 'A') && (c <= 'Z'))
	{
		c += 'a' - 'A';
	}

	return c;
}

void tolower(char *buffer)
{
	char c;

	while (c = *buffer)
	{
		*buffer = tolower(c);

		++buffer;
	}
}

char toupper(char c)
{
	if ((c >= 'a') && (c <= 'z'))
	{
		c += 'A' - 'a';
	}

	return c;
}

void toupper(char *buffer)
{
	char c;

	while (c = *buffer)
	{
		*buffer = toupper(c);

		++buffer;
	}
}

void trimString(char d, String &s)
{
	char *msg = s.msg, ch;
	Uint32 len = 0, off = 0;

	while (ch = msg[off])
		if (ch == d) ++off;
		else { msg[0] = msg[off]; ++len; ++msg; }

	msg[0] = 0;
	s.len = len;
}

bool split(char d, char *in, char *out, Uint32 lx, Uint32 ly)
{
	Uint32 x = 0, y = 0;
	Uint32 start = 0, end = 0;

	bool inHeader = true;	// skipping leading & trailing spaces

	Sint32 slen = (Sint32)std::strlen(in) + 1;

	for (Sint32 i = 0; i < slen; ++i)
	{
		char c = in[i];

		if (c == 0 ||
			c == d	)
		{
			if (x >= lx) return true;

			if (inHeader == false)
			{
				Uint32 len = limit(end - start + 1, ly - 1);
				memcpy(out, in + start, len);
				out[len] = 0;
				inHeader = true;
				start = 0;
				end = 0;
			}
			else
			{
				*out = 0;
			}

			out += ly;
			++x;
		}
		else if (c != ' ')
		{
			if (inHeader)
			{
				start = i;
				inHeader = false;
			}

			end = i;
		}
	}

	return (x == lx);
}

char *basetable	= "0123456789abcdefghijklmnopqrstuv",
	 *zeroes	= "00000000000000000000000000000000",
	 *negsign	= "-";

String getString(Uint32 number, Uint32 base, Uint16 leading, bool sign)
{
	String ret;

	if (number == 0)
	{
		if (leading < 1) leading = 1;

		ret.set(zeroes, leading);

		return ret;
	}

	if (sign)
		if (number & 0x80000000)
			number = ~number + 1;
		else
			sign = false;

	do
	{
		Uint32 digit = number % base;
		number /= base;
		ret.prepend(basetable + digit, 1);
	} while (number);

	Sint32 diff = leading - ret.len;

	if (diff > 0)
		ret.prepend(zeroes, (Uint32)diff);

	if (sign)
		ret.prepend(negsign, 1);

	return ret;
}

int getInteger(char *number, int base)
{
	bool neg = (*number == '-');

	if (neg) ++number;

	int i = 0, m = 1, len = std::strlen(number);

	for (int index = len - 1; index >= 0; --index)
	{
		int value = (int)tolower(number[index]);

		if (value >= '0' && value <= '9')
			value -= '0';
		else
		{
			value -= 'a';
			value += 10;
		}

		i += value * m;
		m *= base;
	}

	if (neg) i = -i;

	return i;
}


//////// 1D math ////////

Uint32 distance(Uint32 a, Uint32 b)
{
	if (b < a)
		return a - b;
	else
		return b - a;
}

Sint32 distance(Sint32 a, Sint32 b)
{
	if (b < a)
		return a - b;
	else
		return b - a;
}


Sint32 sgn(Sint32 a)
{
	if (a > 0)
		return 1;
	else if (a < 0)
		return -1;
	else
		return 0;
}


Uint32 trim(Uint32 large, Uint32 little)
{
	if (large <= little)
		return 0;												// If the subtraction would produce a negative number (or 0 for speed), return 0.
	return (large - little);											// Else, do the subtraction
}


Uint32 limit(Uint32 a, Uint32 max)
{
	if (a <= max)
		return a;
	return max;
}


//////// Quick swaping ////////

void swap(Uint32 &a, Uint32 &b)
{
	a ^= b;
	b ^= a;
	a ^= b;
}

void swap(Uint16 &a, Uint16 &b)
{
	a ^= b;
	b ^= a;
	a ^= b;
}


//////// Multiplication overflow ////////

Uint32 IMULHIDWORD(Uint32 A, Uint32 B)
{
	Uint32 HDW;
	
#ifdef __GNUC__ //Cyan~Fire: Added MinGW-style asm
    asm("imull %0" 
        : "=d"(HDW) 
        : "a"(A), "q"(B) );
#else
	__asm
	{
		mov		eax, A
		imul	B
		mov		HDW, edx
	}
#endif

	return HDW;
}


//////// Quick component separation ////////

void IDIVCOMP(Uint32 value, Uint32 width, Uint32 &x, Uint32 &y)
{
	Uint32 a, b;

#ifdef __GNUC__ //Cyan~Fire
    asm("cdq\n\t"
        "idivl %0"
        : "=d"(a), "=a"(b) 
        : "a"(value), "q"(width) );
#else
	__asm
	{
		mov		eax, value
		cdq
		idiv	width
		mov		a, edx
		mov		b, eax
	}
#endif

	x = a;
	y = b;
}


//////// 2D math ////////

Uint32 distance(Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2)
{
	return SQRT(distancesqr(x1, y1, x2, y2));
}

Uint32 distancesqr(Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2)
{
	Uint32 dx = distance(x1, x2);
	Uint32 dy = distance(y1, y2);

	return dx * dx + dy * dy;
}


//////// Quick rotation ////////

Uint32 ROL(Uint32 x, Uint32 r)
{	// Can be done with _emit and ROL/ROR opcodes
	r &= 31;

	return (x << r) | (x >> (32 - r));
}

Uint32 ROR(Uint32 x, Uint32 r)
{
	r &= 31;

	return (x >> r) | (x << (32 - r));
}

Uint32 ROT(Uint32 x, Sint32 r)
{
	if (sgn(r) == 1)
		return ROL(x, r);
	else
		return ROR(x, -r);
}
