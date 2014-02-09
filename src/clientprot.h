/*
	Client-side protocol by cat02e@fsu.edu
*/


#ifndef CLIENTPROT_H
#define CLIENTPROT_H

#include "datatypes.h"

// -1 is used in many places to designate
// the absence of an optional object stub
#ifndef UNASSIGNED
#define UNASSIGNED 0xFFFF
#endif

#ifndef DEFLATE_CLASS
#define DEFLATE_CLASS		\
	Host *	h	= m->src;	\
	char *	msg	= m->msg;	\
	Uint32	len	= m->len;
#endif

#define SPEC_FREQ 8025

/*	c2s Message types
	--	Original SubSpace protocol
	00	Start of a special header
	01	Arena login
	02	Leave arena
	03	Position packet
	04	<disabled?>
	05	Death message
	06	Chat message
	07	Take green
	08	Spectate player
	09	Password packet
	0A	<disabled?>
	0B	SSUpdate.EXE request
	0C	map.lvl request
	0D	news.txt request
	0E	Voice message
	0F	Frequency change
	10	Attach request
	11	<disabled?>
	12	<disabled?>
	13	Flag request
	14	Leave turret
	15	Drop flags
	16	File transfer
	17	Registration information response
	18	Set ship type
	19	Set personal banner
	1A	Security checksum
	1B	Security violation
	1C	Drop brick
	1D	Change settings
	1E	Personal KoTH timer ended
	1F	Fire a ball
	20	Ball request
	21	Soccer Goal

	Snrrrub sez:

	22 <timestamp (4)> <settings_csum (4)> <exe_csum (4)> <map_csum (4)> <securityViolation (1)>

	basically the checksums are the same ones used for the synchronization (18) packet except the checksum key used is 0. The security violation byte is the same violation byte as the parameter for the violation packet (1b)

	--	Additional Continuum protocol
	23	
	24	Password packet
	??	Object toggling
*/

/*	s2c Message types
	--	Original SubSpace protocol
	01	PlayerID change
	02	You are now in the game
	03	Player entering
	04	Player leaving
	05	Player fired a weapon
	06	Player died
	07	Chat
	08	Player took a prize
	09	Player score changed
	0A	Password packet response
	0B	Soccer goal
	0C	Player voice
	0D	Set player frequency (ship optional)
	0E	Create turret link
	0F	Arena settings
	10	File transfer
	11	<no-op>
	12	Flag position
	13	Flag claim
	14	Flag victory
	15	Destroy turret link
	16	Drop flag
	17	<no-op>
	18	Synchronization
	19	Request file
	1A	Reset score(s)
	1B	Personal ship reset
	1C	Put player in spectator mode / change extra info flag
	1D	Player team and ship changed (redundant)
	1E	Banner flag
	1F	Player banner changed
	20	Collected prize
	21	Brick dropped
	22	Turf flag update
	23	Flag reward granted
	24	Speed zone statistics
	25	Toggle UFO ship
	26	<no-op>
	27	Keep-alive
	28	Player position update
	29	Map information
	2A	Compressed map file
	2B	Set personal KoTH timer
	2C	KoTH game reset
	2D	Add KoTH Time
	2E	Power-ball position update
	2F	Arena directory listing
	30	Got zone banner advertisements
	31	You are now past the login sequence

	--	Additional Continuum protocol
	32	Change personal ship coordinates
	33	Custom login failure message
	34	Continuum version packet
	35	Object toggling
	36	Received object
	37	Damage toggling
	38	*watchdamage message
*/


// Wrapper for raw ship type numbers
// c2s (Change ship) 18 __

enum Ship_Types
{
	SHIP_Warbird,		// 0
	SHIP_Javelin,
	SHIP_Spider,
	SHIP_Leviathan,
	SHIP_Terrier,
	SHIP_Weasel,
	SHIP_Lancaster,
	SHIP_Shark,
	SHIP_Spectator		// 8
};


