#ifndef SHARED_NET_PROTOBUF_SESSION_H
#define SHARED_NET_PROTOBUF_SESSION_H

#include "shared/net/protobuf/codec_proto.h"
#include "shared/net/protobuf/dispatcher_proto.h"
#include "session.h"

namespace shared
{
namespace net
{

class ProtobufSession : public Session
{
public:
	ProtobufSession(EventLoop* ploop, int32_t sid, int32_t sockfd, SessionManager* pmanager);
	virtual ~ProtobufSession();
protected:
	virtual void OnRecv(Buffer *pbuf);
	virtual void OnStartupRegisterMsgHandler() = 0;

	virtual void PacketDefaultHandler(const MessageRef packet);
	virtual void ClearPacketVec();
private:
	virtual void StartupRegisterMsgHandler();
	void HandleRpcMessage(const RpcMessage& msg);
protected:
	ProtobufCodec codec_;
	ProtobufDispatcher dispatcher_;

	typedef std::vector<ProtobufCodec::MessagePtr> PacketPtrVec;
	PacketPtrVec vec_packets_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_SESSION_H
