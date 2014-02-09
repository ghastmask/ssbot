/*
	MERVBot DLL-plugin API by cat02e@fsu.edu
*/


#ifndef DLLCORE_H
#define DLLCORE_H

// Major version means DLL and bot are incompatible.
// Minor version means extra features are available.

/*
	6.8
		Bot2DLL EVENT_MapLoaded
		Bot2DLL EVENT_Init + creation string
	6.7
		Bot2DLL EVENT_BannerChange
	6.6
		Bot2DLL EVENT_PlayerDeath (+flags that were transferred)
	6.5
		Bot2DLL EVENT_FlagGameReset (*flagreset notification)
	6.4
			No changes in the plugin system, I just fixed a bug in the core (player bounties).
			Plugins that provide features based on player bounties need this version marker.
	6.3
		Bot2DLL EVENT_WatchDamage
	6.2
		DLL2Bot EVENT_ChangeSettings
			Apparently Ctm.37 broke the LVZ object toggling, so i'm just using *objon/off from the core for now.
		DLL2Bot EVENT_SendPosition + boolean: reliable
	6.1
		Bot2DLL EVENT_ObjectToggled (LVZ object was toggled on/off)
		DLL2Bot EVENT_ToggleObjects (Toggle LVZ objects on/off)
		Bot2DLL EVENT_Init + brick list
		Bot2DLL EVENT_BrickDropped (team dropped a brick)
	6.0
		Removed Player::tags[] now that you can !load more than one spawn at a time.
		Bot2DLL EVENT_PlayerDeath + bounty
	5.8
		DLL2Bot	EVENT_FireWeapon (send a weapons packet)
	5.7
			To make the bot get in the game and stop sending position packets on its own,
			put it in a Ship, turn off Following, and turn on Flying.
		Bot2DLL	EVENT_Init + map pointer
		Bot2DLL	EVENT_SelfShipReset (my ship got reset)
		Bot2DLL	EVENT_SelfPrize (got a prize)
		Bot2DLL	EVENT_SelfUFO (my UFO toggled)
		Bot2DLL	EVENT_PlayerWeapon (player fired a weapon)
		Bot2DLL EVENT_PositionHook (the core is asking for a position packet to be sent by the DLL)
		DLL2Bot	EVENT_Die (make the bot die to a certain player)
		DLL2Bot	EVENT_Attach (attach to a player)
		DLL2Bot	EVENT_Detach (detach from a player)
		DLL2Bot	EVENT_Following (when not following, it will just sit around)
		DLL2Bot	EVENT_Flying (stops sending telemetry, polls the DLL when it's time to send position packets)
		DLL2Bot	EVENT_Banner (give it a banner)
		DLL2Bot	EVENT_DropBrick (drop a brick)
	5.6
		DLL2Bot EVENT_Team (change bot's team)
		Bot2DLL	EVENT_PlayerShip + old team
		Bot2DLL	EVENT_PlayerTeam + old ship
		Bot2DLL	EVENT_PlayerSpec + old team + old ship
	5.5
		Bot2DLL	EVENT_PlayerShip + old ship
		Bot2DLL	EVENT_ArenaEnter + biller online
	5.4
		Bot2DLL	EVENT_PlayerTeam + old team
		Bot2DLL EVENT_BallMove (a powerball moved)
*/

#define CORE_MAJOR_VERSION 6
#define CORE_MINOR_VERSION 8

#include "player.h"
#include "commtypes.h"
#include "settings.h"

enum Bot2DLL_EventCodes
{
	EVENT_Init,
		/*	DLL has been loaded

			[0] Version hi/lo
			[1] Callback
			[2] Playerlist
			[3] Flaglist
			[4] Map
			[5] Bricklist
			[6] Creation string (v6.8)
		*/

	EVENT_Term,
		/*	DLL has been unloaded
		*/

	EVENT_Tick,
		/*	Happens every 1 second
		*/

	EVENT_ArenaEnter,
		/*	Bot has entered an arena

			[0] Name
			[1] Me Player class
			[2] Boolean: billing server online?
		*/

	EVENT_ArenaSettings,
		/*	Bot has arena settings

			[0] Settings
		*/

