#include "scnet/socket/scwebsocketserver_p.h"

ScWebSocketServer::ScWebSocketServer(const ScString& serverName, SslMode mode)
	: d_ptr(new ScWebSocketServerPrivate(this, serverName, mode))
{
}

ScWebSocketServer::~ScWebSocketServer()
{
	delete d_ptr;
}

bool ScWebSocketServer::listen(const ScHostAddress& address, scuint16 port)
{
	SC_D(ScWebSocketServer);
	return d->listen(address, port);
}

void ScWebSocketServer::close()
{
	SC_D(ScWebSocketServer);
	return d->close();
}
