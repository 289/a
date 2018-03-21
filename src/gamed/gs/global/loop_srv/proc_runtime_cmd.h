#ifndef GAMED_GS_GLOBAL_LOOPSERVICE_PROCESS_CMD_H_
#define GAMED_GS_GLOBAL_LOOPSERVICE_PROCESS_CMD_H_

#include <string>


namespace shared {
namespace net {
	class ProtobufSession;
} // namespace net
} // namespace shared

namespace gamed 
{
	void ProcRuntimeCmd(const std::string& cmd, shared::net::ProtobufSession* session);
} // namespace gamed

#endif // GAMED_GS_GLOBAL_LOOPSERVICE_PROCESS_CMD_H_
