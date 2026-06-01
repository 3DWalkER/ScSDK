#include "scnet/socket/scwebsocket_p.h"

ScWebSocket::ScWebSocket(const ScString& origin, ScWebSocketProtocol::Version version)
	: d_ptr(new ScWebSocketPrivate(this, origin, version))
{
}

ScWebSocket::~ScWebSocket()
{
	delete d_ptr;
}

ScString ScWebSocket::origin() const
{
	SC_D(const ScWebSocket);
	return d->origin;
}

ScWebSocketProtocol::Version ScWebSocket::version() const
{
	SC_D(const ScWebSocket);
	return d->version;
}
