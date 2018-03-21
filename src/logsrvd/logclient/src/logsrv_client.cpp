#include "logsrv_client.h"

#include "shared/net/manager/network.h"

#include "logsrv_session.h"


namespace GLog {

using namespace shared::net;

const char* LogsrvClient::kNameStringId = "LogserviceClient";

Session* LogsrvClient::CreateSession(int sockfd)
{
	return new LogserverSession(NetWork::GetEventLoop(), AllocateSessionId(), sockfd, this);
}

} // namespace GLog