	EVENT_ArenaLeave,
		/*	Bot has left an arena
		*/

	EVENT_ArenaListEntry,
		/*	Entry in an incoming arena list

			[0] Name
			[1] Current?
			[2] Population
		*/

	EVENT_ArenaListEnd,
		/*	Last item of an incoming arena list

			[0] Name
			[1] Current?
			[2] Population
		*/

	EVENT_CreateTurret,
		/*	Player turreted another

			[0] Turreter
			[1] Turretee
		*/

	EVENT_DeleteTurret,
		/*	Player got off another

			[0] Turreter
			[1] Turretee
		*/

	EVENT_PlayerEntering,
		/*	Happens after the bot is in the game

			[0] Player class
		*/

	EVENT_PlayerMove,
		/*	Player changed position

			[0] Player class
		*/

	EVENT_PlayerDeath,
		/*	Player died

			[0] Player class
			[1] Killer player class
			[2] Bounty
		*/

	EVENT_PlayerScore,
		/*	Player changed score

			[0] Player class
		*/

	EVENT_PlayerPrize,
		/*	Player got prized

			[0] Player class
			[1] Prize
		*/

	EVENT_PlayerShip,
		/*	Player changed ship

			[0] Player class
			[1] Old ship
			[2] Old team
		*/

	EVENT_PlayerSpec,
		/*	Player spectating

			[0] Player class
			[1] Old team
			[2] Old ship
		*/

	EVENT_PlayerTeam,
		/*	Player changed team

			[0] Player class
			[1] Old team
			[2] Old ship
		*/

	EVENT_PlayerLeaving,
		/*	Player is leaving the game.

			[0] Player class
		*/

	EVENT_Chat,
		/*	Chat message has been received.

			[0] Type
			[1] Sound
			[2] Player class/UNUSED
			[3] Message
		*/

	EVENT_LocalCommand,
		/*	Command from this arena

			[0] Player class
			[1] Command class
		*/

	EVENT_LocalHelp,
		/*	Help from this arena

			[0] Player class
			[1] Command class
		*/

	EVENT_RemoteCommand,
		/*	Command from another arena

			[0] Player name
			[1] Command class
			[2] Operator level
		*/

	EVENT_RemoteHelp,
		/*	Help from another arena

			[0] Player name
			[1] Command class
			[2] Operator level
		*/

	EVENT_FlagGrab,
		/*	Flag grabbed by a player

			[0] Player class
			[1] Flag class
		*/

	EVENT_FlagDrop,
		/*	Flags dropped by a player

			[0] Player class
		*/

	EVENT_FlagMove,
		/*	Flags moved

			[0] Flag class
		*/

	EVENT_FlagVictory,
		/*	Flag victory!

			[0] Team
			[1] Reward
		*/

	EVENT_FlagReward,
		/*	Periodic flag reward

			[0] Team
			[1] Reward
		*/

	EVENT_TimedGameOver,
		/*	Timed game over

			[0] Winner 1
			[1] Winner 2
			[2] Winner 3
			[3] Winner 4
			[4] Winner 5
		*/

	EVENT_SoccerGoal,
		/*	Soccer goal

			[0] Team
			[1] Points
		*/

	EVENT_File,
		/*	File received

			[0] Filename
		*/
//// 5.4 ////
	EVENT_BallMove,
		/*	Soccer ball moved

			[0] PBall struct pointer (player.h)
		*/
//// 5.7 ////
	EVENT_SelfShipReset,
		/*	My ship got reset
		*/

	EVENT_SelfPrize,
		/*	Got a prize

			[0] Prize number
			[1] Prize count
		*/

	EVENT_SelfUFO,
		/*	My UFO toggled
		*/

	EVENT_PlayerWeapon,
		/*	Player fired a weapon

			[0] Player class
			[1] weaponInfo struct
		*/

