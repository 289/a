#include "logsrv_session.h"

#include "shared/logsys/logging.h"


namespace GLog {

using namespace shared::net;

LogserverSession::LogserverSession(EventLoop *ploop, int32_t sid, int32_t sockfd, SessionManager *pmanager)
	: ProtobufSession(ploop, sid, sockfd, pmanager)
{
}

void LogserverSession::OnAddSession()
{   
	LOG_INFO << "logclient LogserverSession::OnAddSession";
}   

void LogserverSession::OnDelSession()
{   
	LOG_INFO << "logclient LogserverSession::OnDelSession"; 
}   

void LogserverSession::OnStartupRegisterMsgHandler()
{ 
}

} // namespace GLog
