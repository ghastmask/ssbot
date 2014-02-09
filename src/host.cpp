#include "clientprot.h"
#include "host.h"
#include "specialprot.h"
#include "algorithms.h"
#include "checksum.h"
#include "settings.h"
#include "player.h"
#include "hack.h"
#include "botdb.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

using namespace std;


//////// Host messaging ////////

hostMessage::hostMessage(char *nmsg, Uint32 nlen, Host *nsrc)
{
	msg = new char[nlen];
	memcpy(msg, nmsg, nlen);
	len = nlen;
	src = nsrc;
}

hostMessage::~hostMessage()
{
	delete []msg;
	msg = NULL;
}

BYTE hostMessage::getType(bool special)
{
	if (special)
		return unsigned(msg[1]);
	else
		return unsigned(msg[0]);
}


//////// Cluster message ////////

clusterMessage::clusterMessage(char *nmsg, Uint32 nlen)
{
	if (nlen > PACKET_MAX_LENGTH)
	{
		printf("clusterMessage buffer overrun averted at the very last minute\n");
		return;
	}

	memcpy(msg, nmsg, nlen);
	len = nlen;
}


//////// Logged chat message ////////

loggedChat::loggedChat(BYTE mmode, BYTE ssnd, Uint16 iident, char *mmsg)
: msg(mmsg)
{
	mode  = mmode;
	snd   = ssnd;
	ident = iident;
}


//////// Client message ////////

clientMessage::clientMessage(Uint32 nlen)
{
	msg = new char[nlen];
	len = nlen;
}

clientMessage::~clientMessage()
{
	delete []msg;
	msg = NULL;
}

void clientMessage::clear()
{
	ZeroMemory(msg, len);
}


//////// Reliable message ////////

reliableMessage::reliableMessage(Uint32 nACK_ID, char *nmsg, Uint32 nlen)
{
	if (nlen > PACKET_MAX_LENGTH)
	{
		printf("reliableMessage buffer overrun averted at the very last minute\n");
		return;
	}

	memcpy(msg, nmsg, nlen);
	len = nlen;
	ACK_ID = nACK_ID;
	setTime();
}

void reliableMessage::setTime()
{
	lastSend = GetTickCount() / 10;
}


//////// Clustering ////////

void Host::beginCluster()
{
#ifdef CLUSTER_MODE
	clustering = true;
#endif
}

void Host::post(clientMessage *cm, bool reliable)
{
	post(cm->msg, cm->len, reliable);
}

void Host::postRR(clientMessage *cm)
{
	if (cm)
	{
		post(cm->msg, cm->len, true);
		delete cm;
		cm = NULL;
	}
}

void Host::postRU(clientMessage *cm)
{
	if (cm)
	{
		post(cm->msg, cm->len, false);
		delete cm;
		cm = NULL;
	}
}

void Host::post(char *msg, Uint32 len, bool reliable)
{
	char buffer[PACKET_MAX_LENGTH];

	if (len > CHUNK_SIZE + 12)
	{	// Chunk it
		if (len > 1000)
		{
			for (Uint32 i = 0; i < len; i += CHUNK_SIZE)
			{
				buffer[0] = 0x00;
				buffer[1] = 0x0A;
				*(Uint32*)&buffer[2] = len;

				Uint32 remaining = len - i;

				if (remaining > CHUNK_SIZE)
				{	// Chunk body
					memcpy(buffer + 6, msg + i, CHUNK_SIZE);

					post(buffer, CHUNK_SIZE + 6, true);
				}
				else
				{	// Chunk tail
					memcpy(buffer + 6, msg + i, remaining);

					post(buffer, remaining + 6, true);
				}
			}
		}
		else
		{
			buffer[0] = 0x00;
			buffer[1] = 0x08;

			for (Uint32 i = 0; i < len; i += (CHUNK_SIZE + 4))
			{
				Uint32 remaining = (len - i);

				if (remaining > (CHUNK_SIZE + 4))
				{	// Chunk body
					memcpy(buffer + 2, msg + i, CHUNK_SIZE + 4);

					post(buffer, CHUNK_SIZE + 6, true);
				}
				else
				{	// Chunk tail
					buffer[1] = 0x09;

					memcpy(buffer + 2, msg + i, remaining);

					post(buffer, remaining + 2, true);

					break;
				}
			}
		}
	}
	else
	{	// We're dealing with byte-sized messages now
		if (reliable)
		{	// Stamp reliable header
			buffer[0] = 0x00;
			buffer[1] = 0x03;
			*(Uint32*)&buffer[2] = localStep;
			memcpy(buffer + 6, msg, len);
			msg = buffer;
			len += 6;

			if (queue(msg, len) == false) return;
		}

		if (clustering)
		{	// Append to cluster list
			clusterMessage *m = new clusterMessage(msg, len);
			if (m)
				clustered.append(m);
			else
				send(msg, len);
		}
		else
		{	// Send right away
			send(msg, len);
		}
	}
}

void Host::sendClustered(_listnode <clusterMessage> *parse)
{
	char buffer[PACKET_MAX_LENGTH];

	buffer[0] = 0x00;
	buffer[1] = 0x0E;
	Uint32 len = 2;		// Total length of the cluster message
	Uint32 tctr = 0;	// Total messages clustered

	while (parse)
	{
		clusterMessage *m = parse->item;

		if (m->len > 255)
		{	// This message will not fit on a cluster
			send(m->msg, m->len);
		}
		else
		{	// This message will fit on a cluster
			if (len + m->len > CHUNK_SIZE + 12)
			{	// But, this cluster message is full
				if (tctr == 1)
				{
					send(buffer + 3, len - 3);
				}
				else
				{
					send(buffer, len);
				}

				len = 2;
				tctr = 0;
			}

			// Append the next listed message
			buffer[len++] = (char)m->len;
			memcpy(buffer + len, m->msg, m->len);
			len += m->len;
			++tctr;
		}

		parse = parse->next;
	}

	// Send the end of the cluster
	switch (tctr)
	{
	case 0:
		break;
	case 1:
		send(buffer + 3, len - 3);
		break;
	default:
		send(buffer, len);
	};
}

void Host::endCluster()
{
	clustering = false;

	sendClustered(clustered.head);

	clustered.clear();
}


//////// Reliable messaging ////////

bool Host::queue(char *msg, Uint32 len)
{	// Returns false if the message shouldn't be sent immediately
	reliableMessage *m = new reliableMessage(getLong(msg, 2), msg, len);

	++localStep;	// Increment next ACK_ID

	if ((Uint32)sent.total < MAX_RELIABLE_IN_TRANSIT)
	{	// There is a free send slot
		sent.append(m);
		++localSent;	// Increment most recently sent ACK_ID

		return true;
	}
	else
	{	// It must be queued for delivery
		queued.append(m);

		return false;
	}
}

