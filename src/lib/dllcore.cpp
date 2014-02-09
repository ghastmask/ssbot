#include "dllcore.h"


//////// Bot->DLL ////////

BotEvent makeInit(CALL_COMMAND c, CALL_PLIST p, CALL_FLIST f, CALL_MAP m, CALL_BLIST b, CALL_PARAMS params)
{
	BotEvent event;

	event.code = EVENT_Init;

	event.p[0] = (void*)((CORE_MAJOR_VERSION << 16) | (CORE_MINOR_VERSION));
	event.p[1] = (void*)c;
	event.p[2] = p;
	event.p[3] = f;
	event.p[4] = m;
	event.p[5] = b;
	event.p[6] = params;

	return event;
}

BotEvent makeTick()
{
	BotEvent event;

	event.code = EVENT_Tick;

	return event;
}

BotEvent makeArenaEnter(char *name, Player *me, bool biller)
{
	BotEvent event;

	event.code = EVENT_ArenaEnter;

	event.p[0] = name;
	event.p[1] = me;
	event.p[2] = (void*)biller;

	return event;
}

BotEvent makeArenaSettings(arenaSettings *settings)
{
	BotEvent event;

	event.code = EVENT_ArenaSettings;

	event.p[0] = settings;

	return event;
}


BotEvent makeArenaLeave()
{
	BotEvent event;

	event.code = EVENT_ArenaLeave;

	return event;
}

BotEvent makeArenaListEntry(char *name, bool current, int population)
{
	BotEvent event;

	event.code = EVENT_ArenaListEntry;

	event.p[0] = name;
	event.p[1] = *(void**)&current;
	event.p[2] = *(void**)&population;

	return event;
}

BotEvent makeArenaListEnd(char *name, bool current, int population)
{
	BotEvent event;

	event.code = EVENT_ArenaListEnd;

	event.p[0] = name;
	event.p[1] = *(void**)&current;
	event.p[2] = *(void**)&population;

	return event;
}

BotEvent makeCreateTurret(Player *turreter, Player *turretee)
{
	BotEvent event;

	event.code = EVENT_CreateTurret;

	event.p[0] = turreter;
	event.p[1] = turretee;

	return event;
}

BotEvent makeDeleteTurret(Player *turreter, Player *turretee)
{
	BotEvent event;

	event.code = EVENT_DeleteTurret;

	event.p[0] = turreter;
	event.p[1] = turretee;

	return event;
}

BotEvent makePlayerEntering(Player *p)
{
	BotEvent event;

	event.code = EVENT_PlayerEntering;

	event.p[0] = p;

	return event;
}

BotEvent makePlayerMove(Player *p)
{
	BotEvent event;

	event.code = EVENT_PlayerMove;

	event.p[0] = p;

	return event;
}

BotEvent makePlayerWeapon(Player *p, Uint16 wi)
{
	BotEvent event;

	event.code = EVENT_PlayerWeapon;

	event.p[0] = p;
	event.p[1] = *(void**)&wi;

	return event;
}

BotEvent makeWatchDamage(Player *p, Player *k, Uint16 wi, Uint16 energy, Uint16 damage)
{
	BotEvent event;

	event.code = EVENT_WatchDamage;

	event.p[0] = p;
	event.p[1] = k;
	event.p[2] = *(void**)&wi;
	event.p[3] = *(void**)&energy;
	event.p[4] = *(void**)&damage;

	return event;
}

BotEvent makePlayerDeath(Player *p, Player *k, int bounty, int flags)
{
	BotEvent event;

	event.code = EVENT_PlayerDeath;

	event.p[0] = p;
	event.p[1] = k;
	event.p[2] = *(void**)&bounty;
	event.p[3] = *(void**)&flags;

	return event;
}

BotEvent makePlayerPrize(Player *p, int prize)
{
	BotEvent event;

	event.code = EVENT_PlayerPrize;

	event.p[0] = p;
	event.p[1] = *(void**)&prize;

	return event;
}

BotEvent makePlayerScore(Player *p)
{
	BotEvent event;

	event.code = EVENT_PlayerScore;

	event.p[0] = p;

	return event;
}

BotEvent makePlayerShip(Player *p, int oldship, int oldteam)
{
	BotEvent event;

	event.code = EVENT_PlayerShip;

	event.p[0] = p;
	event.p[1] = *(void**)&oldship;
	event.p[2] = *(void**)&oldteam;

	return event;
}

BotEvent makePlayerSpec(Player *p, int oldteam, int oldship)
{
	BotEvent event;

	event.code = EVENT_PlayerSpec;

	event.p[0] = p;
	event.p[1] = *(void**)&oldteam;
	event.p[2] = *(void**)&oldship;

	return event;
}

BotEvent makePlayerTeam(Player *p, int oldteam, int oldship)
{
	BotEvent event;

	event.code = EVENT_PlayerTeam;

	event.p[0] = p;
	event.p[1] = *(void**)&oldteam;
	event.p[2] = *(void**)&oldship;

	return event;
}

