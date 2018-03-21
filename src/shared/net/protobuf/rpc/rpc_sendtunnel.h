#ifndef SHARED_NET_PROTOBUF_RPC_SENDTUNNEL_H_
#define SHARED_NET_PROTOBUF_RPC_SENDTUNNEL_H_

#include <stdint.h>

#include "shared/base/noncopyable.h"


namespace shared {
namespace net {

class RpcMessage;

///
/// rpc tunnel (use to send rpc messages)
///
class RpcSendTunnel : noncopyable
{
public:
	virtual void SendRPC(int32_t sid, const RpcMessage& rpcmessage) = 0;
};

typedef shared::net::RpcSendTunnel* RpcSendTunnelPtr;

} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_RPC_SENDTUNNEL_H_
