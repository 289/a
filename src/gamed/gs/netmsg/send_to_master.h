#ifndef GAMED_GS_NETMSG_SEND_TO_MASTER_H_
#define GAMED_GS_NETMSG_SEND_TO_MASTER_H_

#include <stdint.h>
#include <vector>

#include "shared/base/types.h"
#include "shared/base/singleton.h"
#include "shared/net/protobuf/codec_proto.h"
#include "shared/net/protobuf/rpc/rpc_sendtunnel.h"


namespace shared {
namespace net {
	class ProtoPacket;
} // namespace net
} // namespace shared


namespace combat {
	struct CombatPVEResult;
	struct CombatPVPResult;
}; // namespace combat


namespace gamed {

/**
 * @brief NetToMaster
 *    （1）如果是player发给master的协议，直接在player_sender里写就可以，不用再本文件加接口
 *    （2）发送主体不是player时，需要在这里加接口
 */
class NetToMaster
{
	typedef shared::net::ProtoPacket& PacketRef;
public:
	// ---- thread safe ----
	static void SendProtocol(int32_t masterid, const shared::net::ProtobufCodec::MessageRef msg_ref);
	static void SendToAllMaster(const shared::net::ProtobufCodec::MessageRef msg_ref);
	static void PlayerAbnormalLogout(int32_t masterid, RoleID roleid, int logout_code);
	static void PlayerDataSyncToMaster(int32_t masterid, int64_t roleid, const void* pbuf, size_t size);
	static void SaveCombatPVEResult(int32_t masterid, int32_t combat_id, int64_t roleid, int32_t unid_id, const combat::CombatPVEResult& result);
	static void SaveCombatPVPResult(int32_t masterid, int32_t combat_id, int64_t roleid, int32_t unid_id, const combat::CombatPVPResult& result);
	static void QueryTeamInfo(int32_t masterid, int64_t roleid);
	static void ModifyGlobalCounter(int32_t masterid, int32_t index, int32_t delta);
	static void SetGlobalCounter(int32_t masterid, int32_t index, int32_t value);
}; 

class MasterRpcTunnel : public shared::Singleton<MasterRpcTunnel>, public shared::net::RpcSendTunnel
{
	friend class shared::Singleton<MasterRpcTunnel>;
public:
	static inline MasterRpcTunnel* GetInstance() {
		return &(get_mutable_instance());
	}

	virtual void SendRPC(int32_t sid, const shared::net::RpcMessage& rpcmessage);
};

#define s_pMasterRpcTunnel gamed::MasterRpcTunnel::GetInstance()

} // namespace gamed

#endif // GAMED_GS_NETMSG_SEND_TO_MASTER_H_
