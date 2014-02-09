/*
	Advanced datatypes by cat02e@fsu.edu
*/


#ifndef DATATYPES_H
#define DATATYPES_H

typedef unsigned char	 BYTE;
typedef unsigned char	 uchar;
typedef unsigned char	 Uint8;
typedef unsigned short	 ushort;
typedef unsigned short	 Uint16;
typedef unsigned int	 uint;
typedef unsigned long	 ulong;
typedef unsigned long	 Uint32;

typedef signed char		 Sint8;
typedef signed short	 Sint16;
typedef signed long		 Sint32;

// from Cyan~Fire's MinGW patches
#ifdef __GNUC__
	typedef signed long long	Sint64;
	typedef unsigned long long	Uint64;
#else
	typedef signed __int64	 Sint64;
	typedef unsigned __int64 Uint64;
#endif

#ifndef TwoPI
	#define TwoPI 6.2831853072
#endif

#ifndef PI
	#define PI 3.1415926536
#endif

#ifndef WHEN_COWS_FLY
	#define WHEN_COWS_FLY if (false)
#endif

#ifndef PIOverTwo
	#define PIOverTwo 1.5707963268
#endif

#ifndef PIOverFour
	#define PIOverFour 0.7853981634
#endif

#ifndef NULL
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif

union _regs
{	// Snrrrubteq
	struct
	{
		uint eax, ebx, ecx, edx, esi, edi, ebp, esp;
	} d;
	struct
	{
		ushort ax, hax, bx, hbx, cx, hcx, dx, hdx, si, hsi, di, hdi, bp, hbp, sp, hsp;
	} x;
	struct
	{
		uchar al, ah, hax[2], bl, bh, hbx[2], cl, ch, hcx[2], dl, dh, hdx[2], rest[16];
	} h;
};


Uint32 getLong(BYTE *Message, const Uint32 Offset);			// Get a LONG from a BYTE * field

Uint16 getShort(BYTE *Message, const Uint32 Offset);		// Get a SHORT from a BYTE * field

Uint32 getLong(char *Message, const Uint32 Offset);			// Get a LONG from a char * field

Uint16 getShort(char *Message, const Uint32 Offset);		// Get a SHORT from a char * field

BYTE getByte(char *Message, const Uint32 Offset);			// Get a BYTE from a char * field


#define TEMPLATE template <class AnonymousStruct>

TEMPLATE class _listnode
{
public:
	AnonymousStruct *item;
	_listnode		*next;
	_listnode		*last;

	_listnode(AnonymousStruct *nItem);
	~_listnode();
};


TEMPLATE class _linkedlist
{
public:
	_listnode <AnonymousStruct> *head;
	_listnode <AnonymousStruct> *tail;

	void prepend(AnonymousStruct *item);
	void append(AnonymousStruct *item);
	void kill(_listnode <AnonymousStruct> *node);
	void kill(AnonymousStruct *item);
	void unlist(_listnode <AnonymousStruct> *node);
	void unlist(AnonymousStruct *item);
	void insertAfter(_listnode <AnonymousStruct> *node, AnonymousStruct *item);
	void insertBefore(_listnode <AnonymousStruct> *node, AnonymousStruct *item);
	_listnode <AnonymousStruct> *find(AnonymousStruct *item);
	void clear();

	void beginCyclic();
	void endCyclic();

	Uint32 total;
	bool copy;

	_linkedlist();
	_linkedlist(bool thisisacopy);
	_linkedlist(_linkedlist <AnonymousStruct> &nlist);
	~_linkedlist();
};


TEMPLATE class _jumptable
{
	// "Remote Procedure Call" implementation
	// fixed for MinGW by Cyan~Fire
	typedef void (__stdcall * template_func)(AnonymousStruct *);
	template_func RPC[256];
	// void (__stdcall * RPC [256])(AnonymousStruct *);

public:
	_jumptable();											// Initialize to zero

	void clear();											// Reset list
	void add(BYTE index, void (__stdcall *)(AnonymousStruct *));		// Create a handler
	void kill(BYTE index);									// Delete a handler
	bool call(BYTE index, AnonymousStruct *params);			// Call a handler (false == unhandled)
};


TEMPLATE class _referencetable
{
	AnonymousStruct *LIST[65536];							// Fix by Snrrrub - UID is 16 bits long

public:
	_referencetable();										// Clear list

	void clear();											// Zero list memory
	void add(Uint16 index, AnonymousStruct *data);			// Add an item
	void kill(Uint16 index);								// Nuke an item
	AnonymousStruct *get(Uint16 index);						// Get an item
};

Uint32 STRLEN(const char *string);								// strlen() clone

bool CMPSTART(const char *control, const char *constant);				// does constant start with control?

bool CMPSTR(const char *a, const char *b);								// binary compare


class String
{
public:
	void append(const char *buffer, Uint32 nlen);
	void prepend(const char *buffer, Uint32 nlen);
	void set(const char *buffer, Uint32 nlen);
	void clear();
	void replace(char delimiter, char replaceChar);