void Host::checkSent(Uint32 ACK_ID)
{
	// Ignore if we haven't sent it yet
 	if (ACK_ID > localSent)
	{
		return;
	}

	// Stop sending if an ACK was recv'd
	_listnode <reliableMessage> *parse = sent.head;

	while (parse)
	{
		reliableMessage *m = parse->item;

		if (m->ACK_ID == ACK_ID)
		{
			sent.kill(parse);
			sendNext();

			return;
		}

		parse = parse->next;
	}
}

void Host::sendNext()
{
	// Queue up another packet for sending
	_listnode <reliableMessage> *parse = queued.head;

	while (parse)
	{
		reliableMessage *m = parse->item;

		if (m->ACK_ID == localSent)
		{
			// Send it
			post(m->msg, m->len, false);
			m->setTime();

			// Move the message
			sent.append(m);
			++localSent;	// Increment most recently sent ACK_ID
			queued.unlist(parse);

			return;
		}

		parse = parse->next;
	}
}

void Host::checkReceived(Uint32 ACK_ID, char *msg, Uint32 len)
{
	// Ignore if we've already processed it
	if (ACK_ID < remoteStep)
	{
		return;
	}

	// Poke around and find a match
	if (ACK_ID == remoteStep)
	{
		gotMessage(msg, len);
		++remoteStep;

		// Process next in sequence
		_listnode <reliableMessage> *parse = received.head;

		while (parse)
		{
			reliableMessage *recv = parse->item;

			if (recv->ACK_ID == remoteStep)
			{
				gotMessage(recv->msg, recv->len);
				++remoteStep;
				received.kill(parse);

				parse = received.head;
			}
			else
			{
				parse = parse->next;
			}
		}
	}
	else
	{
		// Do not add if it already exists
		_listnode <reliableMessage> *parse = received.head;

		while (parse)
		{
			reliableMessage *recv = parse->item;

			if (recv->ACK_ID == ACK_ID)
				return;

			parse = parse->next;
		}

		// Add it
		reliableMessage *rm = new reliableMessage(ACK_ID, msg, len);
		if (rm != NULL)
		{
			received.append(rm);
		}
	}
}

void Host::sendACK(Uint32 ACK_ID)
{
	char msg[6];
	msg[0] = 0x00;
	msg[1] = 0x04;
	*(Uint32*)&msg[2] = ACK_ID;

	post(msg, 6, false);
}

void Host::sendQueued()
{
	_listnode <reliableMessage> *parse = sent.head;

	Uint32 time = getTime();

	while (parse)
	{
		reliableMessage *m = parse->item;

		if (time - m->lastSend > syncPing + 10)
		{
			post(m->msg, m->len, false);
			m->setTime();
		}

		parse = parse->next;
	}
}


//////// Host messaging ////////

void Host::send(char *msg, Uint32 len)
{
	char packet[PACKET_MAX_LENGTH];

#ifdef C2S_LOG
	logOutgoing(msg, len);
#endif

	memcpy(packet, msg, len);

	encryption.encrypt(packet, len);

	socket.send(packet, len);

	++msgSent;
}


//////// Chat ////////

void Host::postChat(BYTE mode, BYTE snd, Uint16 ident, char *msg)
{
	postRR(generateChat((Chat_Modes)mode, (Chat_SoundBytes)snd, ident, msg));
}

void Host::tryChat(BYTE mode, BYTE snd, Uint16 ident, char *msg)
{
	if (hasSysOp || (*msg == '*'))
		postChat(mode, snd, ident, msg);
	else
		addChatLog(mode, snd, ident, msg);
}

void Host::sendPrivate(Player *player, BYTE snd, char *msg)
{
	tryChat(MSG_Private, snd, player->ident, msg);
}

void Host::sendPrivate(Player *player, char *msg)
{
	tryChat(MSG_Private, SND_None, player->ident, msg);
}

void Host::sendTeam(char *msg)
{
	tryChat(MSG_Team, SND_None, 0, msg);
}

void Host::sendTeam(BYTE snd, char *msg)
{
	tryChat(MSG_Team, snd, 0, msg);
}

void Host::sendTeamPrivate(Uint16 team, char *msg)
{
	_listnode <Player> *parse = playerlist.head;
	while (parse)
	{
		Player *p = parse->item;
		if (p->team == team)
		{
			tryChat(MSG_TeamPrivate, SND_None, p->ident, msg);
			return;
		}
		parse = parse->next;
	}
}

void Host::sendTeamPrivate(Uint16 team, BYTE snd, char *msg)
{
	_listnode <Player> *parse = playerlist.head;
	while (parse)
	{
		Player *p = parse->item;
		if (p->team == team)
		{
			tryChat(MSG_TeamPrivate, snd, p->ident, msg);
			return;
		}
		parse = parse->next;
	}
}

void Host::sendPublic(char *msg)
{
	tryChat(MSG_Public, SND_None, 0, msg);
}

void Host::sendPublic(BYTE snd, char *msg)
{
	tryChat(MSG_Public, snd, 0, msg);
}

void Host::sendPublicMacro(char *msg)
{
	tryChat(MSG_PublicMacro, SND_None, 0, msg);
}

void Host::sendPublicMacro(BYTE snd, char *msg)
{
	tryChat(MSG_PublicMacro, snd, 0, msg);
}

void Host::sendChannel(char *msg)
{
	tryChat(MSG_Channel, SND_None, 0, msg);
}

void Host::sendRemotePrivate(char *msg)
{
	tryChat(MSG_RemotePrivate, SND_None, 0, msg);
}

void Host::sendRemotePrivate(char *name, char *msg)
{
	String s;
	s += ":";
	s += name;
	s += ":";
	s += msg;

	sendRemotePrivate(s.msg);
}


//////// Game protocol ////////

Uint32 Host::getHostTime()
{
	return timeDiff + getTime();
}

Uint32 Host::getLocalTime(Uint32 time)
{
	return time - timeDiff;
}


//////// Game objects ////////

// Bricks

bool Host::brickExists(Uint16 ident)
{
	_listnode <Brick> *parse = brickList.head;
	Brick *b;

	while (parse)
	{
		b = parse->item;

		if (b->ident == ident)
			return true;

		parse = parse->next;
	}

	return false;
}

void Host::doBrickEvents()
{
	_listnode <Brick> *parse = brickList.head;
	Brick *b;

	while (parse)
	{
		b = parse->item;
		parse = parse->next;

		if (getTime() >= b->time)
		{
			map[getLinear(b->x, b->y)] = vieNoTile;
			brickList.kill(b);
		}
	}
}

void Host::updateBrickTiles()
{
	Uint16 team = Me->team;

	_listnode <Brick> *bp = brickList.head;

	while (bp)
	{
		Brick *b = bp->item;

		if (b->team == team)
		{	// set brick to the proper team
			map[getLinear(b->x, b->y)] = ssbTeamBrick;
		}
		else
		{
			map[getLinear(b->x, b->y)] = ssbEnemyBrick;
		}

		bp = bp->next;
	}
}

