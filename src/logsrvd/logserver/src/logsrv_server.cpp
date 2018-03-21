#include "logsrv_server.h"

#include "shared/net/manager/network.h"

#include "logsrv_session.h"


namespace GLog {

using namespace shared::net;

Session* LogsrvServer::CreateSession(int sockfd)
{
	return new LogsrvSession(NetWork::GetEventLoop(), AllocateSessionId(), sockfd, this);
}

} // namespace GLog
