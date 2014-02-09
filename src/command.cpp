#include "command.h"

#include "algorithms.h"
#include "clientprot.h"
#include "sockets.h"
#include "settings.h"
#include "botdb.h"


//////// Commands ////////

void gotHelp(Host *h, Player *p, Command *c)
{
	char *tfinal = c->final;

	if (!*c->final)
	{	// No parameter
		if (p->access >= OP_Moderator)
		{
			String s;
			s += "MERVBot ";
			s += getLevelString(p->access);
			s += " commandlist  (Send ::!help <topic> for more info)";
			h->sendPrivate(p, s.msg);
		}

		switch (p->access)
		{
		case OP_Duke:
		case OP_Baron:
		case OP_King:
		case OP_Emperor:
		case OP_RockStar:
		case OP_Q:
		case OP_God:
		case OP_Owner:
			{
				h->sendPrivate(p, "lvl 5: !info !autosave !password !closeall !save !read !set !get !setcmd !killcmd !listcmd !log !restart");
			}
		case OP_SysOp:	/* FALL THRU */
			{
				h->sendPrivate(p, "lvl 4: !close !spawn !load !plugins !unload !setbanner !ownbot !error");
			}
		case OP_SuperModerator:	/* FALL THRU */
			{
				h->sendPrivate(p, "lvl 3: !addop !editop !deleteop !go !limit !uptime !listspawns");
			}
		case OP_Moderator:	/* FALL THRU */
			{
				h->sendPrivate(p, "lvl 2: !setlogin !where !zone !say !chat !listchat !clearchat !attach !follow !team !spec !ship !turret !awarp");
			}
			break;
		case OP_Limited:
		case OP_Player:
			{
				String s = "Basic: !listop !help";

				// Limited
				if (h->allowLimited)
				{
					s += (p->access == OP_Limited) ? " !give" : " !own";
				}

				// Moderators
				if (h->botInfo.db->findOperator(p->name))
				{
					s += " !login";
				}

				h->sendPrivate(p, s.msg);
			}
		};

		h->imports->talk(makeLocalHelp(p, c));

		return;
	}
	else
	{
		h->botInfo.db->aliasCommand(c->final);
	}

	{	// Display alias(es)
		String alist = h->botInfo.db->getAliasList(c->final);
		if (alist.len)
		{
			String s;
			s += "AKA  ";
			s += c->final;
			s += " ";
			s += alist;
			h->sendPrivate(p, s.msg);
		}

		// Remove ! from c.final
	}

	switch (p->access)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		if (c->checkParam("owner") || c->checkParam("all"))
		{
			h->sendPrivate(p, "Basic: !info !autosave !password !closeall !save !read !set !get !setcmd !killcmd !listcmd !log !restart");
		}
		else if (c->checkParam("password"))
		{
			h->sendPrivate(p, "!password [bot password] (change or view bot password)");
		}
		else if (c->checkParam("info"))
		{
			h->sendPrivate(p, "!info (describe bot state)");
		}
		else if (c->checkParam("closeall"))
		{
			h->sendPrivate(p, "!closeall (close all bot spawns)");
		}
		else if (c->checkParam("autosave"))
		{
			h->sendPrivate(p, "!autosave <interval> (change time between automatic database backups)");
		}
		else if (c->checkParam("save"))
		{
			h->sendPrivate(p, "!save (manually backup database)");
		}
		else if (c->checkParam("read"))
		{
			h->sendPrivate(p, "!read (manually re-read INI and Operators.txt)");
		}
		else if (c->checkParam("set"))
		{
			h->sendPrivate(p, "!set <section>:<key>:<new value> (change INI key)");
		}
		else if (c->checkParam("get"))
		{
			h->sendPrivate(p, "!get <section>:<key> (retrieve INI key)");
		}
		else if (c->checkParam("setcmd"))
		{
			h->sendPrivate(p, "!setcmd <command>:<new name> (add a command alias)");
		}
		else if (c->checkParam("killcmd"))
		{
			h->sendPrivate(p, "!killcmd <command> (remove a command alias)");
		}
		else if (c->checkParam("listcmd"))
		{
			h->sendPrivate(p, "!listcmd [command] (list all command aliases or just those belonging to one command)");
		}
		else if (c->checkParam("log"))
		{
			h->sendPrivate(p, "!log [maximum] (list the last 5 items in the log, or more)");
		}
		else if (c->checkParam("restart"))
		{
			h->sendPrivate(p, "!restart (relogs all the bots that are currently in a zone)");
		}

	case OP_SysOp:	/* FALL THRU */
		if (c->checkParam("sop") || c->checkParam("all"))
		{
			h->sendPrivate(p, "Basic: !close !spawn !load !plugins !unload !setbanner !ownbot !error");
		}
		else if (c->checkParam("spawn"))
		{
			h->sendPrivate(p, "!spawn [switches...] <bot name>  (create a new bot, not added to spawns.txt  -botname goes last-)");
			h->sendPrivate(p, "!spawn -p=password (set name password)");
			h->sendPrivate(p, "!spawn -s=password (set staff *password)");
			h->sendPrivate(p, "!spawn -a=arena    (set destination arena)");
			h->sendPrivate(p, "!spawn -i=DLL      (import functionality from a DLL)");
		}
		else if (c->checkParam("load"))
		{
			h->sendPrivate(p, "!load <plugin filename> (load a plugin)");
		}
		else if (c->checkParam("plugins"))
		{
			h->sendPrivate(p, "!plugins (list loaded plugins)");
		}
		else if (c->checkParam("unload"))
		{
			h->sendPrivate(p, "!unload <plugin #>/ALL (drop plugin # from !plugins, ALL=unload all plugins)");
		}
		else if (c->checkParam("close"))
		{
			h->sendPrivate(p, "!close (close this bot spawn)");
		}
		else if (c->checkParam("ownbot"))
		{
			h->sendPrivate(p, "!ownbot [on/off] (allow or disallow !own and !give system)");
		}
		else if (c->checkParam("error"))
		{
			h->sendPrivate(p, "!error [on/off]   (create an error broadcast channel for mods)");
			h->sendPrivate(p, "!error -c=channel (set error channel name, default is \"error\")");
			h->sendPrivate(p, "!error -g         (generate a server warning message)");
		}
		else if (c->checkParam("setbanner"))
		{
			h->sendPrivate(p, "!setbanner (make the bot use your banner)");
		}

	case OP_SuperModerator:	/* FALL THRU */
		if (c->checkParam("smod") || c->checkParam("all"))
		{
			h->sendPrivate(p, "Basic: !addop !editop !deleteop !go !limit !uptime !listspawns");
		}
		else if (c->checkParam("uptime"))
		{
			h->sendPrivate(p, "!uptime (display time since bot joined zone and arena)");
		}
		else if (c->checkParam("limit"))
		{
			h->sendPrivate(p, "!limit <numerical level> (remove bot access below given operator level)");
		}
		else if (c->checkParam("go"))
		{
			h->sendPrivate(p, "!go [#]<arena name/public arena number> (change bot's arena)");
			h->sendPrivate(p, "     # (secret arena)");
		}
		else if (c->checkParam("addop"))
		{
			h->sendPrivate(p, "!addop <name>      (create a new operator)");
			h->sendPrivate(p, "!addop -l=level #  (set operator level ::!help levels)");
			h->sendPrivate(p, "!addop -p=password (set password, no password by default)");
			h->sendPrivate(p, "NOTE: Takes effect when the op sends !login [password]");
		}
		else if (c->checkParam("editop"))
		{
			h->sendPrivate(p, "!editop <name>      (edit existing operator, takes effect during !login)");
			h->sendPrivate(p, "!editop -l=level #  (set operator level ::!help levels)");
			h->sendPrivate(p, "!editop -p=password (set password, no password by default)");
			h->sendPrivate(p, "NOTE: Takes effect when the op sends !login [password]");
		}
		else if (c->checkParam("deleteop"))
		{
			h->sendPrivate(p, "!deleteop <name> (remove operator powers)");
			h->sendPrivate(p, "NOTE: Only takes effect after the ex-op leaves the zone");
		}
		else if (c->checkParam("levels"))
		{
			h->sendPrivate(p, "0 Player               Only !listop");
			h->sendPrivate(p, "1 Player(Limited)      Public, limited bot access");
			h->sendPrivate(p, "2 Moderator(Mod)       Control over basic tools");
			h->sendPrivate(p, "3 SuperModerator(SMod) Manages Moderator accounts");
			h->sendPrivate(p, "4 SysOp(SOp)           Control over advanced tools");
			h->sendPrivate(p, "5 Owner(etc)           No restrictions");
		}

	case OP_Moderator:	/* FALL THRU */
		if (c->checkParam("mod") || c->checkParam("all"))
		{
			h->sendPrivate(p, "Basic: !setlogin !where !zone !say !chat !listchat !clearchat !attach !follow !team !spec !ship !turret !awarp");
		}
		else if (c->checkParam("awarp"))
		{
			h->sendPrivate(p, "!awarp [1/0] (turn on/off bot's antiwarp)");
		}
		else if (c->checkParam("version"))
		{
			h->sendPrivate(p, "!version (display bot version)");
		}
		else if (c->checkParam("logout"))
		{
			h->sendPrivate(p, "!logout (release operator powers)");
		}
		else if (c->checkParam("turret"))
		{
			h->sendPrivate(p, "!turret 0/1 (mode: 0=shoots enemies, 1=fires on team command)");
		}
		else if (c->checkParam("setlogin"))
		{
			h->sendPrivate(p, "!setlogin <new password> (change personal login password)");
		}
		//OmegaFirebolt added chatlist command help
		else if (c->checkParam("listchat"))
		{
			h->sendPrivate(p, "!listchar (lists all chat channels that bot is currently on so you don't have to guess)");
		}
		//OmegaFirebolt added chatclear command help
		else if (c->checkParam("clearchat"))
		{
			h->sendPrivate(p, "!clearchat (clears all chat channels so bot is no longer on any chat channels)");
		}
		else if (c->checkParam("chat"))
		{
			h->sendPrivate(p, "!chat <chan[,chan[,chan]]> (set bot chat(s))");
		}
		else if (c->checkParam("attach"))
		{
			h->sendPrivate(p, "!attach <player name> (make bot attach to a player)");
		}
		else if (c->checkParam("follow"))
		{
			h->sendPrivate(p, "!follow <player name> (make bot follow a player)");
		}
		else if (c->checkParam("ship"))
		{
			h->sendPrivate(p, "!ship <ship type> (make bot change ships)");
		}
		else if (c->checkParam("spec"))
		{
			h->sendPrivate(p, "!spec (make bot spectate the arena)");
		}
		else if (c->checkParam("team"))
		{
			h->sendPrivate(p, "!team <team number> (make bot change teams)");
			h->sendPrivate(p, "!team <player name> (return team of player)");
		}
		else if (c->checkParam("where"))
		{
			h->sendPrivate(p, "!where <player name> (return last known player coordinates)");
		}
		else if (c->checkParam("say"))
		{
			h->sendPrivate(p, "!say <message> (make the bot say something)");
			h->sendPrivate(p, "!say //Team");
			h->sendPrivate(p, "!say ;X;Channel");
			h->sendPrivate(p, "!say :Name:Remote");
		}
		else if (c->checkParam("zone"))
		{
			h->sendPrivate(p, "!zone <message> (broadcast message zone-wide w/ nametag)");
		}

	case OP_Limited:	/* FALL THRU */
	case OP_Player:
		if (c->checkParam("help"))
		{
			h->sendPrivate(p, "!help [topic] (display available commands and what they do)");
			h->sendPrivate(p, "<> required  [] optional  () comments  -s[=assign] switches");
		}
		else if (c->checkParam("disabled"))
		{
			h->sendPrivate(p, "Forget about it! This command is disabled");
		}
		else if (c->checkParam("listop"))
		{
			if (p->access == OP_Player)
			{
				h->sendPrivate(p, "!listop (list online operators)");
			}
			else
			{
				h->sendPrivate(p, "!listop -o (list online and offline operators)");
				h->sendPrivate(p, "        -o (list only online operators)");
				h->sendPrivate(p, "NOTE: %prefix for online operators");
			}
		}
		else if (c->checkParam("login"))
		{
			h->sendPrivate(p, "!login [password] (grant self operator status)");
		}
	};

	h->imports->talk(makeLocalHelp(p, c));

	c->final = tfinal;
}


