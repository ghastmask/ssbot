#include "hack.h"

#include "algorithms.h"

#include <math.h>


BYTE TriangulateFireAngle(const Vector &pos, const Vector &vel, Sint32 scalar)
{
	// pos = relative positions
	// vel = relative velocities
	// scalar = velocity of bullets

	double a = vel.x * vel.x + vel.y * vel.y - scalar;
	double b = 2 * (pos.x * vel.x + pos.y * vel.y);
	double c = pos.x * pos.x + pos.y * pos.y;
	double time = (-b - sqrt((b * b) - (a * c * 2))) / (2.0 * a);
	if (time <= 0.0)
		time = (-b + sqrt((b * b) - (a * c * 2))) / (2.0 * a);

	return TriangulateFireAngle(Vector(pos.x + Sint32(double(vel.y) * time), pos.y + Sint32(double(vel.y) * time)));
}

BYTE TriangulateFireAngle(const Vector &rel)
{
	double	dx = -rel.x,
			dy = rel.y;

	if (dy == 0)
		if (dx > 0)
			return 10;
		else
			return 30;

	double angle = atan(dx / dy) + PI;

	if (dy >= 0)
	{
		if (dx >= 0)
			angle -= PI;
		else
			angle += PI;
	}

	return BYTE(angle * 40.0 / TwoPI);
}

BYTE oppositeDirection(BYTE d)
{
	return (d + 20) % 40;
}

/*
String generateName()
{
	SS_LIGHT_PRNG prng(getTime());
	String s;
	Uint16 i;
	char str[2];
	str[1] = 0;

	for (Uint16 x = 0; x < 19; ++x)
	{
		i = prng.getNext();

		if (i & 1)
		{	// caps
			i = prng.getNext();
			*str = 'A' + (i % 26);
		}
		else
		{	// no caps
			i = prng.getNext();
			*str = 'a' + (i % 26);
		}

		s += str;
	}

	return s;
}

void dumpUptime()
{
	Uint32 time = GetTickCount();

	Uint32 minutes = time / (1000 * 60) % 60;
	Uint32 hours = time / (1000 * 60 * 60) % 24;
	Uint32 days = time / (1000 * 60 * 60 * 24);

	printf("System has been online for %i days %i hours %i minutes\n", days, hours, minutes);
}
*/

