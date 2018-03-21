#ifndef LOGSRVD_LOGCLIENT_SRC_LOGSERVER_CLIENT_H_ 
#define LOGSRVD_LOGCLIENT_SRC_LOGSERVER_CLIENT_H_

#include "shared/net/manager/session_manager.h"
#include "shared/base/singleton.h"


namespace GLog {

class LogsrvClient : public shared::net::SessionManager, public shared::Singleton<LogsrvClient>
{
	friend class shared::Singleton<LogsrvClient>;
	static const char* kNameStringId;
public:
	static inline LogsrvClient* GetInstance() {
		return &(get_mutable_instance());
	}

	virtual std::string Identification() const {
		return kNameStringId;
	}

	static std::string Name() {
		return kNameStringId;
	}

protected:
	LogsrvClient() { 
		SetServerType(CLIENT); 
	}

	~LogsrvClient() { }

	virtual shared::net::Session* CreateSession(int sockfd);
};

} // namespace GLog

#endif // LOGSRVD_LOGCLIENT_SRC_LOGSERVER_CLIENT_H_
