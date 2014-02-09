#include "clientprot.h"

#include "algorithms.h"
#include "player.h"
#include "map.h"
#include "checksum.h"
#include "command.h"
#include "hack.h"
#include "botdb.h"
#include "system.h"
#include "host.h"

using namespace std;


//////// S2C protocol ////////

void __stdcall handleUnknown(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		...		?
	*/

	h->logEvent("Unknown message type %i(%i)", msg[0], len);
	h->logIncoming(msg, len);
}

void __stdcall handleKeepAlive(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	*/

	if (len != 1)
	{
		handleUnknown(m);

		return;
	}
}

void __stdcall handleBannerFlag(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Boolean: visible?
	*/

	BYTE toggle = getByte(msg, 1);

	if (toggle || (len != 2))
	{
		handleUnknown(m);

		return;
	}

	// The banner shouldn't be sent to the client that changed his banner, this message is adequate.

//	h->logEvent("Successfully changed personal banner");
}

void __stdcall handleLoginNext(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	*/

	if (len != 1)
	{
		handleUnknown(m);

		return;
	}

//	h->logEvent("Login next");
}

void __stdcall handleInGameFlag(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	*/

	if (len != 1)
	{
		handleUnknown(m);

		return;
	}

	if (!h->inZone)
	{
		h->inZone = true;
		h->zoneJoinTime = GetTickCount();

		if (!h->botInfo.db->runSilent)
		{
			String pw;
			pw += "*";
			pw += h->botInfo.staffpw;

			h->sendPublic(pw.msg);								// Send staff password
			h->sendPublic("*listmod");							// Check for sysop/smod status
			if (h->Me) h->sendPrivate(h->Me, "*relkills 1");	// Reliable kills (subgame 11h+)

			String chats = "?chat=";
			chats += h->botChats;
			h->sendPublic(chats.msg);							// Enter the chats we want
		}
	}

	h->imports->talk(makeArenaEnter(h->botInfo.initialArena, h->Me, h->billerOnline));
	h->arenaJoinTime = GetTickCount();

//	h->logEvent("Spawn connected.");
}

void __stdcall handleBannerAds(hostMessage *m)
{	DEFLATE_CLASS
	// Provided by Ave-iator

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Display mode
		2		2		Width
		4		2		Height
		6		4		Duration
		10		?		Banner indices
	*/

	if (len <= 10)
	{
		handleUnknown(m);

		return;
	}

	BYTE mode = getByte(msg, 1);
	Uint16 width = getShort(msg, 2);
	Uint16 height = getShort(msg, 4);
	Uint32 duration = getLong(msg, 6);

	char *indices = msg + 10;
	Uint32 length = len - 10;

	// I'm not sure how these are packed (W*H+10 != len) ? zlib

	h->logEvent("Got banner advertisement(s)");
}


//////// FTP ////////

void __stdcall handleFileTransfer(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		16		File name
		17		...		[Compressed] file
	*/

	char *name = msg + 1;
	char *buffer = msg + 17;
	Uint32 i, length = len - 17;

	if (len <= 17)
	{
		handleUnknown(m);

		return;
	}

	for (i = 0; i < 16; ++i)
	{
		char c = name[i];

		if (c == 0)
			break;

		switch (c)
		{
		case '/':
		case '\\':
			h->logEvent("WARNING: Invalid downloaded-file name path ignored. (%s)", name);
			return;
		};

		if ((c < ' ') || (c > '~'))
		{
			h->logEvent("WARNING: Invalid downloaded-file name chars ignored.");
			return;
		}
	}

	if (i == 16)
	{
		h->logEvent("WARNING: Unterminated downloaded-file name ignored.");

		return;
	}

	if (*name)
	{	// regular file
		String fname = "get/";
		fname += name;

		ofstream file(fname.msg, ios::binary);
		if (file)
		{
			file.write(buffer, length);

			h->logEvent("Received file: %s", fname.msg);

			h->imports->talk(makeFile(fname.msg));
		}
		else
		{
			h->logEvent("Unable to open file for write: %s", fname.msg);
		}
	}
	else
	{	// news file
		h->downloadingNews = false;

		if (decompress_to_file("get/news.txt", buffer, length))
			h->logEvent("News file successfully transferred!");
		else
			h->logEvent("Unable to decompress news file.");
	}
}

void __stdcall handleFileRequest(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		256		Local file name
		257		16		Remote file name
	*/

	Uint16 i;
	char *fileName = msg + 1;
	char *remoteName = msg + 257;

	if (len != 273)
	{
		h->logEvent("WARNING: Requested-file malformed packet ignored.");
		h->logIncoming(msg, len);

		return;
	}

	if (*fileName == 0)
	{
		h->logEvent("WARNING: Blank requested-file local name ignored.");
		h->logIncoming(msg, len);

		return;
	}

	for (i = 0; i < 256; ++i)
	{
		char c = fileName[i];

		if (c == 0)
			break;

		switch (c)
		{
		case '/':
		case '\\':
			h->logEvent("WARNING: Invalid requested-file name path ignored. (%s)", fileName);
			return;
		};

		if ((c < ' ') || (c > '~'))
		{
			h->logEvent("WARNING: Invalid requested-file name chars ignored.");
			return;
		}
	}

	if (i == 256)
	{
		h->logEvent("WARNING: Unterminated requested-file local name ignored.");

		return;
	}

	/* Put other restrictions here */

	h->logEvent("File request: '%s' -> '%s'", fileName, remoteName);

	String name = "get/";
	name += fileName;

	// Encode and transmit file
	ifstream file(name.msg, ios::binary);
	if (!file)
	{
		h->logEvent("WARNING: Unable to read from file for file transfer");

		return;
	}

	file.seekg (0, ios::end);
	Uint32 length = file.tellg();
	file.seekg (0, ios::beg);

	if (length < 0)
	{
		h->logEvent("WARNING: Unable to get file length for file transfer");

		return;
	}

	char *buffer = new char[length];
	file.read(buffer, length);

	h->postRR(generateFileTransfer(remoteName, buffer, length));

	delete []buffer;
}


//////// Turret ////////

void __stdcall handleCreateTurret(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Turreter ident
		3		2		Turretee ident (when -1, detaching)
	*/

	if (len != 5)
	{
		handleUnknown(m);

		return;
	}

	Uint16 tid1 = getShort(msg, 1);
	Uint16 tid3 = getShort(msg, 3);

	Player *turreter = h->getPlayer(tid1);
	if (!turreter) return;

	if (tid3 == UNASSIGNED)
	{
		// Unattaching
		Player *turretee = turreter->turret;
		if (!turretee) return;

		h->killTurreter(turreter);

//		h->logEvent("%s detached from %s", turreter->name, turretee->name);
	}
	else
	{
		// Attaching
		Player *turretee = h->getPlayer(tid3);
		if (!turretee) return;

		turreter->turret = turretee;

		h->imports->talk(makeCreateTurret(turreter, turretee));

//		h->logEvent("%s turreted %s", turreter->name, turretee->name);
	}
}

void __stdcall handleDeleteTurret(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
	*/

	if (len != 3)
	{
		handleUnknown(m);

		return;
	}

	Player *p = h->getPlayer(getShort(msg, 1));
	if (p == NULL) return;

	// Shaking off turrets
	h->killTurret(p);

	h->logEvent("Deleted turret: %s", p->name);
}


//////// Flags ////////

void __stdcall handleTurfFlagStatus(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	The following are repeated until the end of the message
		1		2		Team for flag X
	*/

	Uint32 index = 1;
	Uint16 team,
		   flag = 0;

	if ((len - 1) & 1)
	{
		handleUnknown(m);

		return;
	}

	while (index < len)
	{
		team = getShort(msg, index);

		Flag *f = h->findFlag(flag);
		f->team = team;
		h->imports->talk(makeFlagMove(f));

		index += 2;
		++flag;
	}

	if (h->gotMap)
	{
		if (!h->loadedFlags)
		{
			h->loadTurfFlags();
			h->logEvent("Loaded turf flags from map data.");

			h->loadedFlags = true;
		}
	}

	h->gotTurf = true;

//	h->logEvent("Got turf update");
}

void __stdcall handleFlagPosition(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Flag ident
		3		2		X tiles
		5		2		Y tiles
		7		2		Team
	*/

	if (len != 9)
	{
		handleUnknown(m);

		return;
	}

	Uint16 flag	= getShort(msg, 1);
	Uint16 x	= getShort(msg, 3);
	Uint16 y	= getShort(msg, 5);
	Uint16 team	= getShort(msg, 7);

	Flag *f = h->findFlag(flag);

	f->x = x;
	f->y = y;
	f->team = team;

//	h->postRR(generateFlagRequest(flag));
//	h->postRR(generateFlagDrop());

	h->imports->talk(makeFlagMove(f));
}

void __stdcall handleFlagVictory(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Team
		3		4		Points
	*/

	if (len != 7)
	{
		handleUnknown(m);

		return;
	}

	Uint16 team = getShort(msg, 1);
	Uint32 points = getLong(msg, 3);

	if (team == UNASSIGNED)
	{
		// Flag game reset
		h->logEvent("Flag game reset.");

		h->imports->talk(makeFlagGameReset());

		_listnode <Player> *parse = h->playerlist.head;

		while (parse)
		{
			Player *p = parse->item;
			parse = parse->next;

			p->flagCount = 0;
		}
	}
	else
	{
		// Team victory
		h->logEvent("Team # %i won the flag game for %i points.", team, points);

		_listnode <Player> *parse = h->playerlist.head;

		// Find winning players
		while (parse)
		{
			Player *p = parse->item;
			parse = parse->next;

			if (p->team == team && p->ship != SHIP_Spectator && !p->safety)
				p->score.flagPoints += points;

			p->flagCount = 0;
		}

		h->imports->talk(makeFlagVictory(team, points));
	}

	h->flagList.clear(); // idea by 50%packetloss
}

void __stdcall handleFlagDrop(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
	*/

	if (len != 3)
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);

	h->dropFlags(ident);

//	h->logEvent("Flags owned by player #%i dropped", ident);
}

void __stdcall handleFlagClaim(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Flag ident
		3		2		Player ident
	*/

	if (len != 5)
	{
		handleUnknown(m);

		return;
	}

	Uint16 flag = getShort(msg, 1);
	Uint16 player = getShort(msg, 3);

	h->claimFlag(flag, player);

//	h->logEvent("Flag #%i owned by player #%i", flag, player);
}

void __stdcall handleFlagReward(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	The following are repeated until the end of the message
		1		2		Team
		3		2		Points
	*/

	Uint32 index = 1;

	if ((len - 1) & 3)
	{
		handleUnknown(m);

		return;
	}

	while (index < len)
	{
		Uint16 team = getShort(msg, index);
		Uint32 points = getShort(msg, index + 2);

//		h->logEvent("Flag reward for team %i: %i", team, points);

		_listnode <Player> *parse = h->playerlist.head;

		while (parse)
		{
			Player *p = parse->item;
			parse = parse->next;

			if (p->team == team && !p->safety) // safety zone fix by whommy
				p->score.flagPoints += points;
		}

		h->imports->talk(makeFlagReward(team, points));

		index += 4;
	}
}


//////// Players ////////

void __stdcall handlePlayerVoice(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
		3		...		Waveform
	*/

	if (len < 3)
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);
	Player *p = h->getPlayer(ident);
	if (!p) return;

	h->logEvent("Voice: %s", p->name);
}

