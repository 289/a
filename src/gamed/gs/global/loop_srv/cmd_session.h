#ifndef GAMED_GS_GLOBAL_LOOPSERVICE_CMD_SESSION_H_
#define GAMED_GS_GLOBAL_LOOPSERVICE_CMD_SESSION_H_

#include "shared/net/manager/protobuf_session.h"


namespace common {
namespace protocol {
namespace global {
	class HelloGameServer;
	class GameServerCmd;
} // namespace global
} // namespace protocol
} // namespace common


namespace gamed {

using namespace shared;
using namespace shared::net;
using namespace common::protocol;

class CmdSession : public ProtobufSession
{
public:
	CmdSession(EventLoop* loop, int32_t sid, int32_t sockfd, SessionManager* manager);

protected:
	void OnAddSession();
	void OnDelSession();
	virtual void OnStartupRegisterMsgHandler();
	void HandleHelloGameServer(const global::HelloGameServer& proto);
	void HandleGameServerCmd(const global::GameServerCmd& proto);

private:
	bool is_verified_;
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_LOOPSERVICE_CMD_SESSION_H_
