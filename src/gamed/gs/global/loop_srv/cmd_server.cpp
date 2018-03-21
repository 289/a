#include "cmd_server.h"

#include "shared/net/manager/network.h"

#include "cmd_session.h"


namespace gamed {

Session* CmdServer::CreateSession(int sockfd)
{
	return new CmdSession(NetWork::GetEventLoop(), AllocateSessionId(), sockfd, this);
}

} // namespace gamed
