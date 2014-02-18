/*
	SubSpace transmission control over core protocol by cat02e@fsu.edu
*/


#ifndef HOST_H
#define HOST_H

#include "Host_Base.h"
#include "datatypes.h"
#include "sockets.h"
#include "chunk.h"
#include "encrypt.h"
#include "player.h"
#include "map.h"
#include "botdll.h"
#include "botinfo.h"
//#include "basewin.h"
//#include "prize.h"

#include <fstream>

// -1 is used in many places to designate
// the absence of an optional object stub
#ifndef UNASSIGNED
	#define UNASSIGNED  0xFFFF
#endif

// Core options

#define PACKET_SILENCE_LIMIT     4000
#define MAX_RELIABLE_IN_TRANSIT    15
#define CHUNK_SIZE                496
#define SLOW_ITERATION             50
#define LOG_INTERVAL             1000
#define SYNC_INTERVAL			 2000

#define COSTLY_SPEEDUP
#define CLUSTER_MODE

// Log packets to disk

#define S2C_LOG
#define C2S_LOG

// Messaging

struct reliableMessage
{
	char msg[PACKET_MAX_LENGTH];			// Message with the reliable header
	Uint32 len;								// Message length
	Uint32 ACK_ID;							// ACK identifier
	Uint32 lastSend;						// Last time it was sent

	void setTime();							// Update lastSend

	reliableMessage(Uint32 nACK_ID,
					char *nmsg,
					Uint32 nlen);			// Fill fields
};


struct clusterMessage
{
	char msg[PACKET_MAX_LENGTH];			// Message without the reliable header
	Uint32 len;								// Message length

	clusterMessage(char *nmsg,
				   Uint32 nlen);			// Fill fields
};


struct hostMessage
{
	char *msg;								// Trimmed buffer
	Uint32 len;								// Message length
	class Host *src;						// Sender of this message

	BYTE getType(bool special);				// Return type byte

	hostMessage(char *msg,
				Uint32 len,
				Host *src);					// Fill fields
	~hostMessage();							// Deallocate msg memory
};


struct loggedChat
{
	BYTE mode;
	BYTE snd;
	Uint16 ident;
	String msg;

	loggedChat(BYTE mode, BYTE snd, Uint16 ident, char *msg);
};


struct clientMessage
{
	char *msg;								// Message buffer
	Uint32 len;								// Message length

	void clear();							// Zero buffer

	clientMessage(Uint32 len);				// Allocate space
	~clientMessage();						// Deallocate msg memory
};

class Host : public Host_Base
{
public:
	char creation_parameters[512];

	/* Socket */
	UDPSocket socket;						// Socket wrapper
	INADDR remote;							// sockaddr wrapper
	void send(char *msg, Uint32 len);		// Simply encrypt and send to host

	/* Inbox */
	Uint32 remoteStep;						// Next ACK_ID expected
	_linkedlist <reliableMessage> received;	// Messages waiting for a lost packet
	void checkReceived(Uint32 ACK_ID,
					   char *msg,
					   Uint32 len);			// Process queued messages or queue another
	void sendACK(Uint32 ACK_ID);			// Send an acknowledgement

	/* Outbox */
	Uint32 localStep;						// Next ACK_ID to be used
	Uint32 localSent;						// Last ACK_ID to be sent
	_linkedlist <reliableMessage> sent;		// Messages waiting for ACKnowledgement
	void checkSent(Uint32 ACK_ID);			// Route ACK messages here
	void sendQueued();						// Resend the lost messages

	_linkedlist <reliableMessage> queued;	// Send-queue overflows go here
	void sendNext();						// Start sending a queued message
	bool queue(char *msg, Uint32 len);		// Add to the queued list

	/* General Commands */
	_jumptable <hostMessage> generalRouter;	// Server/Client protocol

	/* Special Commands */
	_jumptable <hostMessage> specialRouter;	// Core protocol

	/* State */
	Uint32 lastRecv;						// Time since last packet was recv'd
	bool clustering;						// Manage send() timing
	bool gotMap;							// Has the map file
	bool gotTurf;							// Got a turf update