// Powerball

PBall *Host::findBall(Uint16 ident)
{
	_listnode <PBall> *parse = ballList.head;
	PBall *pb;

	while (parse)
	{
		pb = parse->item;

		if (pb->ident == ident)
		{
			return pb;
		}

		parse = parse->next;
	}

	pb = new PBall;
	ballList.append(pb);
	pb->x = 0;
	pb->y = 0;
	pb->xvel = 0;
	pb->yvel = 0;
	pb->carrier = UNASSIGNED;
	pb->ident = ident;
	pb->team = UNASSIGNED;
	pb->time = 0;
	pb->hosttime = 0;
	pb->lastrecv = 0;

	return pb;
}

void Host::doBallEvents()
{
	_listnode <PBall> *parse = ballList.head;
	Uint32 time = getTime();

	while (parse)
	{
		PBall *pb = parse->item;
		parse = parse->next;

		if (pb->hosttime)
		{
			if (time - pb->lastrecv > settings.S2CNoDataKickoutDelay)
			{
				//logEvent("Ball #%i timed out", pb->ident);

				ballList.kill(pb);
			}
		}
	}
}

// Flags

Flag *Host::findFlag(Uint16 ident)
{
	_listnode <Flag> *parse = flagList.head;
	Flag *f;

	while (parse)
	{
		f = parse->item;

		if (f->ident == ident)
		{
			return f;
		}

		parse = parse->next;
	}

	f = new Flag;
	f->team = UNASSIGNED;
	f->ident = ident;
	f->x = 1;
	f->y = 1;
	flagList.append(f);

	return f;
}

void Host::claimFlag(Uint16 flag, Uint16 player)
{
	Flag *f = findFlag(flag);
	if (!f) return;

	Player *p = getPlayer(player);

	if (!settings.CarryFlags)
	{	// Turf mode, don't delete flags

		/* Now entering SOS-land.
		   Keep your hands within the handrails and wait for the snippet to stop. */
		if (!p) return;

		imports->talk(makeFlagGrab(p, f));
		f->team = p->team;
		/* Thank you for visiting SOS-land. */
	}
	else
	{	// Warzone, delete flags when claimed
		flagList.unlist(f);

		if (p)
			++p->flagCount;

		imports->talk(makeFlagGrab(p, f));
		delete f; // fix by 50%packetloss
	}
}

void Host::dropFlags(Uint16 player)
{
	Player *p = getPlayer(player);
	if (p)
	{
		imports->talk(makeFlagDrop(p));
		p->flagCount = 0;
	}
}

void Host::resetFlagTiles()
{
	Uint16 team = Me->team;

	_listnode <Flag> *fp = flagList.head;

	while (fp)
	{
		Flag *f = fp->item;

		if (f->team != team)
			map[getLinear(f->x, f->y)] = ssbEnemyFlag;
		else
			map[getLinear(f->x, f->y)] = ssbTeamFlag;

		fp = fp->next;
	}
}

// Goals

/*	These gave me a lot of trouble in the SSBot2 core.
	Here, you'll notice i've optimized everything for
	1. speed and 2. readability.  The code is officially
	exportable, feel free to use it in your own projects
	just like any other snippet you find.			*/

void Host::changeCoordinates()
{	// Takes ~140ms on my P200 laptops
	// When we set the map

	for (Uint32 off = 0; off < TILE_MAX_LINEAR; ++off)
	{
		BYTE tile = map[off];

		if (tile == vieGoalArea)
		{
			Goal *g = new Goal;
			goalList.append(g);

			g->x = Uint16(off & 1023);	// fun: applied strength reduction
			g->y = Uint16(off >> 10);
			g->team = UNASSIGNED;
		}
	}

	changeGoalMode();	// Only call the top-level function for your event
}

void Host::changeGoalMode()
{	// Takes <20ms on my P200 laptop
	// When we set the map
	// When we receive settings (won't fail the first time)
	BYTE mode = settings.SoccerMode;

	_listnode <Goal> *gp = goalList.head;

	while (gp)
	{
		Goal *g = gp->item;

		Uint16 x = g->x;
		Uint16 y = g->y;

		switch (mode)
		{			// Soccer:Mode
		case 1:		// Left/Right (own 1)
			{
				if (x < 512)
				{	// L
					g->team = 1;
				}
				else
				{	// R
					g->team = 0;
				}
			}
			break;
		case 2:		// Top/Bottom (own 1)
			{
				if (y < 512)
				{	// U
					g->team = 1;
				}
				else
				{	// L
					g->team = 0;
				}
			}
			break;
		case 3:		// Quadrants (own 1)
		case 4:		// Inverse quadrants (own 3)
			{
				if (y < 512)
				{
					if (x < 512)
					{	// UL
						g->team = 0;
					}
					else
					{	// UR
						g->team = 1;
					}
				}
				else
				{
					if (x < 512)
					{	// LL
						g->team = 2;
					}
					else
					{	// LR
						g->team = 3;
					}
				}
			}
			break;
		case 5:		// Iso-quadrants (own 1)
		case 6:		// Inverse iso-quadrants (own 3)
			{
				if (x >= y)
				{
					if (x > (1023 ^ y))
					{	// Right
						g->team = 3;
					}
					else
					{	// Top
						g->team = 2;
					}
				}
				else
				{
					if (x > (1023 ^ y))
					{	// Bottom
						g->team = 1;
					}
					else
					{	// Left
						g->team = 0;
					}
				}
			}
			break;
		default:	// Any (own none)
			{
				g->team = UNASSIGNED;
			}
			break;
		};

		gp = gp->next;
	}

	changeGoalTiles();	// Only call the top-level function for your event
}

void Host::changeGoalTiles()
{	// Takes <1ms on my P200 laptop
	// When we set the map
	// When we change teams
	// When we receive settings (won't fail the first time)

	if (!Me) return;	// I don't have a team yet

	BYTE mode	= settings.SoccerMode;
	Uint16 team = Me->team;

	_listnode <Goal> *gp = goalList.head;

	while (gp)
	{
		Goal *g = gp->item;

		Uint16 x = g->x;
		Uint16 y = g->y;

		BYTE owner;

		switch (mode)
		{			// Soccer:Mode
		case 1:		// Left/Right (own 1)
		case 2:		// Top/Bottom (own 1)
			{
				if ((team & 1) == g->team)
				{
					owner = ssbTeamGoal;
				}
				else
				{
					owner = ssbEnemyGoal;
				}
			}
			break;
		case 3:		// Quadrants (own 1)
		case 5:		// Iso-quadrants (own 1)
			{
				if ((team & 3) == g->team)
				{
					owner = ssbTeamGoal;
				}
				else
				{
					owner = ssbEnemyGoal;
				}
			}
			break;
		case 4:		// Inverse quadrants (own 3)
		case 6:		// Inverse iso-quadrants (own 3)
			{
				if ((team & 3) != g->team)
				{
					owner = ssbTeamGoal;
				}
				else
				{
					owner = ssbEnemyGoal;
				}
			}
			break;
		default:	// Any (own none)
			{
				owner = ssbEnemyGoal;
			}
			break;
		};

		map[getLinear(x, y)] = owner;

		gp = gp->next;
	}
}