BotEvent makeBannerChanged(Player *p)
{
	BotEvent event;

	event.code = EVENT_BannerChanged;

	event.p[0] = p;

	return event;
}

BotEvent makePlayerLeaving(Player *p)
{
	BotEvent event;

	event.code = EVENT_PlayerLeaving;

	event.p[0] = p;

	return event;
}

BotEvent makeSelfShipReset()
{
	BotEvent event;

	event.code = EVENT_SelfShipReset;

	return event;
}

BotEvent makeSelfUFO()
{
	BotEvent event;

	event.code = EVENT_SelfUFO;

	return event;
}

BotEvent makeSelfPrize(Uint16 prize, Uint16 count)
{
	BotEvent event;

	event.code = EVENT_SelfPrize;

	event.p[0] = *(void**)&prize;
	event.p[1] = *(void**)&count;

	return event;
}

BotEvent makeFlagGrab(Player *p, Flag *f)
{
	BotEvent event;

	event.code = EVENT_FlagGrab;

	event.p[0] = p;
	event.p[1] = f;

	return event;
}

BotEvent makeFlagDrop(Player *p)
{
	BotEvent event;

	event.code = EVENT_FlagDrop;

	event.p[0] = p;

	return event;
}

BotEvent makeFlagMove(Flag *f)
{
	BotEvent event;

	event.code = EVENT_FlagMove;

	event.p[0] = *(void**)&f;

	return event;
}

BotEvent makeFlagVictory(int team, int points)
{
	BotEvent event;

	event.code = EVENT_FlagVictory;

	event.p[0] = *(void**)&team;
	event.p[1] = *(void**)&points;

	return event;
}

BotEvent makeFlagGameReset()
{
	BotEvent event;

	event.code = EVENT_FlagGameReset;

	return event;
}

BotEvent makeFlagReward(int team, int points)
{
	BotEvent event;

	event.code = EVENT_FlagReward;

	event.p[0] = *(void**)&team;
	event.p[1] = *(void**)&points;

	return event;
}

BotEvent makeTimedGameOver(Player *p1, Player *p2, Player *p3, Player *p4, Player *p5)
{
	BotEvent event;

	event.code = EVENT_TimedGameOver;

	event.p[0] = p1;
	event.p[1] = p2;
	event.p[2] = p3;
	event.p[3] = p4;
	event.p[4] = p5;

	return event;
}

BotEvent makeSoccerGoal(int team, int points)
{
	BotEvent event;

	event.code = EVENT_SoccerGoal;

	event.p[0] = *(void**)&team;
	event.p[1] = *(void**)&points;

	return event;
}

BotEvent makeFile(char *name)
{
	BotEvent event;

	event.code = EVENT_File;

	event.p[0] = name;

	return event;
}

BotEvent makeChat(int t, int s, Player *p, char *m)
{
	BotEvent event;

	event.code = EVENT_Chat;

	event.p[0] = *(void**)&t;
	event.p[1] = *(void**)&s;
	event.p[2] = p;
	event.p[3] = m;

	return event;
}

BotEvent makeLocalHelp(Player *p, Command *c)
{
	BotEvent event;

	event.code = EVENT_LocalHelp;

	event.p[0] = p;
	event.p[1] = c;

	return event;
}

BotEvent makeLocalCommand(Player *p, Command *c)
{
	BotEvent event;

	event.code = EVENT_LocalCommand;

	event.p[0] = p;
	event.p[1] = c;

	return event;
}

BotEvent makeRemoteHelp(char *p, Command *c, int o)
{
	BotEvent event;

	event.code = EVENT_RemoteHelp;

	event.p[0] = p;
	event.p[1] = c;
	event.p[2] = *(void**)&o;

	return event;
}

BotEvent makeRemoteCommand(char *p, Command *c, int o)
{
	BotEvent event;

	event.code = EVENT_RemoteCommand;

	event.p[0] = p;
	event.p[1] = c;
	event.p[2] = *(void**)&o;

	return event;
}

BotEvent makeBallMove(PBall *pb)
{
	BotEvent event;

	event.code = EVENT_BallMove;

	event.p[0] = pb;

	return event;
}

BotEvent makePositionHook()
{
	BotEvent event;

	event.code = EVENT_PositionHook;

	return event;
}

BotEvent makeObjectToggled(Uint16 obj_n)
{
	BotEvent event;

	event.code = EVENT_ObjectToggled;

	event.p[0] = *(void**)&obj_n;

	return event;
}

BotEvent makeBrickDropped(int x1, int y1, int x2, int y2, int team)
{
	BotEvent event;

	event.code = EVENT_BrickDropped;

	event.p[0] = *(void**)&x1;
	event.p[1] = *(void**)&y1;
	event.p[2] = *(void**)&x2;
	event.p[3] = *(void**)&y2;
	event.p[4] = *(void**)&team;

	return event;
}

BotEvent makeTerm()
{
	BotEvent event;

	event.code = EVENT_Term;

	return event;
}

