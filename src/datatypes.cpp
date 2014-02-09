#include "datatypes.h"

#include <cctype>
#include <cstring>

#include "algorithms.h"


//////// Fielding ////////

Uint32 getLong(BYTE *Message, const Uint32 Offset)
{
	return *(Uint32*)&(Message[Offset]);
}

Uint16 getShort(BYTE *Message, const Uint32 Offset)
{
	return *(Uint16*)&(Message[Offset]);
}

Uint32 getLong(char *Message, const Uint32 Offset)
{
	return *(Uint32*)&(Message[Offset]);
}

Uint16 getShort(char *Message, const Uint32 Offset)
{
	return *(Uint16*)&(Message[Offset]);
}

BYTE getByte(char *Message, const Uint32 Offset)
{
	return *(BYTE*)&(Message[Offset]);
}


//////// Strings ////////


bool CMPSTART(const char *control, const char *constant)
{
	char c;

	while (c = *control)
	{
		if (*constant != c)
			return false;

		++control;
		++constant;
	}

	return true;
}

bool CMPSTR(const char *a, const char *b)
{
	char c;

	while (std::tolower(c = *a) == std::tolower(*b))
	{
		if (!c) return true;

		++a;
		++b;
	}

	return false;
}

String::String()
{
	len = 0;
	msg = new char[1];
	*msg = '\0';
}

String::String(const char *s, Uint32 slen)
{
	len = slen;
	msg = new char[len + 1];
	memcpy(msg, s, len);
	msg[len] = '\0';
}

String::String(const char *s)
{
	len = std::strlen(s);
	msg = new char[len + 1];
	memcpy(msg, s, len);
	msg[len] = '\0';
}

String::String(const Uint32 number)
{
	String s = getString(number, 10, 0, true);

	len = s.len;
	msg = new char[len + 1];
	memcpy(msg, s.msg, len);
	msg[len] = '\0';
}

String::String(const String &s)
{
	len = s.len;
	msg = new char[len + 1];
	memcpy(msg, s.msg, len);
	msg[len] = '\0';
}

String::~String()
{
	if (msg)
	{
		delete []msg;

		len = 0;
		msg = NULL;
	}
}

void String::append(const char *buffer, Uint32 nlen)
{
	Uint32 clen = len + nlen;

	char *buff = new char[clen + 1];
	memcpy(buff, msg, len);
	memcpy(buff + len, buffer, nlen);
	buff[clen] = '\0';

	delete []msg;

	msg = buff;
	len = clen;
}

void String::prepend(const char *buffer, Uint32 nlen)
{
	Uint32 clen = len + nlen;

	char *buff = new char[clen + 1];
	memcpy(buff, buffer, nlen);
	memcpy(buff + nlen, msg, len);
	buff[clen] = '\0';

	delete []msg;

	msg = buff;
	len = clen;
}

void String::set(const char *buffer, Uint32 nlen)
{
	if (msg) delete []msg;

	len = nlen;
	msg = new char[nlen + 1];
	memcpy(msg, buffer, len);
	msg[len] = '\0';
}

void String::clear()
{
	len = 0;
	msg = new char[1];
	*msg = '\0';
}

void String::replace(char delimiter, char replaceChar)
{
	for (Uint32 i = 0; i < len; ++i)
		if (msg[i] == delimiter)
			msg[i] = replaceChar;
}


#ifdef STRING_CAST_CHAR
	String::operator char*()
	{
		return msg;
	}
#endif


bool String::operator==(const char *string)
{
	return CMPSTR(string, msg);
}

bool String::operator==(Uint32 number)
{
	String s = number;

	return CMPSTR(s.msg, msg);
}

bool String::operator==(const String &string)
{
	return CMPSTR(string.msg, msg);
}

bool String::operator!=(const char *string)
{
	return !CMPSTR(string, msg);
}

bool String::operator!=(Uint32 number)
{
	String s = number;

	return !CMPSTR(s.msg, msg);
}

bool String::operator!=(const String &string)
{
	return !CMPSTR(string.msg, msg);
}

void String::operator=(const char *string)
{
	set(string, std::strlen(string));
}

void String::operator=(Uint32 number)
{
	String s = number;

	set(s.msg, s.len);
}

void String::operator=(const String &string)
{
	set(string.msg, string.len);
}

void String::operator+=(const char *string)
{
	append(string, std::strlen(string));
}

void String::operator+=(Uint32 number)
{
	String s = number;

	append(s.msg, s.len);
}

void String::operator+=(const String &string)
{
	append(string.msg, string.len);
}

String String::operator+(const char *string)
{
	String s(*this);
	s.append(string, std::strlen(string));

	return s;
}

String String::operator+(Uint32 number)
{
	String n(number);

	String s(*this);
	s.append(n.msg, n.len);

	return s;
}

String String::operator+(const String &string)
{
	String s(*this);
	s.append(string.msg, string.len);

	return s;
}