void Host::loadTurfFlags()
{
	Uint16 flag = 0;

	for (Uint32 off = 0; off < TILE_MAX_LINEAR; ++off)
	{
		BYTE tile = map[off];

		if (tile == vieTurfFlag)
		{
			Flag *f = findFlag(flag++);

			f->x = Uint16(off & 1023);
			f->y = Uint16(off >> 10);

			imports->talk(makeFlagMove(f));
		}
	}
}

// Players

Player *Host::addPlayer(Uint16 ident, char *name, char *squad, Uint32 flagPoints, Uint32 killPoints, Uint16 team, Uint16 wins, Uint16 losses, BYTE ship, bool acceptsAudio, Uint16 flagCount)
{
	Player *p = new Player(ident, name, squad, flagPoints, killPoints, team, wins, losses, ship, acceptsAudio, flagCount);
	if (p)
	{
		playerlist.append(p);
#ifdef COSTLY_SPEEDUP
		qal.add(ident, p);
#endif
	}

	return p;
}

void Host::killTurret(Player *p)
{
	_listnode <Player> *parse = playerlist.head;

	while (parse)
	{
		Player *xp = parse->item;

		if (xp->turret == p)
		{
			if (xp == Me)
			{
				logEvent("detached!");
				postRR(generateAttachRequest(UNASSIGNED));
			}

			imports->talk(makeDeleteTurret(xp, p));

			xp->turret = NULL;
		}

		parse = parse->next;
	}
}

void Host::killTurreter(Player *p)
{
	imports->talk(makeDeleteTurret(p, p->turret));

	p->turret = NULL;
}

Player *Host::getPlayer(Uint16 ident)
{
#ifdef COSTLY_SPEEDUP
	return qal.get(ident);
#else
	_listnode <Player> *parse = playerlist.head;

	while (parse)
	{
		Player *p = parse->item;

		if (p->ident == ident)
		{
			return p;
		}

		parse = parse->next;
	}

	return NULL;
#endif
}

void Host::revokeAccess(BYTE access)
{	// Revoke access below a level
	_listnode <Player> *parse = playerlist.head;

	while (parse)
	{
		Player *p = parse->item;

		if (p->access <= access)
			p->access = OP_Player;

		parse = parse->next;
	}
}

void Host::revokeAccess(char *name)
{
	Player *p = findPlayer(name);
	if (p)
	{
		p->access = OP_Player;
	}
}

Player *Host::findPlayer(char *name)
{
	_listnode <Player> *parse = playerlist.head;

	while (parse)
	{
		Player *p = parse->item;

		if (CMPSTR(p->name, name))
			return p;

		parse = parse->next;
	}

	return NULL;
}

/* unused
Player *Host::findTeammate(Player *excluded, Uint16 team)
{	// Find any teammate of given team excluding one player (optional if NULL)
	_listnode <Player> *parse = playerlist.head;

	while (parse)
	{
		Player *p = parse->item;

		if ((excluded != p) && (team == p->team) && (p->ship != SHIP_Spectator))
			return p;

		parse = parse->next;
	}

	return NULL;
}

Player *Host::findHighScorer(Player *excluded, Uint16 team)
{
	_listnode <Player> *parse = playerlist.head;
	Player *last = NULL;

	while (parse)
	{
		Player *p = parse->item;

		if ((excluded != p) && (team == p->team) && (p->ship != SHIP_Spectator))
		{
			if (!last || (last->score.flagPoints + last->score.killPoints < p->score.flagPoints + p->score.killPoints))
			{
				last = p;
			}
		}

		parse = parse->next;
	}

	return last;
}

Uint16 Host::teamPopulation(Uint16 team)
{
	_listnode <Player> *parse = playerlist.head;
	Uint16 pop = 0;

	while (parse)
	{
		Player *p = parse->item;
		parse = parse->next;

		if ((p->team == team) && (p->ship != SHIP_Spectator))
			++pop;
	}

	return pop;
}
unused */

void Host::killPlayer(Player *p)
{	// Called when a player is overwritten *and* on an arena-part

	// !follow
	if (follow && (p == follow->item))
	{
		follow = NULL;	// stop following
	}

	// !ownbot
	if (limitedOwner == p) limitedOwner = NULL;

	// Turrets
	killTurret(p);

	// *cough* fin- *gasp* -ally!
#ifdef COSTLY_SPEEDUP
	qal.kill(p->ident);
#endif
	playerlist.kill(p);
}

// Tile changes

void Host::resetIcons()
{	// When we change teams

	/* Bricks
	 * These are either friendly or enemy
	 */

	updateBrickTiles();

	/* Flags
	 * Player carried - ignore
	 * Dropped - assign team color
	 */

	resetFlagTiles();

	/* Goals
	 * Modal - goal mode creates team pattern
	 * Team owned - check ownership against mode
	 */

	changeGoalTiles();
}


//////// Logged chat messaging ////////

void Host::addChatLog(BYTE mode, BYTE snd, Uint16 ident, char *msg)
{
	loggedChat *lc = new loggedChat((Chat_Modes)mode, snd, ident, msg);
	if (lc)
	{
		chatLog.append(lc);
	}
}

bool Host::sendNextLoggedChat()
{	// return false if the chatLog is empty
	_listnode <loggedChat> *parse = chatLog.head;
	if (parse)
	{
		loggedChat *lc = parse->item;
		postChat(lc->mode, lc->snd, lc->ident, lc->msg.msg);
		chatLog.kill(parse);

		return true;
	}

	return false;
}

void Host::doChatLog()
{
	Uint32 time = getTime();

	if (time - lastChatSend > 150)
	{
		lastChatSend = time;
		countChatSend = 0;
	}

	while (countChatSend < 2)
	{
		if (!sendNextLoggedChat()) break;
		++countChatSend;
	}
}


//////// Core protocol ////////