void __stdcall handleScoreReset(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
	*/

	if (len != 3)
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);
	Player *p = h->getPlayer(ident);

	if (ident != UNASSIGNED)
	{
		if (p == NULL) return;

//		h->logEvent("Score reset: %s", p->name);

		p->score.flagPoints = 0;
		p->score.killPoints = 0;
		p->score.wins = 0;
		p->score.losses = 0;

		h->imports->talk(makePlayerScore(p));
	}
	else
	{
//		h->logEvent("Score reset: All");

		_listnode <Player> *parse = h->playerlist.head;

		while (parse)
		{
			Player *p = parse->item;
			parse = parse->next;

			p->score.flagPoints = 0;
			p->score.killPoints = 0;
			p->score.wins = 0;
			p->score.losses = 0;

			h->imports->talk(makePlayerScore(p));
		}
	}
}

void __stdcall handlePlayerPrize(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		4		Timestamp
		5		2		X tiles
		7		2		Y tiles
		9		2		Prize type
		11		2		Player ident
	*/

	if (len != 13)
	{
		handleUnknown(m);

		return;
	}

	Uint32 timestamp = h->getLocalTime(getLong(msg, 1));
	Uint16 x = getShort(msg, 5);
	Uint16 y = getShort(msg, 7);
	Uint16 prize = getShort(msg, 9);

//	h->ps.killGreen(timestamp, x, y);

	Player *p = h->getPlayer(getShort(msg, 11));
	if (p == NULL) return;

	if (timestamp > p->lastPositionUpdate)
	{
		++p->bounty;
		p->move(x << 4, y << 4);
	}

	h->imports->talk(makePlayerPrize(p, prize));

//	h->logEvent("%s picked up prize #%i [t=%i] (%i, %i)", p->name, prize, timestamp, x, y);
}

void __stdcall handleSpeedStats(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Best
		2		2		Your rank
		4		4		Your score
		8		4		Player 1 score
		12		4		Player 2 score
		16		4		Player 3 score
		20		4		Player 4 score
		24		4		Player 5 score
		28		2		Player 1 ident
		30		2		Player 2 ident
		32		2		Player 3 ident
		34		2		Player 4 ident
		36		2		Player 5 ident
	*/

	if (len != 38)
	{
		handleUnknown(m);

		return;
	}

	BYTE best = getByte(msg, 1);
	Uint16 myRank = getShort(msg, 2);
	Uint32 myScore = getLong(msg, 4);

	Uint16 ident1 = getShort(msg, 28);
	Uint16 ident2 = getShort(msg, 30);
	Uint16 ident3 = getShort(msg, 32);
	Uint16 ident4 = getShort(msg, 34);
	Uint16 ident5 = getShort(msg, 36);

	Player *p1 = h->getPlayer(ident1);
	Player *p2 = h->getPlayer(ident2);
	Player *p3 = h->getPlayer(ident3);
	Player *p4 = h->getPlayer(ident4);
	Player *p5 = h->getPlayer(ident5);

	Uint32 score1 = getShort(msg, 8);
	Uint32 score2 = getShort(msg, 12);
	Uint32 score3 = getShort(msg, 16);
	Uint32 score4 = getShort(msg, 20);
	Uint32 score5 = getShort(msg, 24);

	h->logEvent("Timed-Game Over!");

	if (p1) h->logEvent("#1    %i %s", score1, p1->name);
	if (p2) h->logEvent("#2    %i %s", score2, p2->name);
	if (p3) h->logEvent("#3    %i %s", score3, p3->name);
	if (p4) h->logEvent("#4    %i %s", score4, p4->name);
	if (p5) h->logEvent("#5    %i %s", score5, p5->name);

	h->logEvent("Your Rank: #%i  Your Score: %i", myRank, myScore);

	h->imports->talk(makeTimedGameOver(p1, p2, p3, p4, p5));
}

void __stdcall handlePlayerEntering(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
	The following are repeated until the end of the message
		0		1		Type byte
		1		1		Ship type
		2		1		Accepts audio messages
		3		20		Player name (confirmed ASCIIZ)
		23		20		Squad name (confirmed ASCIIZ)
		43		4		Flag points
		47		4		Kill points
		51		2		Player ident
		53		2		Team
		55		2		Wins
		57		2		Losses
		59		2		Turretee ident
		61		2		Flags carried
		63		1		Boolean: Has KoTH
	*/

	Uint32 index = 0;

	if (len & 63)
	{
		handleUnknown(m);

		return;
	}

	while (index < len)
	{
		BYTE ship = getByte(msg, index + 1);
		bool acceptsAudio = (getByte(msg, index + 2) != 0);
		char *name = msg + index + 3;
		char *squad = msg + index + 23;
		Uint32 killPoints = getLong(msg, index + 43);
		Uint32 flagPoints = getLong(msg, index + 47);
		Uint16 ident = getShort(msg, index + 51);
		Uint16 team = getShort(msg, index + 53);
		Uint16 wins = getShort(msg, index + 55);
		Uint16 losses = getShort(msg, index + 57);
		Uint16 turretee = getShort(msg, index + 59);
		Uint16 flagCount = getShort(msg, index + 61);
		bool hasKoTH = getByte(msg, index + 63) ? 1 : 0;

		Player *xp = h->getPlayer(ident);
		if (xp)
		{	// Remove duplicate ident's - happens quite often (Snrrrub noticed this)
			h->killPlayer(xp);
		}

		Player *p = h->addPlayer(ident, name, squad, flagPoints, killPoints, team, wins, losses, ship, acceptsAudio, flagCount);
		p->turret = h->getPlayer(turretee);
		p->koth = hasKoTH;

		if (ident == h->me)
		{
			if (hasKoTH)
				h->postRR(generateKoTHReset());

			p->koth = false;
			h->Me = p;

			h->logEvent("Player entering: %s #%i w/l %i:%i pts %i+%i <-- Me", name, ident, wins, losses, killPoints, flagPoints);
		}
		else if (h->billerOnline)
		{	// Don't do this if the biller is down
			opEntry *op = h->botInfo.db->findOperator(p->name);

			if (op)
			{	// Listed
				if (op->validatePass(""))
				{	// No login password
					p->access = op->getAccess();
					op->addCounter();
				}

				h->logEvent("Player entering: %s #%i w/l %i:%i pts %i+%i <-- Op", name, ident, wins, losses, killPoints, flagPoints);
			}
			else
				h->logEvent("Player entering: %s #%i w/l %i:%i pts %i+%i", name, ident, wins, losses, killPoints, flagPoints);
		}
		else
		{
			h->logEvent("Player entering: %s #%i w/l %i:%i pts %i+%i", name, ident, wins, losses, killPoints, flagPoints);
		}

		if (h->inArena)
			h->imports->talk(makePlayerEntering(p));

		index += 64;
	}
}

void __stdcall handleSetTeam(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
		3		2		Team
		5		1		Ship changed, only if high bit is not set (Snrrrub)
	*/

	if (len != 6)
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);

	Player *p = h->getPlayer(ident);
	if (p == NULL) return;

	Uint16 team = getShort(msg, 3);
	Uint8 ship = getByte(msg, 5);

	Uint16 oldteam = p->team;
	Uint16 oldship = p->ship;

	if ((ship == SHIP_Spectator) && (p->ship != ship))	// implicitly fails if high bit is set on ship type
	{
		if (h->follow && (p == h->follow->item))
			h->follow = NULL;

		p->team = team;
		p->ship = ship;

		h->imports->talk(makePlayerSpec(p, oldteam, oldship));
	}
	else
	{
		if (p->team != team)
		{
			p->team = team;
			h->imports->talk(makePlayerTeam(p, oldteam, oldship));
		}

		if ((ship & 128) == 0)	// if high bit is not set, then a ship change has occured
		if (p->ship != ship)
		{
			p->ship = (enum Ship_Types)ship;
			h->imports->talk(makePlayerShip(p, oldship, oldteam));
		}
	}

	if (ident == h->me)
	{
		if (p->team != team)
		{
			h->resetIcons();
		}
	}

	p->flagCount = 0;

//	h->logEvent("Player changed teams: %s {%i}", p->name, getByte(msg, 5));
}

void __stdcall handleSetTeamAndShip(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Ship type
		2		2		Player ident
		4		2		Team
	*/

	if (len != 6)
	{
		handleUnknown(m);

		return;
	}

	BYTE ship = msg[1];
	Uint16 ident = getShort(msg, 2);
	Player *p = h->getPlayer(ident);
	if (p == NULL) return;
	Uint16 team = getShort(msg, 4);

//	h->logEvent("Player changed team and ship: %s", p->name);

	Uint16 oldteam = p->team;
	Uint16 oldship = p->ship;

	if ((ship == SHIP_Spectator) && (p->ship != ship))
	{
		if (h->follow && (p == h->follow->item))
			h->follow = NULL;

		p->team = team;
		p->ship = ship;

		h->imports->talk(makePlayerSpec(p, oldteam, oldship));
	}
	else
	{
		if (p->team != team)
		{
			p->team = team;
			h->imports->talk(makePlayerTeam(p, oldteam, oldship));
		}

		if (p->ship != ship)
		{
			p->ship = (enum Ship_Types)ship;
			h->imports->talk(makePlayerShip(p, oldship, oldteam));
		}
	}

	if (ident == h->me)
	{
		if (p->team != team)
		{
			h->resetIcons();
		}
	}

	p->flagCount = 0;
}

void __stdcall handlePlayerBanner(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
		3		96		Player banner
	*/

	if (len != 99)
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);
	Player *p = h->getPlayer(ident);
	if (p == NULL) return;
	char *banner = msg + 3;

	p->setBanner(banner);

	h->imports->talk(makeBannerChanged(p));

//	h->logEvent("Set banner: %s", p->name);
}

void __stdcall handlePlayerLeaving(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
	*/

	if (len != 3)
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);

	Player *p = h->getPlayer(ident);
	if (p == NULL) return;

	h->logEvent("Player leaving: %s", p->name);

	h->imports->talk(makePlayerLeaving(p));

	h->killPlayer(p);
}

