#ifndef LOGSRVD_LOGSERVER_SRC_LOGSRV_SERVER_H_
#define LOGSRVD_LOGSERVER_SRC_LOGSRV_SERVER_H_

#include "shared/net/manager/session_manager.h"
#include "shared/base/singleton.h"


namespace GLog {

class LogsrvServer : public shared::net::SessionManager, public shared::Singleton<LogsrvServer>
{
	friend class shared::Singleton<LogsrvServer>;
public:
	static inline LogsrvServer* GetInstance() {
		return &(get_mutable_instance());
	}

	virtual std::string Identification() const {
		return "LogserviceServer";
	}

protected:
	LogsrvServer() {
		SetServerType(SERVER);
	}

	~LogsrvServer() { }

	virtual shared::net::Session* CreateSession(int sockfd);
};

} // namespace GLog

#endif // LOGSRVD_LOGSERVER_SRC_LOGSRV_SERVER_H_
