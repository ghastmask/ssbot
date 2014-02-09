#include "map.h"

#include "algorithms.h"
#include "checksum.h"

#include <string.h>


//////// Tile data ////////
// Assembly rips by Snrrrub and Coconut Emulator

Uint32 makeTileData(Uint16 x, Uint16 y, BYTE type)
{
	return (x & 0xFFF) | ((y & 0xFFF) << 12) | (type << 24);
}

tileData makeTileData(Uint32 raw)
{
	tileData t;

	t.x = Uint16(raw & 0xFFF);
	t.y = Uint16((raw >> 12) & 0xFFF);
	t.type = BYTE(raw >> 24);

	return t;
}


//////// Map coordinates ////////
// Enormous amount of timed saved by FACTS team

Sint32 getNumeric(Sint32 y)
{
	return (y * 20 / TILE_MAX_Y) + 1;
}

char getAlpha(Sint32 x)
{
	return char('A' + x * 20 / TILE_MAX_X);
}

String getCoords(Sint32 x, Sint32 y)
{
	String s;

	char alpha[2];
	alpha[0] = getAlpha(x);
	alpha[1] = '\0';

	s += alpha;
	s += getNumeric(y);

	return s;
}

Uint32 getLinear(Uint32 x, Uint32 y)
{
	return ((y & 1023) << 10) | (x & 1023);
}

void convertFileToMatrix(char *fileData, char *mapData, Uint32 len)
{
	memset(mapData, 0, TILE_MAX_LINEAR);

	// Skip the tileset (if present)
	if ( (fileData[0] == 'B') &&
		 (fileData[1] == 'M')  )
	{
		Uint32 diff = getLong(fileData, 2);	// works: part of the bitmap standard

		fileData += diff;
		len -= diff;
	}

	// Fill map-tiles
	for (Uint32 i = 0; i < len; i += 4)
	{
		tileData t = makeTileData(getLong(fileData, i));

		if (/*t.x < 0 ||*/ t.x >= TILE_MAX_X) continue;
		if (/*t.y < 0 ||*/ t.y >= TILE_MAX_Y) continue;

		/* optimized by average tile frequency */
		if (t.type < vieAsteroidEnd)
		{
			mapData[getLinear(t.x, t.y)] = t.type;
		}
		else if (t.type == vieAsteroidEnd)
		{
			mapData[getLinear(t.x, t.y)] = t.type;
			mapData[getLinear(t.x, t.y+1)] = t.type;
			mapData[getLinear(t.x+1, t.y)] = t.type;
			mapData[getLinear(t.x+1, t.y+1)] = t.type;
		}
		else if (t.type == vieWormhole)
		{
			for (int x = 0; x < 5; x++)
			{
				mapData[getLinear(t.x+x, t.y)] = t.type;
				mapData[getLinear(t.x+x, t.y+1)] = t.type;
				mapData[getLinear(t.x+x, t.y+2)] = t.type;
				mapData[getLinear(t.x+x, t.y+3)] = t.type;
				mapData[getLinear(t.x+x, t.y+4)] = t.type;
			}
		}
		else if (t.type == vieStation)
		{
			for (int x = 0; x < 6; x++)
			{
				mapData[getLinear(t.x+x, t.y)] = t.type;
				mapData[getLinear(t.x+x, t.y+1)] = t.type;
				mapData[getLinear(t.x+x, t.y+2)] = t.type;
				mapData[getLinear(t.x+x, t.y+3)] = t.type;
				mapData[getLinear(t.x+x, t.y+4)] = t.type;
				mapData[getLinear(t.x+x, t.y+5)] = t.type;
			}
		}
		else
		{
			mapData[getLinear(t.x, t.y)] = t.type;
		}
	}
/*	Commented out because level checksum has been invalidated from time to time for some reason

	// Fill border-tiles
	mapData[0] = char(ssbBorder);
	mapData[getLinear(0, 1023)] = char(ssbBorder);
	mapData[getLinear(1023, 0)] = char(ssbBorder);
	mapData[getLinear(1023, 1023)] = char(ssbBorder);
	for (Uint16 x = 1; x < 1023; ++x)
	{
		mapData[getLinear(x,	0)] = char(ssbBorder);
		mapData[getLinear(x, 1023)] = char(ssbBorder);
		mapData[getLinear(0,	x)] = char(ssbBorder);
		mapData[getLinear(1023,	x)] = char(ssbBorder);
	}
*/
}

Uint32 getMapSize(char *mapData)
{
	Uint32 offset = 0;

	for (Uint16 x = 1; x < 1023; ++x)		// Ignore border
	for (Uint16 y = 1; y < 1023; ++y)
	{
		BYTE t = mapData[getLinear(x, y)];

		if (t == 0)
			continue;

		if ( ((t >= vieNormalStart) && (t <= vieFlyUnderEnd)) ||	// Regular tiles
			 ((t >= vieAsteroidStart) && (t <= vieWormhole)) )		// Animated tiles
		{	// The tile is not internal
			++offset;
		}
	}

	return (offset << 2);
}

void convertMatrixToFile(char *mapData, char *_fileData)
{
	Uint32 offset = 0;
	Uint32 *fileData = (Uint32*)_fileData;

	for (Uint16 x = 1; x < 1023; ++x)		// Ignore border
	for (Uint16 y = 1; y < 1023; ++y)
	{
		BYTE t = mapData[getLinear(x, y)];

		if (t == 0)
			continue;

		if ( ((t >= vieNormalStart) && (t <= vieFlyUnderEnd)) ||	// Regular tiles
			 ((t >= vieAsteroidStart) && (t <= vieWormhole)) )		// Animated tiles
		{	// The tile is not internal
			fileData[offset++] = makeTileData(x, y, t);
		}
	}
}
