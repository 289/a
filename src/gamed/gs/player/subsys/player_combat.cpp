#include "player_combat.h"

#include "gs/global/message.h"
#include "gs/global/dbgprt.h"
#include "gs/global/game_util.h"
#include "gs/global/gmatrix.h"
#include "gs/global/game_types.h"
#include "gs/global/gmatrix.h"
#include "gs/global/msg_pack_def.h"
#include "gs/global/glogger.h"
#include "gs/item/item.h"
#include "gs/item/item_list.h"
#include "gs/player/player_sender.h"
#include "gs/player/psession.h"
#include "gs/player/subsys_if.h"
#include "gs/scene/world.h"
#include "gs/obj/npc.h"

#include "gs/template/data_templ/monster_templ.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gamed/client_proto/G2C_proto.h"
#include "game_module/combat/include/combat_header.h"
#include "common/protocol/gen/global/player_combat_data.pb.h"


namespace gamed {

using namespace common::protocol;
using namespace dataTempl;

namespace 
{
	static const int kCombatQueryTimeoutSecs = 3; // 单位秒

	inline void get_buddy_members(const Player* pplayer, combat::pve_combat_param* param)
	{
		std::vector<playerdef::BuddyMemberInfo> tmp_vec;
		if (pplayer->GetBuddyMembers(tmp_vec))
		{
			for (size_t i = 0; i < tmp_vec.size(); ++i)
			{
				combat::pve_combat_param::team_npc teamnpc;
				teamnpc.id  = tmp_vec[i].tpl_id;
				teamnpc.pos = tmp_vec[i].pos_index;
				param->team_npc_vec.push_back(teamnpc);
			}
		}
		else
		{
			param->team_npc_vec.clear();
		}
	}

    inline void build_combat_data(const combat::player_data& pdata, global::PlayerCombatData& combat_data)
    {
        combat_data.set_roleid(pdata.roleid);
        combat_data.set_name(pdata.name);
        combat_data.set_pos(pdata.pos);
        combat_data.set_cls(pdata.cls);
        combat_data.set_gender(pdata.gender);
        combat_data.set_level(pdata.level);
        combat_data.set_master_id(pdata.master_id);
        combat_data.set_weapon_id(pdata.weapon_id);
        combat_data.set_normal_atk_id(pdata.normal_atk_id);
        combat_data.set_hp(pdata.hp); // 应该是取的最大血量
        combat_data.set_mp(pdata.mp);
        combat_data.set_ep(pdata.ep);
        combat_data.set_dying_time(pdata.dying_time);
        combat_data.set_cls_model_src(pdata.cls_model_src);

        for (size_t i = 0; i < pdata.props.size(); ++i)
        {
            combat_data.add_props(pdata.props[i]);
        }

        std::set<int32_t>::iterator it_addon = pdata.addon_skills.begin();
        for (; it_addon != pdata.addon_skills.end(); ++it_addon)
        {
            combat_data.add_addon_skills(*it_addon);
        }

        for (size_t i = 0; i < pdata.skill_tree_vec.size(); ++i)
        {
            const combat::skill_tree& tmp = pdata.skill_tree_vec[i];
            global::PlayerCombatData::SkillTree* tree = combat_data.add_skill_tree_vec();
            tree->set_skill_tree_id(tmp.skill_tree_id);
            tree->set_skill_id(tmp.skill_id);
            tree->set_level(tmp.level);
            tree->set_tmp_level(tmp.tmp_level);
            tree->set_max_level(tmp.max_level);
        }

        // pet data
        combat_data.mutable_pet_data()->set_pet_power(pdata.petdata.pet_power);
        combat_data.mutable_pet_data()->set_pet_power_cap(pdata.petdata.pet_power_cap);
        combat_data.mutable_pet_data()->set_pet_power_gen_speed(pdata.petdata.pet_power_gen_speed);
        combat_data.mutable_pet_data()->set_pet_attack_cd_time(pdata.petdata.pet_attack_cd_time);
        for (size_t i = 0; i < pdata.petdata.pet_vec.size(); ++i)
        {
            const combat::pet_entry& tmp = pdata.petdata.pet_vec[i];
            global::PlayerCombatData::PetEntry* ent = combat_data.mutable_pet_data()->add_pet_vec();
            ent->set_pet_id(tmp.pet_id);
            ent->set_pet_rank(tmp.pet_rank);
            ent->set_pet_blevel(tmp.pet_blevel);
            ent->set_pet_item_idx(tmp.pet_item_idx);
            ent->set_pet_combat_pos(tmp.pet_combat_pos);
        }
    }

} // Anonymous


PlayerCombat::PlayerCombat(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_COMBAT, player),
	  unit_id_(0),
	  combat_id_(0),
	  combat_result_(0),
	  combat_remain_hp_(0),
	  combat_award_exp_(0),
	  combat_award_money_(0),
	  time_waiting_mob_response_(0),
	  is_waiting_mob_response_(false),
	  is_waiting_combat_result_(false),
	  monster_object_id_(0),
	  combat_query_count_(0),
	  combat_query_timeout_(GS_INT32_MAX),
      combat_pvp_type_(playerdef::PVP_TYPE_NONE)
{
	SAVE_LOAD_REGISTER(common::PlayerCombatData, PlayerCombat::SaveToDB, PlayerCombat::LoadFromDB);
}

PlayerCombat::~PlayerCombat()
{
}

void PlayerCombat::OnEnterWorld()
{
	//上线时的处理逻辑放到PlayerGetCombatData中处理，因为在这里同步给客户端初始化顺序错乱.
}

void PlayerCombat::OnLeaveWorld()
{
	combat::PlayerCMD tmp_cmd;
	tmp_cmd.cmd_no  = combat::COMBAT_CMD_PLAYER_OFFLINE;
	tmp_cmd.roleid  = player_.role_id();

	combat::DispatchCmd(tmp_cmd, combat_id_, unit_id_);

    // 玩家的战斗数据发给master，竞技场需要使用
    //SyncPlayerCombatData();
}

void PlayerCombat::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::CombatPlayerJoin, PlayerCombat::CMDHandler_JoinCombat);
	REGISTER_NORMAL_CMD_HANDLER(C2G::CombatPlayerTrigger, PlayerCombat::CMDHandler_TriggerCombat);
	REGISTER_NORMAL_CMD_HANDLER(C2G::CombatSelectSkill, PlayerCombat::CMDHandler_SelectSkill);
	REGISTER_NORMAL_CMD_HANDLER(C2G::CombatPetAttack, PlayerCombat::CMDHandler_PetAttack);
}

