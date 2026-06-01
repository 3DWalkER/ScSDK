#include "scnet/kernel/schostaddress.h"

#include "scutils/utils/scendian.h"
#include "scutils/text/scstring.h"
#include "scnet/utils/scipaddress_p.h"

#include <string.h>

#ifdef SC_OS_WIN
#	include <WS2tcpip.h>
#else
#	include <netinet/in.h>
#endif

class ScIPv6Address
{
public:
	inline scuint8& operator[](int index) { return c[index]; }
	inline scuint8 operator[](int index) const { return c[index]; }
	scuint8 c[16];
};

class ScHostAddressPrivate
{
public:
	void setAddress(scuint32 a4 = 0);
	void setAddress(const scuint8* _a6);
	void setAddress(const ScIPv6Address& _a6) { setAddress(_a6.c); }

	void clear();
	bool parse(const ScString& ipString);

	ScString scopeId;

	union
	{
		ScIPv6Address	  a6;
		struct { scuint32 c[4]; }	  a6_32;
		struct { scuint64 c[2]; } a6_64;
	};

	scuint32 a4;
	unsigned char protocol;

	friend class ScHostAddress;
};

ScHostAddress::ScHostAddress()
	: d(new ScHostAddressPrivate())
{
}

ScHostAddress::ScHostAddress(scuint32 ip4Addr)
	: ScHostAddress()
{
	setAddress(ip4Addr);
}

ScHostAddress::ScHostAddress(const scuint8* ip6Addr)
	: ScHostAddress()
{
	setAddress(ip6Addr);
}

ScHostAddress::ScHostAddress(const ScString& address)
	: ScHostAddress()
{
	setAddress(address);
}

ScHostAddress::ScHostAddress(const ScHostAddress& other)
	: d(other.d)
{
}

ScHostAddress::ScHostAddress(Type type)
	: ScHostAddress()
{
	setAddress(type);
}

ScHostAddress::~ScHostAddress()
{
}

void ScHostAddress::setAddress(scuint32 ip4Addr)
{
	d.detach();
	d->setAddress(ip4Addr);
}

void ScHostAddress::setAddress(const scuint8* ip6Addr)
{
	d.detach();
	d->setAddress(ip6Addr);
}

void ScHostAddress::setAddress(const ScString& addr)
{
	d.detach();
	d->parse(addr);
}

void ScHostAddress::setAddress(Type type)
{
	clear();

	ScIPv6Address ip6;
	memset(&ip6, 0, sizeof ip6);
	unsigned int ip4 = INADDR_ANY;

	switch (type)
	{
	case Null:
		return;
	case Broadcast:
		ip4 = INADDR_BROADCAST;
		break;
	case LocalHost:
		ip4 = INADDR_LOOPBACK;
		break;
	case AnyIPv4:
		break;
	case LocalHostIPv6:
		ip6[15] = 1;
	case AnyIPv6:
		d->setAddress(ip6);
		return;
	case Any:
		d->protocol = ScAbstractSocket::AnyIPProtocol;
	}

	d->setAddress(ip4);
}

ScAbstractSocket::NetworkLayerProtocol ScHostAddress::protocol() const
{
	return ScAbstractSocket::NetworkLayerProtocol(d->protocol);
}

static bool convertToIpv4(unsigned int& a4, const ScIPv6Address& a6, ScHostAddress::ConversionModes mode)
{
	if (ScHostAddress::StrictConversion == mode)
		return false;

	const scuint8* ptr = a6.c;
	if (0 != Sc::fromUnaligned<scuint64>(ptr))
		return false;

	const scuint32 mid = Sc::fromBigEndian<scuint32>(ptr + 8);
	if ((0xffff == mid) && (ScHostAddress::ConvertV4MappedToIPv4 & mode))
	{
		a4 = Sc::fromBigEndian<unsigned int>(ptr + 12);
		return true;
	}

	if (0 != mid)
		return false;

	const scuint32 low = Sc::fromBigEndian<scuint32>(ptr + 12);
	if ((0 == low) && (ScHostAddress::ConvertUnspecifiedAddress & mode))
	{
		a4 = 0;
		return true;
	}

	if ((1 == low) && (ScHostAddress::ConvertLocalHost & mode))
	{
		a4 = INADDR_LOOPBACK;
		return true;
	}

	if ((1 != low) && (ScHostAddress::ConvertV4CompatToIPv4 & mode))
	{
		a4 = low;
		return true;
	}

	return false;
}

scuint32 ScHostAddress::toIPv4Address(bool* ok) const
{
	if (ok)
	{
		scuint32 dummy;
		*ok = ScAbstractSocket::IPv4Protocol == d->protocol || ScAbstractSocket::AnyIPProtocol == d->protocol
			|| (ScAbstractSocket::IPv6Protocol == d->protocol
				&& convertToIpv4(dummy, d->a6, ConversionModes(ScHostAddress::ConvertV4MappedToIPv4
					| ScHostAddress::ConvertUnspecifiedAddress)));
	}
	return d->a4;
}

ScString ScHostAddress::toString() const
{
	ScString s;
	if (d->protocol == ScAbstractSocket::IPv4Protocol
		|| d->protocol == ScAbstractSocket::AnyIPProtocol) 
	{
		scuint32 i = toIPv4Address();
		ScIPAddressUtils::toString(s, i);
	}
	else if (d->protocol == ScAbstractSocket::IPv6Protocol)
	{
		ScIPAddressUtils::toString(s, d->a6.c);
		if (!d->scopeId.isEmpty())
			s.append("%" + d->scopeId);
	}
	return s;
}

void ScHostAddress::clear()
{
	d.detach();
	d->clear();
}

ScHostAddress& ScHostAddress::operator=(Type type)
{
	setAddress(type);
	return *this;
}

void ScHostAddressPrivate::setAddress(scuint32 _a4)
{
	a4 = _a4;
	protocol = ScAbstractSocket::IPv4Protocol;

	a6_64.c[0] = 0;
	if (a4)
	{
		a6_32.c[2] = Sc::toBigEndian(0xffff);
		a6_32.c[3] = Sc::toBigEndian(a4);
	}
	else
		a6_64.c[1] = 0;
}

void ScHostAddressPrivate::setAddress(const scuint8* _a6)
{
	protocol = ScAbstractSocket::IPv6Protocol;
	memcpy(a6.c, _a6, sizeof(a6));
	a4 = 0;
	convertToIpv4(a4, a6, ScHostAddress::ConvertV4MappedToIPv4 | ScHostAddress::ConvertUnspecifiedAddress);
}

void ScHostAddressPrivate::clear()
{
	a4 = 0;
	protocol = ScAbstractSocket::UnknownProtocolType;
	memset(&a6, 0, sizeof(a6));
}

bool ScHostAddressPrivate::parse(const ScString& ipString)
{
	return false;
}