void __stdcall handleChat(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Message type
		2		1		Soundcode
		3		2		Player ident
		5		...\0	Message
	*/

	if (len <= 5)
	{
		h->logEvent("WARNING: Chat message buffer underrun ignored.");
		h->logIncoming(msg, len);

		return;
	}

	if (msg[len - 1])
	{
		h->logEvent("WARNING: Unterminated chat message ignored.");
		h->logIncoming(msg, len);

		return;
	}

	BYTE type = getByte(msg, 1);
	BYTE soundcode = getByte(msg, 2);
	char *text = msg + 5;

	Uint16 ident = getShort(msg, 3);
	Player *p = h->getPlayer(ident);

	if (!*text) return;

	h->imports->talk(makeChat(type, soundcode, p, text));

	switch (type)
	{
	case MSG_Arena:
			h->logEvent("A:%s", text);

			if (!h->hasMod)
			{
				Player *p = h->Me;
				if (p)
				{
					String s;

					s += p->name;
					s += " - Sysop - ";
					if (CMPSTART(s.msg, text))
					{	// SysOp checker
						h->logEvent("^^ I Am SysOp.  Hear me roar!");
						h->hasSysOp = true;
						h->hasSMod = true;
						h->hasMod = true;
						break;
					}

					s = p->name;
					s += " - SMod - ";
					if (CMPSTART(s.msg, text))
					{	// SMod checker
						h->logEvent("^^ I Am SuperModerator.  Hear me roar!");
						h->hasSysOp = false;
						h->hasSMod = true;
						h->hasMod = true;
						break;
					}
				}
			}
		break;
	case MSG_PublicMacro:
			if (p)
			{
				h->logEvent("MAC:%s> %s", p->name, text);

				if (!h->botInfo.db->disablePub)
				if ((*text == '!') || (*text == '.') || (*text == '@'))
					gotCommand(h, p, text + 1);
			}
			else
			{
				h->logEvent("Public macro - %s", text);
			}
		break;
	case MSG_Public:
			if (p)
			{
				h->logEvent("P:%s> %s", p->name, text);

				if (!h->botInfo.db->disablePub)
				if ((*text == '!') || (*text == '.') || (*text == '@'))
					gotCommand(h, p, text + 1);
			}
			else
			{
				h->logEvent("Public - %s", text);
			}
		break;
	case MSG_Team:
			if (p)
			{
				h->logEvent("T:%s> %s", p->name, text);

				if ((*text == '!') || (*text == '.') || (*text == '@'))
					gotCommand(h, p, text + 1);
			}
			else
			{
				h->logEvent("T:%s", text);
			}
		break;
	case MSG_TeamPrivate:
			if (p)
			{
				h->logEvent("TP:%s> %s", p->name, text);

				if ((*text == '!') || (*text == '.') || (*text == '@'))
					gotCommand(h, p, text + 1);
			}
			else
			{
				h->logEvent("TP:%s", text);
			}
		break;
	case MSG_Private:
			if (p)
			{
				h->logEvent("PV:%s> %s", p->name, text);

				if ((*text == '!') || (*text == '.') || (*text == '@'))
					gotCommand(h, p, text + 1);
			}
			else
			{
				h->logEvent("PV:%s", text);
			}
		break;
	case MSG_PlayerWarning:
			if (p)
				h->logEvent("W:%s> %s", p->name, text);
			else
				h->logEvent("W:%s", text);
		break;
	case MSG_RemotePrivate:
			h->logEvent("REM:%s", text);

			if (h->botInfo.db->remoteInterpreter &&
				validRemotePrivate(text))
			{
				char *message = getRemoteCommand(text);
				char *sender = getRemoteName(text);
				if ((*message == '!') || (*message == '.') || (*message == '@'))
					gotRemote(h, sender, message + 1);
			}
		break;
	case MSG_ServerError:
			h->logEvent("ERR:%s", text);

			if (h->broadcastingErrors)
			{
				String s = "1;";
				s += text;
				h->sendChannel(s.msg);
			}
		break;
	case MSG_Channel:
			h->logEvent("C:%s", text);

			if (h->botInfo.db->remoteInterpreter &&
				validRemoteChat(text))
			{
				char *message = getChatCommand(text);
				char *sender = getChatName(text);
				if ((*message == '!') || (*message == '.') || (*message == '@'))
					gotRemote(h, sender, message + 1);
			}
		break;
	default:
		h->logEvent("WARNING: Unknown chat message ignored. (%i)", type);
		h->logIncoming(msg, len);
		return;
	};
}

void __stdcall handleScoreUpdate(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
		3		4		Flag points
		7		4		Kill points
		11		2		Wins
		13		2		Losses
	*/

	if (len != 15)
	{
		handleUnknown(m);

		return;
	}

	Player *p = h->getPlayer(getShort(msg, 1));
	if (p == NULL) return;
	Uint32 killPoints = getLong(msg, 3);
	Uint32 flagPoints = getLong(msg, 7);
	Uint16 wins = getShort(msg, 11);
	Uint16 losses = getShort(msg, 13);

//	h->logEvent("Score update: %s w/l %i:%i pts %i+%i", p->name, wins, losses, killPoints, flagPoints);

	p->score.killPoints = killPoints;
	p->score.flagPoints = flagPoints;
	p->score.wins = wins;
	p->score.losses = losses;

	h->imports->talk(makePlayerScore(p));
}

void __stdcall handleWeaponUpdate(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Rotation
		2		2		Timestamp
		4		2		X pixels
		6		2		Y velocity
		8		2		Player ident
		10		2		X velocity
		12		1		Checksum
		13		1		Togglables
		14		1		Ping
		15		2		Y pixels
		17		2		Bounty
		19		2		Weapon info
	Spectating with ExtraPositionData or *energy
		21		2		Energy
	Spectating with ExtraPositionData
		23		2		S2CLag
		25		2		Timer
		27		4		Item information
	*/

	if ((len != 21) && (len != 23) && (len != 31))
	{
		handleUnknown(m);

		return;
	}

	stateInfo si;
	weaponInfo wi;
	itemInfo ii;

	bool gotItems,
		 gotEnergy;

	if (len >= 23)
		gotEnergy = true;
	else
		gotEnergy = false;

	if (len == 31)
		gotItems = true;
	else
		gotItems = false;

	BYTE direction = getByte(msg, 1);

	// calculate timestamp (straight from subspace)
	Uint32 loword = getShort(msg, 2);
	Uint32 timestamp = h->getHostTime() & 0x7FFFFFFF;

	if ((timestamp & 0x0000FFFF) >= loword)
	{
		timestamp &= 0xFFFF0000;
	}
	else
	{
		timestamp &= 0xFFFF0000;
		timestamp -= 0x00010000;
	}

	timestamp |= loword;	// fill in the low word

	// calculate transit time (straight from subspace)
	Sint32 transit_time = h->getHostTime() - timestamp;

	if ((transit_time < 0) || (transit_time > 30000))
	{
		transit_time = 0;
	}
	else if (transit_time > 4000)
	{
		transit_time = 15;
	}

	Uint16 x		= getShort(msg, 4);
	Sint16 yvel		= getShort(msg, 6);
	Uint16 ident	= getShort(msg, 8);
	Player *p		= h->getPlayer(ident);
	if (p == NULL) return;
	Sint16 xvel		= getShort(msg, 10);
	BYTE checksum	= getByte(msg, 12);
	si.n			= getByte(msg, 13);
	BYTE ping		= getByte(msg, 14);
	Uint16 y		= getShort(msg, 15);
	Uint16 bounty	= getShort(msg, 17);
	wi.n			= getShort(msg, 19);

	p->lastPositionUpdate = timestamp;

	if (x > 0x7fff) x = ~x + 1;	// Fix by Snrrrub
	if (y > 0x7fff) y = ~y + 1;

	msg[12] = 0;
	if (checksum != simpleChecksum(msg, 21))
	{
		h->logEvent("WARNING: Possible hacked subgame. Position checksum mismatch from %s", p->name);

		return;
	}

	if (wi.type != PROJ_None)
		++h->weaponCount;

//	if (p->lastPositionUpdate > timestamp) return;

	p->d		= direction;
	p->bounty	= bounty;

	if (gotEnergy)
		p->energy = getShort(msg, 21);
	else
		p->energy = 0;

	if (gotItems)
	{
		p->S2CLag	= getShort(msg, 23);
		p->timer	= getShort(msg, 25);
	}
	else
	{
		p->S2CLag	= 0;
		p->timer	= 0;
	}

	p->stealth	= si.stealth;
	p->cloak	= si.cloaked;
	p->xradar	= si.xradar;
	p->awarp	= si.awarp;
	p->ufo		= si.ufo;
	p->safety	= si.safety;
	p->flash	= si.flash;

	if (gotItems)
	{
		ii.n		= getLong(msg, 27);
		p->shields	= ii.shields;
		p->supers	= ii.supers;
		p->burst	= ii.burst;
		p->repel	= ii.repel;
		p->thor		= ii.thor;
		p->brick	= ii.brick;
		p->decoy	= ii.decoy;
		p->rocket	= ii.rocket;
		p->portal	= ii.portal;
	}

	// prediction
	p->move(x, y, xvel, yvel);
	p->move(transit_time);

	h->imports->talk(makePlayerMove(p));

	if (wi.type != PROJ_None)
		h->imports->talk(makePlayerWeapon(p, wi.n));

	if (h->follow)
	if (p == h->follow->item)
	{
		Player *Me = h->Me;
		if (Me)
		{
			Me->clone(p);

			if (Me->ship != SHIP_Spectator)
			{
				switch (wi.type)
				{
				case PROJ_Bullet:
				case PROJ_BBullet:
					if (!h->hasSysOp && h->settings.ships[Me->ship].MaxGuns == 0)
					{
						h->sendPosition(timestamp, false);
						return;
					}
					wi.level		= limit(wi.level, h->settings.ships[Me->ship].MaxGuns - 1);
					wi.fireType		= limit(wi.fireType, h->settings.ships[Me->ship].DoubleBarrel);
					if (!h->settings.pw.BouncingBullets) wi.type = PROJ_Bullet;
					break;
				case PROJ_Bomb:
				case PROJ_PBomb:
					if (!h->hasSysOp && h->settings.ships[Me->ship].MaxBombs == 0)
					{
						h->sendPosition(timestamp, false);
						return;
					}
					wi.level		= limit(wi.level, h->settings.ships[Me->ship].MaxBombs - 1);
					wi.shrapCount	= limit(wi.shrapCount, h->settings.ships[Me->ship].ShrapnelMax);
					wi.shrapLevel	= limit(wi.shrapLevel, h->settings.ships[Me->ship].MaxBombs - 1);
					wi.shrapBounce	= limit(wi.shrapBounce, !!h->settings.pw.BouncingBullets);
					wi.fireType		= limit(wi.fireType, !!h->settings.ships[Me->ship].MaxMines);
					break;
				case PROJ_Decoy:
					if (!h->hasSysOp && h->settings.ships[Me->ship].DecoyMax == 0)
					{
						h->sendPosition(timestamp, false);
						return;
					}
					break;
				case PROJ_Thor:
					if (!h->hasSysOp && h->settings.ships[Me->ship].ThorMax == 0)
					{
						h->sendPosition(timestamp, false);
						return;
					}
					break;
				case PROJ_Repel:
					if (!h->hasSysOp && h->settings.ships[Me->ship].RepelMax == 0)
					{
						h->sendPosition(timestamp, false);
						return;
					}
					break;
				case PROJ_Burst:
					if (!h->hasSysOp && h->settings.ships[Me->ship].BurstMax == 0)
					{
						h->sendPosition(timestamp, false);
						return;
					}
					break;
				};

				h->sendPosition(false, timestamp, wi.type, wi.level, wi.shrapBounce, wi.shrapLevel, wi.shrapCount, wi.fireType);
			}
		}
	}

//	h->logEvent("[%i] Got player/weapon position: %s (%i, %i) {%i}", timestamp, p->name, x, y, si.pack);
}

void __stdcall handlePlayerDeath(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Death green
		2		2		Killer ident
		4		2		Killee ident
		6		2		Bounty
		8		2		Number of flags gained by killing player (Snrrrub)
	*/

	if (len != 10)
	{
		handleUnknown(m);

		return;
	}

	Player *killer = h->getPlayer(getShort(msg, 2));
	Player *killee = h->getPlayer(getShort(msg, 4));
	Sint32 bounty = (Sint16)getShort(msg, 6);
	Uint16 flags = getShort(msg, 8);

	if (killer)
	{
		/*	This message is followed by a score update,
			because kills are not visible all the time.
			The killee & killer are assumed to have updated
			their own scores, so no score updates are sent.	*/
		if (killer->flagCount > 0)
		{	// Apparently bounty is only filtered server-side
			bounty *= h->settings.FlaggerKillMultiplier + 1;
		}

		killer->bounty += h->settings.BountyIncreaseForKill;

		++killer->score.wins;
		killer->score.killPoints += bounty;

		// Flag drop
		if (killer == h->Me)
		{
			//h->postRR(generateFlagDrop());
			// NEUT! FFS! Bots winning flag games is really lame.

			// Find ship the bot is not in.
			Ship_Types ship = SHIP_Warbird;
			if (h->Me->ship == SHIP_Warbird)
				ship = SHIP_Javelin;

			Ship_Types old = (Ship_Types)h->Me->ship;

			h->postRR(generateChangeShip(ship));
			h->postRR(generateChangeShip(old));
		}

		killer->flagCount += flags;
	}

	if (killee)
	{
		++killee->score.losses;

		if (killer)
		{
//			h->logEvent("%s(%i) killed by: %s {%i} [%i]", killee->name, bounty, killer->name, getByte(msg, 1), getShort(msg, 8));

			h->imports->talk(makePlayerDeath(killee, killer, bounty, flags));
		}

		killee->flagCount = 0;
	}
}