void PlayerCombat::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_OBJ_TRIGGER_COMBAT, PlayerCombat::MSGHandler_ObjTriggerCombat);
	REGISTER_MSG_HANDLER(GS_MSG_SYS_TRIGGER_COMBAT, PlayerCombat::MSGHandler_SysTriggerCombat);
	REGISTER_MSG_HANDLER(GS_MSG_PLAYER_TRIGGER_COMBAT_RE, PlayerCombat::MSGHandler_PlayerTriggerCombatRe);
	REGISTER_MSG_HANDLER(GS_MSG_COMBAT_PVE_RESULT, PlayerCombat::MSGHandler_CombatPVEResult);
	REGISTER_MSG_HANDLER(GS_MSG_COMBAT_PVP_RESULT, PlayerCombat::MSGHandler_CombatPVPResult);
	REGISTER_MSG_HANDLER(GS_MSG_COMBAT_START, PlayerCombat::MSGHandler_CombatStart);
	REGISTER_MSG_HANDLER(GS_MSG_COMBAT_PVE_END, PlayerCombat::MSGHandler_CombatPVEEnd);
	REGISTER_MSG_HANDLER(GS_MSG_COMBAT_PVP_END, PlayerCombat::MSGHandler_CombatPVPEnd);
	REGISTER_MSG_HANDLER(GS_MSG_HATE_YOU, PlayerCombat::MSGHandler_SomeoneHateYou);
	REGISTER_MSG_HANDLER(GS_MSG_COMPANION_ENTER_COMBAT, PlayerCombat::MSGHandler_CompanionEnterCombat);
	REGISTER_MSG_HANDLER(GS_MSG_QUERY_TEAM_COMBAT, PlayerCombat::MSGHandler_QueryTeamCombat);
	REGISTER_MSG_HANDLER(GS_MSG_QUERY_TEAM_COMBAT_RE, PlayerCombat::MSGHandler_QueryTeamCombatRe);
}

bool PlayerCombat::SaveToDB(common::PlayerCombatData* pData)
{
	pData->unit_id            = unit_id_;
	pData->combat_id          = combat_id_ ;
    pData->combat_type        = combat_type_;
	pData->combat_result      = combat::RESULT_INVALID;
	pData->combat_remain_hp   = 0;
    pData->combat_pet_power   = 0;
	pData->combat_award_exp   = 0;
	pData->combat_award_money = 0;
    pData->combat_pvp_type    = playerdef::PVP_TYPE_NONE;
	pData->combat_mob_killed_list.clear();
	pData->combat_award_items_drop.clear();
	pData->combat_award_items_lottery.clear();
    pData->combat_player_killed_list.clear();
	return true;
}

bool PlayerCombat::LoadFromDB(const common::PlayerCombatData& data)
{
	unit_id_            = data.unit_id;
	combat_id_          = data.combat_id;
    combat_type_        = data.combat_type;
	combat_result_      = data.combat_result;
	combat_remain_hp_   = data.combat_remain_hp;
    combat_pet_power_   = data.combat_pet_power;
	combat_award_exp_   = data.combat_award_exp;
	combat_award_money_ = data.combat_award_money;
    combat_pvp_type_    = data.combat_pvp_type;

    try 
    {
        shared::net::ByteBuffer buffer;
        if (data.combat_award_items_drop.size() > 0)
        {
            buffer.append(data.combat_award_items_drop.c_str(), data.combat_award_items_drop.size());
            buffer >> combat_award_items_drop_;
        }

        if (data.combat_award_items_lottery.size() > 0)
        {
            buffer.append(data.combat_award_items_lottery.c_str(), data.combat_award_items_lottery.size());
            buffer >> combat_award_items_lottery_;
        }

        buffer.clear();
        if (data.combat_mob_killed_list.size() > 0)
        {
            buffer.append(data.combat_mob_killed_list.c_str(), data.combat_mob_killed_list.size());
            buffer >> mob_killed_vec_;
        }

        buffer.clear();
        if (data.combat_player_killed_list.size() > 0)
        {
            buffer.append(data.combat_player_killed_list.c_str(), data.combat_player_killed_list.size());
            buffer >> player_killed_vec_;
        }
    }
    catch (...)
    {
        GLog::log("玩家 %ld战斗存盘数据Load失败！丢弃数据！", player_.role_id());
        return true;
    }

	return true;
}

void PlayerCombat::OnHeartbeat(time_t cur_time)
{
	if (IsWaitMobResponse())
	{
		if (-- time_waiting_mob_response_ <= 0)
		{
			//玩家触发战斗后通知怪物，但是怪物无响应，
			//等待时间超时，自动清除玩家身上的等待标记
			ClrWaitMobResponse();
		}

		CheckQueryTeamCombat();
	}
}

