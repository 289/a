#include "send_to_master.h"

#include "shared/net/protobuf/rpc/rpc.pb.h"

#include "gs/netio/netio_if.h"
#include "game_module/combat/include/combat_header.h"

// proto
#include "common/protocol/gen/G2M/player_abnormal_logout.pb.h"
#include "common/protocol/gen/G2M/save_playerdata.pb.h"
#include "common/protocol/gen/G2M/save_combat_result.pb.h"
#include "common/protocol/gen/G2M/team_msg.pb.h"
#include "common/protocol/gen/G2M/global_counter.pb.h"


namespace gamed {

using namespace common::protocol;

///
/// NetToMaster
///
void NetToMaster::SendProtocol(int32_t masterid, const ProtobufCodec::MessageRef msg_ref)
{
	NetIO::SendToMaster(masterid, msg_ref);
}

void NetToMaster::SendToAllMaster(const shared::net::ProtobufCodec::MessageRef msg_ref)
{
	NetIO::SendToAllMaster(msg_ref);
}

void NetToMaster::PlayerAbnormalLogout(int32_t masterid, RoleID roleid, int logout_code)
{
	G2M::PlayerAbnormalLogout msg;
	msg.set_roleid(roleid);
	msg.set_logout_code(static_cast<G2M::PlayerAbnormalLogout::LogoutCode>(logout_code));

	NetIO::SendToMaster(masterid, msg);
}

void NetToMaster::PlayerDataSyncToMaster(int32_t masterid, int64_t roleid, const void* pbuf, size_t size)
{
	G2M::SavePlayerData msg;
	msg.set_roleid(roleid);
	msg.set_content(pbuf, size);

	NetIO::SendToMaster(masterid, msg);
}

void NetToMaster::SaveCombatPVEResult(int32_t masterid, int32_t combat_id, int64_t roleid, int32_t unit_id, const combat::CombatPVEResult& result)
{
	G2M::SaveCombatResult msg;
	msg.set_roleid(roleid);
	msg.set_unit_id(unit_id);
	msg.set_combat_id(combat_id);
    msg.set_combat_type(G2M::SaveCombatResult::COMBAT_TYPE_PVE);
	msg.set_combat_result(result.result);
	msg.set_combat_remain_hp(result.hp);
    msg.set_combat_pet_power(result.pet_power);

    global::CombatPVEResult* pve_result = msg.mutable_pve_result();
    pve_result->set_combat_award_exp(result.exp);
	pve_result->set_combat_award_money(result.money);

	shared::net::ByteBuffer buffer;
	buffer << (const_cast<combat::CombatPVEResult*>(&result))->items;
	std::string str_items_drop((const char*)buffer.contents(), buffer.size());

	buffer.clear();
	buffer << (const_cast<combat::CombatPVEResult*>(&result))->lottery;
	std::string str_items_lottery((const char*)buffer.contents(), buffer.size());

	buffer.clear();
	buffer << (const_cast<combat::CombatPVEResult*>(&result))->mob_killed_vec;
	std::string str_kill_mob_data((const char*)buffer.contents(), buffer.size());

	pve_result->set_combat_award_items_drop(str_items_drop);
	pve_result->set_combat_award_items_lottery(str_items_lottery);
	pve_result->set_combat_mob_killed_list(str_kill_mob_data);

	NetIO::SendToMaster(masterid, msg);
}

void NetToMaster::SaveCombatPVPResult(int32_t masterid, int32_t combat_id, int64_t roleid, int32_t unit_id, const combat::CombatPVPResult& result)
{
    G2M::SaveCombatResult msg;
    msg.set_roleid(roleid);
    msg.set_unit_id(unit_id);
    msg.set_combat_id(combat_id);
    msg.set_combat_type(G2M::SaveCombatResult::COMBAT_TYPE_PVP);
    msg.set_combat_result(result.result);
    msg.set_combat_remain_hp(result.hp);
    msg.set_combat_pet_power(result.pet_power);

	shared::net::ByteBuffer buffer;
	buffer << (const_cast<combat::CombatPVPResult*>(&result))->player_killed_vec;
	std::string str_kill_player_data((const char*)buffer.contents(), buffer.size());
    msg.mutable_pvp_result()->set_combat_player_killed_list(str_kill_player_data);
    msg.mutable_pvp_result()->set_pvp_type(result.combat_flag);

	NetIO::SendToMaster(masterid, msg);
}

void NetToMaster::QueryTeamInfo(int32_t masterid, int64_t roleid)
{
	G2M::QueryTeamInfo proto;
	proto.set_roleid(roleid);
	NetIO::SendToMaster(masterid, proto);
}

void NetToMaster::ModifyGlobalCounter(int32_t masterid, int32_t index, int32_t delta)
{
	G2M::ModifyGlobalCounter proto;
	proto.set_index(index);
	proto.set_delta(delta);
	NetIO::SendToMaster(masterid, proto);
}

void NetToMaster::SetGlobalCounter(int32_t masterid, int32_t index, int32_t value)
{
    G2M::SetGlobalCounter proto;
    proto.set_index(index);
    proto.set_value(value);
    NetIO::SendToMaster(masterid, proto);
}


///
/// MasterRpcTunnel
///
void MasterRpcTunnel::SendRPC(int32_t sid, const shared::net::RpcMessage& rpcmessage)
{
	NetIO::SendToMasterBySid(sid, (const ProtobufCodec::MessageRef)rpcmessage);
}

} // namespace gamed
