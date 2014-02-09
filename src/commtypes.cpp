#include "commtypes.h"

#include <cctype>
#include <string.h>

#include "algorithms.h"


//////// Validation ////////

bool invalidName(char *name)
{
	Uint32 len = STRLEN(name);
	bool seenSpace = false;

	// Name cannot be blank
	if (len == 0) return true;

	// Keep name under 20 characters
	if (len > 19)
	{
		name[19] = '\0';
		len = 19;
	}

	// First character must be alphanumeric
	if (!isAlphaNumeric(*name)) return true;

	// Cannot end in a space
	if (name[len - 1] == ' ') return true;

	for (Uint32 i = 0; i < len; ++i)
	{
		// Only legal & printable characters
		if ( !isPrintable(name[i])		||
		   (  name[i] == ':'     )		||
		   (  name[i] == '%'     )		 )
		{
			return true;
		}

		// Only one space in a row
		if (name[i] == ' ')
		{
			if (seenSpace)
				return true;
			else
				seenSpace = true;
		}
		else
		{
			seenSpace = false;
		}
	}

	return false;
}

bool invalidArena(char *name)
{
	Uint32 len = STRLEN(name);

	// Blank arena names are permissable
	if (len == 0) return false;

	// Keep name under 11 characters
	if (len > 10)
	{
		name[10] = '\0';
		len = 10;
	}

	// Skip valid prefixes
	Uint32 i = 0;

	if (*name == '#') ++i;

	// Prefix must be followed by arena name
	for (; i < len; ++i)
	{
		// Only alpha-numeric characters
		if (!isAlphaNumeric(name[i]))
		{
			return true;
		}
	}

	return false;
}

bool validRemoteChat(char *text)
{
	bool seen = false;

	for (Uint16 i = 0; i < STRLEN(text); ++i)
	{
		if ((text[i] == '>') &&
			(text[i+1] == ' '))
		{
			if (seen) return false;
			seen = true;
		}
	}

	return seen;
}

bool validRemotePrivate(char *text)
{
	bool seen = false;

	for (Uint16 i = 0; i < STRLEN(text); ++i)
	{
		if ((text[i] == ')') &&
			(text[i+1] == '>'))
		{
			if (seen) return false;
			seen = true;
		}
	}

	return seen;
}

char *getChatName(char *text)
{
	Uint16 i;

	for (i = 0; i < STRLEN(text); ++i)
	{
		if (text[i] == ':')
		{
			text += (i + 1);
			break;
		}
	}

	for (i = 0; i < STRLEN(text); ++i)
	{
		if ((text[i] == '>') &&
			(text[i+1] == ' '))
		{
			text[i] = 0;
			break;
		}
	}

	return text;
}

char *getRemoteName(char *text)
{
	for (Uint16 i = 0; i < STRLEN(text); ++i)
	{
		if ((text[i] == ')') &&
			(text[i+1] == '>'))
		{
			text[i] = 0;
			break;
		}
	}

	return text + 1;
}

char *getChatCommand(char *text)
{
	for (Uint16 i = 0; i < STRLEN(text); ++i)
	{
		if ((text[i] == '>') &&
			(text[i+1] == ' '))
		{
			return text + i + 2;
		}
	}

	return text;
}

char *getRemoteCommand(char *text)
{
	for (Uint16 i = 0; i < STRLEN(text); ++i)
	{
		if ((text[i] == ')') &&
			(text[i+1] == '>'))
		{
			return text + i + 2;
		}
	}

	return text;
}


///////// Switches ////////

_switch::_switch(char t, char *p)
{
	Uint32 len = 0;
	while (p[len++]);	// len == STRLEN(msg) + 1

	param = new char[len];
	if (param) memcpy(param, p, len);

	type = t;
}

_switch::~_switch()
{
	if (param)
	{
		delete []param;
		param = NULL;
	}
}


//////// Command parsing ////////

Command::Command(char *msg)
{
	Uint32 len = 0;
	while (msg[len++]);	// len == STRLEN(msg) + 1
	char *tmp = new char[len];
	Uint32 index = 0;

	cmd = new char[len];
	cmd[0] = '\0';

	final = new char[len];
	final[0] = '\0';

	bool inStub  = true,
			inParam = false,
			inFinal = false,
			seenSpace = false;

	for (Uint32 i = 0; i < len; ++i)
	{
		char c = msg[i];

		switch (c)
		{
		case ' ':
			{
				if (inStub)
				{
					tmp[index++] = 0;

					memcpy(cmd, tmp, index);

					inStub = false;

					index = 0;
				}
				else if (inParam)
				{
					tmp[index++] = 0;

					addParam(tmp);

					inParam = false;

					index = 0;
				}
				else if (inFinal)
					tmp[index++] = c;

				seenSpace = true;
			}
			break;
		case '-':
			{
				if (inStub)
				{
					tmp[index++] = 0;

					memcpy(cmd, tmp, index);

					inStub = false;
					inParam = true;

					index = 0;
				}
				else if (inParam || inFinal)
					tmp[index++] = c;
				else
					inParam = true;

				seenSpace = false;
			}
			break;
		case '\0':
			{
				tmp[index++] = 0;

				if (inStub)
					memcpy(cmd, tmp, index);
				else if (inParam)
					addParam(tmp);
				else
					memcpy(final, tmp, index);

				seenSpace = false;

				delete []tmp;
				tmp = NULL;
			}
			return;
		default:
			if (inStub)
				c = std::tolower(c);
			else if (seenSpace)
			{
				inFinal = true;

				seenSpace = false;
			}

			tmp[index++] = c;
		};
	}
}

bool Command::check(char *msg)
{
	Uint32 i = 0;
	char c;

	do
	{
		c = *msg++;

		if (c != cmd[i++])
			return false;
	} while (c);

	return true;
}

bool Command::checkParam(char *msg)
{
	Uint32 i = 0;
	char c;

	do
	{
		c = *msg++;

		if (c != std::tolower(final[i++]))
			return false;
	} while (c);

	return true;
}

_switch *Command::getParam(char type)
{
	_listnode <_switch> *parse = slist.head;

	while (parse)
	{
		_switch *s = parse->item;

		if (s->type == type)
			return s;

		parse = parse->next;
	}

	return NULL;
}

void Command::addParam(char *msg)
{
	Uint32 index = 0,
		   len   = 0;
	bool inParams = true;
	while (msg[len++]);	// len == STRLEN(msg) + 1

	char *tmp = new char[len];
	char type = '\0';

	for (Uint32 i = 0; i < len; ++i)
	{
		char c = msg[i];

		switch (c)
		{
		case '-':
			{
				if (inParams == false)
				{
					tmp[index++] = c;
				}
			}
		case '=':	/* FALL THRU */
			{
				inParams = false;
			}
			break;
		case '\0':
			{
				if (type && (inParams == false))
				{
					tmp[index++] = 0;

					slist.kill(slist.tail);
					_switch *s = new _switch(type, tmp);
					if (s) slist.append(s);
				}
			}
			return;
		default:
			if (inParams)
			{
				type = std::tolower(c);
				_switch *s = new _switch(type, "");
				if (s) slist.append(s);
			}
			else
			{
				tmp[index++] = c;
			}
		};
	}
}

Command::~Command()
{
	delete []cmd;
	cmd = NULL;
	delete []final;
	final = NULL;
}
