#ifndef SCIPADDRESS_P_H
#define SCIPADDRESS_P_H

#include "scutils/scglobal.h"

typedef scuint32 IPv4Address;
typedef scuint8 IPv6Address[16];

namespace ScIPAddressUtils
{
	void toString(ScString& appendTo, IPv4Address address);
	void toString(ScString& appendTo, const IPv6Address address);
}

#endif // SCIPADDRESS_P_H