// Wrapper for raw prize type numbers

enum Prize_Types
{
	PRIZE_Unknown,		// 0
	PRIZE_Recharge,
	PRIZE_Energy,
	PRIZE_Rotation,
	PRIZE_Stealth,
	PRIZE_Cloak,
	PRIZE_XRadar,
	PRIZE_Warp,
	PRIZE_Guns,
	PRIZE_Bombs,
	PRIZE_BBullets,
	PRIZE_Thruster,
	PRIZE_TopSpeed,
	PRIZE_FullCharge,
	PRIZE_EngineShutdown,
	PRIZE_Multifire,
	PRIZE_Proximity,
	PRIZE_Super,
	PRIZE_Shields,
	PRIZE_Antiwarp,
	PRIZE_Repel,
	PRIZE_Burst,
	PRIZE_Decoy,
	PRIZE_Thor,
	PRIZE_Multiprize,
	PRIZE_Brick,
	PRIZE_Rocket,
	PRIZE_Portal,		// 28
};


// Password response constants
// s2c (Password response) 0A __ 86 00 00 00 00 00 00 00 E8 9C 42 F1 00 00 00 00 00 00 48 C9 1C 28 7C 37 92 E6 00 00 00 00 20 00 00 00

enum Login_Params
{
	LOG_Continue,			// 0   - Move along.
	LOG_NewUser,			// 1   - Unknown player, continue as new user?
	LOG_InvalidPassword,	// 2   - Invalid password for specified user.  The name you have chosen is probably in use by another player, try picking a different name.
	LOG_FullArena,			// 3   - This arena is currently full, try again later.
	LOG_LockedOut,			// 4   - You have been locked out of SubSpace, for more information inquire on Web BBS.
	LOG_NoPermission,		// 5   - You do not have permission to play in this arena, see Web Site for more information.
	LOG_SpectateOnly,		// 6   - You only have permission to spectate in this arena.
	LOG_TooManyPoints,		// 7   - You have too many points to play in this arena, please choose another arena.
	LOG_SlowConnection,		// 8   - Your connection appears to be too slow to play in this arena.
	LOG_NoPermission2,		// 9   - You do not have permission to play in this arena, see Web Site for more information.
	LOG_NoNewConnections,	// 10  - The server is currently not accepting new connections.
	LOG_InvalidName,		// 11  - Invalid user name entered, please pick a different name.
	LOG_ObsceneName,		// 12  - Possibly offensive user name entered, please pick a different name.
	LOG_BillerDown,			// 13  - NOTICE: Server difficulties; this zone is currently not keeping track of scores.  Your original score will be available later.  However, you are free to play in the zone until we resolve this problem.
	LOG_BusyProcessing,		// 14  - The server is currently busy processing other login requests, please try again in a few moments.
	LOG_ExperiencedOnly,	// 15  - This zone is restricted to experienced players only (ie. certain number of game-hours logged).
	LOG_UsingDemoVersion,	// 16  - You are currently using the demo version.  Your name and score will not be kept track of.
	LOG_TooManyDemos,		// 17  - This arena is currently has(sic) the maximum Demo players allowed, try again later.
	LOG_ClosedToDemos,		// 18  - This arena is closed to Demo players.
	LOG_UnknownResponse,	// ... - Unknown response type, please go to Web site for more information and to obtain latest version of the program.
	LOG_NeedModerator = 255	// 255 - Moderator access required for this zone (MGB addition)
};


// Chat parameters
// c2s (Chat message) 06 __ 00 00 00 MM EE SS SS AA GG EE 00