void PlayerCombat::PlayerGetCombatData()
{
	if (!InCombat())
		return;

	/**
	 * 战斗过程中玩家离线:
	 * 1: 战斗过程中下线或掉线，战斗尚未结束;
	 * 2: 战斗过程中下线或掉线，战斗已结束;
	 */
	if (combat_result_ == combat::RESULT_INVALID)
	{
		//战斗尚未结束
		//玩家重新进入战斗
        combat::PlayerCMD tmp_cmd;
        tmp_cmd.cmd_no  = combat::COMBAT_CMD_PLAYER_ONLINE;
        tmp_cmd.roleid  = player_.role_id();
        if (!combat::DispatchCmd(tmp_cmd, combat_id_, unit_id_))
        {
			ClrCombatInfo();
			return;
        }

        OnEnterCombat();
	}
	else
	{
		//战斗已结束
		//同步战斗结果给玩家

        if (combat_type_ == COMBAT_TYPE_PVE)
        {
            //PVE战斗结束

            //玩家获得战斗奖励
            if (combat_award_exp_ > 0)
                player_.IncExp(combat_award_exp_);
            if (combat_award_money_ > 0)
                player_.GainMoney(combat_award_money_);

            //玩家获得掉落物品奖励
            for (size_t i = 0; i < combat_award_items_drop_.size(); ++ i)
            {
                ItemEntry& item = combat_award_items_drop_[i];
                player_.GainItem(item.item_id, item.item_count);
            }

            //玩家获得抽奖物品奖励
            for (size_t i = 0; i < combat_award_items_lottery_.size(); ++ i)
            {
                ItemEntry& item = combat_award_items_lottery_[i];
                player_.GainItem(item.item_id, item.item_count);
            }

            //通知任务系统
            std::map<int32_t, int> task_award_items_map;
            for (size_t i = 0; i < mob_killed_vec_.size(); ++ i)
            {
                MobKilled& entry = mob_killed_vec_[i];
                std::vector<playerdef::ItemEntry> task_items;
                player_.KillMonsterLastTime(entry.mob_id, entry.mob_killed, task_items);
                for (size_t j = 0; j < task_items.size(); ++ j)
                {
                    playerdef::ItemEntry& item = task_items[j];
                    task_award_items_map[item.item_id] += item.item_count;
                }
            }

            //同步战斗结果给客户端
            G2C::CombatPVEEnd packet;
            packet.result = combat_result_;
            packet.exp_gain = combat_award_exp_;
            packet.money_gain = combat_award_money_;
            for (size_t i = 0; i < combat_award_items_drop_.size(); ++ i)
            {
                ItemEntry& item = combat_award_items_drop_[i];

                G2C::ItemEntry entry;
                entry.item_id = item.item_id;
                entry.item_count = item.item_count;
                packet.items_drop.push_back(entry);
            }
            for (size_t i = 0; i < combat_award_items_lottery_.size(); ++ i)
            {
                ItemEntry& item = combat_award_items_lottery_[i];

                G2C::ItemEntry entry;
                entry.item_id = item.item_id;
                entry.item_count = item.item_count;
                packet.items_lottery.push_back(entry);
            }
            std::map<int32_t, int>::iterator it = task_award_items_map.begin();
            for (; it != task_award_items_map.end(); ++ it)
            {
                G2C::ItemEntry entry;
                entry.item_id = it->first;
                entry.item_count = it->second;
                packet.items_task.push_back(entry);
            }
            SendCmd(packet);
        }
        else
        {
            //PVP战斗结束
            G2C::CombatPVPEnd packet;
            packet.result = combat_result_;
            SendCmd(packet);

            //结束处理
            HandleCombatPVPEnd(0, combat_pvp_type_);
        }

		//执行回血逻辑
		if (combat_remain_hp_ > 0)
		{
			player_.RecoverHP(combat_remain_hp_);
		}
		else
		{
			//玩家死亡
			player_.Die();
		}

		ClrCombatInfo();
	}
}

bool PlayerCombat::TerminateCombat()
{
	if (!InCombat())
		return false;

	combat::PlayerCMD tmp_cmd;
	tmp_cmd.cmd_no  = combat::COMBAT_CMD_TERMINATE_COMBAT;
	tmp_cmd.roleid  = player_.role_id();

	return combat::DispatchCmd(tmp_cmd, combat_id_, unit_id_);
}

void PlayerCombat::CMDHandler_JoinCombat(const C2G::CombatPlayerJoin& cmd)
{
    int64_t role_to_join = cmd.roleid_to_join;
	if (!player_.CheckJoinCombat(role_to_join))
		return;
	
	if (!JoinCombat(cmd.combat_id, role_to_join))
	{
		//通知客户端加入失败
		player_.sender()->JoinCombatFail(cmd.combat_id);
	}
}

void PlayerCombat::CMDHandler_SelectSkill(const C2G::CombatSelectSkill& cmd)
{
	combat::PlayerCMD tmp_cmd;
	tmp_cmd.cmd_no          = combat::COMBAT_CMD_SELECT_SKILL;
	tmp_cmd.roleid          = player_.role_id();
    tmp_cmd.params.push_back(cmd.skill_id);
    tmp_cmd.params.push_back(cmd.player_select);

	combat::DispatchCmd(tmp_cmd, combat_id_, unit_id_);
}