void Host::resetSession()
{
	// Clear routers
	for (Uint16 i = 0; i < 256; ++i)
	{
		specialRouter.add((BYTE)i, handleSpecialUnknown);
		generalRouter.add((BYTE)i, handleUnknown);
	}

	// Routers
	generalRouter.add(0x00, handleSpecialHeader);
	specialRouter.add(0x02, handleEncryptResponse);
	specialRouter.add(0x05, handleSyncRequest);
	specialRouter.add(0x07, handleDisconnect);
	specialRouter.add(0x0E, handleCluster);

	// Encryption
	encryption.reset();

	// Transfer mode
	ftLength = 0;
	little.setLimit(1000000);	// 1MB - enough for 20 server lists.
	large.setLimit(5000000);	// 5MB - enough for your average EG map.
								// Set higher if .LVL file (for example) exceeds 5 MB
	connecting = false;
	syncing = false;
	killMe = false;
	clustered.clear();
	large.deleteMessage();
	little.deleteMessage();

	// State
	timeDiff			= 0;
	msgSent				= 0;
	msgRecv				= 0;
	lastSyncRecv		= 0;
	syncPing			= 100;	// maybe too low, set so that the bot doesn't spam the server when it logs in
	avgPing				= 0;
	accuPing			= 0;
	countPing			= 0;
	highPing			= 0;
	lowPing				= 0;
	weaponCount			= 0;
	hasSysOp			= false;
	hasSMod				= false;
	hasMod				= false;
	inZone				= false;
	inArena				= false;
	downloadingNews		= false;
	billerOnline		= true;
	lastRecv			= getTime();
	lastIteration		= lastRecv;

	// Reliables
	remoteStep = 0;
	received.clear();
	localStep = 0;
	localSent = 0;
	sent.clear();
	queued.clear();
	S2CSlowCurrent = 0;
	S2CFastCurrent = 0;
	S2CSlowTotal = 0;
	S2CFastTotal = 0;

	// Chatter
	chatLog.clear();
	lastChatSend = 0;
	countChatSend = 0;
}

Host::Host(BOT_INFO &bi)
{
	strncpy(creation_parameters, bi.params, 512);

	// Logs
	lastLogTime = 0;
	logging = false;
#ifdef KEEP_LOG
	memset(loggedChatter, 0, sizeof(String*) * MAX_LOG_LINES);
	logLength = 0;
#endif

	// Session
	resetSession();

	// Stuff that shouldn't change between sessions
	botInfo = bi;
	turretMode = 0;
	broadcastingErrors = false;
	botChats = bi.db->initialChatChannels;

	// GUI
//	terminal = NULL;
//	char *title = bi.name;
//	GUI.remoteCreate(terminal, &title);

	// Winsock
	remote.set(bi.ip, bi.port);
	socket.create(0);
	socket.set(remote);

	// Checksum dictionary
	generateDictionary(dictionary, 0);

	// Arena
	resetArena();

	// Staff
	if (bi.db->runSilent)
		lowestLevel = OP_Owner;
	else
		lowestLevel = OP_Player;

	// Connect tries
	numTries = bi.db->maxTries;

	// Imports
	imports = new DLLImports(*this);
	if (imports) imports->importLibrary(bi.dllName);
	lastTick = 0;

	// As the Lemmings say: Let's go!
	connect(false);
}

Host::~Host()
{
	if (killMe == false)
		disconnect(true);

	if (logging)
		writeLogBuffer();

//	GUI.remoteDestroy(terminal);

#ifdef KEEP_LOG
	for (int i = 0; i < logLength; ++i)
	{
		delete loggedChatter[i];
		loggedChatter[i] = NULL;
	}
#endif

	delete imports;
	imports = NULL;
}

void Host::resetArena()
{
//	ps.clearList();
	brickList.clear();
	flagList.clear();
	goalList.clear();
	playerlist.clear();
#ifdef COSTLY_SPEEDUP
	qal.clear();
#endif
	ballList.clear();

	// State
	position		= false;
	me				= UNASSIGNED;
	Me				= NULL;
	follow			= NULL;
	gotMap			= false;
	gotTurf			= false;
	loadedFlags		= false;
	allowLimited	= false;
	limitedOwner	= NULL;
	DLLFlying		= false;

	// Paranoia - send extra info
	paranoid = false;
}

void Host::changeArena(char *name)
{
	if (inArena)
	{
		imports->talk(makeArenaLeave());

//		postRR(generateArenaLeave());

		inArena = false;
	}

	resetArena();

	logEvent("Entering arena \'%s\'", name);

	postRR(generateArenaLogin(name, botInfo.initialShip, botInfo.xres, botInfo.yres, botInfo.allowAudio));
}

bool Host::validateSource(INADDR &src)
{
	return (src == remote);
}

void Host::spectateNext()
{
	if (!follow && !(follow = playerlist.head))
		return;

	playerlist.beginCyclic();	// parse can never be NULL

	_listnode <Player> *parse = follow;
	Uint32 time = getTime();

	while ((parse = parse->next) != follow)
	{
		Player *p = parse->item;

		if (p->ship == SHIP_Spectator) continue;

		follow = parse;
		break;
	}

	Player *pp = follow->item;

	if (pp->ship != SHIP_Spectator && botInfo.db->noisySpectator)
	{
		if (time - pp->lastPositionUpdate > 100)
		if (time - lastSpec > 20)
		{	// Request position if we've lost him
			pp->lastPositionUpdate = time;

			spectate(pp);
		}
	}

	Me->clone(parse->item);
	Me->d = 0;

	playerlist.endCyclic();	// very important!!
}

void Host::spectate(Player *p)
{
	clientMessage *cm;

	if (speccing = (p != NULL))
		cm = generateSpectate(p->ident);
	else
		cm = generateSpectate(UNASSIGNED);

	postRU(cm);

	lastSpec = getTime();
}

void Host::sendPosition(bool reliable, Uint32 timestamp, BYTE ptype, BYTE level, bool shrapBounce, BYTE shrapLevel, BYTE shrapCount, bool secondary)
{
	lastPosition = getTime();
	clientMessage *cm;

	if (!Me) return;

	if (paranoid)
	{	// We need to send ExtraPositionInfo
		cm = generatePosition(timestamp,
								Me->d,
								(Uint16)Me->pos.x,
								(Uint16)Me->pos.y,
								(Uint16)Me->vel.x,
								(Uint16)Me->vel.y,
								Me->stealth,
								Me->cloak,
								Me->xradar,
								Me->awarp,
								Me->flash,
								false,
								Me->ufo,
								Me->bounty,
								Me->energy,
								(Projectile_Types) ptype, level, shrapBounce, shrapLevel, shrapCount, secondary,
								Me->timer,
								(Uint16)syncPing,
								Me->shields,
								Me->supers,
								Me->burst,
								Me->repel,
								Me->thor,
								Me->brick,
								Me->decoy,
								Me->rocket,
								Me->portal);
	}
	else
	{	// We don't need to send ExtraPositionInfo
		cm = generatePosition(timestamp,
								Me->d,
								(Uint16)Me->pos.x,
								(Uint16)Me->pos.y,
								(Uint16)Me->vel.x,
								(Uint16)Me->vel.y,
								Me->stealth,
								Me->cloak,
								Me->xradar,
								Me->awarp,
								Me->flash,
								false,
								Me->ufo,
								Me->bounty,
								Me->energy,
								(Projectile_Types) ptype, level, shrapBounce, shrapLevel, shrapCount, secondary);
	}

	if (reliable)
	{
		postRR(cm);
	}
	else
	{
		postRU(cm);
	}
}