void __stdcall handleSpecPlayer(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	Either			Player joining spectator mode
		1		2		Player ident
	Or				Someone is requesting extra position information
		1		1		Boolean: send ExtraPositionInfo
	*/

	if (len == 2)
	{
		bool watched = (getByte(msg, 1) != 0);

		if (watched)
		{
			h->logEvent("Paranoid flag: Someone's requesting ExtraPositionInfo");
		}
		else
		{
			h->logEvent("Paranoid flag: Off");
		}

		h->paranoid = watched;
	}
	else if (len == 3)
	{
		Player *p = h->getPlayer(getShort(msg, 1));
		if (p == NULL) return;

//		h->logEvent("Player spectated: %s", p->name);

		Uint16 oldship = p->ship;
		Uint16 oldteam = p->team;

		p->ship = SHIP_Spectator;

		h->imports->talk(makePlayerSpec(p, oldteam, oldship));

		p->flagCount = 0;
	}
	else
	{
		handleUnknown(m);

		return;
	}
}

void __stdcall handlePlayerPosition(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Rotation
		2		2		Time stamp
		4		2		X pixels
		6		1		Ping
		7		1		Bounty
		8		1		Player ident
		9		1		Togglables
		10		2		Y velocity
		12		2		Y pixels
		14		2		X velocity
	Spectating with ExtraPositionData or *energy
		16		2		Energy
	Spectating with ExtraPositionData
		18		2		S2CLag
		20		2		Timer
		22		4		Item information
	*/

	if ((len != 16) && (len != 18) && (len != 26))
	{
		handleUnknown(m);

		return;
	}

	stateInfo si;
	itemInfo ii;

	bool gotItems,
		 gotEnergy;

	if (len >= 18)
		gotEnergy = true;
	else
		gotEnergy = false;

	if (len == 26)
		gotItems = true;
	else
		gotItems = false;

	BYTE direction = getByte(msg, 1);

	// calculate timestamp (straight from subspace)
	Uint32 loword = getShort(msg, 2);
	Uint32 timestamp = h->getHostTime() & 0x7FFFFFFF;

	if ((timestamp & 0x0000FFFF) >= loword)
	{
		timestamp &= 0xFFFF0000;
	}
	else
	{
		timestamp &= 0xFFFF0000;
		timestamp -= 0x00010000;
	}

	timestamp |= loword;	// fill in the low word

	// calculate transit time (straight from subspace)
	Sint32 transit_time = h->getHostTime() - timestamp;

	if ((transit_time < 0) || (transit_time > 30000))
	{
		transit_time = 0;
	}
	else if (transit_time > 4000)
	{
		transit_time = 15;
	}

	Uint16 x		= getShort(msg, 4);
	BYTE ping		= getByte(msg, 6);
	BYTE bounty		= getByte(msg, 7);
	Uint16 ident	= getByte(msg, 8);
	Player *p		= h->getPlayer(ident);
	if (p == NULL) return;
	si.n			= getByte(msg, 9);
	Sint16 yvel		= getShort(msg, 10);
	Uint16 y		= getShort(msg, 12);
	Sint16 xvel		= getShort(msg, 14);

	p->lastPositionUpdate = timestamp;

	if (x > 0x7fff) x = ~x + 1;	// Fix by Snrrrub
	if (y > 0x7fff) y = ~y + 1;

//	if (p->lastPositionUpdate > timestamp) return;

	p->d		= direction;
	p->bounty	= bounty;

	if (gotEnergy)
		p->energy = getShort(msg, 16);
	else
		p->energy = 0;

	if (gotItems)
	{
		p->S2CLag	= getShort(msg, 18);
		p->timer	= getShort(msg, 20);
	}
	else
	{
		p->S2CLag	= 0;
		p->timer	= 0;
	}

	p->stealth	= si.stealth;
	p->cloak	= si.cloaked;
	p->xradar	= si.xradar;
	p->awarp	= si.awarp;
	p->ufo		= si.ufo;
	p->safety	= si.safety;
	p->flash	= si.flash;

    if (gotItems)
	{
		ii.n		= getLong(msg, 22);
		p->shields	= ii.shields;
		p->supers	= ii.supers;
		p->burst	= ii.burst;
		p->repel	= ii.repel;
		p->thor		= ii.thor;
		p->brick	= ii.brick;
		p->decoy	= ii.decoy;
		p->rocket	= ii.rocket;
		p->portal	= ii.portal;
	}

	// prediction
	p->move(x, y, xvel, yvel);
	p->move(transit_time);

	h->imports->talk(makePlayerMove(p));

	if (h->follow)
	if (p == h->follow->item)
	{
		Player *Me = h->Me;

		if (Me)
		{
			Me->clone(p);

			if (Me->ship == SHIP_Spectator)
			{
				if (h->speccing) h->spectate(NULL);
			}
			else
			{
				h->sendPosition(timestamp, false);
			}
		}
	}

//	h->logEvent("[%i] Got player position: %s (%i, %i) {%i:%i}", timestamp, p->name, x, y, bounty, si.pack);
}

void __stdcall handleKoTHReset(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Adding KoTH timer
		2		4		Timer value
		6		2		Player ident
	*/

	if (len != 8)
	{
		handleUnknown(m);

		return;
	}

	bool adding = getByte(msg, 1) != 0;
	Uint32 timer = getLong(msg, 2);
	Uint16 ident = getShort(msg, 6);
	Player *p = h->getPlayer(ident);

	if (adding)
	{
		if (ident != UNASSIGNED)
		{
			// Add to one
			if (p == NULL) return;

//			h->logEvent("KoTH add: %s", p->name);

			p->koth = true;
		}
		else
		{
			// Add to all
//			h->logEvent("KoTH add: all");

			_listnode <Player> *parse = h->playerlist.head;

			while (parse)
			{
				p = parse->item;
				parse = parse->next;

				p->koth = true;
			}

			h->postRR(generateKoTHReset());
		}
	}
	else
	{
		if (ident != UNASSIGNED)
		{
			// Remove from one
			if (p == NULL) return;

//			h->logEvent("KoTH remove: %s", p->name);

			p->koth = false;
		}
		else
		{
			// Remove from all
//			h->logEvent("KoTH remove: all");

			_listnode <Player> *parse = h->playerlist.head;

			while (parse)
			{
				p = parse->item;
				parse = parse->next;

				p->koth = false;
			}
		}
	}
}

void __stdcall handleAddKoTH(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		4		Additional time
	*/

	if (len != 5)
	{
		handleUnknown(m);

		return;
	}

	Uint32 timerdiff = getLong(msg, 1);

	//h->logEvent("KoTH timer, plus %i", timerdiff);

	h->postRR(generateKoTHReset());
}

void __stdcall handleWatchDamage(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
		3		4		Timestamp
	The following are repeated until the end of the message
		7		2		Attacker ident
		9		2		Weapon info
		11		2		Last energy
		13		2		Damage
		15		1		?
	*/

	if ((len <= 7) || ((len - 7) % 9))
	{
		handleUnknown(m);

		return;
	}

	Uint16 pid = getShort(msg, 1);
	Player *p = h->getPlayer(pid);
	if (!p) return;
	Uint32 timestamp = getLong(msg, 3);

	for (Uint32 i = 7; i < len; i += 9)
	{
		Uint16 kid = getShort(msg, i);
		Player *k = h->getPlayer(kid);
		if (!k) continue;

		weaponInfo wi;
		wi.n = getShort(msg, i + 2);
		Uint16 energy = getShort(msg, i + 4);
		Uint16 damage = getShort(msg, i + 6);

		h->imports->talk(makeWatchDamage(p, k, wi.n, energy, damage));
	}
}


//////// Personal ship ////////

void __stdcall handleIdent(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		My new ident
	Continuum
		3		1		Boolean: ?
	*/

	if ((len != 3) && (len != 4))
	{
		handleUnknown(m);

		return;
	}

	Uint16 ident = getShort(msg, 1);

//	h->logEvent("Personal ident: %i", ident);

	h->me = ident;
}

void __stdcall handleSelfPrize(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Prize count
		3		2		Prize type
	*/

	if (len != 5)
	{
		handleUnknown(m);

		return;
	}

	Uint16 count = getShort(msg, 1);
	Uint16 prize = getShort(msg, 3);

//	h->logEvent("I have been prized");

	h->imports->talk(makeSelfPrize(prize, count));
}

void __stdcall handleSetKoTHTimer(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		4		New timer value
	*/

	if (len != 5)
	{
		handleUnknown(m);

		return;
	}

	Uint32 timer = getLong(msg, 1);

	//	h->logEvent("KoTH timer set to %i", timer);

	h->postRR(generateKoTHReset());
}

void __stdcall handleShipReset(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	*/

	if (len != 1)
	{
		handleUnknown(m);

		return;
	}

//	h->logEvent("Personal ship reset");

	Player *p = h->Me;
	if (p == NULL) return;

	p->awarp	= false;
	p->bounty	= 0;
	p->brick	= 0;
	p->burst	= 0;
	p->cloak	= false;
	p->decoy	= 0;
	p->energy	= 0;
	p->portal	= 0;
	p->repel	= 0;
	p->rocket	= 0;
	p->shields	= false;
	p->stealth	= false;
	p->supers	= false;
	p->thor		= 0;
	p->timer	= 0;
	p->ufo		= false;
	p->xradar	= false;

	h->imports->talk(makeSelfShipReset());
}

void __stdcall handleChangePosition(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		X tiles
		3		2		Y tiles
	*/

	if (len != 5)
	{
		handleUnknown(m);

		return;
	}

	Uint16 x = getShort(msg, 1);
	Uint16 y = getShort(msg, 3);

	if (!h->Me) return;
	h->Me->move(x << 4, y << 4);

	h->imports->talk(makePlayerMove(h->Me));

//	h->logEvent("Personal ship coordinates changed");
}

void __stdcall handleReceivedObject(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		?		?
	*/

	if (len != 0)
	{
		handleUnknown(m);

		return;
	}
}

void __stdcall handleDamageToggle(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Boolean: send damage info
	*/

	if (len != 2)
	{
		handleUnknown(m);

		return;
	}

	bool sendIt = (msg[1] != 0);
}

void __stdcall handleToggleUFO(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Boolean: available
	*/

	if (len != 2)
	{
		handleUnknown(m);

		return;
	}

	bool ufo = getByte(msg, 1) != 0;

//	h->logEvent("Personal UFO: %i", ufo);

	Player *me = h->Me;
	if (me == NULL) return;

	me->ufo = ufo;

	h->imports->talk(makeSelfUFO());
}


//////// Bricks ////////

void __stdcall handleBrickDrop(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	The following are repeated until the end of the message
		1		2		X1 tiles
		3		2		Y1 tiles
		5		2		X2 tiles
		7		2		Y2 tiles
		9		2		Team
		11		2		Brick ident (sent more than once)
		13		4		Timestamp
	*/

	if ((len - 1) & 15)
	{
		handleUnknown(m);

		return;
	}

	for (Uint32 i = 1; i < len; i += 16)
	{
		Uint16 ident = getShort(msg, i + 10);
		if (h->brickExists(ident))
			continue;

		Sint16 x1 = getShort(msg, i + 0);
		Sint16 y1 = getShort(msg, i + 2);
		Sint16 x2 = getShort(msg, i + 4);
		Sint16 y2 = getShort(msg, i + 6);
		Uint16 team = getShort(msg, i + 8);
		Player *p = h->Me;
		if (p == NULL) continue;
		Uint32 timestamp = h->getLocalTime(getLong(msg, i + 12));

		h->logEvent("Brick dropped on team #%i at %i", team, timestamp);

		// Trace brick blocks
		Sint16 mx = (Sint16)sgn(x2 - x1);
		Sint16 my = (Sint16)sgn(y2 - y1);
		Sint16 x = x1, y = y1;

		BYTE brickIcon = (team == p->team) ? ssbTeamBrick : ssbEnemyBrick;
		Brick *b;

		if (mx)
		{
			while (x != x2)
			{
				x += mx;

				b = new Brick;
				b->x = x;
				b->y = y;
				b->time = timestamp + h->settings.BrickTime;
				b->team = team;
				b->ident = ident;
				h->brickList.append(b);
			}
		}
		else
		{
			while (y != y2)
			{
				y += my;

				b = new Brick;
				b->x = x;
				b->y = y;
				b->time = timestamp + h->settings.BrickTime;
				b->team = team;
				b->ident = ident;
				h->brickList.append(b);
			}
		}

		h->imports->talk(makeBrickDropped(x1, y1, x2, y2, team));
	}
}


