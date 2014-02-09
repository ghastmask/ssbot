#include "botdll.h"

#include "datatypes.h"
#include "algorithms.h"
#include "clientprot.h"
#include "host.h"
#include "botdb.h"

typedef void (__stdcall *pfnEnsureInit)(void);
typedef void (__stdcall *pfnForceTerm)(void);

void DLLImports::clearImport(int slot)
{
	if (slot < 0 || slot >= DLL_MAX_LOADED)
		return;

	if (DLL_TALK[slot])
	{
		talk(slot, makeTerm());

		// ** Mixed-mode plugin shutdown **
		// NOTE: This function is not strictly required for mixed-mode DLLs but it is the approach that Microsoft recommends.
		// Any mixed-mode DLL should terminate the C runtime in this function.
		pfnForceTerm pfnDll = (pfnForceTerm)GetProcAddress(DLLhMod[slot], "DllForceTerm");
		if (pfnDll)
			pfnDll();
		// ********************************

		FreeLibrary(DLLhMod[slot]);

		DLLhMod[slot] = NULL;
		DLL_TALK[slot] = NULL;
	}
}

void DLLImports::clearImports()
{
	for (int slot = 0; slot < DLL_MAX_LOADED; ++slot)
	{
		clearImport(slot);
	}
}

void __stdcall listen(BotEvent &event)
{
	Host *h = (Host*)event.handle;

#ifndef _DEBUG
try {
#endif

	switch (event.code)
	{
	case EVENT_Echo:
		{
			char *msg = (char*)event.p[0];

			h->logEvent("Ext: %s", msg);
		}
		break;
	case EVENT_Say:
		{
			int mode = *(int*)&event.p[0];
			int sound = *(int*)&event.p[1];
			int ident = *(int*)&event.p[2];
			char *msg = (char*)event.p[3];

			// Fix team private messages, needs a player ident param, not a team number
			if (mode == MSG_TeamPrivate)
			{
				_listnode <Player> *parse = h->playerlist.head;

				while (parse)
				{
					Player *p = parse->item;

					if (p->team == ident)
					{
						ident = p->ident;

						h->tryChat(mode, sound, ident, msg);

						return;
					}

					parse = parse->next;
				}

				break;
			}

			h->tryChat(mode, sound, ident, msg);
		}
		break;
	case EVENT_Ship:
		{
			int ship = *(int*)&event.p[0];

			h->postRR(generateChangeShip((Ship_Types)ship));
		}
		break;
	case EVENT_Team:
		{
			int team = *(int*)&event.p[0];

			h->postRR(generateChangeTeam(team));
		}
		break;
	case EVENT_Die:
		{
			Player *p = (Player*)event.p[0];

			if (h->Me)
				h->postRR(generateDeath(p->ident, h->Me->bounty));
		}
		break;
	case EVENT_Attach:
		{
			Player *p = (Player*)event.p[0];

			h->postRR(generateAttachRequest(p->ident));
		}
		break;
	case EVENT_Detach:
		{
			h->postRR(generateAttachRequest(UNASSIGNED));
		}
		break;
	case EVENT_Following:
		{
			bool following = *(bool*)&event.p[0];

			h->follow = !following ? NULL : h->playerlist.head;
		}
		break;
	case EVENT_Flying:
		{
			bool flying = *(bool*)&event.p[0];

			h->DLLFlying = flying;
		}
		break;
	case EVENT_DropBrick:
		{
			h->postRU(generateBrickDrop((Uint16)h->Me->tile.x, (Uint16)h->Me->tile.y));
		}
		break;
	case EVENT_Banner:
		{
			BYTE *buffer = (BYTE*)event.p[0];

			h->postRR(generateChangeBanner(buffer));
		}
		break;
	case EVENT_GrabFlag:
		{
			int flag = *(int*)&event.p[0];

			h->postRR(generateFlagRequest(flag));
		}
		break;
	case EVENT_SendPosition:
		{
			bool reliable = *(bool*)&event.p[0];

			h->sendPosition(reliable);
		}
		break;
	case EVENT_FireWeapon:
		{
			weaponInfo *wi = (weaponInfo*)event.p[0];

			h->sendPosition(false, h->getHostTime(), wi->type, wi->level, wi->shrapBounce, wi->shrapLevel, wi->shrapCount, wi->fireType);
		}
		break;
	case EVENT_DropFlags:
		{
			h->postRR(generateFlagDrop());
		}
		break;
	case EVENT_ChangeArena:
		{
			String name = (char*)event.p[0];

			h->botInfo.setArena(name.msg, h->botInfo.initialShip, h->botInfo.xres, h->botInfo.yres, h->botInfo.allowAudio);

			if (!invalidArena(name.msg))
				h->changeArena(name.msg);
		}
		break;
	case EVENT_MoveObjects:
		{
			lvzObject *objects = (lvzObject *)event.p[0];
			int num_objects = *(int*)&event.p[1];
			int player_id = *(int*)&event.p[2];

			if (num_objects < 0)
				break;

			h->postRR(generateObjectModify(player_id, objects, num_objects));
		}
		break;
	case EVENT_GrabBall:
		{
			int id = *(int*)&event.p[0];

			_listnode<PBall> *parse = h->ballList.head;

			while (parse)
			{
				PBall *pb = parse->item;

				if (id == pb->ident)
				{
					h->postRR(generatePowerballRequest(pb->hosttime, id));
					break;
				}

				parse = parse->next;
			}
		}
		break;
	case EVENT_FireBall:
		{
			int id = *(int*)&event.p[0];
			int x = *(int*)&event.p[1];
			int y = *(int*)&event.p[2];
			int xvel = *(int*)&event.p[3];
			int yvel = *(int*)&event.p[4];

			h->postRR(generatePowerballUpdate(h->getHostTime(), id, x, y, xvel, yvel, 0xffff));
			// Cyan~Fire noticed this didn't compile well in MinGW
		}
		break;
	case EVENT_ToggleObjects:
		{
			objectInfo *objects = (objectInfo *)event.p[0];
			int num_objects = *(int*)&event.p[1];
			int player_id = *(int*)&event.p[2];

			if (num_objects < 0)
				break;

			if (h->hasSysOp)
			{
				h->postRR(generateObjectToggle(player_id, objects, num_objects));
			}
			else
			{
				String s;

				s += "*objset ";

				for (int i = 0; i < num_objects; ++i)
				{
					if (objects[i].disabled)
					{
						s += "-";
					}
					else
					{
						s += "+";
					}

					s += objects[i].id;
					s += ",";
				}

				if (player_id == UNASSIGNED)
				{
					h->tryChat(MSG_Public, SND_None, 0, s.msg);
				}
				else
				{
					h->tryChat(MSG_Private, SND_None, player_id, s.msg);
				}
			}
		}
		break;
	case EVENT_ChangeSettings:
		{
			_linkedlist <String> *settings = (_linkedlist <String> *)event.p[0];

			h->postRR(generateChangeSettings(*settings));
		}
		break;
	case EVENT_SpawnBot:
		{
			if (h->botInfo.db->spawns.getConnections() >= h->botInfo.maxSpawns)
				break;

			BOT_INFO bi;
			bi.set(h->botInfo);
 
			String name		= (char*)event.p[0];
			String password = (char*)event.p[1];
			String staff	= (char*)event.p[2];
			String arena	= (char*)event.p[3];

			// Name
			if (invalidName(name.msg))
				break;

			// Password
			if (password.IsEmpty())
				password = bi.password;

			// Staff
			if (staff.IsEmpty())
				staff = bi.staffpw;

			// Arena
			if (arena.IsEmpty())
				arena = bi.initialArena;

			if (invalidArena(arena.msg))
				break;

			bi.setLogin(name.msg, password.msg, staff.msg);
			bi.setArena(arena.msg, bi.initialShip, bi.xres, bi.yres, bi.allowAudio);

			bi.db->spawns.connectHost(bi);
		}
		break;
	}

#ifndef _DEBUG
} catch (...)
{ h->logEvent("ERROR: Exception during EVENT %i", event.code); }
#endif

}

