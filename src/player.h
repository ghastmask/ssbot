/*
	SubSpace actor descriptor by cat02e@fsu.edu
*/


#ifndef PLAYER_H
#define PLAYER_H

#include "datatypes.h"

// Enumerations

#ifndef MAX_TEAMS
	#define MAX_TEAMS 9999
	#define MAX_PUBLIC_TEAMS 100
	#define MAX_USERS 65535
#endif

enum Operator_Level
{
	OP_Player,				// Requests information services
	OP_Limited,				// Player with some privelages
	OP_Moderator,			// Represents player interests
	OP_SuperModerator,		// Manages moderators
	OP_SysOp,				// Adds/removes/modifies available bot services
	OP_Owner,				// Changes internal bot settings
	OP_Duke,
	OP_Baron,
	OP_King,
	OP_Emperor,
	OP_RockStar,
	OP_Q,
	OP_God					// ...Some other ways to say "Owner"
};

char *getLevelString(Operator_Level access);

char *getPrizeString(Sint16 prize);

// Score

struct Score
{
	Uint32 killPoints, flagPoints;
	Uint16 wins, losses;
};

// Player

struct Player
{
	Uint16 ident;
	Operator_Level access;
	Player *turret;

	Uint16 flagCount;

	Uint32 lastPositionUpdate;
	bool acceptsAudio;

	char name[20];
	char squad[20];
	BYTE banner[96];

	BYTE ship;
	BYTE d;

	Uint16 bounty;
	Uint16 energy;

	Uint16 timer;
	Uint16 S2CLag;

	Vector tile, pos, work, vel;

	bool stealth, cloak, xradar, awarp, ufo, flash, safety;

	bool shields, supers;
	BYTE burst, repel, thor, brick, decoy, rocket, portal;

	Uint16 team;

	Score score;

	bool koth;

	void setBanner(char *bbanner);

	void clone(Player *p);
	void move(Sint32 x, Sint32 y, Sint32 vx, Sint32 vy);
	void move(Sint32 x, Sint32 y);
	void move(Sint32 time);

	Player(Uint16 iident, char *nname, char *ssquad, Uint32 fflagPoints, Uint32 kkillPoints, Uint16 tteam, Uint16 wwins, Uint16 llosses, BYTE sship, bool aacceptsAudio, Uint16 fflagCount);
};

// Brick

struct Brick
{
	Uint16 x, y;
	Uint32 time;
	Uint16 team;
	Uint16 ident;
};

// Flag

struct Flag
{
	Uint16 x, y;
	Uint16 team;
	Uint16 ident;
};

// Goal

struct Goal
{
	Uint16 x, y;
	Uint16 team;
};

// Powerball

struct PBall
{
	Uint16 x, y;
	Sint16 xvel, yvel;
	Uint16 team;
	Uint16 ident;
	Uint16 carrier;
	Uint32 time;			// Local timestamp on event (not constant)
	Uint32 hosttime;		// Remote timestamp on event (constant until new event)
	Uint32 lastrecv;		// Last update recv time (for timeout)
};

#endif	// PLAYER_H