void PlayerCombat::CMDHandler_TriggerCombat(const C2G::CombatPlayerTrigger& cmd)
{
	world::worldobj_base_info obj_info;
	if ( !player_.world_plane()->QueryObject(XID(cmd.object_id, XID::TYPE_MONSTER), obj_info) 
	  || maths::squared_distance(player_.pos(), obj_info.pos) > kMaxCombatDistanceSquare )
	{
		return;
	}

    if (!obj_info.can_combat)
    {
        return;
    }

	if (IsWaitMobResponse())
	{
		return;
	}

	if (!player_.CanCombat())
	{
		return;
	}

    // 检查喵类视觉
    const MonsterTempl* ptpl = s_pDataTempl->QueryDataTempl<MonsterTempl>(obj_info.tid);
    if (ptpl == NULL) 
    {
        return;
    }
	if (ptpl->cat_vision > 0 && 
        player_.cat_vision_level() < ptpl->cat_vision)
	{
		__PRINTF("CMDHandler_TriggerCombat喵类视觉等级不够！");
		return;
	}

	XID target;
	MAKE_XID(cmd.object_id, target);
	if (target.IsNpc())
	{
		SetWaitMobResponse(MAX_TIME_WAIT_MOB_RESPONSE);
		player_.SendMsg(GS_MSG_PLAYER_TRIGGER_COMBAT, target);
	}
}

void PlayerCombat::CMDHandler_PetAttack(const C2G::CombatPetAttack& cmd)
{
	combat::PlayerCMD tmp_cmd;
	tmp_cmd.cmd_no         = combat::COMBAT_CMD_PET_ATTACK;
	tmp_cmd.roleid         = player_.role_id();
    tmp_cmd.params.push_back(cmd.combat_pet_inv_pos);

	combat::DispatchCmd(tmp_cmd, combat_id_, unit_id_);
}

int PlayerCombat::MSGHandler_SysTriggerCombat(const MSG& msg)
{
	/**
	 * 消息GS_MSG_SYS_TRIGGER_COMBAT的发送者不确定，
	 * 可能是任务、玩家自己、矿等;
	 */

	msg_sys_trigger_combat* param = (msg_sys_trigger_combat*)(msg.content);
	ASSERT(param && msg.content_len == sizeof(msg_sys_trigger_combat));
	ASSERT(param->monster_group_id > 0 || param->battle_scene_id > 0);

	if (IsWaitMobResponse() || !player_.CanCombat())
	{
		return -1;
	}

	//战斗场景选择规则
	//优先考虑怪物身上的场景ID
	//再考虑玩家所在区域场景ID
	if (param->battle_scene_id <= 0)
	{
		param->battle_scene_id = player_.GetCombatSceneID();
	}

	if (!CreatePVECombat(param->battle_scene_id, param->monster_group_id, 0, false, param->task_id, param->challenge_id))
	{
		return -1;
	}

	return 0;
}

int PlayerCombat::MSGHandler_ObjTriggerCombat(const MSG& msg)
{
	/**
	 * 消息GS_MSG_OBJ_TRIGGER_COMBAT的发送者不确定，
	 * 可以为区域、怪物等；
	 */

	CHECK_CONTENT_PARAM(msg, msg_obj_trigger_combat);
	msg_obj_trigger_combat* param = (msg_obj_trigger_combat*)(msg.content);
	ASSERT(param->monster_group_id > 0 || param->battle_scene_id > 0);

    int32_t landmine_interval = param->landmine_interval;
	if (IsWaitMobResponse() || !player_.CanCombat())
	{
		SendObjTriggerCombatRe(msg.source, false, landmine_interval);
		return -1;
	}

	switch (msg.source.type)
	{
		case XID::TYPE_MONSTER:
		{
			if (player_.IsImmuneActiveMonster())
			{
				SendObjTriggerCombatRe(msg.source, false, landmine_interval);
				return 0;
			}
		}
		break;
		case XID::TYPE_MAP_LANDMINE_AREA:
		{
			if (player_.IsImmuneLandmine())
			{
				SendObjTriggerCombatRe(msg.source, false, landmine_interval);
				return 0;
			}

            /*
			// 检查暗雷累积值
			if (!player_.CheckLandmineAccumulate(landmine_interval))
			{
				SendObjTriggerCombatRe(msg.source, false, landmine_interval);
				return 0;
			}
            */
		}
		break;
		default:
			ASSERT(false);
		break;
	};

	//
	// 世界boss
	//
	bool is_world_boss = false;
	if (param->monster_type == MSG_MT_WORLD_BOSS)
	{
		is_world_boss = true;
	}
	
	//战斗场景选择规则
	//优先考虑怪物身上的场景ID
	//再考虑玩家所在区域场景ID
	if (param->battle_scene_id <= 0)
	{
		param->battle_scene_id = player_.GetCombatSceneID();
	}

	int ret = 0;
	int64_t monster_object_id = param->require_combatend_notify ? msg.source.id : 0;
	if (!CreatePVECombat(param->battle_scene_id, param->monster_group_id, monster_object_id, is_world_boss))
	{
		ret = -1;
	}

	bool is_success = (ret == 0) ? true : false;
	SendObjTriggerCombatRe(msg.source, is_success, landmine_interval);
	return ret;
}

int PlayerCombat::MSGHandler_PlayerTriggerCombatRe(const MSG& msg)
{
    /**
     * 消息GS_MSG_PLAYER_TRIGGER_COMBAT_RE发送者为怪物
     */

	if (IsWaitMobResponse())
	{
		ClrWaitMobResponse();
	}
	else
	{
		ASSERT(false);
	}

	if (!player_.CanCombat())
	{
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_player_trigger_combat_re);
	msg_player_trigger_combat_re* param = (msg_player_trigger_combat_re*)(msg.content);
	if (param->monster_group_id <= 0)
	{
		return 0;
	}

	//战斗场景选择规则
	//优先考虑怪物身上的场景ID
	//再考虑玩家所在区域场景ID
	if (param->battle_scene_id <= 0)
	{
		param->battle_scene_id = player_.GetCombatSceneID();
	}

	int64_t monster_object_id = param->require_combatend_notify ? msg.source.id : 0;

	//
	// 世界boss
	//
	if (param->monster_type == MSG_MT_WORLD_BOSS && player_.IsInTeam())
	{
		combat_query_count_ = player_.BroadCastQueryTeamCombat(msg.source.id);
		if (combat_query_count_ > 0)
		{
			boss_data_.monster_obj_id  = monster_object_id;
			boss_data_.combat_scene_id = param->battle_scene_id;
			boss_data_.mob_group_id    = param->monster_group_id;
			// 等待查询结果
			combat_query_timeout_      = kCombatQueryTimeoutSecs;
			SetWaitMobResponse(combat_query_timeout_*2);
			return 0;
		}
	}

	//
	// 创建战场
	//
	bool is_world_boss = param->monster_type == MSG_MT_WORLD_BOSS;
	if (!CreatePVECombat(param->battle_scene_id, param->monster_group_id, monster_object_id, is_world_boss))
	{
		return 0;
	}

    if (param->require_combatend_notify)
    {
        player_.SendMsg(GS_MSG_P_TRIGGER_COMBAT_SUCCESS, msg.source);
    }
	return 0;
}