bool DLLImports::importLibrary(char *files)
{
	String s = files;

	String plugin = s.split(',');

	if (plugin != s)
		importLibrary(s.msg);

	int slot;

	// Find open import slot
	for (slot = 0; slot < DLL_MAX_LOADED; ++slot)
		if (!DLL_TALK[slot])
			break;

	if (slot == DLL_MAX_LOADED)
		return false;

	// Avoid directory traversal exploits.
	trimString('/', plugin);
	trimString('\\', plugin);

#ifndef _DEBUG
try {
#endif

	DLLhMod[slot] = LoadLibrary(plugin.msg);

#ifndef _DEBUG
} catch (...)
{ h->logEvent("ERROR: Exception in DLLMain while loading plugin %s at slot %i", plugin.msg, slot); }
#endif

	if (!DLLhMod[slot]) return false;

	// ** Mixed-mode plugin initialization **
	// NOTE: This function is not strictly required for mixed-mode DLLs but it is the approach that Microsoft recommends.
	// Any mixed-mode DLL should initialize the C runtime in this function.
	pfnEnsureInit pfnDll = (pfnEnsureInit)GetProcAddress(DLLhMod[slot], "DllEnsureInit");
	if (pfnDll)
		pfnDll();
	// **************************************

	strncpy(ModuleName[slot], plugin.msg, DLL_NAMELEN);

	DLL_TALK[slot] = (CALL_TALK)GetProcAddress(DLLhMod[slot], (LPCSTR)1);

	talk(slot, makeInit(&listen, &(h->playerlist), &(h->flagList), (CALL_MAP)(h->map), &(h->brickList), (CALL_PARAMS)h->creation_parameters));
	if (h->inArena)
	{
		talk(slot, makeArenaEnter(h->botInfo.initialArena, h->Me, h->billerOnline));
		talk(makeArenaSettings(&h->settings));
	}

	return true;
}

void DLLImports::talk(BotEvent event)
{
	for (int slot = 0; slot < DLL_MAX_LOADED; ++slot)
	{
		talk(slot, event);
	}
}

void DLLImports::talk(int slot, BotEvent event)
{
	if (slot < 0 || slot >= DLL_MAX_LOADED)
		return;

	if (DLL_TALK[slot])
	{
		event.handle = h;

#ifndef _DEBUG
try {
#endif

		DLL_TALK[slot](event);

#ifndef _DEBUG
} catch (...)
{ h->logEvent("ERROR: Exception in plugin %s at slot %i during event %i", ModuleName[slot], slot, event.code); }
#endif

	}
}

char *DLLImports::getPlugin(int slot)
{
	if (slot < 0 || slot >= DLL_MAX_LOADED)
		return NULL;
	if (!DLL_TALK[slot]) return NULL;

	return ModuleName[slot];
}

DLLImports::DLLImports(Host &host)
{
	for (int slot = 0; slot < DLL_MAX_LOADED; ++slot)
	{
		DLL_TALK[slot] = NULL;
		DLLhMod[slot] = NULL;
	}

	h = &host;
}

DLLImports::~DLLImports()
{
	clearImports();
}
