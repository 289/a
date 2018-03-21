#include "gs_session.h"

#include "common/protocol/gen/global/gs_runtime_cmd.pb.h"
#include "shared/security/aes.h"
#include "shared/logsys/logging.h"

#include "proc_cmd_input.h"
#include "gs_client.h"


namespace gsCmd {

static const char* kSayHello  = "Hello GameServer";
static const char* kExchangeKey = "eFBEZ0KNmd7Y6scaUvxV2wEadL2ibTXyi";

#define REGISTER(msg) \
	dispatcher_.Register<global::msg>(BIND_MEM_CB(&GSSession::Handle##msg, this))

GSSession::GSSession(EventLoop* loop, int32_t sid, int32_t sockfd, SessionManager* manager)
	: ProtobufSession(loop, sid, sockfd, manager)
{
}

void GSSession::OnAddSession()
{
	LOG_INFO << "GSSession::OnAddSession";
	s_pProcCmd->CountDown();

	// key
	AES aes;
	Buffer keybuf, inputbuf;
	keybuf.Append(kExchangeKey, strlen(kExchangeKey));
	inputbuf.Append(kSayHello, strlen(kSayHello));
	aes.SetParameter(keybuf);
	aes.Update(inputbuf);

	global::HelloGameServer proto;
	proto.set_content(inputbuf.peek(), inputbuf.ReadableBytes());
	SendProtocol(proto);

	// connected
	s_pGSClient->set_connected(true);
}

void GSSession::OnDelSession()
{
	LOG_INFO << "GSSession::OnDelSession";
	s_pGSClient->set_connected(false);
}

void GSSession::OnStartupRegisterMsgHandler()
{
	REGISTER(GSExecuteMessage);
}

void GSSession::HandleGSExecuteMessage(const global::GSExecuteMessage& proto)
{
	fprintf(stderr, "GS result:\n %s\n", proto.message().c_str());
	s_pProcCmd->ServerProcFinish();
}

} // namespace gsCmd