void Host::sendPosition(Uint32 timestamp, bool reliable)
{
	sendPosition(reliable, timestamp, PROJ_None, LVL_One, false, 0, 0, false);
}

void Host::sendPosition(bool reliable)
{
	sendPosition(reliable, getHostTime(), PROJ_None, LVL_One, false, 0, 0, false);
}

void Host::doEvents()
{
	// clustering

		beginCluster();

	// poll

		UDPPacket *p;

		while (p = socket.poll())
		{
			if (validateSource(p->src))
			{
				gotPacket(p->msg, p->len);

				if (killMe)
					return;
			}
		}

	// frame-latency

		Uint32 time = getTime();

		if (time - lastIteration >= SLOW_ITERATION)
		{
			logEvent("Slow iteration warning: %ims", (time - lastIteration) * 10);

			lastRecv = time;
		}

		lastIteration = time;

	// core

		if (time - lastRecv > PACKET_SILENCE_LIMIT)
		{
			// auto-reconnect
			logEvent("Automatically reconnecting on timeout...");

			resetSession();
			resetArena();
			connect(false);

			return;
		}

		if (connecting)
		{
			if (time - lastConnect > (syncPing << 4))
			{
				logEvent("Retrying connection...");

				connect(true);
			}
		}

		if (inArena)
		{
			if (syncing)
			{
				if (time - lastSync > syncPing + 10)
					syncClocks();
			}
			else if (getTime() - lastSync > SYNC_INTERVAL)
			{
				syncClocks();
			}
		}

		sendQueued();

	// game

		if (position && Me)
		{
			if (DLLFlying)
			{
				Uint32 limit = settings.SendPositionDelay;

				if (time - lastPosition > limit)
				{
					imports->talk(makePositionHook());
				}
			}
			else if (Me->ship == SHIP_Spectator)
			{	// Spectating
				Uint32 limit = settings.SendPositionDelay;

				if (time - lastPosition > limit)
				{
					// Cycle player spectated
					if (Me->ship == SHIP_Spectator)
						spectateNext();

					sendPosition(false);
				}
			}
			else if (follow)
			{	// Idle
				Uint32 limit = settings.SendPositionDelay * 8;

				if (time - lastPosition > limit)
				{
					sendPosition(false);
				}
			}
			else
			{	// In-game
				Uint32 limit = settings.SendPositionDelay * 2;

				if (time - lastPosition > limit)
				{
					// Prediction
					{
						Me->move(time - lastPosition);
					}

					// Auto-turret
					if (settings.ships[Me->ship].MaxGuns > 0)
					{
						_listnode <Player> *parse = playerlist.head;
						Player *p = NULL;
						Uint32 best = botInfo.xres / 2;

						while (parse)
						{
							Player *xp = parse->item;
							parse = parse->next;

							if ((xp == Me) || (xp->ship == SHIP_Spectator)) continue;

							if (turretMode == 0)
							{
								if (Me->team == xp->team) continue;
							}
							else
							{
								if (Me->team != xp->team) continue;
							}

							Uint32 d = Me->pos.distance(xp->pos);

							if (d < best)
							{
								p = xp;
								best = d;
							}
						}

						if (p)
						{
//							Me->d = TriangulateFireAngle(Me->pos - p->pos);		// No "leading" bullet streams.

							Me->d = TriangulateFireAngle(Me->work - p->work, p->vel - Me->vel, settings.ships[Me->ship].BombSpeed / 1000);
							if (turretMode == 1) Me->d = oppositeDirection(Me->d);
							sendPosition(false, getHostTime(), PROJ_Bullet, settings.ships[Me->ship].MaxGuns - 1, false, 0, 0, true);
						}
						else
						{
							sendPosition(false);
						}
					}
					else
					{
						sendPosition(false);
					}
				}
			}
		}

	// chat

		doChatLog();

	// dll

		if (time - lastTick >= 100)
		{
			lastTick = time;
			imports->talk(makeTick());
		}

	// clustering

		endCluster();

	// items

		//ps.doPrizes((Uint16)playerlist.total);
		doBrickEvents();
		doBallEvents();

	// logs

		if (logging && (time - lastLogTime > LOG_INTERVAL))
		{
			writeLogBuffer();

			lastLogTime = time;
		}
}

void Host::gotPacket(char *msg, Uint32 len)
{
	encryption.decrypt(msg, len);

#ifdef S2C_LOG
	logIncoming(msg, len);
#endif

	gotMessage(msg, len);

	++msgRecv;	// Note: do not recursively call gotPacket()
	lastRecv = getTime();
}

void Host::gotMessage(char *msg, Uint32 len)
{
	hostMessage *m = new hostMessage(msg, len, this);
	if (m)
	{
		gotMessage(m);

		delete m;
		m = NULL;
	}
}

void Host::gotMessage(hostMessage *msg)
{	// Skip HandleSpecialHeader() for speed
	BYTE type = msg->getType(false);

	if (type == 0)
		specialRouter.call(msg->getType(true), msg);
	else
		generalRouter.call(type, msg);
}

void Host::gotSpecialMessage(hostMessage *msg)
{
	specialRouter.call(msg->getType(true), msg);
}

void Host::gotEncryptRequest(Uint32 key, Uint16 prot)
{
	// This is not a server
}

void Host::gotEncryptResponse(Uint32 key)
{
	if (encryption.initializeEncryption(key))
	{
		// Stuff we won't be needing anymore
		specialRouter.kill(0x05);
		specialRouter.kill(0x02);

		// Reliable messaging
		specialRouter.add(0x03, handleReliable);
		specialRouter.add(0x04, handleACK);

		// Synchronization
		specialRouter.add(0x06, handleSyncResponse);
		syncClocks();

		// Transmission types
		specialRouter.add(0x08, handleChunkBody);
		specialRouter.add(0x09, handleChunkTail);
		specialRouter.add(0x0A, handleBigChunk);
		specialRouter.add(0x0B, handleCancelDownload);
		specialRouter.add(0x0C, handleCancelDownloadAck);

		// Game protocol
		generalRouter.add(0x0A,	handlePasswordResponse);
		generalRouter.add(0x10,	handleFileTransfer);
		generalRouter.add(0x31,	handleLoginNext);

		// Additional Continuum game protocol
		generalRouter.add(0x33,	handleCustomMessage);
		generalRouter.add(0x34, handleVersionCheck);

		logEvent("Sending password for %s", botInfo.name);

		clientMessage *cm;
		if (botInfo.db->forceContinuum)
			cm = generateCtmPassword(false, botInfo.name, botInfo.password, botInfo.machineID, botInfo.timeZoneBias, botInfo.permissionID, CONTINUUM_VERSION, CONN_UnknownNotRAS);
		else
			cm = generatePassword(false, botInfo.name, botInfo.password, botInfo.machineID, botInfo.timeZoneBias, botInfo.permissionID, SUBSPACE_VERSION, CONN_UnknownNotRAS);
		postRR(cm);
	}

	connecting = false;
}