enum Chat_Modes
{
    MSG_Arena,			// 00 Same for *arena, *zone, *szone, **
    MSG_PublicMacro,	// 01 Macro calcs are done client-side
    MSG_Public,			// 02 Public chat
    MSG_Team,			// 03 (//) or (')
    MSG_TeamPrivate,	// 04 Player to all members of another team (")
    MSG_Private,		// 05 Only shows messages addressed to the bot
    MSG_PlayerWarning,	// 06 Red message, with a name tag
    MSG_RemotePrivate,	// 07 Do not trust the sender with any private information.
    MSG_ServerError,	// 08 Red server errors, without a name tag
    MSG_Channel			// 09 Arrives in the same format SubSpace displays
};


// Item info concatenated to the position packets
// c2s (Position packet) 03 00 1D 23 02 00 05 00 27 10 FF SI 25 01 01 00 2D 00 3F 01 WW II 3F 01 00 00 00 00 __ __ __ __

#pragma pack(push)
#pragma pack(1)	// For bitfields

union itemInfo
{
	struct
	{
		Uint32 shields	: 1;	// Has it or not
		Uint32 supers	: 1;
		Uint32 burst	: 4;	// Item counts
		Uint32 repel	: 4;
		Uint32 thor		: 4;
		Uint32 brick	: 4;
		Uint32 decoy	: 4;
		Uint32 rocket	: 4;
		Uint32 portal	: 4;
		Uint32 pack		: 2;	// Probably used somehow
	};

	Uint32 n;
};


// Weapon info concatenated to the position packets
// c2s (Position packet) 03 00 1D 23 02 00 05 00 27 10 FF SI 25 01 01 00 2D 00 3F 01 __ __ 3F 01 00 00 00 00 II TT EE MM

union weaponInfo
{
	struct
	{
		Uint16 type			: 5;	// enum Projectile_Types
		Uint16 level		: 2;	// Only for bombs/bullets
		Uint16 shrapBounce	: 1;	// Bouncing shrapnel?
		Uint16 shrapLevel	: 2;	// Shrapnel level 0..3
		Uint16 shrapCount	: 5;	// 0-31
		Uint16 fireType		: 1;	// Bombs -> Mines, Bullets -> Multifire
	};

	Uint16 n;
};


// Weapon types for weaponInfo.type

enum Projectile_Types
{
	// Seen "in the wild"
	PROJ_None,
	PROJ_Bullet,
	PROJ_BBullet,
	PROJ_Bomb,
	PROJ_PBomb,
	PROJ_Repel,
	PROJ_Decoy,
	PROJ_Burst,
	PROJ_Thor,

	// Internal to the bot
	PROJ_InactiveBullet,
	PROJ_Shrapnel
};


// Weapon levels for weaponInfo.level

enum Weapon_Levels
{
	LVL_One,
	LVL_Two,
	LVL_Three,
	LVL_Four
};


// State info concatenated to the position packets
// c2s (Position packet) 03 00 1D 23 02 00 05 00 27 10 FF __ 25 01 01 00 2D 00 3F 01 WW II 3F 01 00 00 00 00 II TT EE MM

union stateInfo
{
	struct
	{
		BYTE stealth	: 1;
		BYTE cloaked	: 1;
		BYTE xradar		: 1;
		BYTE awarp		: 1;
		BYTE flash		: 1;	// Uncloaking, portaling, etc.
		BYTE safety		: 1;	// In a safety zone
		BYTE ufo		: 1;	// *ufo - Illegal usage caught in sg9+
		BYTE pack		: 1;	// ?
	};

	BYTE n;
};


// Security checksum responses, violation codes
// c2s (Violation notification) 1B __