	bool connecting;						// Until we get an 00 02, send 00 01
	Uint32 lastConnect;
	bool syncing;							// Until we get an 00 06, send 00 05
	Uint32 lastSync;



	Uint32 dictionary[256];					// File checksum dictionary

	/* Transfer modes */
	chunkBuffer little;						// 00 08 / 00 09
	chunkBuffer large;						// 00 0A
	Uint32 ftLength;						// Used to prevent DoS attacks (0 == no transfer)
	_linkedlist <clusterMessage> clustered;	// Clustered messages queued for sending
	void sendClustered(_listnode <clusterMessage> *head);

	/* Message encryption */
	SS_ENCR encryption;						// 00 01 / 00 02

	/* Host communication */
	friend class hostList;
	bool killMe;							// Tells hostList to clean up
	Uint32 lastIteration;					// Last time we had focus

	/* Logging */
	std::ofstream logFile;					// Log file for this bot instance
	String logBuffer;						// Buffered log data
	Uint32 lastLogTime;						// Last time we wrote the buffer to the disk
	bool logging;							// Writing to Chatter.log?
	void writeLogBuffer();					// Dump to disk
	void logLogin();						// Append to Logins.txt

//public:
	/* Chatter */
	_linkedlist <loggedChat> chatLog;		// List of chat messages to-be-sent
	Uint32 lastChatSend;					// Last time segment we reset send count
	Uint16 countChatSend;					// Count since last reset

	void postChat(BYTE mode, BYTE snd, Uint16 ident, char *msg);

	void tryChat(BYTE mode, BYTE snd, Uint16 ident, char *msg);
	void addChatLog(BYTE mode, BYTE snd, Uint16 ident, char *msg);
	bool sendNextLoggedChat();
	void doChatLog();

	Host(BOT_INFO &botInfo);
	~Host();

	// Route packets to the correct Host object
	bool validateSource(INADDR &src);							// Did a message originate here?
	void doEvents();

	// Carpool messages
	void beginCluster();										// Prepare for message carpooling
		void post(char *msg, Uint32 len, bool reliable);		// Queue for clustering
		void post(clientMessage *cm, bool reliable);			// Queue for clustering
		void postRR(clientMessage *cm);							// Post, release, reliable
		void postRU(clientMessage *cm);							// Post, release, unreliable
	void endCluster();											// Send queued, clustered messages

	// Synchronization
	Uint32 getHostTime();
	Uint32 getLocalTime(Uint32 time);


	// Routing
	void gotPacket(char *msg, Uint32 len);
	void gotMessage(char *msg, Uint32 len);
	void gotMessage(hostMessage *msg);
	void gotSpecialMessage(hostMessage *msg);

	// Core in
	void gotEncryptRequest(Uint32 key, Uint16 prot);
	void gotEncryptResponse(Uint32 key);
	void gotEncryptResponse(Uint32 key, BYTE mudge);
	void gotReliable(Uint32 id, char *msg, Uint32 len);
	void gotACK(Uint32 id);
	void gotSyncRequest(Uint32 time);
	void gotSyncRequest(Uint32 time, Uint32 sent, Uint32 recv);
	void gotSyncResponse(Uint32 pingTime, Uint32 pongTime);
	void gotDisconnect();
	void gotChunkBody(char *msg, Uint32 len);
	void gotChunkTail(char *msg, Uint32 len);
	void gotBigChunk(Uint32 total, char *msg, Uint32 len);
	void gotCancelDownload();
	void gotCancelDownloadAck();
	void gotCluster(char *msg, Uint32 len);

	// Core out
	void connect(bool postDisconnect);
	void disconnect(bool notify) override;
	void syncClocks();
	void sendDownloadCancelAck();

	// Game protocol
	void activateGameProtocol();	// Disable password and enable game protocol
	void resetArena();				// Clear and prepare game objects
	void resetSession();			// Reset session completely
	void resetIcons();				// Update team-owned objects' map icons

