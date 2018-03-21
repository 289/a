#ifndef GAMED_GS_GLOBAL_LOOPSERVICE_CMD_SERVER_H_
#define GAMED_GS_GLOBAL_LOOPSERVICE_CMD_SERVER_H_

#include "shared/net/manager/session_manager.h"
#include "shared/base/singleton.h"


namespace gamed {

using namespace std;
using namespace shared;
using namespace shared::net;

class CmdServer : public SessionManager, public Singleton<CmdServer>
{
	friend class Singleton<CmdServer>;
public:
	static inline CmdServer* GetInstance() {
		return &(get_mutable_instance());
	}

	virtual string Identification() const
	{
		return "RuntimeCmdServer";
	}

protected:
	CmdServer()
	{
		SetServerType(SERVER);
	}

	~CmdServer()
	{
	}

	virtual Session* CreateSession(int sockfd);

private:
};

#define s_pCmdServer CmdServer::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_LOOPSERVICE_CMD_SERVER_H_