enum Security_Violations
{
	// NOTE: These may only be sent in response to a security checksum request
	SEC_NormalIntegrity,			// 0x00 - Normal integrity
	SEC_SlowFrameDetected,			// 0x01 - Slow frame detected
	SEC_Energy_CurrentOverTop,		// 0x02 - Current energy higher than top energy
	SEC_Energy_TopOverMax,			// 0x03 - Top energy higher than max energy
	SEC_Energy_MaxWithoutPrize,		// 0x04 - Max energy without getting prizes
	SEC_Recharge_CurrentOverMax,	// 0x05 - Recharge rate higher than max recharge rate
	SEC_Recharge_MaxWithoutPrize,	// 0x06 - Max recharge rate without getting prizes
	SEC_Burst_TooMany,				// 0x07 - Too many burst used
	SEC_Repel_TooMany,				// 0x08 - Too many repel used
	SEC_Decoy_TooMany,				// 0x09 - Too many decoy used
	SEC_Thor_TooMany,				// 0x0A - Too many thor used
	SEC_Brick_TooMany,				// 0x0B - Too many wall blocks used
	SEC_Stealth_WithoutPrize,		// 0x0C - Stealth on but never collected
	SEC_Cloak_WithoutPrize,			// 0x0D - Cloak on but never collected
	SEC_XRadar_WithoutPrize,		// 0x0E - XRadar on but never collected
	SEC_AntiWarp_WithoutPrize,		// 0x0F - AntiWarp on but never collected
	SEC_PBombs_WithoutPrize,		// 0x10 - Proximity bombs but never collected
	SEC_BBullets_WithoutPrize,		// 0x11 - Bouncing bullets but never collected
	SEC_Guns_MaxWithoutPrize,		// 0x12 - Max guns without getting prizes
	SEC_Bombs_MaxWithoutPrize,		// 0x13 - Max bombs without getting prizes
	SEC_Specials_OnTooLong,			// 0x14 - Shields or Super on longer than possible

	// NOTE: This block of codes is the exception.  You may send them at any time.
	SEC_ShipWeaponLimitsTooHigh,	// 0x15 - Saved ship weapon limits too high (burst/repel/etc)
	SEC_ShipWeaponLevelTooHigh,		// 0x16 - Saved ship weapon level too high (guns/bombs)
	SEC_LoginChecksumMismatch,		// 0x17 - Login checksum mismatch (program exited)
	SEC_PositionChecksumMismatch,	// 0x18 - Position checksum mismatch
	SEC_ShipChecksumMismatch,		// 0x19 - Saved ship checksum mismatch

	// NOTE: These may only be sent in response to a security checksum request
	SEC_SoftICE,					// 0x1A - Softice Debugger Running
	SEC_DataChecksumMismatch,		// 0x1B - Data checksum mismatch
	SEC_ParameterMismatch,			// 0x1C - Parameter mismatch
	SEC_UnknownViolation,			// .... - Unknown integrity violation
	SEC_HighLatency = 0x3C			// 0x3C - Unknown integrity violation (High latency in Continuum)
};


// Arena login type-code field

enum Arena_Codes
{
	ARENA_RandomMain = 0xFFFF,	//   ?go
	ARENA_Private	 = 0xFFFD	//   ?go arena  ?go #arena
};


// RegForm player sex field

enum RegForm_Sex
{
	SEX_Male		 = 'M',	// Male
	SEX_Female		 = 'F',	// Female

	SEX_Alien		 = 'A',
	SEX_Bot			 = 'B',
	SEX_Other		 = 'O',
	SEX_Undetermined = 'U',
};


// ConnectType field from *info

enum ConnectType
{
	CONN_Unknown,		// 00 
	CONN_SlowModem,		// 01 
	CONN_FastModem,		// 02 
	CONN_UnknownModem,	// 03 
	CONN_UnknownNotRAS,	// 04 
	CONN_ISDN,			// 05 Will show up as "InvalidValue"
	CONN_PAD,			// 06 Will show up as "InvalidValue"
	CONN_Switch,		// 07 Will show up as "InvalidValue"
	CONN_InvalidValue	// 08 All others are also invalid
};


// Chat message soundbytes
// c2s (Chat) 06 09 __ 01 00 MM EE SS SS AA GG EE 00

