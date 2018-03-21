#ifndef LOGSRVD_LOGSERVER_SRC_LOGSRV_SESSION_H_
#define LOGSRVD_LOGSERVER_SRC_LOGSRV_SESSION_H_

#include "shared/net/manager/protobuf_session.h"


namespace logsrv {
	class LogMessage;
} // namespace logsrv


namespace GLog {

using namespace shared::net;

class LogsrvSession : public shared::net::ProtobufSession
{
public:
	LogsrvSession(EventLoop* ploop, int32_t sid, int32_t sockfd, SessionManager* pmanager);

protected:
	virtual void OnRecv(Buffer* pbuf);
	virtual void OnAddSession();
	virtual void OnDelSession();

	virtual void OnStartupRegisterMsgHandler();
};

} // namespace GLog

#endif // LOGSRVD_LOGSERVER_SRC_LOGSRV_SESSION_H_
