#include "scnet/socket/scwebsocket_p.h"

ScWebSocketPrivate::ScWebSocketPrivate(ScWebSocket* q, const ScString& origin, ScWebSocketProtocol::Version version)
	: q_ptr(q), origin(origin), version(version)
{
}
