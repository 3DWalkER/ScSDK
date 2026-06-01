#include "scnet/utils/scipaddress_p.h"

#include "scutils/text/scstring.h"
#include "scutils/text/sclocaleutils_p.h"

namespace ScIPAddressUtils
{
	static ScString number(scuint8 val, int base = 10)
	{
		char zero(0x30);
		return val ? Sc::ulltoa(val, base, zero) : zero;
	}

	void toString(ScString& appendTo, IPv4Address address)
	{
		appendTo += number(address >> 24) + '.'
			+ number(address >> 16) + '.'
			+ number(address >> 8) + '.'
			+ number(address);
	}

	void toString(ScString& appendTo, const IPv6Address address)
	{
	}
}