	EVENT_PositionHook,
		/*	Core is requesting the DLL to send a position packet
		*/
//// 6.1 ////
	EVENT_ObjectToggled,
		/*	An LVZ object was toggled on/off

			[0] objectInfo struct
		*/
	EVENT_BrickDropped,
		/*	A brick was dropped

			[0] x1
			[1] y1
			[2] x2
			[3] y2
			[4] team
		*/
//// 6.3 ////
	EVENT_WatchDamage,
		/*	*watchdamage output, sans timestamp and extra byte

			[0] Player pointer
			[1] Attacker pointer
			[2] weaponInfo struct
			[3] energy
			[4] damage
		*/
//// 6.4 ////
	EVENT_FlagGameReset,
		/*	*flagreset notification, part of the flag victory packet
		*/
//// 6.7 ////
	EVENT_BannerChanged,
		/*	Player changed their banner

			[0] Player class pointer
		*/
//// 6.8 ////
	EVENT_MapLoaded,
		/*  Map was loaded
		*/
};

enum DLL2Bot_EventCodes
{
	EVENT_Echo,
		/*	Send a message to the terminal

			[0] message
		*/

	EVENT_Say,
		/*	Queue a chat message

			[0] type
			[1] sound
			[2] ident
			[3] message
		*/

	EVENT_Ship,
		/*	Set bot ship/spectate

			[0] ship
		*/

	EVENT_GrabFlag,
		/*	Grab a flag

			[0] flag id
		*/

	EVENT_SendPosition,
		/*	Refresh bot position on server

			[0] boolean: reliable?
		*/

	EVENT_DropFlags,
		/*	Drop held flags
		*/

	EVENT_SpawnBot,
		/*	Requests that a new bot is created

			[0] name
			[1] password
			[2] staff
			[3] arena
		*/

	EVENT_ChangeArena,
		/*	Requests a change of arena

			[0] name
		*/
//// 5.6 ////
	EVENT_Team,
		/*	Set bot team

			[0] team
		*/
//// 5.7 ////
	EVENT_Die,
		/*	Make the bot die to a certain player

			[0] Player class
		*/

	EVENT_Attach,
		/*	Attach to a player

			[0] Player class
		*/

	EVENT_Detach,
		/*	Detach from a player
		*/

	EVENT_Following,
		/*	When not following, it will just sit around

			[0] boolean: following?
		*/

	EVENT_Flying,
		/*	Stops sending telemetry, polls the DLL when it's time to send position packets

			[0] boolean: flying?
		*/

	EVENT_Banner,
		/*	Give it a banner

			[0] BYTE *Banner
		*/

	EVENT_DropBrick,
		/*	Drop a brick
		*/
//// 5.8 ////
	EVENT_FireWeapon,
		/*	Send a weapons packet

			[0] weaponInfo pointer
		*/
//// 6.1 ////
	EVENT_ToggleObjects,
		/*	Toggle LVZ object(s) on/off

			[0] objectInfo struct array
			[1] number of items in array
			[2] player ident
		*/
//// 6.2 ////
	EVENT_ChangeSettings,
		/*	Change arena settings ala ?setsettings

			[0] _linkedlist <String> pointer
		*/
//// 6.6 ////
	EVENT_MoveObjects,
		/*	Move LVZ objects around on the map

			[0] lvzObject struct array
			[1] number of items in array
			[2] player ident
		*/
	EVENT_GrabBall,
		/*	Grab a ball

			[0] ball id
		*/
	EVENT_FireBall,
		/*	Fire the specified ball in a certain direction

			[0] ball id
			[1] x pixel
			[2] y pixel
			[3] x velocity
			[4] y velocity
		*/
};


//////// DLL->Bot ////////

typedef void *CALL_HANDLE;
typedef void (__stdcall *CALL_COMMAND)(struct BotEvent &event);
typedef _linkedlist <Player> *CALL_PLIST;
typedef _linkedlist <Flag> *CALL_FLIST;
typedef _linkedlist <Brick> *CALL_BLIST;
typedef BYTE *CALL_MAP;
typedef char *CALL_PARAMS;


struct BotEvent
{
	CALL_HANDLE handle;

	int code;

	void *p[8];

	BotEvent(const BotEvent &b);
	BotEvent();
};


//////// Bot->DLL ////////