void gotCommand(Host *h, Player *p, char *m)
{

#ifdef DISABLE_COMMANDS
	return;											// Compiler directive to ignore commands
#endif

	if (p->access < h->lowestLevel)
	{	// Limiter is in effect
		opEntry *op = h->botInfo.db->findOperator(p->name);

		if (!op || (op->getAccess() < h->lowestLevel))
		{
			return;
		}
	}

	Command c(m);									// Wrap in a friendly class

	char *tcmd = c.cmd;								// Backup original pointer in case this is an alias
	h->botInfo.db->aliasCommand(c.cmd);				// Check aliases

#ifndef _DEBUG

try {												// Catch all command-crash bugs

#endif // _DEBUG

	switch (p->access)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
		if (c.check("password"))
		{
			String s;
			if (*c.final)
			{
				s += "?password=";
				s += c.final;
				h->sendPublic(s.msg);

				h->botInfo.setLogin(h->botInfo.name, c.final, h->botInfo.staffpw);

				h->sendPrivate(p, "Updated local bot parameters and requested network password change");
			}
			else
			{
				s += "Current bot password: ";
				s += h->botInfo.password;
				h->sendPrivate(p, s.msg);
			}
		}
		else if (c.check("info"))
		{
			String s;

			s += "MISC SysOp:";
			s += h->hasSysOp;
			s += " SMod:";
			s += h->hasSMod;
			s += " Mod:";
			s += h->hasMod;
			s += " Ping:";
			s += h->syncPing;
			s += " Client:";
			if (h->botInfo.db->forceContinuum)
			{
				s += "Ctm ";
				s += CONTINUUM_VERSION;
			}
			else
			{
				s += "VIE ";
				s += SUBSPACE_VERSION;
			}
			s += " DLL:";
			s += h->botInfo.dllName;
			s += " DELT:";
			s += h->timeDiff;
			h->sendPrivate(p, s.msg);

			s = "USER ";
			s += h->botInfo.name;
			s += ":";
			s += h->botInfo.password;

			s += "  ZONE ";
			{	// Get string IP
				INADDR addr(h->botInfo.ip, 0);
				s += addr.getString();
			}
			s += ":";
			s += h->botInfo.port;

			s += "  ARENA \'";
			s += h->botInfo.initialArena;
			s += "\' ship #";
			s += h->botInfo.initialShip;
			s += " @ ";
			s += h->botInfo.xres;
			s += "x";
			s += h->botInfo.yres;
			h->sendPrivate(p, s.msg);

			s = "BAN  MID:";
			s += h->botInfo.machineID;
			s += " PID:";
			s += h->botInfo.permissionID;
			s += " TZB:";
			s += h->botInfo.timeZoneBias;
			h->sendPrivate(p, s.msg);
		}
		else if (c.check("closeall"))
		{
			h->botInfo.db->spawns.massDisconnect();

			break;
		}
		else if (c.check("restart"))
		{
			h->botInfo.db->spawns.massRestart();

			break;
		}
		else if (c.check("log"))
		{
#ifdef KEEP_LOG
			int max = 5;

			if (isNumeric(c.final))
			{
				max = getInteger(c.final, 10);
				if (max < 1) max = 5;
			}

			if (max > h->logLength) max = h->logLength;

			for (int i = h->logLength - max; i < h->logLength; ++i)
			{
				h->sendPrivate(p, h->loggedChatter[i]->msg);
			}

			break;
#endif
		}
		else if (c.check("autosave"))
		{
			if (isNumeric(c.final))
			{
				Uint32 interval = getInteger(c.final, 10);

				if ((interval >= 30) && (interval <= 10000000))
				{
					h->botInfo.db->saveInterval = interval * 100;
				}

				String s;
				s += "Database autosave every ";
				s += h->botInfo.db->saveInterval;
				s += " seconds";
				h->sendPrivate(p, s.msg);
			}
			else
			{
				h->sendPrivate(p, "Format: !autosave <database interval in seconds>");
			}
		}
		else if (c.check("ppl"))	// unlisted, spit out diag report
		{
			_listnode <Player> *parse = h->playerlist.head;
			String s;
			Uint32 count = 0;

			while (parse)
			{
				Player *p = parse->item;
				parse = parse->next;

				s += p->name;
				s += "(";
				s += p->score.wins;
				s += ":";
				s += p->score.losses;
				s += ")@";
				s += p->team;
				++count;

				if (parse)
				{
					if (s.len > 80)
					{
						h->sendPublic(s.msg);
						s = "";
						count = 0;
					}
					else
					{
						s += "  ";
					}
				}
			}

			if (count) h->sendPublic(s.msg);
		}
		else if (c.check("save"))
		{
			h->botInfo.db->saveOperators();
			h->botInfo.db->saveAliases();

			h->sendPrivate(p, "Database save completed.");
		}
		else if (c.check("read"))
		{
			h->botInfo.db->reloadINI(false);
			//OmegaFirebolt added loadOperators2
			h->botInfo.db->loadOperators2();

			h->sendPrivate(p, "Re-loaded INI and Operators.txt settings.");
		}
		else if (c.check("set"))
		{
			String out = c.final;

			String app, key, str;

			app = out.split(':');
			key = out.split(':');
			str = out.split(':');

			WritePrivateProfileString(app.msg, key.msg, str.msg, h->botInfo.db->path);

			String s;
			s += "Set->";
			s += app;
			s += ":";
			s += key;
			s += ":";
			s += str;

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("get"))
		{
			String out = c.final;

			String app, key;

			app = out.split(':');
			key = out.split(':');

			char buffer[128];
			GetPrivateProfileString(app.msg, key.msg, "InvalidTag", buffer, 128, h->botInfo.db->path);
			buffer[127] = '\0';

			String s;
			s += "Get->";
			s += app;
			s += ":";
			s += key;
			s += ":";
			s += buffer;

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("setcmd"))
		{
			tolower(c.final);

			String out = c.final;

			String command, alias;

			command = out.split(':');
			alias = out.split(':');

			h->botInfo.db->aliasCommand(command.msg);

			if (!invalidArena(command.msg) && !invalidArena(alias.msg))
			{
				if (h->botInfo.db->findAlias(alias.msg))
				{
					String s;
					s += "Alias \'";
					s += alias;
					s += "\' already exists";
					h->sendPrivate(p, s.msg);
				}
				else
				{
					h->botInfo.db->addAlias(command.msg, alias.msg);

					String s;
					s += "Created alias \'";
					s += alias;
					s += "\' for command \'";
					s += command;
					s += "\'";
					h->sendPrivate(p, s.msg);
				}
			}
			else
			{
				String s;
				s += "Invalid command or alias \'";
				s += alias;
				s += "\':\'";
				s += command;
				s += "\'";
				h->sendPrivate(p, s.msg);
			}
		}
		else if (c.check("killcmd"))
		{
			tolower(c.final);

			String s;
			if (h->botInfo.db->killAlias(c.final))
			{
				s += "Removed alias \'";
				s += c.final;
				s += "\'";
			}
			else
			{
				s += "Alias \'";
				s += c.final;
				s += "\' does not exist";
			}
			h->sendPrivate(p, s.msg);
		}
		else if (c.check("listcmd"))
		{
			bool anycmd = false;
			String s;
			Uint32 count = 0;
			s += "Cmd: ";
			char *cmd = c.final;

			if (*cmd)
			{
				h->botInfo.db->aliasCommand(cmd);

				s += cmd;
				++count;
			}
			else
			{
				break;
			}
			for (auto const & alias : h->botInfo.db->aliasList)
			{
				if (!alias.isCmd(cmd)) continue;

				if (count++) s += " ";
				s += alias.getAlias();
				anycmd = true;

				if (s.len > 80)
				{
					h->sendPrivate(p, s.msg);
					s = "Cmd: ";
					count = 0;
				}
			}

			if (!anycmd)	h->sendPrivate(p, "No aliases for that command.");
			else if (count)	h->sendPrivate(p, s.msg);
		}
		else if (c.check("pball"))
		{
			int id = getInteger(c.final, 10);
			_listnode<PBall> *parse = h->ballList.head;

			while (parse)
			{
				PBall *pb = parse->item;
				parse = parse->next;

				if (pb->ident == id)
				{
					h->postRR(generatePowerballRequest(pb->hosttime, id));
					break;
				}
			}
		}
		else if (c.check("test2"))
		{
			objectInfo oi;

			oi.id = 5656;
			oi.disabled = 0;

			h->postRR(generateObjectToggle(-1, &oi, 1));
		}
		else if (c.check("test_obj"))
		{
			Player *pp = h->findPlayer(c.final);
			if (!pp) break;

			lvzObject obj[8];
			memset(obj, 0, sizeof(obj));

			for (int ii = 0; ii < 8; ++ii)
			{
				obj[ii].id = 101 + ii;
				obj[ii].mapobj = 1;
				obj[ii].change_image = 1;
				obj[ii].image = 1;
			}

			h->postRR(generateObjectModify(-1, obj, 8));
		}

	case OP_SysOp:	/* FALL THRU */
		if (c.check("spawn"))
		{
			if (h->botInfo.db->spawns.getConnections() >= h->botInfo.maxSpawns)
			{
				h->sendPrivate(p, "Aborted: Too many bot spawns are connected or connecting. Try again soon");

				break;
			}

			BOT_INFO bi;
			bi.set(h->botInfo);

			String name;
			String password;
			String staff;
			String arena;
			String dll;

			_switch *s;

			// Name
			if (invalidName(c.final))
			{
				h->sendPrivate(p, "Aborted: The bot name you gave me is invalid");

				break;
			}
			name = c.final;

			// Password
			if (s = c.getParam('p'))
			{
				password = s->param;
			}
			else
			{
				password = bi.password;
			}

			// Staff
			if (s = c.getParam('s'))
			{
				staff = s->param;
			}
			else
			{
				staff = bi.staffpw;
			}

			// Imports
			if (s = c.getParam('i'))
			{
				dll = s->param;
			}
			else
			{
				dll = bi.dllName;
			}

			// Arena
			if (s = c.getParam('a'))
			{
				arena = s->param;
			}
			else
			{
				arena = bi.initialArena;
			}

			if (invalidArena(arena.msg))
			{
				h->sendPrivate(p, "Aborted: Arena name you gave me is invalid");

				break;
			}

			bi.setLogin(name.msg, password.msg, staff.msg);
			bi.setSpawn(dll.msg);
			bi.setArena(arena.msg, bi.initialShip, bi.xres, bi.yres, bi.allowAudio);

			String m;

			if (bi.db->spawns.connectHost(bi))
			{
				m += "Attempting to spawn ";
				m += name;
				m += ". If successful, it will be in arena \'";
				m += arena;
				m += "\' in a few seconds...";
			}
			else
			{
				m += "Error while attempting to spawn ";
				m += name;
				m += ". A bot with this name is already active";
			}
			h->sendPrivate(p, m.msg);
		}
		else if (c.check("door"))
		{
			if (!*c.final)
			{
				h->sendPrivate(p, "Syntax: !door <door mode>");
				break;
			}

			_linkedlist <String> settings;

			settings.append(new String("Door:DoorMode:" + String(c.final)));

			h->postRR(generateChangeSettings(settings));
		}
		else if (c.check("load"))
		{
			if (!*c.final)
			{
				h->sendPrivate(p, "Syntax: !load <filename>");
				break;
			}

			if (h->imports->importLibrary(c.final))	// Attempt to load new callbacks
			{
				String s;
				s += "Successfully loaded module(s) \'";
				s += c.final;
				s += "\'.";
				h->sendPrivate(p, s.msg);

				s.clear();
				bool seen = false;

				for (int slot = 0; slot < DLL_MAX_LOADED; ++slot)
				{
					char *name = h->imports->getPlugin(slot);
					if (!name) continue;

					if (seen) s += ", ";
					s += name;
					seen = true;
				}

				h->botInfo.setSpawn(s.msg);
			}
			else
			{
				h->sendPrivate(p, "I was unable to load the plugin you requested");
			}
		}
		else if (c.check("plugins"))
		{
			bool found = false;

			for (int slot = 0; slot < DLL_MAX_LOADED; ++slot)
			{
				char *name = h->imports->getPlugin(slot);
				if (!name) continue;

				String s = "slot ";
				s += slot;
				s += ": ";
				s += name;
				h->sendPrivate(p, s.msg);

				found = true;
			}

			if (!found)
				h->sendPrivate(p, "No loaded plugins");
		}
		else if (c.check("unload"))
		{
			if (isNumeric(c.final))
			{
				int n = getInteger(c.final, 10);

				h->imports->clearImport(n);

				h->sendPrivate(p, "Unloaded plugin");

				String s;
				bool seen = false;

				for (int slot = 0; slot < DLL_MAX_LOADED; ++slot)
				{
					char *name = h->imports->getPlugin(slot);
					if (!name) continue;

					if (seen) s += ", ";
					s += name;
					seen = true;
				}

				h->botInfo.setSpawn(s.msg);
			}
			else if (c.checkParam("all"))
			{
				h->imports->clearImports();

				h->sendPrivate(p, "Unloaded all plugins");

				h->botInfo.setSpawn("");
			}
			else
			{
				h->sendPrivate(p, "Invalid syntax. Send ::!help unload");
			}
		}
		else if (c.check("setbanner"))
		{
			h->postRR(generateChangeBanner(p->banner));

			h->sendPublic("*banner please");
		}
		else if (c.check("ownbot"))
		{
			if (c.checkParam("on"))
			{
				h->allowLimited = true;
			}
			else if (c.checkParam("off"))
			{
				h->allowLimited = false;
				h->revokeAccess(OP_Limited);
			}

			if (h->allowLimited)
			{
				h->sendPrivate(p, "!own and !give commands allowed, listed in !help");
			}
			else
			{
				h->sendPrivate(p, "!own and !give commands disallowed");
			}
		}
		else if (c.check("close"))
		{
			if (h->botInfo.db->spawns.getConnections() <= 1)
			{
				h->sendPrivate(p, "Aborted: Too few bot spawns are connected or connecting. Try again soon");

				break;
			}

			h->disconnect(true);
		}
		else if (c.check("error"))
		{
			if (c.getParam('g'))
			{
				h->postRR(generateViolation(SEC_ShipChecksumMismatch));

				break;
			}

			String chan;

			_switch *i;
			if (i = c.getParam('c'))
			{
				chan = i->param;
			}
			else
			{
				chan = "error";
			}

			if (c.checkParam("on"))
			{
				h->broadcastingErrors = true;
			}
			else if (c.checkParam("off"))
			{
				h->broadcastingErrors = false;
			}

			String change = "?chat=";
			change += chan;
			h->sendPublic(change.msg);

			String s = "Error broadcasting ";
			if (h->broadcastingErrors)	s += "on, ";
			else						s += "off, ";
			s += "sending to channel \"";
			s += chan;
			s += "\"";

			h->sendPrivate(p, s.msg);
		}

	case OP_SuperModerator:	/* FALL THRU */
		if (c.check("uptime"))
		{
			Uint32 currTime = GetTickCount();
			Uint32 arenaUptime = currTime - h->arenaJoinTime;
			Uint32 zoneUptime = currTime - h->zoneJoinTime;

			String s;
			s += "Alive in zone:(";
			s += zoneUptime / (1000 * 60 * 60 * 24);
			s += "d ";
			s += (zoneUptime / (1000 * 60 * 60)) % 24;
			s += "h ";
			s += (zoneUptime / (1000 * 60)) % 60;
			s += "m), in arena:(";
			s += arenaUptime / (1000 * 60 * 60 * 24);
			s += "d ";
			s += (arenaUptime / (1000 * 60 * 60)) % 24;
			s += "h ";
			s += (arenaUptime / (1000 * 60)) % 60;
			s += "m)";
			h->sendPrivate(p, s.msg);
		}
		else if (c.check("listspawns"))
		{
			// AGH
			_listnode<Host> *parse = h->botInfo.db->spawns.list.head;
			Uint32 n = 0;

			while (parse)
			{
				Host *host = parse->item;

				String s;
				s += n++;
				s += ".  TypedName:";
				s += host->botInfo.name;
				s += "  Arena:";
				s += host->botInfo.initialArena;
				if (host->Me)
				{
					s += "  PlayingName:";
					s += host->Me->name;
				}
				else
				{
					s += "  Not in an arena";
				}

				h->sendPrivate(p, s.msg);

				parse = parse->next;
			}
		}
		else if (c.check("limit"))
		{
			if (*c.final)
			{
				if (!isNumeric(c.final))
				{
					h->sendPrivate(p, "Format: !limit <numerical operator level>");

					break;
				}

				Uint16 limit = getInteger(c.final, 10);
				if (limit > OP_Owner) limit = OP_Owner;

				if (limit > p->access)
				{
					h->sendPrivate(p, "Denied! Commiting bot suicide ain't cool");

					break;
				}

				h->lowestLevel = (Operator_Level)limit;
			}

			String s;
			s += "Limiting bot access to ";
			s += getLevelString(h->lowestLevel);
			s += " or higher";
			h->sendPrivate(p, s.msg);
		}
		else if (c.check("addop"))
		{
			// Only valid names
			if (invalidName(c.final))
			{
				String s;
				s += "Invalid operator name \'";
				s += c.final;
				s += "\'";
				h->sendPrivate(p, s.msg);

				break;
			}

			opEntry *op;
			Operator_Level access;

			// Only new names
			op = h->botInfo.db->findOperator(c.final);

			if (op)
			{
				h->sendPrivate(p, "Cannot recreate an existing operator.  ::!listop  ::!help addop");

				break;
			}

			// Process level switch
			_switch *i;
			if ((i = c.getParam('l')) && isNumeric(i->param))
			{
				access = (Operator_Level)getInteger(i->param, 10);

				if (access >= p->access && p->access < OP_Owner)
					access = (Operator_Level)(p->access - 1);

				if (access < 0)
					access = OP_Moderator;
			}
			else
				access = OP_Moderator;

			// Add operator
			op = h->botInfo.db->addOperator(c.final, "", access);

			if (!op)
			{
				h->sendPrivate(p, "The fates have conspired against you: addOperator() failed");

				break;
			}

			// Notify requesting operator
			String s;
			s += "Created ";
 			s += getLevelString(op->getAccess());
			s += ": ";
			s += op->getName();
			if ((i = c.getParam('p')) && (i->param[0]))
			{
				s += ", with password ";
				s += i->param;

				op->setPassword(i->param);
			}
			else
			{
				s += ", without a password";
			}

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("go"))
		{
			if (invalidArena(c.final))
			{
				String s;
				s += "Denied! Invalid arena name \'";
				s += c.final;
				s += "\'";
				h->sendPrivate(p, s.msg);

				break;
			}

			String s;
			s += "Moving to \'";
			s += c.final;
			s += "\'";
			h->sendRemotePrivate(p->name, s.msg);

			h->botInfo.setArena(c.final, h->botInfo.initialShip, h->botInfo.xres, h->botInfo.yres, h->botInfo.allowAudio);

			h->changeArena(c.final);
		}
		else if (c.check("editop"))
		{
			opEntry *op;

			op = h->botInfo.db->findOperator(c.final);

			if (!op)
			{
				h->sendPrivate(p, "Cannot modify non-existant operators.  ::!listop  ::!help editop");

				break;
			}

			if (op->getAccess() >= p->access && p->access < OP_Owner)
			{
				h->sendPrivate(p, "Cannot modify peer or senior operators.  ::!listop  ::!help editop");

				break;
			}

			_switch *i;
			if ((i = c.getParam('l')) && isNumeric(i->param))
			{
				Operator_Level access = (Operator_Level)getInteger(i->param, 10);

				if (access < 0)
				{
					h->sendPrivate(p, "Invalid access level.  ::!listop  ::!help editop");

					break;
				}

				if (access > p->access && p->access < OP_Owner)
				{
					h->sendPrivate(p, "Cannot promote above your level.  ::!listop  ::!help editop");

					break;
				}

				op->setAccess(access);
			}

			// Notify requesting operator
			String s;
			s += "Updated. ";
 			s += getLevelString(op->getAccess());
			s += ": ";
			s += op->getName();

			if (i = c.getParam('p'))
			{
				op->setPassword(i->param);

				if (i->param[0])
				{
					s += ", changed password ";
					s += i->param;

					op->setPassword(i->param);
				}
				else
				{
					s += ", removed the password";
				}
			}

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("deleteop"))
		{
			// Determine which operator is being requested removed
			opEntry *xop = h->botInfo.db->findOperator(c.final);

			if (!xop)
			{
				h->sendPrivate(p, "Removing a non-existant operator isn't groovy baby");

				break;
			}

			if (CMPSTR(xop->getName(), p->name))
			{
				h->sendPrivate(p, "Thou shalt not commit bot suicide");

				break;
			}

			// Run the request through level restrictions
			if (p->access <= xop->getAccess() && p->access < OP_Owner)
			{
				h->sendPrivate(p, "Thou shalt not remove thine peers and senior operators");

				break;
			}

			// Remove the operator
			if (h->botInfo.db->removeOperator(xop->getName()))
			{
				h->sendPrivate(p, "Successfully removed operator");
			}
			else
			{
				h->sendPrivate(p, "Failed to remove operator");
			}
		}

	case OP_Moderator:	/* FALL THRU */
		if (c.check("setlogin"))
		{
			opEntry *op = h->botInfo.db->findOperator(p->name);
			op->setPassword(c.final);

			String s;

			if (c.checkParam(""))
			{
				s += "Login password no longer required";
			}
			else
			{
				s += "Login password changed to ";
				s += c.final;
			}

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("version"))	// unlisted, display bot version
		{
			h->sendPrivate(p, VERSION_STRING);
		}
		else if (c.check("awarp"))
		{
			if (h->Me)
			{
				if (c.checkParam("0"))
					h->Me->awarp = 0;
				else if (c.checkParam("1"))
					h->Me->awarp = 1;

				switch (h->Me->awarp)
				{
				case 0:		h->sendPrivate(p, "Mode set: 0, I will not engage antiwarp.");			break;
				case 1:		h->sendPrivate(p, "Mode set: 1, I will project an anti-warp field around my ship.");	break;
				};
			}
		}
		else if (c.check("turret"))
		{
			if (c.checkParam("0"))
				h->turretMode = 0;
			else if (c.checkParam("1"))
				h->turretMode = 1;

			switch (h->turretMode)
			{
			case 0:		h->sendPrivate(p, "Mode set: 0, when in-game and not following I will fire against enemy teams");			break;
			case 1:		h->sendPrivate(p, "Mode set: 1, when in-game and not following I will act like a team-controlled turret");	break;
			default:	h->sendPrivate(p, "Unknown mode set.  This be a problem you should talk to Catid about");					break;
			};
		}
		else if (c.check("drop"))	// unlisted, drops carried flags
		{
			h->postRR(generateFlagDrop());
		}
		else if (c.check("where"))
		{
			Player *xp;

			if (!*c.final)
				xp = p;
			else
				xp = h->findPlayer(c.final);

			if (xp)
			{
				Sint32 x = xp->tile.x,
					   y = xp->tile.y;

				String s;
				s += "Last seen ";
				s += xp->name;
				s += " @ (";
				s += x;
				s += ", ";
				s += y;
				s += ") [";
				s += getCoords(x, y);
				s += "]";

				s += " stl:";
				s += xp->stealth;
				s += " clk:";
				s += xp->cloak;
				s += " x:";
				s += xp->xradar;
				s += " awp:";
				s += xp->awarp;
				s += " ufo:";
				s += xp->ufo;
				s += " fsh:";
				s += xp->flash;
				s += " sft:";
				s += xp->safety;
				h->sendPrivate(p, s.msg);
			}
		}
		else if (c.check("spec"))
		{
			h->postRR(generateChangeShip(SHIP_Spectator));

			h->botInfo.setArena(h->botInfo.initialArena, SHIP_Spectator, h->botInfo.xres, h->botInfo.yres, h->botInfo.allowAudio);
		}
		else if (c.check("follow"))
		{
			Player *xp;
			String s;

			if (!*c.final)
			{
				if (h->follow)
				{
					s += "Currently following ";
					s += h->follow->item->name;
				}
				else
				{
					s += "Currently not following anyone";
				}

				s += ". Type !follow <name> to start following";

				h->sendPrivate(p, s.msg);

				break;
			}

			if (c.checkParam("on"))
			{	// because people ignore the !help reference
				xp = p;
			}
			else
			{
				xp = h->findPlayer(c.final);
			}

			if (xp)
			{
				h->follow = h->playerlist.find(xp);

				s += "Started following ";
				s += xp->name;
				s += ". Will start once player is visible";

				if (h->Me)
				{
					h->Me->clone(xp);

					if (h->Me->ship == SHIP_Spectator)
					{
						h->postRR(generateChangeShip(SHIP_Warbird));

						h->botInfo.setArena(h->botInfo.initialArena, SHIP_Warbird, h->botInfo.xres, h->botInfo.yres, h->botInfo.allowAudio);
					}
				}

				h->sendPrivate(p, s.msg);
			}
			else
			{
				if (h->follow)
				{
					s += "Stopped following ";
					s += h->follow->item->name;

					if (h->Me)
					{
						h->Me->vel = 0;
					}

					h->follow = NULL;
				}
				else
				{
					s += "Stopped following";
				}

				h->sendPrivate(p, s.msg);
			}
		}
		else if (c.check("attach"))
		{
			Player *xp;

			if (!*c.final || c.checkParam("on"))
			{	// because people ignore the !help reference
				xp = p;
			}
			else
			{
				xp = h->findPlayer(c.final);
			}

			if (xp)
			{
				String s;
				s += "Attaching to ";
				s += xp->name;
				s += "...";
				h->sendPrivate(p, s.msg);

				h->postRR(generateAttachRequest(xp->ident));

				h->follow = h->playerlist.find(xp);

				h->Me->turret = xp;
			}
			else
			{
				h->sendPrivate(p, "Detached");

				h->postRR(generateAttachRequest(UNASSIGNED));

				h->Me->turret = NULL;
			}
		}
		else if (c.check("ship"))
		{
			Uint8 ship = (getInteger(c.final, 10) - 1) & 7;

			h->botInfo.setArena(h->botInfo.initialArena, (Ship_Types)ship, h->botInfo.xres, h->botInfo.yres, h->botInfo.allowAudio);

			h->postRR(generateChangeShip((Ship_Types)ship));
		}
		else if (c.check("team"))
		{
			if (isNumeric(c.final))
			{
				Uint16 team = getInteger(c.final, 10);
				team %= MAX_TEAMS;

				h->postRR(generateChangeTeam(team));
			}
			else
			{
				String s;
				Player *px;

				if (!*c.final)
				{
					px = p;
				}
				else
				{
					px = h->findPlayer(c.final);
				}

				if (px)
				{
					s += "Found ";
					s += px->name;
					s += " on team ";
					s += px->team;
				}
				else
				{
					s += "Unable to find ";
					s += c.final;
				}

				h->sendPrivate(p, s.msg);
			}
		}
		//OmegaFirebolt added chatlist command
		else if(c.check("listchat"))
		{
			String s = "Current Chat Channels: ";
			s+= h->botChats;
			h->sendPrivate(p, s.msg);
		}
		//OmegaFirebolt added chatclear command
		else if(c.check("clearchat"))
		{
			if (h->broadcastingErrors)
			{
				h->sendPrivate(p, "Cannot clear chat channel while broadcasting errors");
			}
			else
			{
				h->sendPublic("?chat=");

				h->sendPrivate(p, "Chat channels cleared.");

				h->botChats = "";
			}
		}
		else if (c.check("chat"))
		{
			if (*c.final)
			{
				if (h->broadcastingErrors)
				{
					h->sendPrivate(p, "Cannot change chat channel while broadcasting errors");
				}
				else
				{
					String s = "?chat=";
					s += c.final;
					h->sendPublic(s.msg);

					s = "chat=";
					s += c.final;
					h->sendPrivate(p, s.msg);

					//OmegaFirebolt added ChatChannelList to keep track of channels
					h->botChats = c.final;
				}
			}
			else
			{
				h->sendPrivate(p, "You must provide channel name(s): !chat squad_chat,newbie_chat,zone_chat");
			}
		}
		else if (c.check("say"))
		{
			switch (c.final[0])
			{
			case '/':	// Team
				if (c.final[1] != '/') break;

				if ( (c.final[2] != '*') &&
					 (c.final[2] != '?')  )
				{
					String s = c.final + 2;

					char bong[32];
					int i = s.firstInstanceOf('%');

					if (i >= 0)
					{
						int j, k = i;

						for (j = 0; j < 32; ++j)
						{
							char c = s.msg[++k];

							if (c >= '0' && c <= '9')
								bong[j] = c;
							else
							{
								bong[j] = '\0';
								break;
							}
						}

						s = s.left(i) + s.right(s.len - (i + j + 1));
					}

					h->sendTeam(getInteger(bong, 10), s.msg);
				}
				break;
			case '\'':	// Team
				if ( (c.final[1] != '*') &&
					 (c.final[1] != '?')  )
				{
					String s = c.final + 1;

					char bong[32];
					int i = s.firstInstanceOf('%');

					if (i >= 0)
					{
						int j, k = i;

						for (j = 0; j < 32; ++j)
						{
							char c = s.msg[++k];

							if (c >= '0' && c <= '9')
								bong[j] = c;
							else
							{
								bong[j] = '\0';
								break;
							}
						}

						s = s.left(i) + s.right(s.len - (i + j + 1));
					}

					h->sendTeam(getInteger(bong, 10), s.msg);
				}
				break;
			case ';':	// Channel
				if ( (c.final[1] != '*') &&
					 (c.final[1] != '?')  )
				{
					h->sendChannel(c.final + 1);
				}
				break;
			case ':':	// Remote private
				if (validRemotePrivate(c.final))
				{
					h->sendRemotePrivate(c.final);
				}
				break;
			default:	// Public
				if ( (c.final[0] != '*') &&
					 (c.final[0] != '?')  )
				{
					String s = c.final;

					char bong[32];
					int i = s.firstInstanceOf('%');

					if (i >= 0)
					{
						int j, k = i;

						for (j = 0; j < 32; ++j)
						{
							char c = s.msg[++k];

							if (c >= '0' && c <= '9')
								bong[j] = c;
							else
							{
								bong[j] = '\0';
								break;
							}
						}

						s = s.left(i) + s.right(s.len - (i + j + 1));
					}

					h->sendPublic(getInteger(bong, 10), s.msg);
				}
			};
		}
		else if (c.check("zone"))
		{
			if (!*c.final)
			{
				h->sendPrivate(p, "Here's how \'!zone\' works: !zone <message>  (I will add a -nametag to the end)");

				break;
			}

			String s;
			s += "*zone";
			s += c.final;
			s += " -";
			s += p->name;
			h->sendPublic(s.msg);
		}
		else if (c.check("kill"))	// disabled in commands.txt by default
		{
			h->postRR(generateDeath(p->ident, 500));
		}
		else if (c.check("ident"))	// unlisted, get player ident
		{
			Player *xp = h->findPlayer(c.final);

			if (xp)
			{
				String s;
				s += "Player ";
				s += xp->name;
				s += ": ";
				s += xp->ident;
				h->sendPrivate(p, s.msg);
			}
			else
			{
				h->sendPrivate(p, "Cannot see this player");
			}
		}
		else if (c.check("whereflags"))	// unlisted, get flag coords
		{
			_listnode <Flag> *parse = h->flagList.head;
			String s = "Flags: ";
			int team = 10000;

			while (parse)
			{
				Flag *f = parse->item;

				s += " ";

				if (f->team != team)
				{
					team = f->team;

					if (team != UNASSIGNED)
					{
						s += f->team;
					}
					else
					{
						s += "unowned";
					}
					s += ":";
				}

				s += getCoords(f->x, f->y);

				parse = parse->next;
			}

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("logout"))	// unlisted, remove operator powers
		{
			p->access = OP_Player;
		}

	case OP_Limited:	/* FALL THRU */
		if (h->allowLimited && c.check("give"))
		{
			h->revokeAccess(OP_Limited);
			h->limitedOwner = NULL;

			Player *xp = h->findPlayer(c.final);
			if (xp && (xp->access == OP_Player))
			{
				xp->access = OP_Limited;

				String s = "You have been granted bot ownership powers by ";
				s += p->name;
				s += ". Send ::!help for extended commands";
				h->sendPrivate(xp, s.msg);

				h->sendPrivate(p, "Your ownership powers have been successfully transferred");
			}
			else
			{
				h->sendPrivate(p, "Your ownership powers have been successfully released");
			}
		}
	case OP_Player:	/* FALL THRU */
		if (c.check("help"))
		{
			gotHelp(h, p, &c);
		}
/*		else if (c.check("settings"))
		{
			_linkedlist<String> setts;
			setts.append(new String("Misc:SheepMessage:catid wuz here"));

			h->post(generateChangeSettings(setts), true);
		}
*/		else if (c.check("disabled"))
		{
			h->sendPrivate(p, "Forget about it! This command is disabled");
		}
		else if (c.check("botlag"))	// unlisted, display session latency
		{
			// PING Current:BLEH ms  Average:BLEH ms  Low:BLEH ms  High:BLEH ms
			String s;
			s += "PING Current:";
			s += h->syncPing * 10;
			s += " ms  Average:";
			s += h->avgPing * 10;
			s += " ms  Low:";
			s += h->lowPing * 10;
			s += " ms  High:";
			s += h->highPing * 10;
			s += " ms  Delta:";
			s += h->timeDiff * 10;
			s += " ms";

			h->sendPrivate(p, s.msg);
		}
		else if (c.check("sex"))	// unlisted, have sex with bot
		{
			h->sendPrivate(p, SND_Girl, "Unf! Unf! Unf!");
		}
		else if (c.check("sheep"))	// unlisted, sheeply bah
		{
			h->sendPrivate(p, SND_Sheep, "Bah!");
		}
		else if (h->allowLimited && c.check("own"))
		{
			Player *xp = h->limitedOwner;
			if (xp)
			{
				String s;
				s += "Player ";
				s += xp->name;
				s += " has claimed me";

				h->sendPrivate(p, s.msg);
			}
			else if (p->access == OP_Player)
			{
				h->sendPrivate(p, "You are now my owner. Send ::!help for extended commands");

				p->access = OP_Limited;
				h->limitedOwner = p;
			}
			else
			{
				h->sendPrivate(p, "You cannot claim Limited ownership as an operator of higher rank");
			}
		}
		else if (c.check("listop"))
		{
			Operator_Level access = OP_Player;
			Uint32 total = 0;
			bool onlineOnly;
			String l;

			// compute onlineOnly
			if (p->access == OP_Player)
			{
				onlineOnly = true;
			}
			else
			{
				onlineOnly = (c.getParam('o') != NULL);
			}

			if (onlineOnly)
			{	// only online operators
				_listnode <Player> *parse = h->playerlist.head;

				while (parse)
				{
					Player *o = parse->item;
					parse = parse->next;

					if (o->access == OP_Player) continue;

					if (access != o->access)
					{
						l += getLevelString(access = o->access);
						l += ": ";
					}

					l += o->name;

					++total;

					if (parse)
					{
						if (l.len > 80)
						{
							h->sendPrivate(p, l.msg);
							l.clear();
							access = OP_Player;
						}
						else
						{
							l += "  ";
						}
					}
				}
			}
			else
			{	// online and offline operators
				for (auto const & o : h->botInfo.db->opList)
				{
					if (access != o.getAccess())
					{
						l += getLevelString(access = o.getAccess());
						l += ": ";
					}

					Player *xp = h->findPlayer(o.getName());
					if (xp && (xp->access != OP_Player))
						l += "%";

					l += o.getName();

					++total;

					if (l.len > 80)
					{
						h->sendPrivate(p, l.msg);
						l.clear();
						access = OP_Player;
					}
					else
					{
						l += "  ";
					}
				}
			}

			if (total)
				h->sendPrivate(p, l.msg);
			else
				h->sendPrivate(p, "No operators");
		}
		else if (c.check("login"))
		{
			opEntry *op = h->botInfo.db->findOperator(p->name);

			if (op)
			{	// Listed
				if (!h->billerOnline && op->validatePass(""))
				{
					op->addFailure();

					h->sendPrivate(p, "Unable to log you on while the biller is down. This failure has been logged");

					break;
				}

				if (op->validatePass(c.final))
				{	// Grant access

					if (p == h->limitedOwner) h->limitedOwner = NULL;	// reset Limited ownership

					p->access = op->getAccess();
					op->addCounter();

					Uint32 failure = op->getFailureCount();
					Uint32 overall = op->getOverallCount();

					String s;
					s += getLevelString(p->access);
					s += ": Welcome. Sessions  ";

					s += overall;
					if (overall == 1)	s += " logged  ";
					else				s += " logged  ";

					s += failure;
					if (failure == 1)	s += " failed ";
					else				s += " failed ";

					s += " from your handle";

					h->sendPrivate(p, s.msg);
				}
				else
				{	// Bad password
					op->addFailure();

					h->sendPrivate(p, "Bad password. This failure has been logged");

					break;
				}
			}
			else if (p->access > OP_Player)
			{	// Not listed, but has powers already
				h->sendPrivate(p, "Login failure: account revoked. Please contact your senior staff");
			}
		}
	};

	h->imports->talk(makeLocalCommand(p, &c));

#ifndef _DEBUG

} catch (...)	// Catch command-crash bugs
{
	h->sendPrivate(p, SND_Inconceivable, "Oh no!  You've managed to find a command-crash bug!  Report to cat02e@fsu.edu");
}