enum Chat_SoundBytes
{							// Soundcodes courtesy of MGB
	SND_None,				// 0  = Silence
	SND_BassBeep,			// 1  = BEEP!
	SND_TrebleBeep,			// 2  = BEEP!
	SND_ATT,				// 3  = You're not dealing with AT&T
	SND_Discretion,			// 4  = Due to some violent content, parental discretion is advised
	SND_Hallellula,			// 5  = Hallellula
	SND_Reagan,				// 6  = Ronald Reagan
	SND_Inconceivable,		// 7  = Inconceivable
	SND_Churchill,			// 8  = Winston Churchill
	SND_SnotLicker,			// 9  = Listen to me, you pebble farting snot licker
	SND_Crying,				// 10 = Crying
	SND_Burp,				// 11 = Burp
	SND_Girl,				// 12 = Girl
	SND_Scream,				// 13 = Scream
	SND_Fart,				// 14 = Fart1
	SND_Fart2,				// 15 = Fart2
	SND_Phone,				// 16 = Phone ring
	SND_WorldUnderAttack,	// 17 = The world is under attack at this very moment
	SND_Gibberish,			// 18 = Gibberish
	SND_Ooooo,				// 19 = Ooooo
	SND_Geeee,				// 20 = Geeee
	SND_Ohhhh,				// 21 = Ohhhh
	SND_Ahhhh,				// 22 = Awwww
	SND_ThisGameSucks,		// 23 = This game sucks
	SND_Sheep,				// 24 = Sheep
	SND_CantLogIn,			// 25 = I can't log in!
	SND_MessageAlarm,		// 26 = Beep
	SND_StartMusic = 100,	// 100= Start music playing
	SND_StopMusic,			// 101= Stop music
	SND_PlayOnce,			// 102= Play music for 1 iteration then stop
	SND_VictoryBell,		// 103= Victory bell
	SND_Goal				// 104= Goal!
};


#pragma pack(pop)	// End of bitfields


#include "host.h"


// Continuum object toggling
// s2c (Object toggle) 35 __ __

#pragma pack(push)	// More bitfields
#pragma pack(1)

union objectInfo
{
	struct
	{
		Uint16 id			: 15;	// Object ident
		Uint16 disabled		: 1;	// 1=off, 0=on
	};

	Uint16 n;
};

// Continuum object modification
// c2s (Object modify) 0a <pid(2)> <subtype=36(1)> <array of modifiers>

enum _ObjectLayers
{
	LAYER_BelowAll,
	LAYER_AfterBackground,
	LAYER_AfterTiles,
	LAYER_AfterWeapons,
	LAYER_AfterShips,
	LAYER_AfterGauges,
	LAYER_AfterChat,
	LAYER_TopMost,
};

enum _ObjectModes
{
	MODE_ShowAlways,
	MODE_EnterZone,
	MODE_EnterArena,
	MODE_Kill,
	MODE_Death,
	MODE_ServerControlled,
};

typedef struct	/* 11 by */
{
	BYTE  change_xy    :  1;  // what properties to change for this object
	BYTE  change_image :  1;
	BYTE  change_layer :  1;
	BYTE  change_time  :  1;
	BYTE  change_mode  :  1;
	BYTE  reserved     :  3;

	WORD  mapobj       :  1;
	WORD  id           : 15;
	WORD  x, y;  // for screen objects, upper 12 bits are value, lower 4 are relative to what corner
	BYTE  image;
	BYTE  layer;
	WORD  time         : 12;  // 1/10th seconds
	WORD  mode         :  4;  // 0=AlwaysOn 5=ServerControlled
} lvzObject;

#pragma pack(pop)	// End of bitfields


// s2c Message handler prototypes

