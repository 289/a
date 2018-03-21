#include "gs_client.h"

#include "shared/net/manager/network.h"

#include "gs_session.h"


namespace gsCmd {

Session* GSClient::CreateSession(int sockfd)
{
	return new GSSession(NetWork::GetEventLoop(), AllocateSessionId(), sockfd, this);
}

} // namespace gsCmd