//////// Synchronization/Security ////////

void __stdcall handleSynchronization(hostMessage *m)
{	DEFLATE_CLASS
	// Fields contributed by Kavar!'s research

	/*	Field	Length	Description
		0		1		Type byte
		1		4		Green seed
		5		4		Door seed
		9		4		Timestamp (Kavar! mistook it for a seed)
		13		4		Checksum generator key
	*/

	if (len != 17)
	{
		handleUnknown(m);

		return;
	}

	Uint32 greenSeed   = getLong(msg, 1);
	Uint32 doorSeed    = getLong(msg, 5);
	Uint32 timestamp   = h->getLocalTime(getLong(msg, 9));
	Uint32 checksumKey = getLong(msg, 13);

	h->logEvent("Got synchronization packet [%i][%i]", timestamp, checksumKey);

//	h->ps.randomize(greenSeed, timestamp);

	// fast?
	h->S2CFastCurrent = (Uint16)h->msgRecv;

	h->S2CSlowTotal += 0;
	h->S2CFastTotal += (Uint16)h->msgRecv;

	if ((checksumKey != 0) && (h->gotMap))
	{
		h->postRR(generateSecurityChecksum(generateParameterChecksum(checksumKey, (Uint32*)&h->settings),
										   generateEXEChecksum(checksumKey),
										   generateLevelChecksum(checksumKey, (char*)h->map),
										   (Uint16)h->msgRecv,
										   (Uint16)h->syncPing,
										   (Uint16)h->avgPing,
										   (Uint16)h->lowPing,
										   (Uint16)h->highPing,
										   h->weaponCount,
										   h->S2CSlowCurrent,
										   h->S2CFastCurrent,
										   h->S2CSlowTotal,
										   h->S2CFastTotal,
										   false));
	}

	h->S2CFastCurrent = 0;
	h->S2CSlowCurrent = 0;

	// Start sending position updates (game)
	if (!h->position)
	{
		h->position = true;
		h->lastPosition = 0;
	}

	// Start sending time synchronization requests (core)
	h->inArena = true;
}


//////// Powerball ////////

void __stdcall handleSoccerGoal(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Team
		3		4		Points
	*/

	if (len != 7)
	{
		handleUnknown(m);

		return;
	}

	Uint16 team = getShort(msg, 1);
	Uint32 points = getLong(msg, 3);

	_listnode <Player> *parse = h->playerlist.head;

	while (parse)
	{
		Player *p = parse->item;
		parse = parse->next;

		if (p->team == team)
			p->score.flagPoints += points;
	}

	h->imports->talk(makeSoccerGoal(team, points));

	h->logEvent("Team #%i made a goal for %i points", team, points);
}

void __stdcall handleBallPosition(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Powerball ident
		2		2		X pixels
		4		2		Y pixels
		6		2		X velocity
		8		2		Y velocity
		10		2		Owner ident
		12		4		Timestamp
	*/

	if (len != 16)
	{
		handleUnknown(m);

		return;
	}

	BYTE ball = getByte(msg, 1);
	Uint16 x = getShort(msg, 2);
	Uint16 y = getShort(msg, 4);
	Sint16 xvel = getShort(msg, 6);
	Sint16 yvel = getShort(msg, 8);
	Uint16 carrier = getShort(msg, 10);
	Player *p = h->getPlayer(carrier);
	Uint32 hosttime = getLong(msg, 12);
	Uint32 timestamp = h->getLocalTime(hosttime);

	PBall *pb = h->findBall(ball);

	if (hosttime == 0)
	{	// Carried
		if (p)
		{	// Someone is carrying it
//			h->logEvent("Ball #%i @ (%i->%i, %i->%i) carried by %s", ball, x, xvel, y, yvel, p->name);

			pb->carrier = p->ident;
			pb->team = p->team;
		}
		else
		{
			pb->carrier = UNASSIGNED;
			pb->team = UNASSIGNED;
		}
	}
	else if (pb->hosttime != hosttime)
	{	// Uncarried
//		h->logEvent("Ball #%i @ (%i->%i, %i->%i)", ball, x, xvel, y, yvel);

		pb->carrier = UNASSIGNED;
		pb->team = UNASSIGNED;
	}

	pb->x = x;
	pb->y = y;
	if (hosttime == 0)
	{
		pb->xvel = 0;
		pb->yvel = 0;
	}
	else
	{
		pb->xvel = xvel;
		pb->yvel = yvel;
	}
	pb->time = timestamp;
	pb->hosttime = hosttime;
	pb->lastrecv = getTime();

	h->imports->talk(makeBallMove(pb));
}


//////// Login ////////

void __stdcall handleMapInfo(hostMessage *m)
{	DEFLATE_CLASS
	// This massive task takes ~200ms on my P200 laptop

	/*	Field	Length	Description
		0		1		Type byte
		1		16		Map name
		17		4		Map checksum
	The map size is optional.  It's sent in subgame if client version is not 134,
	and ASSS may potentially send it anyway.
		21		4		Map cmplen
	The following are optionally repeated until the end of the message
		25		16		LVZ name
		41		4		LVZ checksum
		45		4		LVZ cmplen
	*/

	if (len != 21) // not SubSpace
	{
		if ((len < 25) || ((len - 1) % 24)) // not Continuum either
		{
			handleUnknown(m);

			return;
		}
		else
		{
			for (Uint32 i = 25; i < len; i += 24)
			{
				char *filename = msg + i;
				Uint32 checksum = getLong(msg + i, 16);
				Uint32 cmplen = getLong(msg + i, 20);

//				h->logEvent("LVZ: %s [chk:%i cmplen:%i]", filename, checksum, cmplen);
			}
		}
	}

	char *filename = msg + 1;
	Uint32 checksum = getLong(msg, 17);
	String name = "lvl/";
	name += filename;

	if (checksum != getFileChecksum(name.msg, h->dictionary))
	{
		/* We cannot calculate the checksum without Ctm encryption, so we always assume we do not have the important stuff. */

		if (len >= 25)
			h->logEvent("Downloading map file: %s [chk:%i cmplen:%i]", name.msg, checksum, getLong(msg, 21));
		else
			h->logEvent("Downloading map file: %s", name.msg);

		h->postRR(generateLevelRequest());
	}
	else
	{	// This block takes ~100ms on my desktop computer
		h->logEvent("Reading map file: %s", name.msg);

		ifstream file(name.msg, ios::binary);
		if (!file)
		{
			h->logEvent("Unable to load map file.");
			return;
		}

		file.seekg (0, ios::end);
		Uint32 length = file.tellg();
		file.seekg (0, ios::beg);
		char *buffer = new char[length];
		file.read(buffer, length);

		convertFileToMatrix(buffer, (char*)h->map, length);

		h->changeCoordinates();

		delete []buffer;

		h->gotMap = true;

		if (h->gotTurf)
		{
			if (!h->loadedFlags)
			{
				h->loadTurfFlags();
				h->logEvent("Loaded turf flags from map data.");

				h->loadedFlags = true;
			}
		}

		h->imports->talk(makeMapLoaded());
	}
}

void __stdcall handleMapFile(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		16		Map name
		17		...		Compressed map
	*/

	if (len <= 17)
	{
		handleUnknown(m);

		return;
	}

	char *name = msg + 1;
	char *compressed = msg + 17;
	Uint32 length = len - 17;

	if (name[15])
	{
		h->logEvent("WARNING: Unterminated downloaded-file name ignored.");

		return;
	}

	for (Uint16 i = 0; i < 16; ++i)
	{
		char c = name[i];

		if (c == 0)
			break;

		switch (c)
		{
		case '/':
		case '\\':
			h->logEvent("WARNING: Invalid downloaded-file name path ignored. (%s)", name);
			return;
		default:
			break;
		};

		if ((c < ' ') || (c > '~'))
		{
			h->logEvent("WARNING: Invalid downloaded-file name chars ignored.");
			return;
		}
	}

	String fname = "lvl/";
	fname += name;

	if (decompress_to_file(fname.msg, compressed, length))
	{
		h->logEvent("Map download complete. Reading: %s", fname.msg);

		ifstream file(fname.msg, ios::binary);
		if (!file)
		{
			h->logEvent("Unable to read map file.");
			return;
		}

		file.seekg (0, ios::end);
		length = file.tellg();
		file.seekg (0, ios::beg);
		char *buffer = new char[length];
		file.read(buffer, length);

		convertFileToMatrix(buffer, (char*)h->map, length);

		h->changeCoordinates();

		delete []buffer;

		h->gotMap = true;

		h->imports->talk(makeMapLoaded());
	}
	else
		h->logEvent("Unable to decompress map file!");
}

void __stdcall handleCustomMessage(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		...\0	Message
	*/

	if (len <= 1)
	{
		handleUnknown(m);

		return;
	}

	char *text = msg + 1;

	if ((len < 2) || text[len - 2])
	{
		h->logEvent("WARNING: Unterminated custom message ignored.");
		h->logIncoming(msg, len);

		return;
	}

	h->logEvent("Custom response: %s", text);
}

void __stdcall handleVersionCheck(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Version number
		3		4		Checksum
	*/

	if (len != 7)
	{
		handleUnknown(m);

		return;
	}

	Uint16 version	= getShort(msg, 1);
	Uint32 checksum = getLong(msg, 3);

	h->logEvent("Remote Ctm version %i [chk:%i]", version, checksum);
}

void __stdcall handleObjectToggle(hostMessage *m)
{	DEFLATE_CLASS
	// Contributed by SOS
	// Ctm .37 handled this wrong, only one object could be toggled at a time

	/*	Field	Length	Description
		0		1		Type byte
	The following are repeated until the end of the message
		1		2		Object Info
	*/

	if ((len & 1) != 1)
	{
		handleUnknown(m);

		return;
	}

	objectInfo obj;
	Uint32 index = 1;

	while (index < len)
	{
		obj.n = getShort(msg, index);

		h->imports->talk(makeObjectToggled(obj.n));

		if (obj.disabled)
		{
			h->logEvent("Object number %i disabled", obj.id);
		}
		else
		{
			h->logEvent("Object number %i enabled", obj.id);
		}

		index += 2;
	}
}

void __stdcall handleArenaList(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
	The following are repeated until the end of the message
		1		...\0	Arena name
		?		2		Arena population
	*/

	if (len <= 3)
	{
		handleUnknown(m);

		return;
	}

	Uint32 index = 1,
		   length;

	while ((index + 3) <= len)
	{
		char *name = msg + index;
		length = STRLEN(name);

		Uint16 population = getShort(msg, index + length + 1);

		if (population > 0x7fff)
		{
			population = ~population + 1;

			h->logEvent("Arena: %s [%i] <-- I am here", name, population);

			if (index + length + 6 > len)
				h->imports->talk(makeArenaListEnd(name, true, population));
			else
				h->imports->talk(makeArenaListEntry(name, true, population));
		}
		else
		{
			h->logEvent("Arena: %s [%i]", name, population);

			if (index + length + 6 > len)
				h->imports->talk(makeArenaListEnd(name, false, population));
			else
				h->imports->talk(makeArenaListEntry(name, false, population));
		}

		index += length + 3;
	}

	if (index > len)
	{
		h->logEvent("WARNING: Malformed arena list ignored");
		h->logIncoming(msg, len);
	}
}

