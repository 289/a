#include "logsrv_session.h"

#include "shared/logsys/logging.h"

#include "global_init.h"
#include "worker_thread.h"


namespace GLog {

LogsrvSession::LogsrvSession(EventLoop* ploop, int32_t sid, int32_t sockfd, SessionManager* pmanager)
	    : ProtobufSession(ploop, sid, sockfd, pmanager)
{
}

void LogsrvSession::OnAddSession()
{
	LOG_INFO << "LogsrvSession::OnAddSession";
}

void LogsrvSession::OnDelSession()
{
	LOG_INFO << "LogsrvSession::OnDelSession";
}

void LogsrvSession::OnRecv(Buffer* pbuf)
{
	codec_.ParseMessages(this, pbuf, vec_packets_);

	PacketPtrVec::iterator it = vec_packets_.begin();
	for (; it != vec_packets_.end(); ++it)
	{
		GetWorker()->PutProtocol(*it);
	}
	vec_packets_.clear();
}

void LogsrvSession::OnStartupRegisterMsgHandler()
{
}

} // namespace GLog
