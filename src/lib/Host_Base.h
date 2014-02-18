
#include "datatypes.h"
#include "sockets.h"
#include "chunk.h"
#include "encrypt.h"
#include "player.h"
#include "map.h"
#include "botdll.h"
#include "botinfo.h"

#ifndef HOST_BASE_H
#define HOST_BASE_H

struct Player;
struct clientMessage;

#define MAX_LOG_LINES              50
#define KEEP_LOG

class Host_Base
{
public:
	Host_Base(BOT_INFO & info);
	virtual ~Host_Base() { }
	
	virtual void sendPrivate(Player *player, char *msg) = 0;
	virtual void sendPrivate(Player *player, BYTE snd, char *msg) { }

	virtual void sendTeam(char *msg) = 0;
	virtual void sendTeam(BYTE snd, char *msg) { }

	virtual void sendTeamPrivate(Uint16 team, char *msg) = 0;
	virtual void sendTeamPrivate(Uint16 team, BYTE snd, char *msg) { }

	virtual void sendPublic(char *msg) = 0;
	virtual void sendPublic(BYTE snd, char *msg) { }

	virtual void sendPublicMacro(char *msg) = 0;
	virtual void sendPublicMacro(BYTE snd, char *msg) { }

	virtual void sendChannel(char *msg) = 0;
	virtual void sendRemotePrivate(char *msg) = 0;
	virtual void sendRemotePrivate(char *name, char *msg) = 0;
	virtual void postRR(clientMessage *cm) = 0;	// Post, release, reliable
	virtual void revokeAccess(BYTE access) =0 ;		// Reset player access for everyone under given level
	virtual void revokeAccess(char *name) = 0;		// Reset player access for one player given name

	virtual void changeArena(char *name) = 0;
	virtual void disconnect(bool notify) = 0;
	Player *findPlayer(const char *name);
	// Player
	_linkedlist <Player> playerlist;

	BOT_INFO botInfo;
	Player *Me;						// My state
	friend class DLLImports;
	DLLImports *imports;

	// Up time
	Uint32 arenaJoinTime, zoneJoinTime;

	Operator_Level lowestLevel;				// Lowest level command access enabled
	bool allowLimited;						// Enable !ownbot !own !give commands
	char turretMode;						// 0:auto-turret, 1:controlled-turret
	bool hasSysOp;							// Result of sysop check
	bool hasSMod;							// Result of smod check
	bool hasMod;							// Result of mod check

	Player *limitedOwner;					// Track Limited bot owner
	bool broadcastingErrors;				// Zone-error broadcast channel

	Uint32 msgSent;							// Total distinct messages sent
	Uint32 msgRecv;							// Total distinct messages recv'd
	Uint32 lastSyncRecv;					// Used by subspace to invalidate slow or fast time syncs
	Sint32 timeDiff;						// Delta T between server and client - changes over time
	Uint32 syncPing;						// Average host response time to sync requests
	Uint32 accuPing;						// Ping time accumulator for average ping time
	Uint32 countPing;						// Ping count accumulator for average ping time
	Uint32 avgPing;							// Average ping time
	Uint32 highPing;						// Highest ping outlier
	Uint32 lowPing;							// Lowest ping outlier

	String botChats;						// ?chat channels that the bot has subscribed to
	bool billerOnline;						// Biller down? filter op's without passwords
	
	_listnode <Player> *follow;		// Followed player
	_linkedlist <PBall> ballList;
	_linkedlist <Goal> goalList;
	_linkedlist <Flag> flagList;	// only lists uncarried flags
	_linkedlist <Brick> brickList;

#ifdef KEEP_LOG
	int logLength;							// Total log entries
	String *loggedChatter[MAX_LOG_LINES];	// Log of bot chatter for !log
#endif

};

#endif