void __stdcall handleArenaSettings(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1428	Settings buffer
	The type byte is included in checksums
	*/

	if (len != 1428)
	{
		handleUnknown(m);

		return;
	}

	h->logEvent("Re-reading arena settings...");
/*
	// Watch changes
	if (h->settings.Version == 15)
	{
		char *settings = (char*)&h->settings;

		for (Uint32 i = 0; i < 1428; ++i)
		{
			if (settings[i] != msg[i])
			{
				String s;
				s += "Change at ";
				s += i;
				s += ".  Old:";
				s += settings[i];
				s += ".  New:";
				s += msg[i];
				h->logEvent("%s", s);
				h->sendPublic(SND_Inconceivable, s.msg);
			}
		}

		String s;
		s += "First four bytes of settings: ";
		s += settings[0];
		s += ", ";
		s += settings[1];
		s += ", ";
		s += settings[2];
		s += ", ";
		s += settings[3];
		s += ".";
		h->sendPublic(SND_Inconceivable, s.msg);
	}
/**/
	// Update host setting pool
	arenaSettings *settings = &h->settings;
	memcpy(settings, msg, 1428);

	// Update prize system parameters
//	h->ps.setSettings(&h->settings, h->map);

	// Update goal mode
	h->changeGoalMode();

	h->imports->talk(makeArenaSettings(settings));

//	h->logEvent("...Finished reading settings.");
}

void __stdcall handlePasswordResponse(hostMessage *m)
{	DEFLATE_CLASS

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Accept response meaning
		2		4		Server version
		6		4		<unused>
		10		4		EXE checksum
		14		4		<unused>
		18		1		<unused>
		19		1		Boolean: Request registration form
		20		4		SSEXE cksum with seed of zero; if this and EXE checksum are -1, bestows supervisor privs to the client
		24		4		News checksum (0 = no news file)
		28		4		<unused>
		32		4		<unused>
	*/

	if (len != 36)
	{
		handleUnknown(m);

		return;
	}

	BYTE meaning = getByte(msg, 1);
	Uint32 version = getLong(msg, 2);
	Uint32 EXEChecksum = getLong(msg, 10);
//	Uint32 LocalEXEChecksum = getFileChecksum("subspace.bin", h->dictionary);
	Uint32 LocalEXEChecksum = 0xF1429CE8;
	bool requestRegForm = (getByte(msg, 19) != 0);
	Uint32 newsChecksum = getLong(msg, 24);
	Uint32 unk6 = getLong(msg, 6);
	Uint32 unk14 = getLong(msg, 14);
	bool unk18 = (getByte(msg, 18) != 0);
	Uint32 SS_EXE_cksum_0 = getLong(msg, 20);
	Uint32 unk28 = getLong(msg, 28);
	Uint32 unk32 = getLong(msg, 32);
	BOT_INFO *botInfo = &h->botInfo;

	if (requestRegForm)
	{
		h->postRR(generateRegForm(botInfo->realName,
								  botInfo->email,
								  botInfo->city,
								  botInfo->state,
								  botInfo->sex,
								  botInfo->age,
								  botInfo->playAtHome,
								  botInfo->playAtWork,
								  botInfo->playAtSchool,
								  botInfo->processor,
								  botInfo->regName,
								  botInfo->regOrg));

		h->logEvent("Sending registration form");
	}

//	if (LocalEXEChecksum == -1)
//		LocalEXEChecksum = 0xF1429CE8;

	if (SS_EXE_cksum_0 != -1 && SS_EXE_cksum_0 != generateEXEChecksum(0))
	{
		h->logEvent("WARNING: SS EXE checksum mismatch on login.  May require privileged access to remain connected.");
	}

	if (EXEChecksum == -1 && SS_EXE_cksum_0 == -1)
	{
		h->logEvent("EXE checksum and (random) server checksum were sent: I have VIP access in this zone!  ...$");
	}
	else if (EXEChecksum == 0)
	{
		h->logEvent("Problem found with server: Server doesn't have a copy of subspace.exe so it sent me a zero checksum.");
	}
	else if (EXEChecksum != LocalEXEChecksum)
	{
		h->logEvent("Possible virus: remote EXE checksum mismatch (local:%i) (remote:%i)", LocalEXEChecksum, EXEChecksum);
		h->logEvent("Privileged access is required to remain connected to this zone.");
//		h->logEvent(" FIELDS: 6:%i 14:%i 18:%i 20:%i 28:%i 32:%i", unk6, unk14, unk18, unk20, unk28, unk32);
	}

	if ((!h->downloadingNews) && (newsChecksum != 0) && (newsChecksum != getFileChecksum("get/news.txt", h->dictionary)))
	{	// News checksum will be invalid if EXE checksum is invalid.
		h->postRR(generateNewsRequest());

		h->logEvent("Downloading latest news.txt [%i]", newsChecksum);

		h->downloadingNews = true;
	}

	--h->numTries;

	switch (meaning)
	{
	case LOG_Continue:			h->logEvent("Password accepted for %s.", h->botInfo.name);
		{
			h->activateGameProtocol();
			h->changeArena(botInfo->initialArena);

			h->logLogin();
		}
		break;
	case LOG_NewUser:			h->logEvent("Unknown player, continue as new user?");
		{
			if (h->numTries <= 0)
			{
				h->logEvent("Number of login tries[of %i] exhausted for %s. Bye!", h->botInfo.db->maxTries, h->botInfo.name);
				h->disconnect(true);
				break;
			}
			else
				h->logEvent("Creating account for %s", h->botInfo.name);

			clientMessage *cm;
			if (h->botInfo.db->forceContinuum)
				cm = generateCtmPassword(true, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, CONTINUUM_VERSION, CONN_UnknownNotRAS);
			else
				cm = generatePassword(true, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, SUBSPACE_VERSION, CONN_UnknownNotRAS);

			h->postRR(cm);
		}
		break;
	case LOG_InvalidPassword:	h->logEvent("Invalid password for specified user.  The name you have chosen is probably in use by another player, try picking a different name.");
			h->disconnect(true);
		break;
	case LOG_FullArena:			h->logEvent("This arena is currently full, try again later.");
		{
			if (h->numTries <= 0)
			{
				h->logEvent("Number of login tries[of %i] exhausted for %s. Bye!", h->botInfo.db->maxTries, h->botInfo.name);
				h->disconnect(true);
				break;
			}
			else
				h->logEvent("Sending password for %s", h->botInfo.name);

			clientMessage *cm;
			if (h->botInfo.db->forceContinuum)
				cm = generateCtmPassword(false, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, CONTINUUM_VERSION, CONN_UnknownNotRAS);
			else
				cm = generatePassword(false, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, SUBSPACE_VERSION, CONN_UnknownNotRAS);

			h->postRR(cm);
		}
		break;
	case LOG_LockedOut:			h->logEvent("You have been locked out of SubSpace, for more information inquire on Web BBS.");
			h->disconnect(true);
		break;
	case LOG_NoPermission:		h->logEvent("You do not have permission to play in this arena(1), see Web Site for more information.");
			h->disconnect(true);
		break;
	case LOG_SpectateOnly:		h->logEvent("You only have permission to spectate in this arena.");
		{
			h->activateGameProtocol();
			h->changeArena(botInfo->initialArena);

			h->logLogin();
		}
		break;
	case LOG_TooManyPoints:		h->logEvent("You have too many points to play in this arena, please choose another arena.");
			h->disconnect(true);
		break;
	case LOG_SlowConnection:	h->logEvent("Your connection appears to be too slow to play in this arena.");
			h->disconnect(true);
		break;
	case LOG_NoPermission2:		h->logEvent("You do not have permission to play in this arena(2), see Web Site for more information.");
			h->disconnect(true);
		break;
	case LOG_NoNewConnections:	h->logEvent("The server is currently not accepting new connections.");
		{
			if (h->numTries <= 0)
			{
				h->logEvent("Number of login tries[of %i] exhausted for %s. Bye!", h->botInfo.db->maxTries, h->botInfo.name);
				h->disconnect(true);
				break;
			}
			else
				h->logEvent("Sending password for %s", h->botInfo.name);

			clientMessage *cm;
			if (h->botInfo.db->forceContinuum)
				cm = generateCtmPassword(false, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, CONTINUUM_VERSION, CONN_UnknownNotRAS);
			else
				cm = generatePassword(false, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, SUBSPACE_VERSION, CONN_UnknownNotRAS);

			h->postRR(cm);
		}
		break;
	case LOG_InvalidName:		h->logEvent("Invalid user name entered, please pick a different name.");
			h->disconnect(true);
		break;
	case LOG_ObsceneName:		h->logEvent("Possibly offensive user name entered, please pick a different name.");
			h->disconnect(true);
		break;
	case LOG_BillerDown:		h->logEvent("NOTICE: Server difficulties; this zone is currently not keeping track of scores.  Your original score will be available later.  However, you are free to play in the zone until we resolve this problem.");
								h->logEvent("WARNING: Disabling operator accounts without passwords until biller is restored.");
		{
			h->billerOnline = false;
			h->activateGameProtocol();
			h->changeArena(botInfo->initialArena);

			h->logLogin();
		}
		break;
	case LOG_BusyProcessing:	h->logEvent("The server is currently busy processing other login requests, please try again in a few moments.");
		{
			if (h->numTries <= 0)
			{
				h->logEvent("Number of login tries[of %i] exhausted for %s. Bye!", h->botInfo.db->maxTries, h->botInfo.name);
				h->disconnect(true);
				break;
			}
			else
				h->logEvent("Sending password for %s", h->botInfo.name);

			clientMessage *cm;
			if (h->botInfo.db->forceContinuum)
				cm = generateCtmPassword(false, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, CONTINUUM_VERSION, CONN_UnknownNotRAS);
			else
				cm = generatePassword(false, botInfo->name, botInfo->password, botInfo->machineID, botInfo->timeZoneBias, botInfo->permissionID, SUBSPACE_VERSION, CONN_UnknownNotRAS);

			h->postRR(cm);
		}
		break;
	case LOG_ExperiencedOnly:	h->logEvent("This zone is restricted to experienced players only (ie. certain number of game-hours logged).");
			h->disconnect(true);
		break;
	case LOG_UsingDemoVersion:	h->logEvent("You are currently using the demo version.  Your name and score will not be kept track of.");
			h->disconnect(true);
		break;
	case LOG_TooManyDemos:		h->logEvent("This arena is currently has(sic) the maximum Demo players allowed, try again later.");
			h->disconnect(true);
		break;
	case LOG_ClosedToDemos:		h->logEvent("This arena is closed to Demo players.");
			h->disconnect(true);
		break;
	default:					h->logEvent("Unknown response type, please go to Web site for more information and to obtain latest version of the program.");
			h->disconnect(true);
	};
}


//////// C2S protocol ////////

clientMessage *generateViolation		(Security_Violations code)
{	clientMessage *ret = new clientMessage(2);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Violation code
	*/

	msg[0] = 0x1B;
	msg[1] = code;

	return ret;
}

clientMessage *generateChangeShip		(Ship_Types ship)
{	clientMessage *ret = new clientMessage(2);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Ship type
	*/

	msg[0] = 0x18;
	msg[1] = ship;

	return ret;
}

