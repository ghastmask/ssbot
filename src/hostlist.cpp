#include "hostlist.h"

#include "clientprot.h"
#include "host.h"
#include "botinfo.h"


//////// Host list ////////

hostList::hostList()
{
	maximumHosts = 0;
}

hostList::~hostList()
{
}

void hostList::doEvents()
{
	_listnode <Host> *parse = list.head;

	while (parse)
	{
		Host *h = parse->item;
		parse = parse->next;

		h->doEvents();

		if (h->killMe)
		{
			list.kill(h);
		}
	}
}

Uint32 mix_ctr = 0;

bool hostList::connectHost(BOT_INFO &botInfo)
{
	if (!findSpawn(botInfo.name))
	{
		Uint32 ip = botInfo.ip;

		if ((ip & 0x000000ff) == 0x0000007f) // 127.x.x.x
		{
			ip += mix_ctr;

			if ((ip & 0xff000000) == 0)
			{
				ip += 0x01000000;
				mix_ctr += 0x01000000;
			}

			botInfo.ip = ip;
			mix_ctr += 0x00000100;
		}

		Host *h = new Host(botInfo);
		if (h)
		{
			list.append(h);
			return true;
		}
	}

	return false;
}

void hostList::massDisconnect()
{
	_listnode <Host> *parse = list.head;

	while (parse)
	{
		Host *h = parse->item;
		parse = parse->next;

		h->disconnect(true);
	}
}

void hostList::massRestart()
{
	_listnode <Host> *parse = list.head;

	while (parse)
	{
		Host *h = parse->item;
		parse = parse->next;

		// simulate a server disconnect
		h->disconnect(true);
		h->gotDisconnect();
		h->killMe = false;
	}
}

Uint32 hostList::getConnections()
{
	return list.total;
}

Host *hostList::findSpawn(char *name)
{
	_listnode <Host> *parse = list.head;

	while (parse)
	{
		Host *h = parse->item;
		parse = parse->next;

		if (CMPSTR(h->botInfo.name, name)) return h;
	}

	return NULL;
}
