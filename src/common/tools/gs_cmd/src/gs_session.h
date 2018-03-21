#ifndef COMMON_TOOLS_GSCMD_GS_SESSION_H_
#define COMMON_TOOLS_GSCMD_GS_SESSION_H_

#include "shared/net/manager/protobuf_session.h"


namespace common {
namespace protocol {
namespace global {
	class GSExecuteMessage;
} // namespace global
} // namespace protocol
} // namespace common


namespace gsCmd {

using namespace shared;
using namespace shared::net;
using namespace common::protocol;

/**
 * @brief GSSession
 */
class GSSession : public ProtobufSession
{
public:
	GSSession(EventLoop* loop, int32_t sid, int32_t sockfd, SessionManager* manager);

protected:
	void OnAddSession();
	void OnDelSession();
	virtual void OnStartupRegisterMsgHandler();
	void HandleGSExecuteMessage(const global::GSExecuteMessage& proto);
};

} // namespace gsCmd

#endif // COMMON_TOOLS_GSCMD_GS_SESSION_H_