	char *msg;
	Uint32 len;

	String();
	String(const char *s);
	String(const char *s, Uint32 slen);
	String(const Uint32 s);
	String(const String &s);
	~String();

#ifdef STRING_CAST_CHAR
	operator char*();
#endif

	bool operator==(const char *string);
	bool operator==(Uint32 string);
	bool operator==(const String &string);

	bool operator!=(const char *string);
	bool operator!=(Uint32 string);
	bool operator!=(const String &string);

	void operator=(const char *string);
	void operator=(Uint32 string);
	void operator=(const String &string);

	void operator+=(const char *string);
	void operator+=(Uint32 string);
	void operator+=(const String &string);

	String operator+(const char *string);
	String operator+(Uint32 string);
	String operator+(const String &string);

	String left(Uint32 len);				// take left
	String right(Uint32 len);				// take right
	String mid(Uint32 offset, Uint32 len);	// take from index
	String split(char delimiter);			// returns left, keeps right
	String trim();							// trim spaces
	String ltrim();							// trim spaces left
	String rtrim();							// trim spaces right
	String lfill(Uint32 len);				// fill spaces to the left
	String rfill(Uint32 len);				// fill spaces to the right

	int firstInstanceOf(char c);

	void lcase();
	void ucase();

	int toInteger();
	int toInteger(int base);

	bool IsEmpty();
};

String operator+(const char *prepended, const String &string);

class Vector
{
public:
	Sint32 x, y;

	Vector();
	Vector(const Vector &v);
	Vector(Sint32 x, Sint32 y);

	bool operator==(const Vector &v);
	bool operator!=(const Vector &v);

	void operator=(const Vector &v);
	void operator=(Sint32 n);

	void operator+=(const Vector &v);
	void operator-=(const Vector &v);
	void operator/=(const Vector &v);
	void operator*=(const Vector &v);

	Vector operator+(const Vector &v);
	Vector operator-(const Vector &v);
	Vector operator/(const Vector &v);
	Vector operator*(const Vector &v);

	void operator+=(Sint32 n);
	void operator-=(Sint32 n);
	void operator/=(Sint32 n);
	void operator*=(Sint32 n);

	Vector operator+(Sint32 n);
	Vector operator-(Sint32 n);
	Vector operator/(Sint32 n);
	Vector operator*(Sint32 n);

	void set(Sint32 x, Sint32 y);

	Sint32 distance(const Vector &v);
};


//////// Linked list node ////////

TEMPLATE _listnode <AnonymousStruct>::_listnode (AnonymousStruct *nItem)
{
	item = nItem;

	next = NULL;
	last = NULL;
}

TEMPLATE _listnode <AnonymousStruct>::~_listnode()
{
	if (item)
	{
		delete item;
		item = NULL;
	}
}


//////// Linked list ////////

TEMPLATE _linkedlist <AnonymousStruct>::_linkedlist()
{
	total	= 0;

	head	= NULL;
	tail	= NULL;

	copy	= false;
}

TEMPLATE _linkedlist <AnonymousStruct>::_linkedlist(bool thisisacopy)
{
	total	= 0;

	head	= NULL;
	tail	= NULL;

	copy	= thisisacopy;
}

TEMPLATE _linkedlist <AnonymousStruct>::_linkedlist(_linkedlist <AnonymousStruct> &nlist)
{
	total	= 0;

	head	= NULL;
	tail	= NULL;

	copy	= true;

	_listnode <AnonymousStruct> *parse = nlist.head;

	while (parse)
	{
		AnonymousStruct *as = parse->item;
		parse = parse->next;

		append(as);
	}
}

TEMPLATE void _linkedlist <AnonymousStruct>::beginCyclic()
{
	tail->next = head;
}

TEMPLATE void _linkedlist <AnonymousStruct>::endCyclic()
{
	tail->next = NULL;
}

TEMPLATE inline _linkedlist <AnonymousStruct>::~_linkedlist()
{
	clear();
}

TEMPLATE void _linkedlist <AnonymousStruct>::clear()
{
	if (head)
	{
		_listnode <AnonymousStruct> *parse = head;
		_listnode <AnonymousStruct> *next;

		if (copy)
		{
			while (parse)
			{
				next = parse;

				parse = parse->next;

				next->item = NULL;
				delete next;
				next = NULL;

				if (parse == head) break;
			}
		}
		else
		{
			while (parse)
			{
				next = parse;

				parse = parse->next;

				delete next;
				next = NULL;

				if (parse == head) break;
			}
		}

		total = 0;

		head = NULL;
	}

	tail = NULL;
}

TEMPLATE void _linkedlist <AnonymousStruct>::prepend(AnonymousStruct *item)
{
	_listnode <AnonymousStruct> *new_head = new _listnode <AnonymousStruct> (item);

	new_head->last = NULL;
	new_head->next = head;

	if (tail == NULL)
		tail = new_head;
	else
		head->last = new_head;

	head = new_head;

	total++;
}