BotEvent makeInit				(CALL_COMMAND c, CALL_PLIST p, CALL_FLIST f, CALL_MAP m, CALL_BLIST b, CALL_PARAMS params);

BotEvent makeTick				();

BotEvent makeArenaEnter			(char *name, Player *me, bool biller);
BotEvent makeArenaSettings		(arenaSettings *settings);
BotEvent makeArenaLeave			();
BotEvent makeArenaListEntry		(char *name, bool current, int population);
BotEvent makeArenaListEnd		(char *name, bool current, int population);

BotEvent makePlayerEntering		(Player *p);

BotEvent makeCreateTurret		(Player *turreter, Player *turretee);
BotEvent makeDeleteTurret		(Player *turreter, Player *turretee);

BotEvent makePlayerMove			(Player *p);
BotEvent makePlayerWeapon		(Player *p, Uint16 wi);
BotEvent makeWatchDamage		(Player *p, Player *k, Uint16 wi, Uint16 energy, Uint16 damage);
BotEvent makePlayerDeath		(Player *p, Player *k, int bounty, int flags);
BotEvent makePlayerPrize		(Player *p, int prize);
BotEvent makePlayerScore		(Player *p);
BotEvent makePlayerShip			(Player *p, int oldship, int oldteam);
BotEvent makePlayerSpec			(Player *p, int oldteam, int oldship);
BotEvent makePlayerTeam			(Player *p, int oldteam, int oldship);
BotEvent makeBannerChanged		(Player *p);

BotEvent makePlayerLeaving		(Player *p);

BotEvent makeFlagGrab			(Player *p, Flag *f);
BotEvent makeFlagDrop			(Player *p);
BotEvent makeFlagMove			(Flag *f);
BotEvent makeFlagGameReset		();
BotEvent makeFlagVictory		(int team, int points);
BotEvent makeFlagReward			(int team, int points);

BotEvent makeSelfShipReset		();
BotEvent makeSelfPrize			(Uint16 prize, Uint16 count);
BotEvent makeSelfUFO			();

BotEvent makeTimedGameOver		(Player *p1, Player *p2, Player *p3, Player *p4, Player *p5);

BotEvent makeSoccerGoal			(int team, int points);

BotEvent makeFile				(char *name);

BotEvent makeChat				(int t, int s, Player *p, char *m);
BotEvent makeLocalHelp			(Player *p, Command *c);
BotEvent makeLocalCommand		(Player *p, Command *c);
BotEvent makeRemoteHelp			(char *p, Command *c, int o);
BotEvent makeRemoteCommand		(char *p, Command *c, int o);

BotEvent makeBrickDropped		(int x1, int y1, int x2, int y2, int team);

BotEvent makeBallMove			(PBall *pb);

BotEvent makePositionHook		();

BotEvent makeObjectToggled		(Uint16 obj_n);

BotEvent makeTerm				();

BotEvent makeMapLoaded			();

//////// DLL->Bot ////////

BotEvent makeEcho				(char *m);
BotEvent makeSay				(int t, int s, int i, char *m);

BotEvent makeShip				(int s);
BotEvent makeTeam				(int t);

BotEvent makeSendPosition		(bool reliable);
BotEvent makeFireWeapon			(void *weapon_info);

BotEvent makeGrabFlag			(int f);
BotEvent makeDropFlags			();

BotEvent makeDeath				(Player *p);
BotEvent makeAttach				(Player *p);
BotEvent makeDetach				();
BotEvent makeFollowing			(bool f);
BotEvent makeFlying				(bool f);
BotEvent makeBanner				(BYTE *b);
BotEvent makeDropBrick			();

BotEvent makeGrabBall			(int id);
BotEvent makeFireBall			(int id, short x, short y, short xvel, short yvel);

BotEvent makeToggleObjects		(Uint16 player, Uint16 *objects, int num_objects);
BotEvent makeMoveObjects		(Uint16 player, void *objects, int num_objects); // (lvzObject*)objects

BotEvent makeSpawnBot			(char *name, char *password, char *staff, char *arena);
BotEvent makeChangeArena		(char *name);
BotEvent makeChangeSettings		(_linkedlist <String> *settings);

#endif	// DLLCORE_H