#endif // _DEBUG

	c.cmd = tcmd;	// Restore pointer from alias string
}

void gotRemoteHelp(Host *h, char *p, Command *c, Operator_Level l)
{
	char *tfinal = c->final;						// Backup original pointer in case this is an alias

	if (!*c->final)
	{
		switch (l)
		{
		case OP_Duke:
		case OP_Baron:
		case OP_King:
		case OP_Emperor:
		case OP_RockStar:
		case OP_Q:
		case OP_God:
		case OP_Owner:
		case OP_SysOp:
			h->sendRemotePrivate(p, "Commands: !help !listop !chat !version !go !close");
			break;
		case OP_SuperModerator:	/* FALL THRU */
			h->sendRemotePrivate(p, "Commands: !help !listop !chat !version !go");
			break;
		case OP_Moderator:	/* FALL THRU */
			h->sendRemotePrivate(p, "Commands: !help !listop !chat !version");
			break;
		};

		h->imports->talk(makeRemoteHelp(p, c, l));

		return;
	}
	else
	{
		h->botInfo.db->aliasCommand(c->final);			// Check aliases
	}

	switch (l)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
	case OP_SysOp:
		if (c->checkParam("close"))
		{
			h->sendRemotePrivate(p, "!close (close all but last bot spawn)");
		}

	case OP_SuperModerator:	/* FALL THRU */
		if (c->checkParam("go"))
		{
			h->sendRemotePrivate(p, "!go <arena> (change bot arena)");
		}

	case OP_Moderator:	/* FALL THRU */
		if (c->checkParam("help"))
		{
			h->sendRemotePrivate(p, "!help [topic] (display available commands and what they do)");
			h->sendRemotePrivate(p, "<> required  [] optional  () comments  -s[=assign] switches");
		}
		else if (c->checkParam("listop"))
		{
			h->sendRemotePrivate(p, "!listop (display online operators)");
		}
		else if (c->checkParam("chat"))
		{
			h->sendRemotePrivate(p, "!chat <channel> (set bot chat channel)");
		}
		else if (c->checkParam("version"))
		{
			h->sendRemotePrivate(p, "!version (retrieve version string/maker)");
		}

	case OP_Limited:	/* FALL THRU */
	case OP_Player:
		if (c->checkParam("disabled"))
		{
			h->sendRemotePrivate(p, "Forget about it! This command is disabled");
		}
		break;
	};

	h->imports->talk(makeRemoteHelp(p, c, l));

	c->final = tfinal;
}