int PlayerCombat::MSGHandler_CombatStart(const MSG& msg)
{
    /*本函数已废弃，暂时保留*/
	/*只有战场创建者会收到本消息*/
	/*用来通知周边队友进入战斗*/
	//ASSERT(msg.source.id == combat_id_);
	//player_.BroadCastEnterCombat(combat_id_);
	return 0;
}

int PlayerCombat::MSGHandler_CombatPVEEnd(const MSG& msg)
{
	/*参与战斗的每个人都会收到本消息*/
	ASSERT(msg.source.id == combat_id_);

	//脱离战斗
	OnLeaveCombat();

	//执行回血逻辑
	if (combat_remain_hp_ > 0)
	{
		//重置玩家血量
		player_.RecoverHP(combat_remain_hp_);
		player_.state().SetNormal();
	}
	else
	{
		//玩家死亡
		player_.Die();
	}

	ClrCombatInfo();
	return 0;
}

int PlayerCombat::MSGHandler_CombatPVPEnd(const MSG& msg)
{
	/*参与战斗的每个人都会收到本消息*/
	ASSERT(msg.source.id == combat_id_);

	//脱离战斗
	OnLeaveCombat();

    //执行回血逻辑
	if (combat_remain_hp_ > 0)
	{
		//重置玩家血量
		player_.RecoverHP(combat_remain_hp_);
		player_.state().SetNormal();
	}
	else
	{
        //玩家死亡
        player_.Die();
	}

	ClrCombatInfo();
    return 0;
}

int PlayerCombat::MSGHandler_CombatPVEResult(const MSG& msg)
{
	ASSERT(msg.source.id == combat_id_);

	combat::CombatPVEResult result;
    MsgContentUnmarshal(msg, result);

    if (result.result == combat::RESULT_FAIL && ((player_.CombatFail(result.taskid) && result.taskid != 0) || result.challengeid != 0))
    {
        // 对于检查战斗失败的任务战斗或者界面挑战，失败时不让玩家死亡
        result.hp = 1;
    }
	
    //通知玩家战斗结束
	G2C::CombatPVEEnd packet;
	packet.result = result.result;
    //修正玩家获得经验
    double factor = 1.0f + player_.value_prop().GetCombatAwardExpAdjustFactor();
    packet.exp_gain = result.exp * factor;
    //修正玩家获得金钱
    factor = 1.0f + player_.value_prop().GetCombatAwardMoneyAdjustFactor();
    packet.money_gain = result.money * factor;

	for (size_t i = 0; i < result.items.size(); ++ i)
	{
		const combat::ItemEntry& item = result.items[i];

		G2C::ItemEntry entry;
		entry.item_id = item.item_id;
		entry.item_count = item.item_count;
		packet.items_drop.push_back(entry);
	}
	for (size_t i = 0; i < result.lottery.size(); ++ i)
	{
		const combat::ItemEntry& item = result.lottery[i];

		G2C::ItemEntry entry;
		entry.item_id = item.item_id;
		entry.item_count = item.item_count;
		packet.items_lottery.push_back(entry);
	}
	/*std::map<int32_t, int>::iterator it = task_award_items_map.begin();
	for (; it != task_award_items_map.end(); ++ it)
	{
		G2C::ItemEntry entry;
		entry.item_id = it->first;
		entry.item_count = it->second;
		packet.items_task.push_back(entry);
	}*/
	SendCmd(packet);

    // 如果界面BOSS挑战通知结果
    if (result.challengeid != 0)
    {
        player_.WinBossChallenge(result.result, result.challengeid, result.monster_group_id);
    }

	//玩家获得金钱和经验
	player_.IncExp(packet.exp_gain);
	player_.GainMoney(packet.money_gain);

	//获取宠物经验和能量
	player_.GainPetExp(packet.exp_gain);
	player_.SetPetPower(result.pet_power);

	//玩家获得普通掉落和全局掉落奖励
	for (size_t i = 0; i < result.items.size(); ++ i)
	{
		combat::ItemEntry& item = result.items[i];
		player_.GainItem(item.item_id, item.item_count);
	}

	//通知任务系统
	//std::map<int32_t, int> task_award_items_map;
	for (size_t i = 0; i < result.mob_killed_vec.size(); ++ i)
	{
		combat::MobKilled& mob = result.mob_killed_vec[i];

		std::vector<playerdef::ItemEntry> task_items;
		player_.KillMonster(mob.mob_tid, mob.mob_count, task_items);
		//for (size_t j = 0; j < task_items.size(); ++ j)
		//{
			//playerdef::ItemEntry& item = task_items[j];
			//task_award_items_map[item.item_id] += item.item_count;
		//}
	}

	//玩家获得抽奖奖励
	for (size_t i = 0; i < result.lottery.size(); ++ i)
	{
		combat::ItemEntry& item = result.lottery[i];
		player_.GainItem(item.item_id, item.item_count);
	}

	//更新玩家血量
	combat_remain_hp_ = result.hp;
	return 0;
}