	// DLL imports
	Uint32 lastTick;
	friend class DLLImports;

	// Prizes
//	prizeSystem ps;

	bool brickExists(Uint16 ident);
	void doBrickEvents();
	void updateBrickTiles();

	void changeGoalMode();
	void changeGoalTiles();

	PBall *findBall(Uint16 ident);
	void doBallEvents();


	Flag *findFlag(Uint16 ident);
	void claimFlag(Uint16 flag, Uint16 player);
	void dropFlags(Uint16 player);
	void loadTurfFlags();
	void resetFlagTiles();

	// Chat
	void sendPrivate(Player *player, char *msg) override;
	void sendPrivate(Player *player, BYTE snd, char *msg) override;

	void sendTeam(char *msg) override;
	void sendTeam(BYTE snd, char *msg) override;

	void sendTeamPrivate(Uint16 team, char *msg) override;
	void sendTeamPrivate(Uint16 team, BYTE snd, char *msg) override;

	void sendPublic(char *msg) override;
	void sendPublic(BYTE snd, char *msg) override;

	void sendPublicMacro(char *msg) override;
	void sendPublicMacro(BYTE snd, char *msg) override;

	void sendChannel(char *msg) override;			// #;Message
	void sendRemotePrivate(char *msg) override;		// :Name:Messsage
	void sendRemotePrivate(char *name, char *msg) override;


#ifdef COSTLY_SPEEDUP
	_referencetable <Player> qal;
#endif

	Uint16 me;						// My ident
	Player *Me;						// My state

	bool speccing;					// Attempting to spectate followed player?
	Uint32 lastSpec;				// Last time at which we tried to spec

	void spectateNext();
	void spectate(Player *p);

	Player *addPlayer(Uint16 ident, char *name, char *squad, Uint32 flagPoints, Uint32 killPoints, Uint16 team, Uint16 wins, Uint16 losses, BYTE ship, bool acceptsAudio, Uint16 flagCount);
	Player *getPlayer(Uint16 ident);
	void killPlayer(Player *p);

	void killTurret(Player *p);		// Shaking off turrets
	void killTurreter(Player *p);	// Detaching from a host


//	Player *findTeammate(Player *excluded, Uint16 team);
//	Player *findHighScorer(Player *excluded, Uint16 team);

//	Uint16 teamPopulation(Uint16 team);

	void revokeAccess(BYTE access);		// Reset player access for everyone under given level
	void revokeAccess(char *name);		// Reset player access for one player given name

	// Statistics

	Uint32 weaponCount;
	Uint16 S2CSlowCurrent;
	Uint16 S2CFastCurrent;
	Uint16 S2CSlowTotal;
	Uint16 S2CFastTotal;

	Sint32 numTries;						// Number of tries remaining before he stops logging in
	bool billerOnline;						// Biller down? filter op's without passwords
	bool downloadingNews;					// Do not download the news file twice
	bool inZone;							// First time bot enters arena, then constant
	bool inArena;							// Always tracks whether bot is in arena or not
	bool paranoid;							// Required to send extra information?

	bool position;							// Sending position update messages
	bool DLLFlying;							// Is the DLL in charge of the bot's position?


	Uint32 lastPosition;

	void sendPosition(bool reliable, Uint32 timestamp, BYTE ptype, BYTE level, bool shrapBouncing, BYTE shrapLevel, BYTE shrapCount, bool secondary);
	void sendPosition(bool reliable);
	void sendPosition(Uint32 timestamp, bool reliable);


	// Arena state
	bool loadedFlags;	// Loaded turf flags?
	BYTE map[TILE_MAX_LINEAR];
	arenaSettings settings;

	void changeArena(char *name) override;
	void changeCoordinates();

	// Logs
//	MDIChild *terminal;

	void logEvent(char *format, ...);
	void logIncoming(char *packet, Uint32 len);
	void logOutgoing(char *packet, Uint32 len);
};

String getTimeString();

#endif	// HOST_H