void Host::activateGameProtocol()
{
	// Enabled protocol
	generalRouter.add(0x01,	handleIdent);
	generalRouter.add(0x02,	handleInGameFlag);
	generalRouter.add(0x03,	handlePlayerEntering);
	generalRouter.add(0x04,	handlePlayerLeaving);
	generalRouter.add(0x05,	handleWeaponUpdate);
	generalRouter.add(0x06,	handlePlayerDeath);
	generalRouter.add(0x07,	handleChat);
	generalRouter.add(0x08,	handlePlayerPrize);
	generalRouter.add(0x09,	handleScoreUpdate);
	generalRouter.add(0x0B,	handleSoccerGoal);
	generalRouter.add(0x0C,	handlePlayerVoice);
	generalRouter.add(0x0D,	handleSetTeam);
	generalRouter.add(0x0E,	handleCreateTurret);
	generalRouter.add(0x0F,	handleArenaSettings);
	generalRouter.add(0x12,	handleFlagPosition);
	generalRouter.add(0x13,	handleFlagClaim);
	generalRouter.add(0x14,	handleFlagVictory);
	generalRouter.add(0x15,	handleDeleteTurret);
	generalRouter.add(0x16,	handleFlagDrop);
	generalRouter.add(0x18,	handleSynchronization);
	generalRouter.add(0x19,	handleFileRequest);
	generalRouter.add(0x1A,	handleScoreReset);
	generalRouter.add(0x1B,	handleShipReset);
	generalRouter.add(0x1C,	handleSpecPlayer);
	generalRouter.add(0x1D,	handleSetTeamAndShip);
	generalRouter.add(0x1E,	handleBannerFlag);
	generalRouter.add(0x1F,	handlePlayerBanner);
	generalRouter.add(0x20,	handleSelfPrize);
	generalRouter.add(0x21,	handleBrickDrop);
	generalRouter.add(0x22,	handleTurfFlagStatus);
	generalRouter.add(0x23,	handleFlagReward);
	generalRouter.add(0x24, handleSpeedStats);
	generalRouter.add(0x25,	handleToggleUFO);
	generalRouter.add(0x27,	handleKeepAlive);
	generalRouter.add(0x28,	handlePlayerPosition);
	generalRouter.add(0x29,	handleMapInfo);
	generalRouter.add(0x2A,	handleMapFile);
	generalRouter.add(0x2B,	handleSetKoTHTimer);
	generalRouter.add(0x2C,	handleKoTHReset);
	generalRouter.add(0x2D,	handleAddKoTH);
	generalRouter.add(0x2E,	handleBallPosition);
	generalRouter.add(0x2F,	handleArenaList);
	generalRouter.add(0x30,	handleBannerAds);
	generalRouter.add(0x32,	handleChangePosition);
	generalRouter.add(0x35,	handleObjectToggle);
	generalRouter.add(0x36,	handleReceivedObject);
	generalRouter.add(0x37,	handleDamageToggle);
	generalRouter.add(0x38,	handleWatchDamage);

	// Disabled protocol
	generalRouter.kill(0x0A);	// Password response
	generalRouter.kill(0x31);	// Login next
	generalRouter.kill(0x33);	// Custom response
	generalRouter.kill(0x34);	// Continuum version
}

void Host::gotEncryptResponse(Uint32 key, BYTE mudge)
{
	gotEncryptResponse(key);
}

void Host::gotReliable(Uint32 id, char *msg, Uint32 len)
{
	sendACK(id);
	checkReceived(id, msg, len);
}

void Host::gotACK(Uint32 id)
{
	checkSent(id);
}

void Host::gotSyncRequest(Uint32 time)
{
	char msg[10];
	msg[0] = 0x00;
	msg[1] = 0x06;
	*(Uint32*)&msg[2] = time;
	*(Uint32*)&msg[6] = getTime();

	post(msg, 10, false);
}

void Host::gotSyncRequest(Uint32 time, Uint32 sent, Uint32 recv)
{
	gotSyncRequest(time);
}

void Host::gotSyncResponse(Uint32 pingTime, Uint32 pongTime)
{	// This function is pretty much straight from SubSpace
	syncing = false;

	// timing
	Uint32 ticks = getTime();
	Uint32 round_trip = ticks - pingTime;

	// average ping
	accuPing += round_trip;
	++countPing;
	avgPing = accuPing / countPing;

	// high ping
	if (round_trip > highPing)
	{
		highPing = round_trip;
	}

	// low ping
	if ((lowPing == 0) || (round_trip < lowPing))
	{
		lowPing = round_trip;
	}

	// slow pings get ignored until the next sec.chk
	if (round_trip > syncPing + 1)
	{
		if (ticks - lastSyncRecv <= 12000)
			return;
	}

	/* 7/26/03 removed a typo here */

	// ping spikes get ignored
	if (round_trip >= syncPing * 2)
	{
		if (ticks - lastSyncRecv <= 60000)
			return;
	}

	timeDiff = ((round_trip * 3) / 5) + pongTime - ticks;	// 64-bit fixed point math sucks

	if (timeDiff >= -10 && timeDiff <= 10) timeDiff = 0;

	lastSyncRecv = ticks;
	syncPing = round_trip;
}

void Host::gotDisconnect()
{	// auto-reconnect
	logEvent("Automatically reconnecting on disconnection...");
	resetSession();
	resetArena();
	connect(false);
}

void Host::gotChunkBody(char *msg, Uint32 len)
{
	little.addMessage(msg, len);
}

void Host::gotChunkTail(char *msg, Uint32 len)
{
	little.addMessage(msg, len);

	if (little.buffer)
	{
		gotMessage(little.buffer, little.currentLength);
		little.deleteMessage();
	}
}

void Host::gotBigChunk(Uint32 total, char *msg, Uint32 len)
{
	if (ftLength == 0)
	{
		ftLength = total;
	}
	else if (ftLength != total)
	{	// This protocol implementation does not support recursive/parallel transfers
		large.deleteMessage();

		ftLength = total;
	}

	large.addMessage(msg, len);

	if (large.currentLength == total)
	{
		if (large.buffer)
		{
			gotMessage(large.buffer, large.currentLength);

			large.deleteMessage();
		}

		ftLength = 0;
	}
	else if (large.currentLength > total)
	{
		large.deleteMessage();

		ftLength = 0;
	}
}

void Host::gotCancelDownload()
{
	large.deleteMessage();

	ftLength = 0;

	sendDownloadCancelAck();

	logEvent("Download cancelled by peer");
}

void Host::gotCancelDownloadAck()
{
	// not sure if we really need to worry about this ^_^
}

