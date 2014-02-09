/*
	Math hacks by cat02e@fsu.edu
*/


#ifndef HACK_H
#define HACK_H

#include "datatypes.h"

double solveY(String &function, double x);

BYTE TriangulateFireAngle(const Vector &rel, const Vector &vel, Sint32 scalar);	// use prediction

BYTE TriangulateFireAngle(const Vector &rel);								// no prediction

BYTE oppositeDirection(BYTE d);										// opposite direction

#endif	// HACK_H