int PlayerCombat::MSGHandler_CombatPVPResult(const MSG& msg)
{
	ASSERT(msg.source.id == combat_id_);

	combat::CombatPVPResult result;
    MsgContentUnmarshal(msg, result);

    combat_result_    = result.result;
	combat_remain_hp_ = result.hp;
    combat_pet_power_ = result.pet_power;

    //result.combat_creater;

    //更新宠物能量值
	player_.SetPetPower(combat_pet_power_);

    //通知玩家战斗结束
	G2C::CombatPVPEnd packet;
	packet.result = result.result;
	SendCmd(packet);

    //TODO
    //击杀玩家还未处理
    //

    //不同pvp类型战斗的结束处理
    HandleCombatPVPEnd(result.combat_creator, result.combat_flag);

    return 0;
}

int PlayerCombat::MSGHandler_SomeoneHateYou(const MSG& msg)
{
    G2C::ObjectHateYou packet;
    packet.obj_id = msg.source.id;
    player_.sender()->SendCmd(packet);
	return 0;
}

int PlayerCombat::MSGHandler_CompanionEnterCombat(const MSG& msg)
{
	//周围组队的队友或者同阵营的玩家进入战斗
    CHECK_CONTENT_PARAM(msg, msg_companion_enter_combat);
    const msg_companion_enter_combat& param = *(const msg_companion_enter_combat*)msg.content;
	
	if (InCombat() || !player_.CheckJoinCombat(msg.source.id))
	{
		return 0;
	}

	if (player_.IsImmuneTeamCombat())
	{
		player_.sender()->TeammateEnterCombatFail(player_.role_id());
		return 0;
	}

	world::player_base_info info;
	if (player_.world_plane()->QueryPlayer(msg.source.id, info))
	{
		if (info.pos.squared_distance(player_.pos()) > kBCEnterCombatRangeSquare)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	
	JoinCombat(param.combat_id, msg.source.id);

    // 把自己的信息同步给world-boss
    XID boss;
    MAKE_XID(param.world_boss_id, boss);
    if (boss.IsNpc())
    {
        SyncInfoToWorldBoss(param.world_boss_id);
    }
	return 0;
}

int PlayerCombat::MSGHandler_QueryTeamCombat(const MSG& msg)
{
	// 队友发来的，查询战斗id的消息（共享血量战斗）
	if (InCombat() && 
		player_.IsTeammate(msg.source.id) && 
		msg.param == monster_object_id_)
	{
		msg_query_team_combat_re param;
		param.combat_id      = combat_id_;
		param.monster_obj_id = msg.param;
		player_.SendMsg(GS_MSG_QUERY_TEAM_COMBAT_RE, msg.source, &param, sizeof(param));
	}
	else
	{
		msg_query_team_combat_re param;
		param.combat_id      = 0;
		param.monster_obj_id = 0;
		player_.SendMsg(GS_MSG_QUERY_TEAM_COMBAT_RE, msg.source, &param, sizeof(param));
	}

	return 0;
}

int PlayerCombat::MSGHandler_QueryTeamCombatRe(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_query_team_combat_re);
	const msg_query_team_combat_re& param = *(msg_query_team_combat_re*)msg.content;

	if (!player_.CanCombat() || InCombat() || !IsWaitMobResponse())
	{
		combat_query_count_ = 0;
		return 0;
	}

	// 超时会给自己发该消息
	if (msg.source != player_.object_xid() && !player_.IsTeammate(msg.source.id))
		return 0;

	int combat_id = param.combat_id;
	if (combat_id == 0)
	{
		if (--combat_query_count_ < 0)
		{
			combat_query_count_ = 0;

			// 开启一场新的boss战
			if (boss_data_.mob_group_id   == 0 || 
				boss_data_.monster_obj_id == 0 ||
				boss_data_.monster_obj_id != param.monster_obj_id)
			{
				LOG_ERROR << "boss战保存数据有误！" 
                    << " - " << boss_data_.mob_group_id
                    << " - " << boss_data_.monster_obj_id
                    << " - " << boss_data_.monster_obj_id 
                    << " - " << param.monster_obj_id;
				return 0;
			}

			CreatePVECombat(boss_data_.combat_scene_id, boss_data_.mob_group_id, boss_data_.monster_obj_id, true);
			boss_data_.clear();
		}
	}
	else
	{
		if (!JoinCombat(combat_id, msg.source.id))
		{
			//通知客户端加入失败
			player_.sender()->JoinCombatFail(combat_id);
		}

        // 把自己的信息同步给world-boss
        SyncInfoToWorldBoss(boss_data_.monster_obj_id);
	}

	return 0;
}

bool PlayerCombat::CreatePVECombat(int32_t combat_scene_id, int32_t mob_group_id, int32_t monster_obj_id, bool is_world_boss, int32_t taskid, int32_t challenge_id)
{
	if (combat_scene_id < 0 || mob_group_id <= 0)
    {
		return false;
    }

	if (combat::QueryCombat(combat_id_))
    {
		return false;
    }

	if (combat::QueryCombatUnit(unit_id_))
    {
		return false;
    }

    if (player_.GetHP() <= 0)
    {
        return false;
    }

	int teammate_count = player_.QueryTeammateCountEnterCombat();

	combat::pve_combat_param param;
	param.creator = player_.role_id();
	param.task_id = taskid;
	param.world_id = player_.world_xid().id;
	param.combat_scene_id = combat_scene_id;
	param.mob_group_id = mob_group_id;
	param.world_monster_id = monster_obj_id;
	param.init_player_count = teammate_count + 1;
	param.is_world_boss = is_world_boss;
    param.challenge_id = challenge_id;
	BuildPlayerData(param.playerdata);

	//获取npc伙伴
    if (challenge_id == 0)
    {
	    get_buddy_members(&player_, &param);
    }

	int32_t __unit_id = 0;
	int32_t __combat_id = 0;
	if (!combat::CreatePVECombat(param, __combat_id, __unit_id))
	{
		return false;
	}

	unit_id_           = __unit_id;
	combat_id_         = __combat_id;
    combat_type_       = COMBAT_TYPE_PVE;
	monster_object_id_ = monster_obj_id;

	//成功加入战斗
	OnEnterCombat();

	//通知周边队友进入战斗
    if (challenge_id == 0)
    {
        int32_t boss_id = is_world_boss ? monster_obj_id : 0;
	    player_.BroadCastEnterCombat(combat_id_, boss_id);
    }

    // 把自己的信息同步给world-boss
    if (is_world_boss)
    {
        SyncInfoToWorldBoss(monster_obj_id);
    }

	return true;
}

