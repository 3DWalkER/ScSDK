#ifndef SCWEBSOCKET_H
#define SCWEBSOCKET_H

#include "scnet/scnetglobal.h"
#include "scutils/io/scurl.h"

class ScWebSocketPrivate;

class SC_API_EXPORT ScWebSocket
{
	SC_DISABLE_COPY(ScWebSocket)
public:
	explicit ScWebSocket(const ScString &origin = ScString(), ScWebSocketProtocol::Version version = ScWebSocketProtocol::VersionLatest);
	~ScWebSocket();

	ScString origin() const;
	ScWebSocketProtocol::Version version() const;

private:
	ScWebSocketPrivate* d_ptr;
	SC_DECLARE_PRIVATE(ScWebSocket)
};

#endif // SCWEBSOCKET_H