clientMessage *generateSpectate			(Uint16 player)
{	clientMessage *ret = new clientMessage(3);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
	*/

	msg[0] = 0x08;
	*(Uint16*)&msg[1] = player;

	return ret;
}

clientMessage *generateChangeTeam		(Uint16 team)
{	clientMessage *ret = new clientMessage(3);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Team
	*/

	msg[0] = 0x0F;
	*(Uint16*)&msg[1] = team;

	return ret;
}

clientMessage *generateChangeBanner		(BYTE *buffer)
{	clientMessage *ret = new clientMessage(97);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		96		Banner
	*/

	msg[0] = 0x19;
	memcpy(msg + 1, buffer, 96);

	return ret;
}

clientMessage *generateDeath			(Uint16 player, Uint16 bounty)
{	clientMessage *ret = new clientMessage(5);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
		3		2		Bounty
	*/

	msg[0] = 0x05;
	*(Uint16*)&msg[1] = player;
	*(Uint16*)&msg[3] = bounty;

	return ret;
}

clientMessage *generateChat				(Chat_Modes type, Chat_SoundBytes soundcode, Uint16 player, char *text)
{	Uint32 len = limit(STRLEN(text), 250);
	clientMessage *ret = new clientMessage(6 + len);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Chat type
		2		1		Soundcode
		3		2		Player ident
		5		...\0	Message
	*/

	msg[0] = 0x06;
	msg[1] = type;
	msg[2] = soundcode;
	*(Uint16*)&msg[3] = player;
	strncpy(msg + 5, text, len + 1);

	return ret;
}

clientMessage *generateRegForm			(char *name, char *email, char *city, char *state, RegForm_Sex sex, BYTE age, bool playAtHome, bool playAtWork, bool playAtSchool, Uint32 processor, char *regName, char *regOrg)
{	clientMessage *ret = new clientMessage(766);
	if (ret == NULL) return NULL;
	ret->clear();
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		32		Real name
		33		64		Email
		97		32		City
		129		24		State
		153		1		Sex('M'/'F')
		154		1		Age
	Connecting from...
		155		1		Home
		156		1		Work
		157		1		School
	System information
		158		4		Processor type (586)
		162		2		?
		164		2		?
	Windows registration information (SSC RegName ban)
		166		40		Real name
		206		40		Organization
	Windows NT-based OS's do not send any hardware information (DreamSpec HardwareID ban)
		246		40		System\CurrentControlSet\Services\Class\Display\0000
		286		40		System\CurrentControlSet\Services\Class\Monitor\0000
		326		40		System\CurrentControlSet\Services\Class\Modem\0000
		366		40		System\CurrentControlSet\Services\Class\Modem\0001
		406		40		System\CurrentControlSet\Services\Class\Mouse\0000
		446		40		System\CurrentControlSet\Services\Class\Net\0000
		486		40		System\CurrentControlSet\Services\Class\Net\0001
		526		40		System\CurrentControlSet\Services\Class\Printer\0000
		566		40		System\CurrentControlSet\Services\Class\MEDIA\0000
		606		40		System\CurrentControlSet\Services\Class\MEDIA\0001
		646		40		System\CurrentControlSet\Services\Class\MEDIA\0002
		686		40		System\CurrentControlSet\Services\Class\MEDIA\0003
		726		40		System\CurrentControlSet\Services\Class\MEDIA\0004
	*/

	msg[0] = 0x17;
	strncpy(msg + 1, name, 32);
	strncpy(msg + 33, email, 64);
	strncpy(msg + 97, city, 32);
	strncpy(msg + 129, state, 24);
	msg[153] = sex;
	msg[154] = age;
	msg[155] = playAtHome ? 1 : 0;
	msg[156] = playAtWork ? 1 : 0;
	msg[157] = playAtSchool ? 1 : 0;
	*(Uint32*)&msg[158] = processor;
	*(Uint16*)&msg[162] = 0xC000;	// magic number 1, i've checked the subspace API imports
	*(Uint16*)&msg[164] = 2036;		// magic number 2, nothing matches AFAIK =/
	strncpy(msg + 166, regName, 40);
	strncpy(msg + 206, regOrg, 40);

	// I didn't cache the hardware stuff, it's a pain in the butt.

	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Display\\0000", "DriverDesc", msg + 246);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Monitor\\0000", "DriverDesc", msg + 286);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Modem\\0000", "DriverDesc", msg + 326);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Modem\\0001", "DriverDesc", msg + 366);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Mouse\\0000", "DriverDesc", msg + 406);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Net\\0000", "DriverDesc", msg + 446);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Net\\0001", "DriverDesc", msg + 486);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\Printer\\0000", "DriverDesc", msg + 526);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0000", "DriverDesc", msg + 566);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0001", "DriverDesc", msg + 606);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0002", "DriverDesc", msg + 646);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0003", "DriverDesc", msg + 686);
	getServiceString(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\MEDIA\\0004", "DriverDesc", msg + 726);

	return ret;
}

clientMessage *generateKoTHReset		()
{	clientMessage *ret = new clientMessage(1);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	*/

	msg[0] = 0x1E;

	return ret;
}

clientMessage *generateSecurityChecksum	(Uint32 parameterChecksum, Uint32 EXEChecksum, Uint32 levelChecksum, Uint16 S2CRelOut, Uint16 ping, Uint16 avgPing, Uint16 lowPing, Uint16 highPing, Uint32 weaponCount, Uint16 S2CSlowCurrent, Uint16 S2CFastCurrent, Uint16 S2CSlowTotal, Uint16 S2CFastTotal, bool slowFrame)
{	clientMessage *ret = new clientMessage(40);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		4		Weapon count
		5		4		Parameter checksum
		9		4		EXE checksum
		13		4		Level checksum
		17		4		S2CSlowTotal
		21		4		S2CFastTotal
		25		2		S2CSlowCurrent
		27		2		S2CFastCurrent
		29		2		? S2CRelOut
		31		2		Ping
		33		2		Average ping
		35		2		Low ping
		37		2		High ping
		39		1		Boolean: Slow frame detected
	*/

	msg[0] = 0x1A;
	*(Uint32*)&msg[1] = weaponCount;
	*(Uint32*)&msg[5] = parameterChecksum;
	*(Uint32*)&msg[9] = EXEChecksum;
	*(Uint32*)&msg[13] = levelChecksum;
	*(Uint32*)&msg[17] = S2CSlowTotal;
	*(Uint32*)&msg[21] = S2CFastTotal;
	*(Uint16*)&msg[25] = S2CSlowCurrent;
	*(Uint16*)&msg[27] = S2CFastCurrent;
	*(Uint16*)&msg[29] = S2CRelOut;
	*(Uint16*)&msg[31] = ping;
	*(Uint16*)&msg[33] = avgPing;
	*(Uint16*)&msg[35] = lowPing;
	*(Uint16*)&msg[37] = highPing;
	msg[39] = slowFrame ? 1 : 0;

	return ret;
}

clientMessage *generatePassword			(bool newUser, char *name, char *pass, Uint32 machineID, Uint16 timezoneBias, Uint32 permissionID, Uint16 clientVersion, BYTE connectType)
{	clientMessage *ret = new clientMessage(101);
	if (ret == NULL) return NULL;
	ret->clear();
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Boolean: New user
		2		32		Name
		34		32		Password
		66		4		Machine ident
		70		1		ConnectType (*info)
		71		2		Timezone bias
		73		2		?
		75		2		Client version
		77		4		444 : set if anims.dll is present, else zero (anims.dll version)
		81		4		555 : set if anims.dll is present, else zero (anims.dll version)
		85		4		Permission ident
		89		12		reserved, set to zero
	*/

	msg[0] = 0x09;
	msg[1] = newUser ? 1 : 0;
	strncpy(msg + 2, name, 32);
	strncpy(msg + 34, pass, 32);
	*(Uint32*)&msg[66] = machineID;
	msg[70]	= connectType;
	*(Uint16*)&msg[71] = timezoneBias;
	*(Uint16*)&msg[73] = 0;
	*(Uint16*)&msg[75] = clientVersion;
	*(Uint32*)&msg[77] = 444;
	*(Uint32*)&msg[81] = 555;
	*(Uint32*)&msg[85] = permissionID;

	return ret;
}

clientMessage *generateCtmPassword		(bool newUser, char *name, char *pass, Uint32 machineID, Uint16 timezoneBias, Uint32 permissionID, Uint16 clientVersion, BYTE connectType)
{	clientMessage *ret = new clientMessage(165);
	if (ret == NULL) return NULL;
	ret->clear();
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Boolean: New user
		2		32		Name
		34		32		Password
		66		4		Machine ident
		70		1		ConnectType (*info)
		71		2		Timezone bias
		73		2		?
		75		2		Client version
		77		4		444
		81		4		555
		85		5*16	? capture
	*/

	const Uint8 capture[] = {
		0x00, 0x00, 0x00, 0x00,  0x01, 0x00, 0x00, 0x7F,  0xB4, 0x07, 0xE0, 0xF2,  0xF5, 0xA5, 0x00, 0x00,
		0xAC, 0xDF, 0xF5, 0x91,  0x7A, 0xE2, 0x55, 0xD9,  0x00, 0x00, 0x00, 0x00,  0xA0, 0x54, 0xD9, 0x57,
		0x49, 0x6A, 0xDC, 0xD6,  0x8C, 0x8F, 0xE0, 0xDC,  0x2B, 0xF0, 0x07, 0x61,  0x60, 0x4F, 0xB4, 0x62,
		0x87, 0x52, 0xCF, 0x6E,  0x5D, 0xB9, 0x30, 0x3D,  0x3D, 0x3D, 0x01, 0x1B,  0x89, 0x19, 0x7C, 0x70,
		0x8F, 0x5C, 0xDF, 0x37,  0x0D, 0x9A, 0x0C, 0x86,  0x72, 0x86, 0x96, 0x8C,  0x94, 0x86, 0x4D, 0x66
	};

	msg[0] = 0x24;
	msg[1] = newUser ? 1 : 0;
	strncpy(msg + 2, name, 32);
	strncpy(msg + 34, pass, 32);
	*(Uint32*)&msg[66] = machineID;
	msg[70]	= connectType;
	*(Uint16*)&msg[71] = timezoneBias;
	*(Uint16*)&msg[73] = 0;
	*(Uint16*)&msg[75] = clientVersion;
	*(Uint32*)&msg[77] = 444;
	*(Uint32*)&msg[81] = 555;
	memcpy(msg+85, capture, 5*16);

	return ret;
}

clientMessage *generatePowerballRequest	(Uint32 timestamp, BYTE ball)
{	clientMessage *ret = new clientMessage(6);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Ball ident
		2		4		Timestamp
	*/

	msg[0] = 0x20;
	msg[1] = ball;
	*(Uint32*)&msg[2] = timestamp;

	return ret;
}

clientMessage *generatePowerballUpdate	(Uint32 timestamp, BYTE ball, Uint16 x, Uint16 y, Uint16 xvelocity, Uint16 yvelocity, Uint16 owner)
{	clientMessage *ret = new clientMessage(16);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Ball ident
		2		2		X pixels
		4		2		Y pixels
		6		2		X velocity
		8		2		Y velocity
		10		2		Owner ident
		12		4		Timestamp
	*/

	msg[0] = 0x1F;
	msg[1] = ball;
	*(Uint16*)&msg[2] = x;
	*(Uint16*)&msg[4] = y;
	*(Uint16*)&msg[6] = xvelocity;
	*(Uint16*)&msg[8] = yvelocity;
	*(Uint16*)&msg[10] = owner;
	*(Uint32*)&msg[12] = timestamp;

	return ret;
}

