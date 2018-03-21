#ifndef SHARED_NET_RPC_TUNNEL_H
#define SHARED_NET_RPC_TUNNEL_H

#include <set>
#include "shared/net/protobuf/rpc/rpc_sendtunnel.h"

namespace shared
{
namespace net
{

class SessionManager;
class RpcMessage;

class RpcTunnel : public RpcSendTunnel
{
	friend class RpcService;
public:
	enum TunnelType
	{
		SERVER = 0,
		CLIENT,
	};

	void AddManager(SessionManager* pmanager);
	virtual void SendRPC(int32_t sid, const RpcMessage& packet);
private:
	void SetTunnelType(TunnelType type);
	void SendServerRpc(int32_t sid, const RpcMessage& packet);
	void SendClientRpc(int32_t sid, const RpcMessage& packet);
	
	TunnelType type_;
	typedef std::set<SessionManager*> ManagerSet;
	ManagerSet managers_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_RPC_TUNNEL_H
