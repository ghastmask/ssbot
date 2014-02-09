/*
	Map file conversion and related constants by cat02e@fsu.edu
*/


#ifndef MAP_H
#define MAP_H

#include "datatypes.h"
#include "prng.h"
#include "settings.h"


#define TILE_MAX_X 0x400
#define TILE_MAX_Y 0x400

#define TILE_MAX_LINEAR 0x100000

#define PIXEL_MAX_X 0x4000
#define PIXEL_MAX_Y 0x4000

Uint32 getLinear(Uint32 x, Uint32 y);									// Convert rectangular coords to linear coords

struct tileData
{
	Uint16 x, y;														// Coordinates
	BYTE type;															// Type
};

Sint32 getNumeric(Sint32 y);
char getAlpha(Sint32 x);
String getCoords(Sint32 x, Sint32 y);

tileData makeTileData(Uint32 raw);										// File data is in blocks of 32 bits
Uint32 makeTileData(Uint16 x, Uint16 y, BYTE type);						// Produce just such a block

void convertFileToMatrix(char *fileData, char *mapData, Uint32 len);	// Convert from disk to memory format

Uint32 getMapSize(char *mapData);										// Retrieve the length of the map on disk
void convertMatrixToFile(char *mapData, char *_fileData);				// Convert the map to disk format

/* VIE map format

 * X	= WORD((N >> 12) & 0xFFF);	// This puts actual map dimensions at 4095x4095 possible tiles.
 * Y	= WORD(N & 0xFFF);
 * TILE = BYTE(v >> 24);

 * 0		= No tile
 * 1-19		= Normal tiles
 * 20		= Border
 * 21-161	= Normal tiles
 * 162-165	= Vertical doors
 * 166-169	= Horizontal doors
 * 170		= Flag
 * 171		= Safe zone
 * 172		= Goal area
 * 173-175	= Fly over tiles
 * 176-191	= Fly under tiles

 * Warning: Deviating from this format may invalidate the security checksum.

 */

enum MapTileFormat
{
	vieNoTile,			// These are VIE constants

	vieNormalStart		= 1,
	vieBorder			= 20,	// Borders are not included in the .lvl files
	vieNormalEnd		= 161,	// Tiles up to this point are part of sec.chk

	vieVDoorStart		= 162,
	vieVDoorEnd			= 165,

	vieHDoorStart		= 166,
	vieHDoorEnd			= 169,

	vieTurfFlag			= 170,

	vieSafeZone			= 171,	// Also included in sec.chk

	vieGoalArea			= 172,

	vieFlyOverStart		= 173,
	vieFlyOverEnd		= 175,
	vieFlyUnderStart	= 176,
	vieFlyUnderEnd		= 190,

	vieAsteroidStart	= 216,
	vieAsteroidEnd		= 218,

	vieStation			= 219,

	vieWormhole			= 220,

	ssbTeamBrick,				// These are internal
	ssbEnemyBrick,

	ssbTeamGoal,
	ssbEnemyGoal,

	ssbTeamFlag,
	ssbEnemyFlag,

	ssbPrize,

	ssbBorder					// Use ssbBorder instead of vieBorder to fill border
};

#endif	// MAP_H
