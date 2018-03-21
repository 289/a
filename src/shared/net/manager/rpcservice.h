#ifndef SHARED_NET_RPCSERVICE_H
#define SHARED_NET_RPCSERVICE_H

#include "rpc_tunnel.h"
#include "shared/net/protobuf/rpc/rpc_service.h"

namespace shared
{
namespace net
{

class SessionManager;
class RpcClient;

class RpcService
{
public:
	static void Init();
	static void Release();
	static void Server(SessionManager* pmanager);
	static void Client(SessionManager* pmanager);
	static void OnRecvRpcMessage(int32_t sendback_sid, const RpcMessage& msg);

	template<typename REQUEST, typename RESPONSE, typename STUB>
	static void RpcCall(REQUEST request, int32_t sid, const typename RpcCallbackT<RESPONSE>::RpcMessageTCallback& callback);
private:
	static RpcClient* s_prpcclient_;
	static RpcTunnel s_server_tunnel_;
	static RpcTunnel s_client_tunnel_;
};

template<typename REQUEST, typename RESPONSE, typename STUB>
void RpcService::RpcCall(REQUEST request, int32_t sid, const typename RpcCallbackT<RESPONSE>::RpcMessageTCallback& callback)
{
	ServiceStub* pstub = s_prpcclient_->CreateStub<STUB>(request);
	s_prpcclient_->RpcCall<RESPONSE>(sid, pstub, callback);
}

#define RPCCALL(RpcPrefix, request, sid, callback) \
	RpcService::RpcCall<RpcPrefix##Request, RpcPrefix##Response, RpcPrefix##Stub>(request, sid, callback)

} // namespace net
} // namespace shared

#endif // SHARED_NET_RPCSERVICE_H