TEMPLATE void _linkedlist <AnonymousStruct>::append(AnonymousStruct *item)
{
	_listnode <AnonymousStruct> *new_tail = new _listnode <AnonymousStruct> (item);

	new_tail->last = tail;
	new_tail->next = NULL;

	if (head == NULL)
		head = new_tail;
	else
		tail->next = new_tail;

	tail = new_tail;

	total++;
}

TEMPLATE void _linkedlist <AnonymousStruct>::insertAfter(_listnode <AnonymousStruct> *node, AnonymousStruct *item)
{
	if (node == tail)
	{
		append(item);
	}
	else
	{
		_listnode <AnonymousStruct> *lastnode = node;
		_listnode <AnonymousStruct> *nextnode = node->next;
		_listnode <AnonymousStruct> *new_node = new _listnode <AnonymousStruct> (item);

		new_node->last = lastnode;
		new_node->next = nextnode;
		if (lastnode) lastnode->next = new_node;
		if (nextnode) nextnode->last = new_node;

		total++;
	}
}

TEMPLATE void _linkedlist <AnonymousStruct>::insertBefore(_listnode <AnonymousStruct> *node, AnonymousStruct *item)
{
	if (node == head)
	{
		prepend(item);
	}
	else
	{
		_listnode <AnonymousStruct> *lastnode = node->last;
		_listnode <AnonymousStruct> *nextnode = node;
		_listnode <AnonymousStruct> *new_node = new _listnode <AnonymousStruct> (item);

		new_node->last = lastnode;
		new_node->next = nextnode;
		if (lastnode) lastnode->next = new_node;
		if (nextnode) nextnode->last = new_node;

		total++;
	}
}

TEMPLATE void _linkedlist <AnonymousStruct>::kill(_listnode <AnonymousStruct> *node)
{
	if (tail == node)
	{
		tail = tail->last;
		if (tail) tail->next = NULL;
	}
	else
	{
		if (node->last) node->last->next = node->next;
	}

	if (head == node)
	{
		head = head->next;
		if (head) head->last = NULL;
	}
	else
	{
		if (node->next) node->next->last = node->last;
	}

	if (copy)
		node->item = NULL;

	delete node;
	node = NULL;

	total--;
}

TEMPLATE void _linkedlist <AnonymousStruct>::kill(AnonymousStruct *item)
{
	_listnode <AnonymousStruct> *parse = head;

	while (parse)
	{
		if (parse->item == item)
		{
			kill(parse);
			return;
		}

		parse = parse->next;
		if (parse == head) break;
	}
}

TEMPLATE _listnode <AnonymousStruct> *_linkedlist <AnonymousStruct>::find(AnonymousStruct *item)
{
	_listnode <AnonymousStruct> *parse = head;

	while (parse)
	{
		if (parse->item == item)
		{
			return parse;
		}

		parse = parse->next;
		if (parse == head) break;
	}

	return NULL;
}

TEMPLATE void _linkedlist <AnonymousStruct>::unlist(_listnode <AnonymousStruct> *node)
{
	copy = true;
	kill(node);
	copy = false;
}

TEMPLATE void _linkedlist <AnonymousStruct>::unlist(AnonymousStruct *item)
{
	copy = true;
	kill(item);
	copy = false;
}


//////// Jump table ////////

TEMPLATE inline _jumptable <AnonymousStruct>::_jumptable()
{
	clear();
}

TEMPLATE inline void _jumptable <AnonymousStruct>::clear()
{
	for (int i = 0; i < 256; ++i)
		RPC[i] = NULL;
}

TEMPLATE inline void _jumptable <AnonymousStruct>::add(BYTE index, void (__stdcall *function)(AnonymousStruct *))
{
	RPC[index] = function;
}

TEMPLATE inline void _jumptable <AnonymousStruct>::kill(BYTE index)
{
	RPC[index] = NULL;
}

TEMPLATE inline bool _jumptable <AnonymousStruct>::call(BYTE index, AnonymousStruct *params)
{
	if (RPC[index])
	{
		RPC[index](params);

		return true;
	}

	return false;
}


//////// Reference table ////////

TEMPLATE inline _referencetable <AnonymousStruct>::_referencetable()
{
	clear();
}

TEMPLATE inline void _referencetable <AnonymousStruct>::clear()
{
	for (int i = 0; i < 65536; ++i)
		LIST[i] = NULL;
}

TEMPLATE inline void _referencetable <AnonymousStruct>::add(Uint16 index, AnonymousStruct *data)
{
	LIST[index] = data;
}

TEMPLATE inline void _referencetable <AnonymousStruct>::kill(Uint16 index)
{
	LIST[index] = NULL;
}

TEMPLATE inline AnonymousStruct *_referencetable <AnonymousStruct>::get(Uint16 index)
{
	return LIST[index];
}

#endif	// DATATYPES_H
