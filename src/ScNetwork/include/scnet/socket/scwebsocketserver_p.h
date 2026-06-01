#ifndef SCWEBSOCKETSERVER_P_H
#define SCWEBSOCKETSERVER_P_H

#include "scnet/socket/scwebsocketserver.h"
#include "scutils/text/scstring.h"

class ScWebSocketServerPrivate
{
	SC_DECLARE_PUBLIC(ScWebSocketServer)
public:
	explicit ScWebSocketServerPrivate(ScWebSocketServer *q, const ScString &serverName, ScWebSocketServer::SslMode mode);
	~ScWebSocketServerPrivate();

	bool listen(const ScHostAddress& address = ScHostAddress::Any, scuint16 port = 0);
	void close();

	ScWebSocketServer* q_ptr;

	ScString serverName;
	ScWebSocketServer::SslMode secureMode;

	class ScWebSocketServerContext *m_pContext;
};

#endif // SCWEBSOCKETSERVER_P_H