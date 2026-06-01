#include "scnet/kernel/schostaddress.h"
#include "scutils/text/scstring.h"
#include "scnet/socket/scwebsocketserver.h"
#include "scutils/system/scapplication.h"

int main(int argc, char *argv[])
{
	ScApplication a(argc, argv);

	ScWebSocketServer server("Mode", ScWebSocketServer::NonSecureMode);
	server.listen(ScHostAddress::Any, 12002);

	return a.exec();
}