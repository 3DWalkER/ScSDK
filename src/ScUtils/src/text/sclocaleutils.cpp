#include "scutils/text/sclocaleutils_p.h"

#include "scutils/text/scstring.h"

SC_BEGIN_NAMESPACE

ScString ulltoa(scuint64 l, int base, const char zero)
{
	char buff[65]; // length of MAX_ULLONG in base 2
	char* p = buff + 65;

	if (base != 10 || '0' == zero)
	{
		while (l != 0)
		{
			int c = l % base;

			--p;

			if (c < 10)
				*p = '0' + c;
			else
				*p = c - 10 + 'a';

			l /= base;
		}
	}
	else
	{
		while (l != 0)
		{
			int c = l % base;

			*(--p) = zero + c;

			l /= base;
		}
	}

	return ScString(p, 65 - (p - buff));

}

SC_END_NAMESPACE