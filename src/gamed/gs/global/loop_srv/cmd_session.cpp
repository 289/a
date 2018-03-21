#include "cmd_session.h"

#include "common/protocol/gen/global/gs_runtime_cmd.pb.h"
#include "shared/logsys/logging.h"
#include "shared/security/aes.h"

#include "proc_runtime_cmd.h"


namespace gamed {

static const char* kSayHello  = "Hello GameServer";
static const char* kExchangeKey = "dFBEZ0KNmd7Y6scaUvxV2wEadL2ibTXyi";

#define REGISTER(msg) \
	dispatcher_.Register<global::msg>(BIND_MEM_CB(&CmdSession::Handle##msg, this))

CmdSession::CmdSession(EventLoop* loop, int32_t sid, int32_t sockfd, SessionManager* manager)
	: ProtobufSession(loop, sid, sockfd, manager),
	  is_verified_(false)
{
}

void CmdSession::OnAddSession()
{
	LOG_INFO << "CmdSession::OnAddSession() Runtime cmd tools connected!";
}

void CmdSession::OnDelSession()
{
	LOG_INFO << "CmdSession::OnDelSession() Runtime cmd tools disconnected!";
}

void CmdSession::OnStartupRegisterMsgHandler()
{
	REGISTER(HelloGameServer);
	REGISTER(GameServerCmd);
}

void CmdSession::HandleHelloGameServer(const global::HelloGameServer& proto)
{
	AES aes;
	Buffer keybuf, inputbuf;
	keybuf.Append(kExchangeKey, strlen(kExchangeKey));
	inputbuf.Append(proto.content().c_str(), proto.content().size());
	aes.SetParameter(keybuf);
	aes.Update(inputbuf);

	std::string tmpstr;
	tmpstr.assign(inputbuf.peek(), inputbuf.ReadableBytes());
	if (tmpstr.compare(kSayHello) != 0)
	{
		LOG_WARN << "runtime cmd tools 密钥不正确，断开连接!";
		Shutdown();
	}
	else
	{
		is_verified_ = true;
	}
}

void CmdSession::HandleGameServerCmd(const global::GameServerCmd& proto)
{
	if (!is_verified_)
	{
		Shutdown();
	}
	else // verified
	{
		ProcRuntimeCmd(proto.cmd(), this);
	}
}

} // namespace gamed
