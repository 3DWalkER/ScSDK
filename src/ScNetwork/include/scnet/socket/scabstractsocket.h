#ifndef SCABSTRACTSOCKET_H
#define SCABSTRACTSOCKET_H

#include "scutils/scglobal.h"

class SC_API_EXPORT ScAbstractSocket
{
public:
	enum SocketType
	{
		TcpSocket,
		UdpSocket,
		SctpSocket,
		UnknownSocketType = -1
	};

	enum NetworkLayerProtocol
	{
		IPv4Protocol,
		IPv6Protocol,
		AnyIPProtocol,
		UnknownProtocolType = -1
	};

	enum SocketError
	{
		NoError,
		IllegalStateError,
	};

	enum SocketState
	{
		UnconnectedState,
		HostLookupState,
		ConnectingState,
		ConnectedState,
		BoundState,
		ListeningState,
		ClosingState
	};

	virtual ~ScAbstractSocket();
};

#endif // SCABSTRACTSOCKET_H