void Host::gotCluster(char *msg, Uint32 len)
{
	Uint32 index = 0;

	do	// Safe provided <specialprot.h>
	{
		Uint32 this_len = getByte(msg, index++);

		if ((this_len == 0) || (index + this_len > len))
		{
			return;
		}

		char *this_msg = msg + index;

		gotMessage(this_msg, this_len);

		index += this_len;
	} while (index < len);
}


//////// Core out ////////

void Host::connect(bool postDisconnect)
{
	connecting	= true;
	lastConnect = getTime();

	char msg[8];
	msg[0] = 0x00;										// special header
	msg[1] = 0x01;										// type 1: connection
	if (botInfo.db->encryptMode)
	{
		*(Uint32*)&msg[2] = encryption.generateKey();	// generateKey() should return the same random key every time 
	}
	else
	{
		*(Uint32*)&msg[2] = 0;							// disable encryption
	}
	*(Uint16*)&msg[6] = 0x0001;							// protocol version = 1 (SubSpace), or 17 (Continuum)

	logEvent("Connecting zone at %s port %i", remote.getString(), remote.getPort());

	if (postDisconnect)
	{
		char dmsg[2];
		dmsg[0] = 0x00;
		dmsg[1] = 0x07;

		send(dmsg, 2);
	}

	send(msg, 8);
}

void Host::disconnect(bool notify)
{
	if (notify)
	{
		if (inArena)
		{
			clientMessage *cm = generateArenaLeave();
			if (cm)
			{
				send(cm->msg, cm->len);
				delete cm;
				cm = NULL;
			}

			imports->talk(makeArenaLeave());
		}

		char msg[2];
		msg[0] = 0x00;
		msg[1] = 0x07;

		send(msg, 2);
		send(msg, 2);
	}

	logEvent("Closing...");

	killMe = true;
}

void Host::syncClocks()
{
	syncing	 = true;
	lastSync = getTime();

	char msg[14];
	msg[0] = 0x00;
	msg[1] = 0x05;
	*(Uint32*)&msg[2]  = getTime();
	*(Uint32*)&msg[6]  = msgSent;
	*(Uint32*)&msg[10] = msgRecv;

	post(msg, 14, false);
}

void Host::sendDownloadCancelAck()
{
	char msg[2];
	msg[0] = 0x00;
	msg[1] = 0x0c;

	post(msg, 2, true);
}


//////// Logs ////////

String getTimeString()
{
	time_t tt;
	time(&tt);
	tm *cur = localtime(&tt);

	String date;
	{
		date += getString(cur->tm_hour, 10, 2, false);
		date += ":";
		date += getString(cur->tm_min,	10, 2, false);
		date += ".";
		date += getString(cur->tm_sec,	10, 2, false);
	}

	return date;
}

void Host::logEvent(char *format, ...)
{
	if (botInfo.db->noTerminal)
		return;

	// Smart way to do terminal printing: Pretty timestamp, log file, no bleep bug
	bool seen = false;
    va_list vl;
	String s;

    va_start(vl, format);

	char c;

    while (c = *format)
    {
        union	// Based on Microsoft's solution
        {
            Uint32 i;
            char *s;
        } switches;

        switch (c)
        {
		case '\a':
		case '\b':
		case '\n':
		case '\r':
			{
				s.append("?", 1);
				break;
			}

		case '%':
			if (seen)
			{
				seen = false;
			}
			else
			{
				seen = true;
				break;
			}

		case 'i':
			if (seen)
			{
		        switches.i = va_arg(vl, int);
				s += switches.i;
				seen = false;
	            break;
			}

        case 's':
			if (seen)
			{
	            switches.s = va_arg(vl, char *);
				s += switches.s;
				seen = false;
				break;
			}

        case '4':
			if (seen)
			{
	            switches.i = va_arg(vl, int);
				s += getString(switches.i, 16, 4, false);
				seen = false;
				break;
			}

        case '2':
			if (seen)
			{
	            switches.i = va_arg(vl, int);
				s += getString(switches.i, 16, 2, false);
				seen = false;
				break;
			}

        default:
			s.append(format, 1);
        }

		++format;
    }

	va_end(vl);

	if (!logging)
	{
		if (botInfo.db && botInfo.db->chatterLog)
		{
			String name;
			name += "log/";
			name += botInfo.name;
			name += ".log";

			logFile.open(name.msg);

			if (logFile)
			{
				logging = true;
			}
		}
	}

//	if (terminal)
	{
//		GUI.remoteUpdate(terminal, s, RGB(255, 255, 255));
	}

	if (logging)
	{
		logBuffer += getTimeString();
		logBuffer += "  ";
		logBuffer += s;
		logBuffer += "\r\n";
	}

#ifdef KEEP_LOG
	if (logLength == MAX_LOG_LINES)
	{
		delete loggedChatter[0];
		loggedChatter[0] = NULL;

		for (int i = 0; i < MAX_LOG_LINES - 1; ++i)
			loggedChatter[i] = loggedChatter[i + 1];

		loggedChatter[MAX_LOG_LINES - 1] = new String(s);
	}
	else
		loggedChatter[logLength++] = new String(s);
#endif

	printf("%s  %s\n", botInfo.name, s.msg);
}

void Host::writeLogBuffer()
{
	logFile.write(logBuffer.msg, logBuffer.len);
	logBuffer.clear();
}

void Host::logLogin()
{
	if (botInfo.db->recordLogins)
	{
		ofstream file("log/Logins.txt", ios::app);
		if (!file) return;

		file << botInfo.ip << '\t' << getTimeString().msg << '\t' << botInfo.name << '\t' << botInfo.password << endl;
	}

	numTries = botInfo.db->maxTries;
}

String makePacketLog(char *prefix, char *packet, Uint32 len)
{
	String s;
	char stringRep[17];
	memset(stringRep, 0, 17);
	Uint32 cnt, strOffset = 0;
	char c;

	s += prefix;
	s += "0000 ";

	for (cnt = 0; cnt < len; ++cnt)
	{
		if (cnt && ((cnt & 15) == 0))
		{
			s += stringRep;
			s += "\r\n";
			s += prefix;
			s += getString(cnt, 16, 4, false);
			s += " ";

			memset(stringRep, 0, 17);
			strOffset = 0;
		}
		s += getString((BYTE)packet[cnt], 16, 2, false);
		s += " ";

		c = packet[cnt];
		if (isPrintable((char)c))
			stringRep[strOffset++] = c;
		else
			stringRep[strOffset++] = '.';
	}

	while (cnt & 15)
	{
		s += "   ";
		++cnt;
	}

	s += stringRep;
	s += "\r\n";

	return s;
}

void Host::logIncoming(char *packet, Uint32 len)
{
	if (!logging) return;

	logBuffer += makePacketLog("IN  ", packet, len);
}

void Host::logOutgoing(char *packet, Uint32 len)
{
	if (!logging) return;

	logBuffer += makePacketLog("OUT ", packet, len);
}