void __stdcall handleUnknown			(hostMessage *m);
void __stdcall handleInGameFlag			(hostMessage *m);
void __stdcall handleChat				(hostMessage *m);
void __stdcall handleBannerAds			(hostMessage *m);
void __stdcall handleCustomMessage		(hostMessage *m);
void __stdcall handleFileTransfer		(hostMessage *m);
void __stdcall handleFileRequest		(hostMessage *m);
void __stdcall handleLoginNext			(hostMessage *m);
void __stdcall handleIdent				(hostMessage *m);
void __stdcall handleTurfFlagStatus		(hostMessage *m);
void __stdcall handlePlayerPrize		(hostMessage *m);
void __stdcall handleSelfPrize			(hostMessage *m);
void __stdcall handlePlayerEntering		(hostMessage *m);
void __stdcall handleSetTeam			(hostMessage *m);
void __stdcall handleSetTeamAndShip		(hostMessage *m);
void __stdcall handlePlayerBanner		(hostMessage *m);
void __stdcall handlePlayerLeaving		(hostMessage *m);
void __stdcall handleCreateTurret		(hostMessage *m);
void __stdcall handleDeleteTurret		(hostMessage *m);
void __stdcall handleBallPosition		(hostMessage *m);
void __stdcall handleArenaList			(hostMessage *m);
void __stdcall handleFlagVictory		(hostMessage *m);
void __stdcall handleScoreReset			(hostMessage *m);
void __stdcall handleFlagDrop			(hostMessage *m);
void __stdcall handleFlagClaim			(hostMessage *m);
void __stdcall handleFlagReward			(hostMessage *m);
void __stdcall handlePlayerPosition		(hostMessage *m);
void __stdcall handleKoTHReset			(hostMessage *m);
void __stdcall handleAddKoTH			(hostMessage *m);
void __stdcall handleShipReset			(hostMessage *m);
void __stdcall handleArenaSettings		(hostMessage *m);
void __stdcall handleChangePosition		(hostMessage *m);
void __stdcall handleToggleUFO			(hostMessage *m);
void __stdcall handleBrickDrop			(hostMessage *m);
void __stdcall handleMapInfo			(hostMessage *m);
void __stdcall handleMapFile			(hostMessage *m);
void __stdcall handleKeepAlive			(hostMessage *m);
void __stdcall handleSpecPlayer			(hostMessage *m);
void __stdcall handleFlagPosition		(hostMessage *m);
void __stdcall handleSynchronization	(hostMessage *m);
void __stdcall handleSoccerGoal			(hostMessage *m);
void __stdcall handleScoreUpdate		(hostMessage *m);
void __stdcall handleWeaponUpdate		(hostMessage *m);
void __stdcall handlePlayerDeath		(hostMessage *m);
void __stdcall handlePasswordResponse	(hostMessage *m);
void __stdcall handleBannerFlag			(hostMessage *m);
void __stdcall handleSpeedStats			(hostMessage *m);
void __stdcall handlePlayerVoice		(hostMessage *m);
void __stdcall handleSetKoTHTimer		(hostMessage *m);
void __stdcall handleVersionCheck		(hostMessage *m);
void __stdcall handleObjectToggle		(hostMessage *m);
void __stdcall handleWatchDamage		(hostMessage *m);
void __stdcall handleReceivedObject		(hostMessage *m);
void __stdcall handleDamageToggle		(hostMessage *m);


// c2s Message generator prototypes

clientMessage *generateViolation		(Security_Violations code);
clientMessage *generateChangeShip		(Ship_Types ship);
clientMessage *generateSpectate			(Uint16 player);
clientMessage *generateChangeTeam		(Uint16 team);
clientMessage *generateChangeBanner		(BYTE *buffer);
clientMessage *generateDeath			(Uint16 player, Uint16 bounty);
clientMessage *generateChat				(Chat_Modes type, Chat_SoundBytes soundcode, Uint16 player,
										 char *text);	// Limit 200 on message length
clientMessage *generateRegForm			(char *name, char *email, char *city, char *state,
										 RegForm_Sex sex, BYTE age, bool playAtHome, bool playAtWork,
										 bool playAtSchool, Uint32 processor, char *regName, char *regOrg);
