#ifndef SCHOSTADDRESS_H
#define SCHOSTADDRESS_H

#include "scutils/utils/scflags.h"
#include "scutils/utils/scexplicitlysharedpointer.h"
#include "scnet/socket/scabstractsocket.h"

class ScHostAddressPrivate;

class SC_API_EXPORT ScHostAddress
{
public:
	enum Type
	{
		Null,
		Broadcast,
		LocalHost,
		LocalHostIPv6,
		Any,
		AnyIPv6,
		AnyIPv4
	};

	enum ConversionMode
	{
		ConvertV4MappedToIPv4 = 0x01,
		ConvertV4CompatToIPv4 = 0x02,
		ConvertUnspecifiedAddress = 0x04,
		ConvertLocalHost = 0x08,
		TolerantConversion = 0xff,
		StrictConversion = 0
	};
	SC_DECLARE_FLAGS(ConversionModes, ConversionMode)

	ScHostAddress();
	explicit ScHostAddress(scuint32 ip4Addr);
	explicit ScHostAddress(const scuint8* ip6Addr);
	explicit ScHostAddress(const ScString& address);
	ScHostAddress(const ScHostAddress& other);
	ScHostAddress(Type type);
	~ScHostAddress();

	void setAddress(scuint32 ip4Addr);
	void setAddress(const scuint8* ip6Addr);
	void setAddress(const ScString& addr);
	void setAddress(Type type);

	ScAbstractSocket::NetworkLayerProtocol protocol() const;

	scuint32 toIPv4Address(bool* ok = nullptr) const;
	ScString toString() const;

	void clear();

	ScHostAddress& operator=(Type type);

private:
	friend class ScHostAddressPrivate;
	ScExplicitlySharedPointer<ScHostAddressPrivate> d;
};
SC_DECLARE_OPERATORS_FOR_FLAGS(ScHostAddress::ConversionModes)

#endif // SCHOSTADDRESS_H