clientMessage *generateSoccerGoal		(Uint32 timestamp, BYTE ball)
{	clientMessage *ret = new clientMessage(6);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Ball ident
		2		4		Timestamp
	*/

	msg[0] = 0x21;
	msg[1] = ball;
	*(Uint32*)&msg[2] = timestamp;

	return ret;
}

clientMessage *generateFlagRequest		(Uint16 flag)
{	clientMessage *ret = new clientMessage(3);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Flag ident
	*/

	msg[0] = 0x13;
	*(Uint16*)&msg[1] = flag;

	return ret;
}

clientMessage *generateFlagDrop			()
{	clientMessage *ret = new clientMessage(1);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	*/

	msg[0] = 0x15;

	return ret;
}

clientMessage *generateEXERequest		()
{	clientMessage *ret = new clientMessage(1);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	*/

	msg[0] = 0x0B;

	return ret;
}

clientMessage *generateLevelRequest		()
{	clientMessage *ret = new clientMessage(1);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	*/

	msg[0] = 0x0C;

	return ret;
}

clientMessage *generateNewsRequest		()
{	clientMessage *ret = new clientMessage(1);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	*/

	msg[0] = 0x0D;

	return ret;
}

clientMessage *generateArenaLogin		(char *arena, Ship_Types ship, Uint16 xres, Uint16 yres, bool allowAudio)
{	clientMessage *ret = new clientMessage(26);
	if (ret == NULL) return NULL;
	ret->clear();
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		1		Ship type
	Note: When !AllowAudio, remote messages aren't heard in private arenas (1.34.11f)
		2		2		Boolean: Allow audio
		4		2		X resolution
		6		2		Y resolution
		8		2		Main arena number
		10		16		Arena name
	*/

	msg[0] = 0x01;
	msg[1] = ship;
	*(Uint16*)&msg[2] = (allowAudio ? 1 : 0);
	*(Uint16*)&msg[4] = xres;
	*(Uint16*)&msg[6] = yres;

	if (*arena == '\0')
	{	// Random main arena
		*(Uint16*)&msg[8] = ARENA_RandomMain;
	}
	else if (isNumeric(arena))
	{	// Specific main arena
		*(Uint16*)&msg[8] = getInteger(arena, 10);
	}
	else
	{	// Private arena
		*(Uint16*)&msg[8] = ARENA_Private;
		strncpy(msg + 10, arena, 16);
	}

	return ret;
}

clientMessage *generateArenaLeave		()
{	clientMessage *ret = new clientMessage(1);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	*/

	msg[0] = 0x02;

	return ret;
}

clientMessage *generateAttachRequest	(Uint16 player)
{	clientMessage *ret = new clientMessage(3);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		2		Player ident
	*/

	msg[0] = 0x10;
	*(Uint16*)&msg[1] = player;

	return ret;
}

clientMessage *generateBrickDrop		(Uint16 xtile, Uint16 ytile)
{	clientMessage *ret = new clientMessage(5);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		2		X tiles
		3		2		Y tiles
	*/

	msg[0] = 0x1C;
	*(Uint16*)&msg[1] = xtile;
	*(Uint16*)&msg[3] = ytile;

	return ret;
}

clientMessage *generatePosition			(Uint32 timestamp, BYTE direction, Uint16 x, Uint16 y, Uint16 xvelocity,
										 Uint16 yvelocity, bool stealth, bool cloaked, bool xradar, bool awarp,
										 bool flash, bool safety, bool ufo, Uint16 bounty, Uint16 energy,
										 Projectile_Types type, BYTE level, bool shrapBounce, BYTE shrapLevel,
										 BYTE shrap, bool secondary, Uint16 timer, Uint16 S2CLag, bool shields,
										 bool supers, BYTE burst, BYTE repel, BYTE thor, BYTE brick, BYTE decoy,
										 BYTE rocket, BYTE portal)
{	clientMessage *ret = new clientMessage(32);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
	This is the extended version of the c2s position update
		0		1		Type byte
		1		1		Direction
		2		4		Timestamp
		6		2		X velocity
		8		2		Y pixels
		10		1		Checksum
		11		1		Togglables
		12		2		X pixels
		14		2		Y velocity
		16		2		Bounty
		18		2		Energy
		20		2		Weapon info
		22		2		Energy
		24		2		S2C latency
		26		2		Timer
		28		4		Item information
	*/

	stateInfo si;
	si.awarp	= awarp;
	si.cloaked	= cloaked;
	si.flash	= flash;
	si.pack		= 0;
	si.safety	= safety;
	si.stealth	= stealth;
	si.ufo		= ufo;
	si.xradar	= xradar;

	weaponInfo wi;
	wi.shrapBounce	= shrapBounce;
	wi.shrapLevel	= shrapLevel;
	wi.shrapCount	= shrap;
	wi.fireType		= secondary;
	wi.level		= level;
	wi.type			= type;

	itemInfo ii;
	ii.brick	= brick;
	ii.burst	= burst;
	ii.decoy	= decoy;
	ii.pack		= 0;
	ii.portal	= portal;
	ii.repel	= repel;
	ii.rocket	= rocket;
	ii.shields	= shields;
	ii.supers	= supers;
	ii.thor		= thor;

	msg[0] = 0x03;
	msg[1] = direction;
	*(Uint32*)&msg[2] = timestamp;
	*(Uint16*)&msg[6] = xvelocity;
	*(Uint16*)&msg[8] = y;
	msg[10] = 0;
	msg[11] = si.n;
	*(Uint16*)&msg[12] = x;
	*(Uint16*)&msg[14] = yvelocity;
	*(Uint16*)&msg[16] = bounty;
	*(Uint16*)&msg[18] = energy;
	*(Uint16*)&msg[20] = wi.n;

	msg[10] = simpleChecksum(msg, 22);

	*(Uint16*)&msg[22] = energy;
	*(Uint16*)&msg[24] = S2CLag;
	*(Uint16*)&msg[26] = timer;
	*(Uint32*)&msg[28] = ii.n;

	return ret;
}

clientMessage *generatePosition			(Uint32 timestamp, BYTE direction, Uint16 x, Uint16 y, Uint16 xvelocity,
										 Uint16 yvelocity, bool stealth, bool cloaked, bool xradar, bool awarp,
										 bool flash, bool safety, bool ufo, Uint16 bounty, Uint16 energy,
										 Projectile_Types type, BYTE level, bool shrapBounce, BYTE shrapLevel,
										 BYTE shrap, bool secondary)
{	clientMessage *ret = new clientMessage(22);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
	This is the basic version of the c2s position update
		0		1		Type byte
		1		1		Direction
		2		4		Timestamp
		6		2		X velocity
		8		2		Y pixels
		10		1		Checksum
		11		1		Togglables
		12		2		X pixels
		14		2		Y velocity
		16		2		Bounty
		18		2		Energy
		20		2		Weapon info
	*/

	stateInfo si;
	si.awarp	= awarp;
	si.cloaked	= cloaked;
	si.flash	= flash;
	si.pack		= 0;
	si.safety	= safety;
	si.stealth	= stealth;
	si.ufo		= ufo;
	si.xradar	= xradar;

	weaponInfo wi;
	wi.shrapBounce	= shrapBounce;
	wi.shrapLevel	= shrapLevel;
	wi.shrapCount	= shrap;
	wi.fireType		= secondary;
	wi.level		= level;
	wi.type			= type;

	msg[0] = 0x03;
	msg[1] = direction;
	*(Uint32*)&msg[2] = timestamp;
	*(Uint16*)&msg[6] = xvelocity;
	*(Uint16*)&msg[8] = y;
	msg[10] = 0;
	msg[11] = si.n;
	*(Uint16*)&msg[12] = x;
	*(Uint16*)&msg[14] = yvelocity;
	*(Uint16*)&msg[16] = bounty;
	*(Uint16*)&msg[18] = energy;
	*(Uint16*)&msg[20] = wi.n;

	msg[10] = simpleChecksum(msg, 22);

	return ret;
}

clientMessage *generateFileTransfer		(char *fileName, char *buffer, Uint32 len)
{	clientMessage *ret = new clientMessage(17 + len);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
	This is the basic version of the c2s position update
		0		1		Type byte
		1		16		File name
		17		...		Compressed file
	*/

	msg[0] = 0x16;
	strncpy(msg + 1, fileName, 16);
	memcpy(msg + 17, buffer, len);

	return ret;
}

clientMessage *generateSendNewVoice		(BYTE id, Uint16 player, char *buffer, Uint32 len)
{	clientMessage *ret = new clientMessage(4 + len);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
	This is the basic version of the c2s position update
		0		1		Type byte
		1		1		Voice ident
		2		2		Player ident
		4		...		Compressed voice message
	*/

	msg[0] = 0x0E;
	msg[1] = id;
	*(Uint16*)&msg[2] = player;
	memcpy(msg + 4, buffer, len);

	return ret;
}

clientMessage *generateSendOldVoice		(BYTE id, Uint16 player)
{	clientMessage *ret = new clientMessage(4);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
	This is the basic version of the c2s position update
		0		1		Type byte
		1		1		Voice ident
		2		2		Player ident
	*/

	msg[0] = 0x0E;
	msg[1] = id;
	*(Uint16*)&msg[2] = player;

	return ret;
}

clientMessage *generateObjectModify		(Uint16 player, lvzObject *objects, Uint16 num_objects)
{	clientMessage *ret = new clientMessage(4 + 11 * num_objects);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte: 0x0a
		1		2		Player ident (-1 = all players)
		3		1		LVZ type byte: 0x36
	The following are repeated until the end of the message
		4		11		LVZ bitfield
	*/

	msg[0] = 0x0a;
	*(Uint16*)&msg[1] = player;
	msg[3] = 0x36;

	memcpy(msg + 4, objects, 11 * num_objects);

	return ret;
}

clientMessage *generateObjectToggle		(Uint16 player, objectInfo *objects, Uint16 num_objects)
{	clientMessage *ret = new clientMessage(4 + 2 * num_objects);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte: 0x0a
		1		2		Player ident (-1 = all players)
		3		1		LVZ type byte: 0x35
	The following are repeated until the end of the message
		4		2		LVZ bitfield
	*/

	msg[0] = 0x0a;
	*(Uint16*)&msg[1] = player;
	msg[3] = 0x35;

	memcpy(msg + 4, objects, 2 * num_objects);

	return ret;
}

clientMessage *generateTakeGreen		(Uint32 timestamp, Sint16 prize, Uint16 x, Uint16 y)
{	clientMessage *ret = new clientMessage(11);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
		1		4		? Timestamp
		5		2		X
		7		2		Y
		9		2		Prize
	*/

	msg[0] = 0x07;
	*(Uint32*)&msg[1] = timestamp;
	*(Sint16*)&msg[5] = prize;
	*(Uint16*)&msg[7] = x;
	*(Uint16*)&msg[9] = y;

	return ret;
}

clientMessage *generateChangeSettings	(_linkedlist <String> &settings)
{
	_listnode <String> *parse = settings.head;
	int bytes = 3;	// type byte + 2 term chars

	while (parse)
	{
		bytes += parse->item->len + 1;
		parse = parse->next;
	}

	clientMessage *ret = new clientMessage(bytes);
	if (ret == NULL) return NULL;
	char *msg = ret->msg;

	/*	Field	Length	Description
		0		1		Type byte
	The following are repeated until the end of the message
		1		...		"Weasel:SoccerBallSpeed:9000" ASCIIZ
	End of the message
		...		2		"\0\0"
	*/

	msg[0] = 0x1d;

	parse = settings.head;
	int i = 1;

	while (parse)
	{
		String *s = parse->item;

		strncpy(msg + i, s->msg, s->len + 1);	// copy string\0

		i += s->len + 1;

		parse = parse->next;
	}

	msg[i++] = '\0';
	msg[i] = '\0';

	return ret;
}
