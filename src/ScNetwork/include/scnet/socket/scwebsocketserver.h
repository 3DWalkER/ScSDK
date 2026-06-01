#ifndef SCWEBSOCKETSERVER_H
#define SCWEBSOCKETSERVER_H

#include "scnet/kernel/schostaddress.h"

class ScWebSocketServerPrivate;

class SC_API_EXPORT ScWebSocketServer
{
	SC_DECLARE_PRIVATE(ScWebSocketServer)
public:
	enum SslMode
	{
		SecureMode = 0,
		NonSecureMode
	};

	explicit ScWebSocketServer(const ScString &serverName, SslMode mode);
	~ScWebSocketServer();

	bool listen(const ScHostAddress& address = ScHostAddress::Any, scuint16 port = 0);
	void close();

private:
	ScWebSocketServerPrivate* d_ptr;
};

#endif // SCWEBSOCKETSERVER_H