void gotRemote(Host *h, char *p, char *m)
{

#ifdef DISABLE_COMMANDS
	return;											// Compiler directive to ignore commands
#endif

	// Retrieve operator level
	opEntry *op = h->botInfo.db->findOperator(p);

	Operator_Level l;

	if (op)
	{
		l = op->getAccess();
	}
	else
	{
		l = OP_Player;
	}

	// Operators with passwords may not use remote private messages [optional]
	if (!h->botInfo.db->remoteOperator)
	{
		if (op->validatePass(""))
		{
			l = OP_Player;
		}
	}

	// Heavens no!
	if (!h->billerOnline) l = OP_Player;

	// Limiter is in effect
	if (h->lowestLevel > OP_Player)
	{
		if (l < h->lowestLevel)
		{
			return;
		}
	}

	// Process command
	Command c(m);									// Wrap in a friendly class

	char *tcmd = c.cmd;							// Backup original pointer in case this is an alias
	h->botInfo.db->aliasCommand(c.cmd);				// Check aliases

#ifndef _DEBUG

try {												// Catch all command-crash bugs

#endif // _DEBUG

	switch (l)
	{
	case OP_Duke:
	case OP_Baron:
	case OP_King:
	case OP_Emperor:
	case OP_RockStar:
	case OP_Q:
	case OP_God:
	case OP_Owner:
	case OP_SysOp:
		if (c.check("close"))
		{
			h->disconnect(true);
		}

	case OP_SuperModerator:	/* FALL THRU */
		if (c.check("go"))
		{
			if (invalidArena(c.final))
			{
				String s;
				s += "Denied! Invalid arena name \'";
				s += c.final;
				s += "\'";
				h->sendRemotePrivate(p, s.msg);

				break;
			}

			String s;
			s += "Moving to \'";
			s += c.final;
			s += "\'";
			h->sendRemotePrivate(p, s.msg);

			h->botInfo.setArena(c.final, h->botInfo.initialShip, h->botInfo.xres, h->botInfo.yres, h->botInfo.allowAudio);

			h->changeArena(c.final);
			break;
		}

	case OP_Moderator:	/* FALL THRU */
		if (c.check("help"))
		{
			gotRemoteHelp(h, p, &c, l);
			break;
		}
		else if (c.check("listop"))
		{
			Operator_Level access = OP_Player;
			Uint32 total = 0;
			String l;

			_listnode <Player> *parse = h->playerlist.head;

			while (parse)
			{
				Player *o = parse->item;
				parse = parse->next;

				if (o->access == OP_Player) continue;

				if (access != o->access)
				{
					l += getLevelString(access = o->access);
					l += ": ";
				}

				l += o->name;

				++total;

				if (parse)
				{
					if (l.len > 80)
					{
						h->sendRemotePrivate(p, l.msg);
						l.clear();
						access = OP_Player;
					}
					else
					{
						l += "  ";
					}
				}
			}

			if (total)
				h->sendRemotePrivate(p, l.msg);
			else
				h->sendRemotePrivate(p, "No operators");
			break;
		}
		else if (c.check("chat"))
		{
			if (*c.final)
			{
				if (h->broadcastingErrors)
				{
					h->sendRemotePrivate(p, "Cannot change chat channel while broadcasting errors");
				}
				else
				{
					String s = "?chat=";
					s += c.final;
					h->sendPublic(s.msg);

					s = "chat=";
					s += c.final;
					h->sendRemotePrivate(p, s.msg);
				}
			}
			else
			{
				h->sendRemotePrivate(p, "You must provide channel name(s): !chat squad_chat,newbie_chat,zone_chat");
			}
			break;
		}
		else if (c.check("version"))
		{
			h->sendRemotePrivate(p, VERSION_STRING);
			break;
		}

	case OP_Limited:	/* FALL THRU */
	case OP_Player:
		if (c.check("help"))
		{
			gotRemoteHelp(h, p, &c, l);
			break;
		}
		else if (c.check("lag"))	// unlisted, display session latency
		{
			String s;
			s += "PING Current:";
			s += h->syncPing * 10;
			s += " ms  Average:";
			s += h->avgPing * 10;
			s += " ms  Low:";
			s += h->lowPing * 10;
			s += " ms  High:";
			s += h->highPing * 10;
			s += " ms  Delta:";
			s += h->timeDiff * 10;
			s += " ms";

			h->sendRemotePrivate(p, s.msg);
			break;
		}
		else if (c.check("disabled"))
		{
			h->sendRemotePrivate(p, "Forget about it! This command is disabled");
		}
	};

	h->imports->talk(makeRemoteCommand(p, &c, l));

#ifndef _DEBUG

} catch (...)	// Catch command-crash bugs
{
	h->sendRemotePrivate(p, "Oh no!  You've managed to find a command-crash bug!  Report to cat02e@fsu.edu");
}

#endif // _DEBUG

	c.cmd = tcmd;	// Restore pointer from alias string
}