bool PlayerCombat::StartPVPCombat(int32_t combat_scene_id, playerdef::CombatPVPType type, playerdef::StartPVPResult& result)
{
    if (!CreatePVPCombat(combat_scene_id, type))
        return false;

    result.combat_id = combat_id_;
    return true;
}

bool PlayerCombat::CreatePVPCombat(int32_t combat_scene_id, int32_t pvp_type)
{
	if (combat_scene_id < 0)
		return false;

	if (combat::QueryCombat(combat_id_))
		return false;

	if (combat::QueryCombatUnit(unit_id_))
		return false;

	combat::pvp_combat_param param;
	param.creator         = player_.role_id();
	param.world_id        = player_.world_xid().id;
    param.combat_flag     = pvp_type;
	param.combat_scene_id = combat_scene_id;
	BuildPlayerData(param.playerdata);

	int32_t __unit_id = 0;
	int32_t __combat_id = 0;
	if (!combat::CreatePVPCombat(param, __combat_id, __unit_id))
	{
		return false;
	}

	unit_id_     = __unit_id;
	combat_id_   = __combat_id;
    combat_type_ = COMBAT_TYPE_PVE;

	//成功加入战斗
	OnEnterCombat();

	//通知周边队友进入战斗
	player_.BroadCastEnterCombat(combat_id_, 0);

    return true;
}

void PlayerCombat::OnEnterCombat()
{
	///
	/// 成功加入战斗需要做的处理
	///

	//设置战斗状态
	player_.state().SetCombat();
	player_.visible_state().SetCombatFlag(combat_id_);

	//清空session
	player_.AddSessionAndStart(new PlayerEmptySession(&player_));

	//广播进入战斗
	player_.sender()->EnterCombat(combat_id_);
}

void PlayerCombat::OnLeaveCombat()
{
	///
	/// 玩家离开战斗后需要做的处理
	///

	//清除战斗标记
	player_.visible_state().ClrCombatFlag();

	//广播离开战斗
	player_.sender()->LeaveCombat();
}

void PlayerCombat::ClrCombatInfo()
{
	unit_id_                   = 0;
	combat_id_                 = 0;
    combat_type_               = 0;
	combat_result_             = 0;
	combat_remain_hp_          = 0;
    combat_pet_power_          = 0;
	combat_award_exp_          = 0;
	combat_award_money_        = 0;
	time_waiting_mob_response_ = 0;
	is_waiting_mob_response_   = false;
	is_waiting_combat_result_  = false;
	monster_object_id_         = 0;
	combat_query_count_        = 0;

	mob_killed_vec_.clear();
    player_killed_vec_.clear();
	combat_award_items_drop_.clear();
	combat_award_items_lottery_.clear();
}

void PlayerCombat::BuildPlayerData(combat::player_data& data)
{
	data.roleid        = player_.role_id();
	data.name          = player_.rolename();
	data.pos           = player_.GetCombatPosition();
	data.cls           = player_.role_class();
	data.gender        = player_.gender();
	data.level         = player_.level();
	data.weapon_id     = player_.GetWeaponID();
	data.master_id     = player_.master_id();
	data.normal_atk_id = Gmatrix::GetPlayerNormalAttackID(player_.role_class());
    data.cls_model_src = Gmatrix::GetPlayerModelPath(player_.role_class());
    data.dying_time    = player_.value_prop().GetPlayerDyingTime();

	//设置基础属性
	data.hp = player_.GetHP();
	data.mp = 0;
	data.ep = 0;

	//计算战斗中玩家的属性
	PropertyPolicy::CalPlayerCombatProp(&player_, data.props);

	//纠正职业属性2
	if (player_.role_class() >= CLS_PRIMARY_ARCHER && player_.role_class() <= CLS_S_RAKE_ARCHER)
	{
		data.mp = data.props[1];
	}

	std::vector<playerdef::SkillTreeInfo> skill_info_vec;
	player_.QueryAvailableSkillTree(skill_info_vec);

	//设置技能数据
	data.skill_tree_vec.resize(skill_info_vec.size());
	for (size_t i = 0; i < skill_info_vec.size(); ++ i)
	{
		playerdef::SkillTreeInfo& info       = skill_info_vec[i];

		data.skill_tree_vec[i].skill_tree_id = info.skill_tree_id;
		data.skill_tree_vec[i].skill_id      = info.skill_id;
		data.skill_tree_vec[i].level         = info.level;
		data.skill_tree_vec[i].tmp_level     = info.tmp_level;
		data.skill_tree_vec[i].max_level     = info.max_level;
	}

	//设置天赋技能
	//std::vector<int32_t> talent_skills;
	//player_.QueryTalentSkill(talent_skills);
	//data.talent_skill_vec = talent_skills;
	player_.QueryTalentSkill(data.addon_skills);

    //设置称号附加技能
    //std::vector<int32_t> title_skills;
    //player_.QueryTitleSkill(title_skills);
    //data.title_skill_vec = title_skills;
	player_.QueryTitleSkill(data.addon_skills);

    //设置附魔附加技能
	player_.QueryEnhanceSkill(data.addon_skills);

    //设置附魔附加技能
	player_.QueryCardSkill(data.addon_skills);

	int32_t pet_power = 0;
	int32_t pet_power_cap = 0;
	int32_t pet_power_gen_speed = 0;
	player_.QueryPetPowerInfo(pet_power, pet_power_cap, pet_power_gen_speed);

	//设置宠物能量信息
	data.petdata.pet_power = pet_power;
	data.petdata.pet_power_cap = pet_power_cap;
	data.petdata.pet_power_gen_speed = pet_power_gen_speed;
    data.petdata.pet_attack_cd_time = player_.QueryPetAttackCDTime();

	std::vector<playerdef::PetInfo> pet_info_list;
	player_.QueryCombatPetInfo(pet_info_list);

	//设置战宠栏
	data.petdata.pet_vec.resize(pet_info_list.size());
	for (size_t i = 0; i < pet_info_list.size(); ++ i)
	{
		playerdef::PetInfo& pet = pet_info_list[i];

		data.petdata.pet_vec[i].pet_id         = pet.pet_id;
		data.petdata.pet_vec[i].pet_rank       = pet.pet_rank;
		data.petdata.pet_vec[i].pet_level      = pet.pet_level;
		data.petdata.pet_vec[i].pet_blevel     = pet.pet_blevel;
		data.petdata.pet_vec[i].pet_item_idx   = pet.pet_item_idx;
		data.petdata.pet_vec[i].pet_combat_pos = pet.pet_combat_pos;
	}
}

