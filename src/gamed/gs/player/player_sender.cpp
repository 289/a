#include "player_sender.h"

#include "shared/base/callback_bind.h"
#include "shared/net/packet/codec_packet.h"
#include "gamed/client_proto/G2C_proto.h"

#include "gs/scene/world.h"
#include "gs/scene/slice.h"
#include "gs/netmsg/send_to_master.h"
#include "gs/item/item.h"
#include "gs/item/item_data.h"

#include "player.h"

// rpc
#include "common/rpc/gen/masterd/change_name.pb.h"

// proto
#include "common/protocol/gen/G2M/player_abnormal_logout.pb.h"
#include "common/protocol/gen/G2M/player_change_map.pb.h"
#include "common/protocol/gen/G2M/player_cmd_router.pb.h"
#include "common/protocol/gen/G2M/team_msg.pb.h"
#include "common/protocol/gen/G2M/chat_msg.pb.h"


namespace gamed {

using namespace common;
using namespace common::protocol;

PlayerSender::PlayerSender(Player& player)
	: player_(player)
{ 
}

PlayerSender::~PlayerSender()
{
}

void PlayerSender::ErrorMessage(int err_no) const
{
	G2C::ErrorMessage packet;
	packet.error_num = err_no;
	SendToSelf(packet);
}

void PlayerSender::ErrorMessageToOther(RoleID roleid, int32_t linkid, int32_t sid_in_link, int err_no) const
{
    G2C::ErrorMessage packet;
	packet.error_num = err_no;
    SendToOther(roleid, linkid, sid_in_link, packet);
}

void PlayerSender::SendToSelf(PacketRef packet) const
{
	int32_t linkid     = player_.link_id();
	RoleID roleid      = player_.role_id();
	int32_t client_sid = player_.sid_in_link();
	NetToLink::SendS2CGameData(linkid, roleid, client_sid, packet);
}

void PlayerSender::SendToOther(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet) const
{
	NetToLink::SendS2CGameData(linkid, roleid, sid_in_link, packet);
}

void PlayerSender::AutoBroadcastCSMsg(PacketRef packet) const
{
	ElsePlayersInFieldOfView::const_iterator it = else_players_in_view_.begin();
	for (; it != else_players_in_view_.end(); ++it)
	{
		NetToLink::SendS2CMulticast(it->first, it->second, packet);
	}
}

void PlayerSender::PullBackToValidPos(uint16_t move_seq, const A2DVECTOR& pos)
{
	G2C::PullPlayerBackToValidPos packet;
	packet.pos      = pos;
	packet.move_seq = move_seq;

	SendToSelf(packet);
}

void PlayerSender::BroadcastPlayerPullBack(const A2DVECTOR& pos)
{
	G2C::BroadcastPlayerPullBack packet;
	packet.roleid  = player_.role_id();
	packet.pos     = pos;

	AutoBroadcastCSMsg(packet);
}

void PlayerSender::MoveStart(const A2DVECTOR& dest, uint16_t speed, uint8_t mode)
{
	G2C::PlayerStartMove packet;
	packet.roleid   = player_.role_id();
	packet.dest     = dest;
	packet.speed    = speed;
	packet.mode     = mode;

	AutoBroadcastCSMsg(packet);
}

void PlayerSender::Move(const A2DVECTOR& cur_pos, const A2DVECTOR& dest, uint16_t speed, uint8_t mode)
{
	G2C::PlayerMove packet;
	packet.roleid   = player_.role_id();
	packet.cur_pos  = cur_pos;
	packet.dest     = dest;
	packet.speed    = speed;
	packet.mode     = mode;

	AutoBroadcastCSMsg(packet);
}

void PlayerSender::MoveStop(const A2DVECTOR& pos, uint16_t speed, uint8_t mode, uint8_t dir)
{
	G2C::PlayerStopMove packet;
	packet.roleid    = player_.role_id();
	packet.pos       = pos;
	packet.speed     = speed;
	packet.mode      = mode;
	packet.dir       = dir;

	AutoBroadcastCSMsg(packet);
}

void PlayerSender::ElsePlayerEnterView(RoleID playerid, int32_t link_id, int32_t sid_in_link)
{
	// 地图是否相互可见
	if (!player_.can_see_each_other())
		return;

	// 正常地图
	NetToLink::MulticastPlayerInfo tmpplayerinfo;
	tmpplayerinfo.role_id     = playerid;
	tmpplayerinfo.sid_in_link = sid_in_link;
	else_players_in_view_[link_id].push_back(tmpplayerinfo);

	G2C::PlayerEnterView packet;
	packet.player_info.roleid     = player_.role_id();
	packet.player_info.pos        = player_.pos();
	packet.player_info.equip_crc  = player_.equip_crc();
	packet.player_info.dir        = player_.dir();
	packet.player_info.role_class = player_.role_class();
	packet.player_info.gender     = player_.gender();
	packet.player_info.level      = player_.level();
	packet.player_info.weapon_id  = player_.GetWeaponID();
	packet.player_info.visible_state = player_.visible_state();
	SendToOther(playerid, link_id, sid_in_link, packet);
}

void PlayerSender::ElsePlayerLeaveView(RoleID id)
{
	int count = 0;
	ElsePlayersInFieldOfView::iterator it_map = else_players_in_view_.begin();
	while (it_map != else_players_in_view_.end())
	{
		ElsePlayerVec& players_vec = it_map->second;
		ElsePlayerVec::iterator it_vec = players_vec.begin();
		while (it_vec != players_vec.end())
		{
			if ((*it_vec).role_id == id) {
				it_vec = players_vec.erase(it_vec);
				++count;
			}
			else {
				++it_vec;
			}
		}
		++it_map;
	}

	// 现在异步处理是有可能EnterView还没处理，
	// 就LeaveView了,所以不能ASSERT。
	//ASSERT(count == 1); 
	if (count != 1 && player_.can_see_each_other())
	{
		LOG_WARN << "PlayerSender::ElsePlayerLeaveView count:" << count;
	}

	// be careful drama map
	if (count > 0)
	{
		G2C::PlayerLeaveView packet;
		packet.roleid = id;
		SendToSelf(packet);
	}
}

void PlayerSender::ObjectEnterView(XID xid, int32_t tid, int32_t eid, uint8_t dir, const A2DVECTOR& pos)
{
	G2C::ObjectEnterView packet;
	packet.obj_info.obj_id   = static_cast<int32_t>(xid.id); 
	packet.obj_info.tid      = tid;
	packet.obj_info.eid      = eid;
	packet.obj_info.dir      = dir;
	packet.obj_info.pos      = pos;
	SendToSelf(packet);
}

void PlayerSender::ObjectLeaveView(int64_t id)
{
	G2C::ObjectLeaveView packet;
	packet.obj_id = static_cast<int32_t>(id);
	SendToSelf(packet);
}

void PlayerSender::PlayerEnterEventArea()
{
	G2C::EnterEventArea packet;
	SendToSelf(packet);
}

void PlayerSender::EnterRuleArea(int32_t elem_id)
{
	G2C::EnterRuleArea packet;
	packet.elem_id = elem_id;
	SendToSelf(packet);
}

void PlayerSender::LeaveRuleArea(int32_t elem_id)
{
	G2C::LeaveRuleArea packet;
	packet.elem_id = elem_id;
	SendToSelf(packet);
}

void PlayerSender::ElsePlayerQueryExtProp(RoleID else_playerid, int32_t link_id, int32_t sid_in_link)
{
	G2C::ElsePlayerExtProp packet;
	packet.else_player_roleid = player_.role_id();
	player_.QueryExtProp(packet.props);
	SendToOther(else_playerid, link_id, sid_in_link, packet);
}

void  PlayerSender::ElsePlayerQueryEquipCRC(RoleID else_playerid, int32_t link_id, int32_t sid_in_link)
{
	G2C::ElsePlayerEquipCRC packet;
	packet.else_player_roleid = player_.role_id();
	packet.equip_crc = player_.equip_crc();
	SendToOther(else_playerid, link_id, sid_in_link, packet);
}

void PlayerSender::ElsePlayerQueryEquipment(RoleID else_playerid, int32_t link_id, int32_t sid_in_link)
{
	G2C::ElsePlayerEquipment packet;
	packet.else_player_roleid = player_.role_id();

	int cap = 0;
	std::vector<itemdata> list;
	player_.QueryItemlist(1/*Item::EQUIPMENT*/, cap, list);

	packet.equipment.resize(list.size());
	for (size_t i = 0; i < list.size(); ++ i)
	{
		itemdata& item = list[i];
		G2C::ElsePlayerEquipment::EquipInfo& equip = packet.equipment[i];

		equip.type = item.id;
		equip.index = item.index;
		equip.proc_type = item.proc_type;
		equip.expire_date = item.expire_date;

		equip.content.clear();
		if (item.content.size() > 0)
		{
			size_t size = item.content.size();
			equip.content.resize(size);
			::memcpy(equip.content.data(), item.content.data(), size);
		}
	}

	SendToOther(else_playerid, link_id, sid_in_link, packet);
}

void PlayerSender::ElsePlayerQueryStaticRoleInfo(RoleID else_playerid, int32_t link_id, int32_t sid_in_link)
{
	G2C::GetStaticRoleInfo_Re packet;
	packet.roleid      = player_.role_id();
	packet.create_time = player_.create_time();
	packet.first_name  = player_.first_name();
	packet.middle_name = player_.middle_name();
	packet.last_name   = player_.last_name();
	SendToOther(else_playerid, link_id, sid_in_link, packet);
}

void PlayerSender::PlayerDataSendToMaster(PlayerData* playerdata)
{
	shared::net::Buffer tmpbuf;
	if (playerdata->Serialize(&tmpbuf))
	{
		LOG_ERROR << "pdata_->Serialize() PlayerData serialize error";
		return;
	}
	NetToMaster::PlayerDataSyncToMaster(player_.master_id(), player_.role_id(), tmpbuf.peek(), tmpbuf.ReadableBytes());
}

void PlayerSender::DBSaveErrorLogout()
{
	NetToMaster::PlayerAbnormalLogout(player_.master_id(), player_.role_id(), G2M::PlayerAbnormalLogout::SAVE_TO_DB_ERR);
}

void PlayerSender::PlayerFatalError()
{
	NetToMaster::PlayerAbnormalLogout(player_.master_id(), player_.role_id(), G2M::PlayerAbnormalLogout::PLAYER_FATAL_ERR);
}

void PlayerSender::LogoutErrorOccurred()
{
	NetToMaster::PlayerAbnormalLogout(player_.master_id(), player_.role_id(), G2M::PlayerAbnormalLogout::UNKNOWN_ERR);
}

void PlayerSender::NotifyPlayerMoveProp(float run_speed)
{
	G2C::NotifyPlayerMoveProp packet;
	packet.run_speed = run_speed;
	SendToSelf(packet);
}

void PlayerSender::NotifySelfPos()
{
	G2C::NotifySelfPos packet;
	packet.map_id = player_.world_id();
	packet.pos    = player_.pos();
	SendToSelf(packet);
}

static void MakeItemInfo(G2C::SelfItemInfoList::ItemInfo& info, const itemdata& data)
{
	info.type        = data.id;
	info.index       = data.index;
	info.count       = data.count;
	info.proc_type   = data.proc_type;
	info.expire_date = data.expire_date;
}

void PlayerSender::GetAllInventory()
{
	G2C::SelfItemDetailListAll packet;

#define SAVE_INVENTORY(where) \
	{ \
		int inv_cap = 0; \
		std::vector<itemdata> itemlist; \
		player_.QueryItemlist(where, inv_cap, itemlist); \
		G2C::SelfItemDetailListAll::Inventory g2c_inventory; \
		g2c_inventory.inv_cap = inv_cap; \
		g2c_inventory.item_list.resize(itemlist.size()); \
		for (size_t i = 0; i < itemlist.size(); ++ i) \
		{ \
			MakeItemDetail(g2c_inventory.item_list[i], itemlist[i]); \
		} \
		packet.inv_list.push_back(g2c_inventory); \
	}

    for (int32_t i = Item::INVENTORY; i < Item::INV_MAX; ++i)
    {
	    SAVE_INVENTORY(i);
    }
#undef SAVE_INVENTORY

	SendToSelf(packet);
}

void PlayerSender::GainItem(int where, int type, int amount, 
                            int expire_date, size_t last_idx, int last_amount, int gain_mode,
                            const void* content, size_t content_len, uint16_t content_crc)
{
	G2C::PlayerGainItem packet;
	packet.where            = where;
	packet.item_type        = type;
	packet.item_count       = amount;
	packet.expire_date      = expire_date;
	packet.last_slot_idx    = last_idx;
	packet.last_slot_amount = last_amount;
    packet.gain_mode        = gain_mode;
	packet.content_crc      = content_crc;
	packet.content.clear();

	if (content != NULL && content_len > 0)
	{
		packet.content.resize(content_len);
		::memcpy(packet.content.data(), content, content_len);
	}
	SendToSelf(packet);
}

void PlayerSender::SelfItemData(int where, const itemdata& data, int update_type)
{
	G2C::SelfItemDetail packet;
	packet.where  = where;
    packet.uptype = update_type;
	MakeItemDetail(packet.detail, data);
	SendToSelf(packet);
}

void PlayerSender::SelfItemListBrief(int where, int inv_cap, const std::vector<itemdata>& list)
{
	G2C::SelfItemInfoList packet;
	packet.where = where;
	packet.inv_cap = inv_cap;
	packet.list.resize(list.size());
	for (size_t i = 0; i < list.size(); ++ i)
	{
		MakeItemInfo(packet.list[i], list[i]);
	}
	SendToSelf(packet);
}

void PlayerSender::SelfItemListDetail(int where, int inv_cap, const std::vector<itemdata>& list)
{
	G2C::SelfItemDetailList packet;
	packet.where = where;
	packet.inv_cap = inv_cap;
	packet.list.resize(list.size());
	for (size_t i = 0; i < list.size(); ++ i)
	{
		MakeItemDetail(packet.list[i], list[i]);
	}
	SendToSelf(packet);
}

void PlayerSender::DropInvItem(int type, size_t index, int count)
{
	G2C::DropInvItem packet;
	packet.type  = type;
	packet.index = index;
	packet.count = count;
	SendToSelf(packet);
}

void PlayerSender::MoveInvItem(size_t dest, size_t src, int count)
{
	G2C::MoveInvItem packet;
	packet.dest_idx = dest;
	packet.src_idx  = src;
	packet.count    = count;
	SendToSelf(packet);
}

void PlayerSender::SellInvItem(int8_t where, size_t index, int count)
{
	G2C::SellInvItem packet;
    packet.where = where;
	packet.index = index;
	packet.count = count;
	SendToSelf(packet);
}

void PlayerSender::ExchangeInvItem(size_t index1, size_t index2)
{
	G2C::ExchangeInvItem packet;
	packet.index1 = index1;
	packet.index2 = index2;
	SendToSelf(packet);
}

void PlayerSender::EquipItem(size_t idx_inv, size_t idx_equip)
{
	G2C::EquipItem packet;
	packet.idx_inv     = idx_inv;
	packet.idx_equip   = idx_equip;
	SendToSelf(packet);
}

void PlayerSender::UndoEquip(size_t idx_inv, size_t idx_equip)
{
	G2C::UndoEquip packet;
	packet.idx_inv   = idx_inv;
	packet.idx_equip = idx_equip;
	SendToSelf(packet);
}

void PlayerSender::UseItem(int where, int type, size_t index, int count)
{
	G2C::UseItem packet;
	packet.where = where;
	packet.type  = type;
	packet.index = index;
	packet.count = count;
	SendToSelf(packet);
}

void PlayerSender::UseItemError(G2C::UseItemErrorCode err_num, int type)
{
    G2C::UseItemError packet;
    packet.error_num = static_cast<int16_t>(err_num);
    packet.type      = type;
    SendToSelf(packet);
}

void PlayerSender::RefineEquipReply(int where, const itemdata& data)
{
	G2C::RefineEquip_Re packet;
	packet.where = where;
	MakeItemDetail(packet.detail, data);
	SendToSelf(packet);
}

void PlayerSender::GetOwnMoney()
{
	G2C::GetOwnMoney packet;
	packet.money = player_.GetMoney();
	SendToSelf(packet);
}

void PlayerSender::GainMoney(int64_t money)
{
	G2C::GainMoney packet;
	packet.amount = money;
	SendToSelf(packet);
}

void PlayerSender::SpendMoney(int64_t money)
{
	G2C::SpendMoney packet;
	packet.cost = money;
	SendToSelf(packet);
}

void PlayerSender::GetOwnScore()
{
	G2C::PlayerScore packet;
	packet.score_total = player_.GetScoreTotal();
	packet.score_used = player_.GetScoreUsed();
	SendToSelf(packet);
}

void PlayerSender::GainScore(int32_t score)
{
	G2C::GainScore packet;
	packet.amount = score;
	SendToSelf(packet);
}

void PlayerSender::SpendScore(int32_t score)
{
	G2C::SpendScore packet;
	packet.cost = score;
	SendToSelf(packet);
}

void PlayerSender::SetCoolDown(int cd_index, int cd_interval)
{
	G2C::SetCoolDown packet;
	packet.cd_ent.index    = cd_index;
	packet.cd_ent.interval = cd_interval;
	SendToSelf(packet);
}

void PlayerSender::SendCmd(PacketRef packet) const
{
	SendToSelf(packet);
}

void PlayerSender::BroadCastCmd(PacketRef packet) const
{
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::PlayerGainExp(int32_t exp)
{
	G2C::PlayerGainExp packet;
	packet.exp_gain = exp;
	packet.exp_cur = player_.GetExp();
	SendToSelf(packet);
}

void PlayerSender::PlayerLevelUp()
{
	G2C::PlayerLevelUp packet;
	packet.roleid = player_.role_id();
	packet.new_level = player_.level();
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::PlayerDead()
{
	G2C::PlayerDead packet;
	packet.roleid = player_.role_id();
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::PlayerResurrect()
{
	G2C::PlayerResurrect packet;
	packet.roleid = player_.role_id();
	packet.hp = player_.GetHP();
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}
/*
void PlayerSender::PlayerTransferCls()
{
	G2C::TransferCls packet;
	packet.roleid = player_.role_id();
	packet.dest_cls = player_.role_class();
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::PlayerTransferGender()
{
	G2C::TransferGender packet;
	packet.roleid = player_.role_id();
	packet.gender = player_.gender();
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}
*/
void PlayerSender::PlayerVisibleInfoChange(int64_t visible_mask, const std::vector<int32_t>& visible_list)
{
	G2C::PlayerVisibleInfoChange packet;
	packet.roleid = player_.role_id();
	packet.visible_mask = visible_mask;
	packet.visible_list = visible_list;
    SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::PlayerUpdateTitle(int32_t title_id)
{
    G2C::BroadcastPlayerTitle packet;
    packet.roleid = player_.role_id();
    packet.title_id = title_id;
    AutoBroadcastCSMsg(packet);
}

void PlayerSender::PlayerBaseInfo()
{
	G2C::PlayerBaseInfo packet;
	packet.hp        = player_.GetHP();
	packet.max_hp    = player_.GetMaxHP();
	packet.mp        = player_.GetMP();
	packet.max_mp    = player_.GetMaxMP();
	packet.ep        = player_.GetEP();
	packet.max_ep    = player_.GetEP();
	packet.exp       = player_.GetExp();
	packet.level     = player_.GetLevel();
	packet.cat_exp   = player_.GetCatVisionExp();
	packet.cat_level = player_.GetCatVisionLevel();
	SendToSelf(packet);
}

void PlayerSender::PlayerVolatileInfo()
{
	G2C::PlayerVolatileInfo packet;
	packet.hp = player_.GetHP();
	packet.mp = player_.GetMP();
	packet.ep = player_.GetEP();
	SendToSelf(packet);
}

void PlayerSender::PlayerExtendProp()
{
	//保存变化的属性
	std::vector<int32_t> extprop_list;
	int32_t mask = player_.GetRefreshExtProp();
	int32_t index = 0;
	while (mask)
	{
		while (index < PROP_INDEX_HIGHEST)
		{
			if (mask & (1 << index))
				break;
			else
				++ index;
		}

		int32_t prop_value = player_.GetMaxProp(index);
		extprop_list.push_back(prop_value);

		//消去最低位1
		mask &= mask-1;
	};

	ASSERT(extprop_list.size() <= PROP_INDEX_HIGHEST);

	G2C::PlayerExtendProp packet;
	packet.prop_mask = player_.GetRefreshExtProp();
	packet.props = extprop_list;
	SendToSelf(packet);
}

void PlayerSender::EnterCombat(int32_t combat_id)
{
	G2C::ElsePlayerEnterCombat packet;
	packet.roleid    = player_.role_id();
	packet.combat_id = combat_id;
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::LeaveCombat()
{
	G2C::ElsePlayerLeaveCombat packet;
	packet.roleid    = player_.role_id();
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::JoinCombatFail(int32_t combat_id)
{
	G2C::CombatPlayerJoinFail packet;
	packet.combat_id = combat_id;
	SendToSelf(packet);
}

void PlayerSender::TeammateEnterCombatFail(int64_t teammate_roleid)
{
	G2C::TeammateEnterCombatFail packet;
	packet.teammate_roleid = teammate_roleid;
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::LearnSkill_Re(int skill_idx)
{
	G2C::LearnSkill_Re packet;
	packet.skill_idx = skill_idx;
	SendToSelf(packet);
}

void PlayerSender::SwitchSkill_Re(int skill_idx)
{
	G2C::SwitchSkill_Re packet;
	packet.skill_idx = skill_idx;
	SendToSelf(packet);
}

void PlayerSender::UpdateSkillTree(int sk_tree_id, int skill_idx, int lvl_cur, int lvl_tmp, bool is_active)
{
	G2C::UpdateSkillTree packet;
	packet.skill_tree_id = sk_tree_id;
	packet.skill_idx     = skill_idx;
	packet.cur_level     = lvl_cur;
	packet.tmp_level     = lvl_tmp;
	packet.is_active     = is_active ? 1 : 0;
	SendToSelf(packet);
}

void PlayerSender::NpcGreeting(const XID& xid)
{
	G2C::NpcGreeting packet;
	packet.npc_id = xid.id;
	SendToSelf(packet);
}

void PlayerSender::TeamMemberQueryRe(RoleID query_roleid, int32_t linkid, int32_t sid_in_link)
{
	G2C::QueryTeamMemberRe packet;
	packet.member_roleid = player_.role_id();
	packet.level         = player_.level();;
	packet.weapon_id     = player_.GetWeaponID();
	packet.combat_value  = player_.CalcCombatValue();
	packet.is_in_combat  = player_.InCombat();
	packet.cls           = player_.role_class();

	SendToOther(query_roleid, linkid, sid_in_link, packet);
}

void PlayerSender::MapTeamQueryMemberRe(RoleID query_roleid, int32_t linkid, int32_t sid_in_link)
{
	G2C::MapTeamQueryMemberRe packet;
	packet.member_roleid = player_.role_id();
	packet.level         = player_.level();;
	packet.weapon_id     = player_.GetWeaponID();
	packet.combat_value  = player_.CalcCombatValue();
	packet.is_in_combat  = player_.InCombat();
	packet.cls           = player_.role_class();

	SendToOther(query_roleid, linkid, sid_in_link, packet);
}

void PlayerSender::JoinTeamReq(RoleID requester, const std::string& first_name, const std::string& mid_name, const std::string& last_name, int32_t invite)
{
	G2C::JoinTeamReq packet;
	packet.requester = requester;
	packet.first_name = first_name;
	packet.mid_name = mid_name;
	packet.last_name = last_name;
	packet.invite    = invite;

	SendToSelf(packet);
}

void PlayerSender::QueryTeamInfo()
{
	NetToMaster::QueryTeamInfo(player_.master_id(), player_.role_id());
}

void PlayerSender::ConveneTeammate()
{
    G2M::ConveneTeammate proto;
    proto.set_leaderid(player_.role_id());
    proto.set_world_id(player_.world_id());
    proto.set_pos_x(player_.pos().x);
    proto.set_pos_y(player_.pos().y);

    SendToMaster(proto);
}

bool PlayerSender::PlayerChangeMap(int32_t target_world_id, const A2DVECTOR& pos, const shared::net::Buffer& playerdata)
{
	G2M::PlayerChangeMapRequest proto;
    proto.set_userid(player_.user_id());
	proto.mutable_data()->set_src_mapid(player_.world_id());
	proto.mutable_data()->set_src_x(player_.pos().x);
	proto.mutable_data()->set_src_y(player_.pos().y);
	proto.mutable_data()->set_des_mapid(target_world_id);
	proto.mutable_data()->set_des_x(pos.x);
	proto.mutable_data()->set_des_y(pos.y);
	proto.mutable_data()->set_roleid(player_.role_id());
	proto.mutable_data()->set_link_id(player_.link_id());
	proto.mutable_data()->set_client_sid_in_link(player_.sid_in_link());
	proto.mutable_data()->set_content(playerdata.peek(), playerdata.ReadableBytes());

	if (IS_INS_MAP(target_world_id))
	{
		if (!player_.GetRuntimeInsInfo(target_world_id, *proto.mutable_ins_info()))
		{
			LOG_WARN << "玩家" << player_.role_id() << "传送副本的runtime数据有误！" << target_world_id;
			return false;
		}
	}
    else if (IS_BG_MAP(target_world_id))
    {
        if (!player_.GetRuntimeBGInfo(target_world_id, *proto.mutable_bg_info()))
		{
			LOG_WARN << "玩家" << player_.role_id() << "传送战场的runtime数据有误！" << target_world_id;
			return false;
		}
    }

    // 是否需要把大世界组队带入该地图
    if (player_.IsChangeMapFillMapTeam(target_world_id))
    {
        proto.set_need_map_team(true);
    }

	// correction of src pos
    // ins_map or bg_map
    if (!IS_NORMAL_MAP(player_.world_id()))
	{
		int32_t src_wid;
		A2DVECTOR tmp_pos;
		ASSERT(player_.GetSpecSavePos(src_wid, tmp_pos));
		proto.mutable_data()->set_src_mapid(src_wid);
		proto.mutable_data()->set_src_x(tmp_pos.x);
		proto.mutable_data()->set_src_y(tmp_pos.y);
	}

	SendToMaster(proto);
    return true;
}

void PlayerSender::JoinTeam(RoleID other_roleid)
{
	G2M::JoinTeam proto;
	proto.set_roleid(player_.role_id());
	proto.set_other_roleid(other_roleid);
	SendToMaster(proto);
}

void PlayerSender::JoinTeamRes(bool invite, bool accept, RoleID requester)
{
	G2M::JoinTeamRes proto;
	proto.set_roleid(player_.role_id());
	proto.set_invite(invite);
	proto.set_accept(accept);
	proto.set_requester(requester);
	SendToMaster(proto);
}

void PlayerSender::LeaveTeam()
{
	G2M::LeaveTeam proto;
	proto.set_roleid(player_.role_id());
	SendToMaster(proto);
}

void PlayerSender::SendChatMsg(int8_t channel, int64_t sender, int64_t receiver, const std::string& sender_name, const std::string& msg)
{
	G2M::ChatMsg proto;
	proto.set_channel((G2M::ChatMsg_ChannelType)channel);
	proto.set_sender(sender);
	proto.set_sender_name(sender_name);
	proto.set_receiver(receiver);
	proto.set_msg(msg);
	SendToMaster(proto);
}

void PlayerSender::SendToMaster(const shared::net::ProtobufCodec::MessageRef proto) const
{
	NetToMaster::SendProtocol(player_.master_id(), proto);
}

void PlayerSender::SendCmdByMaster(int64_t roleid, PacketRef packet) const
{
	G2M::PlayerCmdRouter proto;
	proto.set_cmd_type_no(packet.GetType());
	proto.set_roleid(roleid);
	// GetSize() == 0 判断是否已经做了Marshal, 防止循环里多次Marshal
	if (packet.GetSize() == 0 && !ProtoPacketCodec::MarshalPacket(packet))
	{
		LOG_ERROR << "SendCmdByMaster() MarshalPacket error packet_type:" << packet.GetType();
		return;
	}
	proto.set_content(packet.GetContent(), packet.GetSize());
	SendToMaster(proto);
}

void PlayerSender::TaskNotifyClient(uint16_t type, const void* buf, size_t size)
{
	G2C::TaskNotifyClient packet;
	packet.type = type;
	packet.databuf.assign((const char*)buf, size);
	SendToSelf(packet);
}

void PlayerSender::AchieveNotifyClient(uint16_t type, const void* buf, size_t size)
{
    G2C::AchieveNotifyClient packet;
    packet.type = type;
	packet.databuf.assign((const char*)buf, size);
	SendToSelf(packet);
}

void PlayerSender::TransferPrepare(int32_t elem_id)
{
	G2C::TransferPrepare packet;
    packet.elem_id = elem_id;
	SendToSelf(packet);
}

void PlayerSender::MapKickoutCountdown(int32_t secs)
{
	G2C::MapKickoutCountdown packet;
	packet.countdown_secs = secs;
	SendToSelf(packet);
}

void PlayerSender::GatherStart(int64_t matter_id, int32_t use_time, int32_t gather_seq_no, int32_t mattertid)
{
	///
	/// 发给自己
	///
	
	// 普通矿
	if (matter_id != player_.role_id())
	{
		G2C::SelfGatherStart packet_s;
		packet_s.matter_id     = static_cast<int32_t>(matter_id);
		packet_s.use_time      = use_time;
		packet_s.gather_seq_no = gather_seq_no;
		SendToSelf(packet_s);

		// 广播给别人
		G2C::GatherStart packet_b;
		packet_b.role_id   = player_.role_id();
		packet_b.matter_id = static_cast<int32_t>(matter_id);
		packet_b.use_time  = use_time;
		AutoBroadcastCSMsg(packet_b);
	}
	else // 剧情矿
	{
		G2C::DramaGatherStart packet_s;
		packet_s.matter_tid     = static_cast<int32_t>(mattertid);
		packet_s.use_time      = use_time;
		packet_s.gather_seq_no = gather_seq_no;
		SendToSelf(packet_s);
	}
}

void PlayerSender::GatherStop(int64_t matter_id, int8_t reason, int32_t mattertid)
{
	if (matter_id != player_.role_id())
	{
		G2C::GatherStop packet;
		packet.role_id = player_.role_id();
		packet.obj_id  = matter_id;
		packet.reason  = reason;
		SendToSelf(packet);

		AutoBroadcastCSMsg(packet);
	}
	else
	{
		G2C::DramaGatherStop packet;
		packet.matter_tid = mattertid;
		packet.reason     = reason;
		SendToSelf(packet);
	}
}

void PlayerSender::UpdateVisibleBuff(const std::vector<PlayerVisibleState::BuffInfo>& buff_vec)
{
	G2C::UpdateVisibleBuff packet;
	packet.roleid = player_.role_id();
	packet.visible_buff_vec = buff_vec;
	SendToSelf(packet);
	AutoBroadcastCSMsg(packet);
}

void PlayerSender::CatVisionGainExp(int32_t exp)
{
	G2C::CatVisionGainExp packet;
	packet.exp_gain = exp;
	packet.exp_cur  = player_.GetCatVisionExp();
	SendToSelf(packet);
}

void PlayerSender::CatVisionLevelUp()
{
	G2C::CatVisionLevelUp packet;
	packet.new_level = player_.GetCatVisionLevel();
	SendToSelf(packet);
}

void PlayerSender::CatVisionOpen_Re(int result)
{
	G2C::OpenCatVision_Re packet;
	packet.result = result;
	SendToSelf(packet);
}

void PlayerSender::CatVisionClose_Re(int result)
{
	G2C::CloseCatVision_Re packet;
	packet.result = result;
	SendToSelf(packet);
}

void PlayerSender::CatVisionClose()
{
	G2C::CloseCatVision packet;
	SendToSelf(packet);
}

void PlayerSender::ObjectBWList(const playerdef::ObjectBWList& obj_bw_list)
{
	G2C::ObjectBWList packet;
	packet.black_list = obj_bw_list.black_list;
	packet.white_list = obj_bw_list.white_list;
	SendToSelf(packet);
}

void PlayerSender::ObjectBWListChange(int32_t templ_id, bool is_black, bool is_add)
{
	G2C::ObjectBWListChange packet;
	packet.templ_id = templ_id;
	packet.is_black = static_cast<int8_t>(is_black);
	packet.is_add   = static_cast<int8_t>(is_add);
	SendToSelf(packet);
}

void PlayerSender::ChangeNameRe(const masterRpc::ChangeNameProxyResponse& response, bool is_timeout)
{
	// err code
	int8_t err_code = -1;
	if (is_timeout)
	{
		err_code = G2C::ChangeName_Re::TIMEOUT;
	}
	else if (response.error() == masterRpc::ChangeNameProxyResponse::NAME_EXIST)
	{
		err_code = G2C::ChangeName_Re::NAME_EXIST;
	}
	else if (response.error() != masterRpc::ChangeNameProxyResponse::NO_ERROR)
	{
		err_code = G2C::ChangeName_Re::RPC_RETURN_ERR;
	}
	else
	{
		err_code = G2C::ChangeName_Re::NO_ERR;
	}

	// name type
	int8_t name_type;
	if (response.type() == masterRpc::NAME_FIRST)
	{
		name_type = playerdef::NT_FIRST_NAME;
	}
	else if (response.type() == masterRpc::NAME_MID)
	{
		name_type = playerdef::NT_MIDDLE_NAME;
	}
	else if (response.type() == masterRpc::NAME_LAST)
	{
		name_type = playerdef::NT_LAST_NAME;
	}
	else // error
	{
		return;
	}

	// success
	if (err_code == G2C::ChangeName_Re::NO_ERR)
	{
		G2C::ChangeName_Re packet;
		packet.err_code    = err_code;
		packet.name_type   = name_type;
		packet.name        = response.name();
		SendToSelf(packet);

		G2C::BroadcastChangeName b_packet;
		b_packet.role_id   = player_.role_id();
		b_packet.name_type = name_type;
		b_packet.name      = response.name();
		AutoBroadcastCSMsg(b_packet);
	}
	else // error
	{
		G2C::ChangeName_Re packet;
		packet.err_code    = err_code;
		packet.name_type   = name_type;
		packet.name        = std::string();
		SendToSelf(packet);
	}
}

void PlayerSender::SelfStaticRoleInfo()
{
	G2C::SelfStaticRoleInfo packet;
	packet.create_time = player_.create_time();
	packet.first_name  = player_.first_name();
	packet.middle_name = player_.middle_name();
	packet.last_name   = player_.last_name();
	SendToSelf(packet);
}

void PlayerSender::PlayerEnterInsMap(int32_t ins_create_time, int32_t ins_templ_id)
{
	G2C::PlayerEnterInsMap packet;
	packet.ins_create_time = ins_create_time;
	packet.ins_templ_id    = ins_templ_id;
	SendToSelf(packet);
}

void PlayerSender::PlayerLeaveInsMap(int32_t ins_templ_id)
{
    G2C::PlayerLeaveInsMap packet;
    packet.ins_templ_id = ins_templ_id;
    SendToSelf(packet);
}

void PlayerSender::GetInstanceDataRe(int32_t ins_templ_id, bool is_cross_realm)
{
	G2C::GetInstanceData_Re packet;
	packet.ins_templ_id   = ins_templ_id;
    packet.is_cross_realm = is_cross_realm ? 1 : 0;
	SendToSelf(packet);
}

void PlayerSender::PlayerEnterBGMap(int32_t bg_create_time, int32_t bg_templ_id)
{
    G2C::PlayerEnterBGMap packet;
	packet.bg_create_time = bg_create_time;
	packet.bg_templ_id    = bg_templ_id;
	SendToSelf(packet);
}

void PlayerSender::PlayerLeaveBGMap(int32_t bg_templ_id)
{
    G2C::PlayerLeaveBGMap packet;
    packet.bg_templ_id = bg_templ_id;
    SendToSelf(packet);
}

void PlayerSender::GetBattleGroundDataRe(int32_t bg_templ_id, bool is_cross_realm)
{
	G2C::GetBattleGroundData_Re packet;
	packet.bg_templ_id    = bg_templ_id;
    packet.is_cross_realm = is_cross_realm ? 1 : 0;
	SendToSelf(packet);
}

void PlayerSender::ShopShoppingFailed(int ret, int32_t goods_id, int32_t goods_count, int32_t goods_remains)
{
	G2C::ShopShopping_Re packet;
	packet.result        = ret;
	packet.goods_id      = goods_id;
	packet.goods_count   = goods_count;
	packet.goods_remains = goods_remains;
	SendToSelf(packet);
}

void PlayerSender::ElsePlayerQueryItem(int8_t result, const itemdata& data, int64_t query_roleid, int32_t linkid, int32_t sid_in_link)
{
	G2C::QueryItemInfo_Re packet;
	packet.result = result;
	if (result == G2C::QueryItemInfo_Re::ERR_SUCCESS)
	{
		MakeItemDetail(packet.detail, data);
	}
	SendToOther(query_roleid, linkid, sid_in_link, packet);
}

void PlayerSender::LevelUpCard(int8_t result, int8_t lvlup_type)
{
    G2C::LevelUpCard_Re packet;
    packet.result = result;
    packet.lvlup_type      = lvlup_type;
    SendToSelf(packet);
}

void PlayerSender::PlayerDuelPacket(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet)
{
    SendToOther(roleid, linkid, sid_in_link, packet);
}

void PlayerSender::PunchCardPacket(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet)
{
    SendToOther(roleid, linkid, sid_in_link, packet);
}

void PlayerSender::EnhanceReply(int32_t err, int8_t slot_index, int32_t enhance_id, int32_t count)
{
    G2C::EnhanceRe packet;
    packet.err = err;
    packet.slot_index = slot_index;
    packet.enhance_id = enhance_id;
    packet.count = count;
	SendToSelf(packet);
}

} // namespace gamed

