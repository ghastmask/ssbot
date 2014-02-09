#include "player.h"

#include "settings.h"
#include "algorithms.h"
#include "map.h"

#include <string.h>


//////// Enumerations ////////

char *getLevelString(Operator_Level access)
{
	switch (access)
	{
	case OP_Player:			return "Player";
	case OP_Limited:		return "Limited";
	case OP_Moderator:		return "Mod";
	case OP_SuperModerator:	return "SMod";
	case OP_SysOp:			return "SOp";
	case OP_Owner:			return "Owner";
	case OP_Duke:			return "Duke";
	case OP_Baron:			return "Baron";
	case OP_King:			return "King";
	case OP_Emperor:		return "Emperor";
	case OP_RockStar:		return "RockStar";
	case OP_Q:				return "Q";
	case OP_God:			return "God";
	default:				return "UNKNOWN";
	};
}

char *getPrizeString(Sint16 prize)
{
	switch (prize % 29)
	{
	case -28:	return "Lost portal";
	case -27:	return "Lost rocket";
	case -26:	return "Lost brick";
	case -25:	return "UNKNOWN-25";
	case -24:	return "Lost thor";
	case -23:	return "Lost decoy";
	case -22:	return "Lost burst";
	case -21:	return "Lost repel";
	case -20:	return "Lost antiwarp";
	case -19:	return "Lost shrapnel";
	case -18:	return "Lost Shields!";
	case -17:	return "Lost Super!";
	case -16:	return "Lost proximity";
	case -15:	return "Lost multifire";
	case -14:	return "Engines shutdown (severe)";
	case -13:	return "Energy depleted";
	case -12:	return "Top speed downgrade";
	case -11:	return "Thrusters downgrade";
	case -10:	return "Lost Bouncing bullets";
	case -9:	return "Bomb downgrade";
	case -8:	return "Gun downgrade";
	case -7:	return "Warp!";
	case -6:	return "XRadar downgrade";
	case -5:	return "Cloak downgrade";
	case -4:	return "Stealth downgrade";
	case -3:	return "Rotation downgrade";
	case -2:	return "Energy downgrade";
	case -1:	return "Recharge downgrade";
	case  0:	return "Ship reset";
	case  1:	return "Recharge upgrade";
	case  2:	return "Energy upgrade";
	case  3:	return "Rotation";
	case  4:	return "Stealth";
	case  5:	return "Cloak";
	case  6:	return "XRadar";
	case  7:	return "Warp!";
	case  8:	return "Gun upgrade";
	case  9:	return "Bomb upgrade";
	case  10:	return "Bouncing bullets";
	case  11:	return "Thrusters upgrade";
	case  12:	return "Top speed upgrade";
	case  13:	return "Full charge";
	case  14:	return "Engines shutdown";
	case  15:	return "Multifire";
	case  16:	return "Proximity bombs";
	case  17:	return "Super!";
	case  18:	return "Shields!";
	case  19:	return "Shrapnel increase";
	case  20:	return "Antiwarp upgrade";
	case  21:	return "Repel";
	case  22:	return "Burst";
	case  23:	return "Decoy";
	case  24:	return "Thor!";
	case  25:	return "Multiprize!";
	case  26:	return "Brick";
	case  27:	return "Rocket";
	case  28:	return "Portal";
	default:	return "UNKNOWN";
	};
}


//////// Player ////////

void Player::setBanner(char *bbanner)
{
	memcpy(banner, bbanner, 96);
}

Player::Player(Uint16 iident, char *nname, char *ssquad, Uint32 fflagPoints, Uint32 kkillPoints, Uint16 tteam, Uint16 wwins, Uint16 llosses, BYTE sship, bool aacceptsAudio, Uint16 fflagCount)
{
	strncpy(name, nname, 20);
	strncpy(squad, ssquad, 20);

	memset(banner, 0, 96);

	access = OP_Player;

	lastPositionUpdate = getTime();

	ident	= iident;
	ship	= sship;
	team	= tteam;

	score.flagPoints	= fflagPoints;
	score.killPoints	= kkillPoints;
	score.wins			= wwins;
	score.losses		= llosses;

	koth	= false;
	d		= 0;
	bounty	= 0;
	energy	= 0;
	timer	= 0;
	S2CLag	= 0;

	move(TILE_MAX_X * 8, TILE_MAX_X * 8, 0, 0);

	stealth = false;
	cloak	= false;
	xradar	= false;
	awarp	= false;
	ufo		= false;
	flash	= false;

	shields = false;
	supers	= false;
	burst	= 0;
	repel	= 0;
	thor	= 0;
	brick	= 0;
	decoy	= 0;
	rocket	= 0;
	portal	= 0;

	turret	= NULL;

	acceptsAudio = aacceptsAudio;

	flagCount = fflagCount;
}

void Player::move(Sint32 time)
{
	work += (vel * time);

	pos = work / 1000;
	tile = pos / 16;
}

void Player::move(Sint32 x, Sint32 y)
{
	work.set(x * 1000, y * 1000);
	vel = 0;

	pos = work / 1000;
	tile = pos / 16;
}

void Player::move(Sint32 x, Sint32 y, Sint32 vx, Sint32 vy)
{
	work.set(x * 1000, y * 1000);
	vel.set(vx, vy);

	pos = work / 1000;
	tile = pos / 16;
}

void Player::clone(Player *p)
{
	tile	= p->tile;
	pos		= p->pos;
	work	= p->work;
	vel		= p->vel;
	d		= p->d;
}