clientMessage *generateKoTHReset		();
clientMessage *generateSecurityChecksum	(Uint32 parameterChecksum, Uint32 EXEChecksum, Uint32 levelChecksum,
										 Uint16 S2C_RelOut, Uint16 ping, Uint16 avgPing, Uint16 lowPing,
										 Uint16 highPing, Uint32 weaponCount, Uint16 S2CSlowCurrent,
										 Uint16 S2CFastCurrent, Uint16 S2CSlowTotal, Uint16 S2CFastTotal,
										 bool slowFrame);
clientMessage *generatePassword			(bool newUser, char *name, char *pass, Uint32 machineID,
										 Uint16 timezoneBias, Uint32 permissionID, Uint16 clientVersion,
										 BYTE connectType);
clientMessage *generateCtmPassword		(bool newUser, char *name, char *pass, Uint32 machineID,
										 Uint16 timezoneBias, Uint32 permissionID, Uint16 clientVersion,
										 BYTE connectType);
clientMessage *generatePowerballRequest	(Uint32 timestamp, BYTE ball);
clientMessage *generatePowerballUpdate	(Uint32 timestamp, BYTE ball, Uint16 x, Uint16 y,
										 Uint16 xvelocity, Uint16 yvelocity, Uint16 owner);
clientMessage *generateSoccerGoal		(Uint32 timestamp, BYTE ball);
clientMessage *generateFlagRequest		(Uint16 flag);
clientMessage *generateFlagDrop			();
clientMessage *generateEXERequest		();
clientMessage *generateLevelRequest		();
clientMessage *generateNewsRequest		();
clientMessage *generateArenaLogin		(char *arena, Ship_Types ship, Uint16 xres, Uint16 yres, bool allowAudio);
clientMessage *generateArenaLeave		();
clientMessage *generateAttachRequest	(Uint16 player);
clientMessage *generateBrickDrop		(Uint16 xtile, Uint16 ytile);
clientMessage *generatePosition			(Uint32 timestamp, BYTE direction, Uint16 x, Uint16 y, Uint16 xvelocity, Uint16 yvelocity,
										 bool stealth, bool cloaked, bool xradar, bool awarp, bool flash, bool safety, bool ufo,
										 Uint16 bounty, Uint16 energy, Projectile_Types type, BYTE level, bool shrapBounce, BYTE shrapLevel,
										 BYTE shrapCount, bool secondary, Uint16 timer, Uint16 S2CLag, bool shields, bool super,
										 BYTE burst, BYTE repel, BYTE thor, BYTE brick, BYTE decoy, BYTE rocket, BYTE portal);
clientMessage *generatePosition			(Uint32 timestamp, BYTE direction, Uint16 x, Uint16 y, Uint16 xvelocity, Uint16 yvelocity,
										 bool stealth, bool cloaked, bool xradar, bool awarp, bool flash, bool safety, bool ufo,
										 Uint16 bounty, Uint16 energy, Projectile_Types type, BYTE level, bool shrapBounce, BYTE shrapLevel,
										 BYTE shrapCount, bool secondary);
clientMessage *generateFileTransfer		(char *fileName, char *buffer, Uint32 len);
clientMessage *generateSendNewVoice		(BYTE id, Uint16 player, char *buffer, Uint32 len);
clientMessage *generateSendOldVoice		(BYTE id, Uint16 player);
clientMessage *generateObjectModify		(Uint16 player, lvzObject *objects, Uint16 num_objects);
clientMessage *generateObjectToggle		(Uint16 player, objectInfo *objects, Uint16 num_objects);
clientMessage *generateTakeGreen		(Uint32 timestamp, Sint16 prize, Uint16 x, Uint16 y);
clientMessage *generateChangeSettings	(_linkedlist <String> &settings);
clientMessage *generateTaskSwitch		(_linkedlist <String> &settings);

#endif	// CLIENTPROT_H
