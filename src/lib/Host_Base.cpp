#include "Host_Base.h"
#include "player.h"

Host_Base::
Host_Base(BOT_INFO & info)
{
	botInfo = info;
}


Player *Host_Base::findPlayer(const char *name)
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