BotEvent makeMapLoaded()
{
	BotEvent event;

	event.code = EVENT_MapLoaded;

	return event;
}

//////// DLL->Bot ////////

BotEvent makeEcho(char *m)
{
	BotEvent event;

	event.code = EVENT_Echo;

	event.p[0] = m;

	return event;
}

BotEvent makeSay(int t, int s, int i, char *m)
{
	BotEvent event;

	event.code = EVENT_Say;

	event.p[0] = *(void**)&t;
	event.p[1] = *(void**)&s;
	event.p[2] = *(void**)&i;
	event.p[3] = m;

	return event;
}

BotEvent makeDeath(Player *p)
{
	BotEvent event;

	event.code = EVENT_Die;

	event.p[0] = p;

	return event;
}

BotEvent makeAttach(Player *p)
{
	BotEvent event;

	event.code = EVENT_Attach;

	event.p[0] = p;

	return event;
}

BotEvent makeDetach()
{
	BotEvent event;

	event.code = EVENT_Detach;

	return event;
}

BotEvent makeFollowing(bool f)
{
	BotEvent event;

	event.code = EVENT_Following;

	event.p[0] = *(void**)&f;

	return event;
}

BotEvent makeFlying(bool f)
{
	BotEvent event;

	event.code = EVENT_Flying;

	event.p[0] = *(void**)&f;

	return event;
}

BotEvent makeBanner(BYTE *b)
{
	BotEvent event;

	event.code = EVENT_Banner;

	event.p[0] = b;

	return event;
}

BotEvent makeDropBrick()
{
	BotEvent event;

	event.code = EVENT_DropBrick;

	return event;
}

BotEvent makeShip(int s)
{
	BotEvent event;

	event.code = EVENT_Ship;

	event.p[0] = *(void**)&s;

	return event;
}

BotEvent makeTeam(int t)
{
	BotEvent event;

	event.code = EVENT_Team;

	event.p[0] = *(void**)&t;

	return event;
}

BotEvent makeGrabFlag(int f)
{
	BotEvent event;

	event.code = EVENT_GrabFlag;

	event.p[0] = *(void**)&f;

	return event;
}

BotEvent makeSendPosition(bool reliable)
{
	BotEvent event;

	event.code = EVENT_SendPosition;

	event.p[0] = *(void**)&reliable;

	return event;
}

BotEvent makeDropFlags()
{
	BotEvent event;

	event.code = EVENT_DropFlags;

	return event;
}

BotEvent makeSpawnBot(char *name, char *password, char *staff, char *arena)
{
	BotEvent event;

	event.code = EVENT_SpawnBot;

	event.p[0] = name;
	event.p[1] = password;
	event.p[2] = staff;
	event.p[3] = arena;

	return event;
}

BotEvent makeChangeArena(char *name)
{
	BotEvent event;

	event.code = EVENT_ChangeArena;

	event.p[0] = name;

	return event;
}

BotEvent makeFireWeapon(void *weapon_info)
{
	BotEvent event;

	event.code = EVENT_FireWeapon;

	event.p[0] = weapon_info;

	return event;
}

BotEvent makeToggleObjects(Uint16 player, Uint16 *objects, int num_objects)
{
	BotEvent event;

	event.code = EVENT_ToggleObjects;

	event.p[0] = objects;
	event.p[1] = *(void**)&num_objects;
	event.p[2] = *(void**)&player;

	return event;
}

BotEvent makeMoveObjects(Uint16 player, void *objects, int num_objects)
{
	BotEvent event;

	event.code = EVENT_MoveObjects;

	event.p[0] = objects;
	event.p[1] = *(void**)&num_objects;
	event.p[2] = *(void**)&player;

	return event;
}

BotEvent makeGrabBall(int id)
{
	BotEvent event;

	event.code = EVENT_GrabBall;

	event.p[0] = *(void**)&id;

	return event;
}

BotEvent makeFireBall(int id, short x, short y, short xvel, short yvel)
{
	BotEvent event;

	event.code = EVENT_FireBall;

	event.p[0] = *(void**)&id;
	event.p[1] = *(void**)&x;
	event.p[2] = *(void**)&y;
	event.p[3] = *(void**)&xvel;
	event.p[4] = *(void**)&yvel;

	return event;
}

BotEvent makeChangeSettings(_linkedlist <String> *settings)
{
	BotEvent event;

	event.code = EVENT_ChangeSettings;

	event.p[0] = settings;

	return event;
}


//////// Event Class ////////

BotEvent::BotEvent(const BotEvent &b)
{
	code = b.code;

	p[0] = b.p[0];
	p[1] = b.p[1];
	p[2] = b.p[2];
	p[3] = b.p[3];
	p[4] = b.p[4];
	p[5] = b.p[5];
	p[6] = b.p[6];
	p[7] = b.p[7];
}

BotEvent::BotEvent()
{
	code = 0;

	p[0] = 0;
	p[1] = 0;
	p[2] = 0;
	p[3] = 0;
	p[4] = 0;
	p[5] = 0;
	p[6] = 0;
	p[7] = 0;
}
