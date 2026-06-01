#ifndef SCWEBSOCKET_P_H
#define SCWEBSOCKET_P_H

#include "scwebsocket.h"

class ScWebSocketPrivate
{
	SC_DECLARE_PUBLIC(ScWebSocket)
public:
	explicit ScWebSocketPrivate(ScWebSocket *q, const ScString& origin, ScWebSocketProtocol::Version version);

	ScWebSocket* q_ptr;

	ScString origin;
	ScWebSocketProtocol::Version version;
};

#endif // SCWEBSOCKET_P_H