String operator+(const char *prepended, const String &string)
{
	String s(prepended);
	s.append(string.msg, string.len);

	return s;
}

int String::firstInstanceOf(char c)
{
	for (int i = 0; i < (int)len; ++i)
		if (msg[i] == c)
			return i;

	return -1;
}

String String::left(Uint32 nlen)
{
	return String(msg, limit(nlen, len));
}

String String::right(Uint32 nlen)
{
	nlen = limit(nlen, len);

	return String(msg + (len - nlen), nlen);
}

String String::mid(Uint32 offset, Uint32 nlen = 32000)
{
	if (offset >= len) return String();

	return String(msg + offset, limit(nlen, len - offset));
}

String String::split(char delimiter)
{
	for (Uint32 i = 0; i < len; ++i)
		if (msg[i] == delimiter)
		{
			String s = mid(i + 1);
			String save = s.trim();

			s = left(i);
			String ret = s.trim();

			set(save.msg, save.len);

			return ret;
		}

	String ret = trim();
	clear();

	return ret;
}

String String::ltrim()
{
	for (Uint32 i = 0; i < len; ++i)
		if (msg[i] != ' ')
			return right(len - i);

	return String();
}

String String::rtrim()
{
	for (Uint32 i = len - 1; i >= 0; --i)
		if (msg[i] != ' ')
			return left(i + 1);

	return String();
}

String String::trim()
{
	String s(ltrim());
	return s.rtrim();
}

char *spaces = "                                                                                                              ";

String String::lfill(Uint32 nlen)
{
	if (len >= nlen)
		return right(nlen);

	String s;
	s.set(spaces, nlen - len);
	s.append(msg, len);

	return s;
}

String String::rfill(Uint32 nlen)
{
	if (len >= nlen)
		return left(nlen);

	String s(*this);
	s.append(spaces, nlen - len);

	return s;
}

void String::lcase()
{
	tolower(msg);
}

void String::ucase()
{
	toupper(msg);
}

int String::toInteger()
{
	return getInteger(msg, 10);
}

int String::toInteger(int base)
{
	return getInteger(msg, base);
}

bool String::IsEmpty()
{
	return (*msg == '\0');
}


//////// Vectors ////////

Vector::Vector()
{
	x = 0;
	y = 0;
}

Vector::Vector(const Vector &v)
{
	x = v.x;
	y = v.y;
}

Vector::Vector(Sint32 nx, Sint32 ny)
{
	x = nx;
	y = ny;
}

bool Vector::operator==(const Vector &v)
{
	return x == v.x && y == v.y;
}

bool Vector::operator!=(const Vector &v)
{
	return x != v.x || y != v.y;
}

void Vector::operator=(const Vector &v)
{
	x = v.x;
	y = v.y;
}

void Vector::operator=(Sint32 n)
{
	x = n;
	y = n;
}

Vector Vector::operator+(const Vector &v)
{
	Vector a(x + v.x, y + v.y);
	return a;
}

void Vector::operator+=(const Vector &v)
{
	x += v.x;
	y += v.y;
}

Vector Vector::operator-(const Vector &v)
{
	Vector a(x - v.x, y - v.y);
	return a;
}

void Vector::operator-=(const Vector &v)
{
	x -= v.x;
	y -= v.y;
}

Vector Vector::operator*(const Vector &v)
{
	Vector a(x * v.x, y * v.y);
	return a;
}

void Vector::operator*=(const Vector &v)
{
	x *= v.x;
	y *= v.y;
}

Vector Vector::operator/(const Vector &v)
{
	Vector a(x / v.x, y / v.y);
	return a;
}

void Vector::operator/=(const Vector &v)
{
	x /= v.x;
	y /= v.y;
}

Vector Vector::operator+(Sint32 n)
{
	Vector a(x + n, y + n);
	return a;
}

void Vector::operator+=(Sint32 n)
{
	x += n;
	y += n;
}

Vector Vector::operator-(Sint32 n)
{
	Vector a(x - n, y - n);
	return a;
}

void Vector::operator-=(Sint32 n)
{
	x -= n;
	y -= n;
}

Vector Vector::operator*(Sint32 n)
{
	Vector a(x * n, y * n);
	return a;
}

void Vector::operator*=(Sint32 n)
{
	x *= n;
	y *= n;
}

Vector Vector::operator/(Sint32 n)
{
	if (n == 0) return *this;
	Vector a(x / n, y / n);
	return a;
}

void Vector::operator/=(Sint32 n)
{
	if (n == 0) return;
	x /= n;
	y /= n;
}

void Vector::set(Sint32 nx, Sint32 ny)
{
	x = nx;
	y = ny;
}

Sint32 Vector::distance(const Vector &v)
{
	Sint32 a = (x - v.x),
		   b = (y - v.y),
		   c = a * a + b * b;

	return SQRT(c);
}
