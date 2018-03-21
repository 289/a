#ifndef SHARED_NET_SESSION_H
#define SHARED_NET_SESSION_H

#include <string>
#include <google/protobuf/message.h>
#include "shared/net/tcpconnection.h"

namespace shared
{
namespace net
{

class SessionManager;
class ProtoPacket;
class RpcMessage;

class Session : public TcpConnection
{
public:
	typedef google::protobuf::Message& MessageRef;

	Session(EventLoop *ploop, int32_t sid, int32_t sockfd, SessionManager *pmanager);

	void ConnectCompleted();
	void Close();
	virtual void SendProtocol(ProtoPacket& packet);
	void SendProtocol(MessageRef packet);
	void SendProtocol(RpcMessage& packet);

protected:
	virtual void OnRecv(Buffer* pbuf) = 0;
	virtual void OnAddSession() = 0;
	virtual void OnDelSession() = 0;

	virtual void StartupRegisterMsgHandler() = 0;
	virtual void ClearPacketVec() = 0;
	virtual void OnConnected();
	virtual void OnClose();
protected:
	SessionManager *pmanager_;
	int32_t status_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_SESSION_H
