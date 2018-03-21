#ifndef LOGSRVD_LOGCLIENT_SRC_LOGSERVER_SESSION_H_
#define LOGSRVD_LOGCLIENT_SRC_LOGSERVER_SESSION_H_

#include "shared/net/manager/protobuf_session.h"


namespace GLog {

class LogserverSession : public shared::net::ProtobufSession
{
public:
	LogserverSession(shared::net::EventLoop* ploop, 
			         int32_t sid, 
					 int32_t sockfd, 
					 shared::net::SessionManager *pmanager);

protected:
	virtual void OnAddSession();
	virtual void OnDelSession();
	virtual void OnStartupRegisterMsgHandler();
};

} // namespace GLog

#endif // LOGSRVD_LOGCLIENT_SRC_LOGSERVER_SESSION_H_