bool PlayerCombat::FirstJoinPVPCombat(int32_t combat_id)
{
    if (!player_.CanCombat())
        return false;

    if (!JoinCombat(combat_id, 0))
        return false;

    //通知周边队友进入战斗
	player_.BroadCastEnterCombat(combat_id, 0);
    return true;
}

bool PlayerCombat::JoinCombat(int32_t combat_id, int64_t role_to_join)
{
	combat::player_data playerdata;
	BuildPlayerData(playerdata);

	int32_t unit_id = 0;
	if (combat::JoinCombat(player_.role_id(), playerdata, role_to_join, combat_id, unit_id))
	{
		unit_id_ = unit_id;
		combat_id_ = combat_id;

        //确定是PVE还是PVP战斗
        int __combat_type = COMBAT_TYPE_INVALID;
        int __type = combat::QueryCombatType(combat_id);
        if (__type == combat::OBJ_TYPE_COMBAT_PVE)
            __combat_type = COMBAT_TYPE_PVE;
        else if (__type == combat::OBJ_TYPE_COMBAT_PVP)
            __combat_type = COMBAT_TYPE_PVP;
        else
            ASSERT(false);

        if (combat_type_ != COMBAT_TYPE_INVALID)
        {
            ASSERT(combat_type_ == __combat_type);
        }

		OnEnterCombat();
		return true;
	}

	return false;
}

void PlayerCombat::CheckQueryTeamCombat()
{
	if (--combat_query_timeout_ <= 0)
	{
		combat_query_count_   = 0;
		combat_query_timeout_ = GS_INT32_MAX;

		msg_query_team_combat_re param;
		param.monster_obj_id = boss_data_.monster_obj_id;
		param.combat_id      = 0;
		player_.SendMsg(GS_MSG_QUERY_TEAM_COMBAT_RE, player_.object_xid(), &param, sizeof(param));
	}
}

void PlayerCombat::SendObjTriggerCombatRe(const XID& target, bool is_success, int32_t interval)
{
	msg_obj_trigger_combat_re param;
	param.is_success        = is_success;
    param.landmine_interval = interval;
	player_.SendMsg(GS_MSG_OBJ_TRIGGER_COMBAT_RE, target, &param, sizeof(param));
}

void PlayerCombat::SyncPlayerCombatData()
{
    if (!IS_NORMAL_MAP(player_.world_id()))
        return;

    combat::player_data player_data;
    BuildPlayerData(player_data);

    // 修改数值
    player_data.hp = player_.GetMaxHP();

    global::PlayerCombatData combat_data;
    build_combat_data(player_data, combat_data);
    player_.sender()->SendToMaster(combat_data);
} 

void PlayerCombat::HandleCombatPVPEnd(RoleID creator, int32_t pvp_type)
{
    playerdef::PVPEndExtraData extra_data;
    playerdef::PVPEndInfo info;
    info.pvp_type = pvp_type;
    info.creator  = creator;
    player_.HandleCombatPVPEnd(info, extra_data);
    if (combat_remain_hp_ <= 0)
    {
        combat_remain_hp_ = extra_data.remain_hp;
    }
}

void PlayerCombat::SyncInfoToWorldBoss(int32_t monster_obj_id)
{
    XID target;
    MAKE_XID(monster_obj_id, target);
    if (target.IsObject())
    {
        msgpack_wb_player_info param;
        param.roleid       = player_.role_id();
        param.masterid     = player_.master_id();
        param.cls          = player_.role_class();
        param.gender       = player_.gender();
        param.level        = player_.level();
        param.combat_value = player_.CalcCombatValue();
        param.first_name   = player_.first_name();
        param.middle_name  = player_.middle_name();
        param.last_name    = player_.last_name();

        shared::net::ByteBuffer buf;
        MsgContentMarshal(param, buf);
        player_.SendMsg(GS_MSG_WB_COLLECT_PLAYER_INFO, target, buf.contents(), buf.size());
    }
    else
    {
        LOG_ERROR << "monster_obj_id居然不是Object？？";
        return;
    }
}

} // namespace gamed
