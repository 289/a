#include "world_boss_imp.h"

#include "game_module/combat/include/combat_def.h"
#include "gs/global/sys_mail_config.h"
#include "gs/global/global_data.h"
#include "gs/global/gmatrix.h"
#include "gs/global/randomgen.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/monster_templ.h"
#include "gs/template/data_templ/world_boss_award.h"
#include "gs/template/data_templ/battleground_templ.h"
#include "gs/player/player.h"
#include "gs/scene/world.h"

#include "npc_def.h"


namespace gamed {

using namespace npcdef;
using namespace dataTempl;

namespace {

    void FillMailAttach(int index, const WorldBossAwardTempl* ptpl, playerdef::SysMail& sysmail)
    {
        for (size_t i = 0; i < ptpl->ranking[index].item_list.size(); ++i)
        {
            playerdef::MailAttach attach;
            attach.id    = ptpl->ranking[index].item_list[i].tid;
            attach.count = ptpl->ranking[index].item_list[i].count;
            sysmail.attach_list.push_back(attach);
        }
    }

    void BuildGlobalRecord(int64_t damage, const msgpack_wb_player_info& pinfo, 
            globalData::WorldBossRecord::PInfo& record)
    {
        record.damage       = damage;
        record.role_id      = pinfo.roleid;
        record.first_name   = pinfo.first_name;
        record.mid_name     = pinfo.middle_name;
        record.last_name    = pinfo.last_name;
        record.level        = pinfo.level;
        record.cls          = pinfo.cls;
        record.combat_value = pinfo.combat_value;
        record.gender       = pinfo.gender;
    }

} // Anonymous

WorldBossImp::WorldBossImp(Npc& npc)
	: MonsterImp(npc),
      reset_global_record_(true),
      is_in_worldboss_map_(false),
      timestamp_of_born_(0)
{
}

WorldBossImp::~WorldBossImp()
{
}

bool WorldBossImp::OnInit()
{
    if (!MonsterImp::OnInit())
        return false;

    timestamp_of_born_ = g_timer->GetSysTime();
    return true;
}

void WorldBossImp::OnHasInsertToWorld()
{
    if (IS_BG_MAP(npc_.world_id()))
    {
        world::battleground_info info;
        if (npc_.world_plane()->GetBGInfo(info))
        {
            const BattleGroundTempl* pbattle = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(info.bg_templ_id);
            if (pbattle && pbattle->bg_type == BattleGroundTempl::BGT_PVE_WORLD_BOSS)
            {
                is_in_worldboss_map_    = true;
                std::set<int32_t> master_set;
                Gmatrix::GetAllMasterId(master_set);
                std::set<int32_t>::iterator it_master = master_set.begin();
                for (; it_master != master_set.end(); ++it_master)
                {
                    TryResetGlobalData(*it_master);
                    RetainMasterID(*it_master);
                }
            }
        }
    }
}

void WorldBossImp::TryResetGlobalData(int32_t master_id)
{
    globalData::WorldBossInfo::TimeValue time_val;
    time_val.timestamp = timestamp_of_born_;
    time_val.cur_time  = g_timer->GetSysTime();
    s_pGlobalData->Delete<globalData::WorldBossInfo>(master_id, npc_.templ_id(), &time_val);

    globalData::WorldBossRecord::DeleteValue value;
    value.timestamp = timestamp_of_born_;
    s_pGlobalData->Delete<globalData::WorldBossRecord>(master_id, npc_.templ_id(), &value);
}

bool WorldBossImp::CheckStateToCombat()
{
	// 是否在等待战斗，或者已经进入战斗
	if (get_state() == MON_STATE_WAITING_COMBAT)
	{
		return false;
	}
	return true;
}

void WorldBossImp::HandleCombatEnd(const msg_combat_end& msg_param)
{
    // 不做处理
}

void WorldBossImp::HandleBossCombatResult(const MSG& msg)
{
	combat::WorldBossCombatStatus combat_re;
	MsgContentUnmarshal(msg, combat_re);
    RefreshDamageList(combat_re);

	msg_combat_end tmp_param;
	if (combat_re.status == combat::WBST_BOSS_ALIVE)
	{
        // boss胜利
		tmp_param.set_combat_win(true);
	    MonsterImp::HandleCombatEnd(tmp_param);
	}
	else if (combat_re.status == combat::WBST_BOSS_DEAD)
	{
        // 计算奖励
        CalcCombatResult();
        NotifyWorldBossDead();
        // boss死亡
		tmp_param.set_combat_win(false);
	    MonsterImp::HandleCombatEnd(tmp_param);
        // 记录日志
        GLog::log("世界BOSS死亡，elem_id:%d tid:%d", npc_.elem_id(), npc_.templ_id());
	}
}

void WorldBossImp::RefreshDamageList(const combat::WorldBossCombatStatus& result)
{
    int64_t total_damage     = 0;
    int64_t combat_value_sum = 0;
    damage_res_map_.clear();
    for (size_t i = 0; i < result.dmg_list.size(); ++i)
    {
        DamageResult dam_res;
        dam_res.damage = 0;
        dam_res.info.clear();

        const combat::WorldBossCombatStatus::DamageEntry& ent = result.dmg_list[i];
        total_damage += ent.damage;
        PlayerInfoMap::iterator it_player = player_info_map_.find(ent.roleid);
        if (it_player != player_info_map_.end())
        {
            combat_value_sum += it_player->second.combat_value;
            dam_res.info      = it_player->second;
            dam_res.damage    = ent.damage;
        }
        else
        {
            // 立即查找玩家
            world::player_extra_info extra_info;
            if (npc_.world_plane()->QueryPlayerExtra(ent.roleid, extra_info))
            {
                world::player_base_info base_info;
                if (npc_.world_plane()->QueryPlayer(ent.roleid, base_info))
                {
                    msgpack_wb_player_info wb_pinfo;
                    wb_pinfo.roleid       = base_info.xid.id;
                    wb_pinfo.masterid     = base_info.master_id;
                    wb_pinfo.cls          = base_info.cls;
                    wb_pinfo.gender       = base_info.gender;
                    wb_pinfo.level        = base_info.level;
                    wb_pinfo.combat_value = base_info.combat_value;
                    wb_pinfo.first_name   = extra_info.first_name;
                    wb_pinfo.middle_name  = extra_info.middle_name;
                    wb_pinfo.last_name    = extra_info.last_name;
                    // update
                    player_info_map_[ent.roleid] = wb_pinfo;
                    dam_res.info      = wb_pinfo;
                    dam_res.damage    = ent.damage;
                    combat_value_sum += wb_pinfo.combat_value;
                    RetainMasterID(wb_pinfo.masterid);
                }
            }
        }

        damage_res_map_[ent.roleid] = dam_res;
    }

    if (combat_value_sum <= 0 || total_damage <= 0)
    {
        if (result.status != combat::WBST_BOSS_ALIVE && 
            result.status != combat::WBST_BOSS_DEAD)
        {
            __PRINTF("世界BOSS本场的总伤害或总战斗力小于等于0! ...... total_damage:%ld combat_value_sum:%ld", 
                    total_damage, combat_value_sum);
        }
        return;
    }
    
    // calculate
    DamageResultMap::iterator it_dam = damage_res_map_.begin();
    for (; it_dam != damage_res_map_.end(); ++it_dam)
    {
        DamageResult& dam_res = it_dam->second;
        double weight  = (double)dam_res.info.combat_value / (double)combat_value_sum;
        dam_res.damage = ((double)total_damage * weight) + 0.5f;
        dam_res.damage = (dam_res.damage == 0) ? 1 : dam_res.damage;
        if (dam_res.damage <= 0)
            continue;

        // 记录下本场该玩家的最终伤害
        damage_info_map_[it_dam->first] += dam_res.damage;
    }

    if (is_in_worldboss_map_)
    {
        globalData::WorldBossRecord::RecordValue global_record;
        global_record.timestamp = timestamp_of_born_;
        for (size_t i = 0; i < result.dmg_list.size(); ++i)
        {
            globalData::WorldBossRecord::PInfo record_info;
            DamageResultMap::const_iterator it_con_dam = damage_res_map_.find(result.dmg_list[i].roleid);
            ASSERT(it_con_dam != damage_res_map_.end());
            const DamageResult& dam_res = it_con_dam->second;

            DamageInfoMap::const_iterator it_info = damage_info_map_.find(result.dmg_list[i].roleid);
            ASSERT(it_info != damage_info_map_.end());

            // global record
            BuildGlobalRecord(it_info->second, dam_res.info, record_info);
            global_record.info_vec.push_back(record_info);
        }

        // refresh global record
        globalData::WorldBossRecord::QueryValue query_value;
        query_value.role_id     = 0;
        query_value.query_count = 1;
        for (MasterSet::iterator it_re = master_set_.begin(); it_re != master_set_.end(); ++it_re)
        {
            if (s_pGlobalData->Query<globalData::WorldBossRecord>((*it_re), npc_.templ_id(), query_value))
            {
                // set change
                s_pGlobalData->Modify<globalData::WorldBossRecord>((*it_re), npc_.templ_id(), global_record);
            }
            else // 该master还没有记录
            {
                // set all damage list to this master
                SyncGlobalRecord(*it_re);
            }
        }
    }
}

void WorldBossImp::CalcCombatResult()
{
    const MonsterTempl* pmonster = s_pDataTempl->QueryDataTempl<MonsterTempl>(npc_.templ_id());
    if (pmonster->world_boss_award_tid <= 0)
    {
        LOG_ERROR << "世界BOSS eid: " << npc_.elem_id() << " 没有配奖励！";
        return;
    }
    const WorldBossAwardTempl* ptpl = s_pDataTempl->QueryDataTempl<WorldBossAwardTempl>(pmonster->world_boss_award_tid);
    if (ptpl == NULL)
    {
        LOG_ERROR << "没有找到世界BOSS的奖励模板tid: " << pmonster->world_boss_award_tid 
            << " NPC-elemid: " << npc_.elem_id();
        return;
    }

    DamageInfoMultimap damage_multi;
    DamageInfoMap::iterator it_dam = damage_info_map_.begin();
    for (; it_dam != damage_info_map_.end(); ++it_dam)
    {
        damage_multi.insert(std::make_pair(it_dam->second, it_dam->first));
    }

    // 没有玩家对boss造成伤害
    if (damage_multi.empty() || ptpl->ranking.size() <= 0)
    {
        LOG_ERROR << "没有玩家对BOSS造成伤害？为什么会进入BOSS战斗结算？eid: "
            << npc_.elem_id() << " tid: " << npc_.templ_id();
        return;
    }

    // get mail format
    SysMailConfig::Entry mail_format;
    s_pSysMailCfg->GetWorldBossMail(npc_.templ_id(), mail_format);
    playerdef::SysMail sysmail;
    sysmail.attach_score = 0;
    sysmail.sender      = mail_format.sender;
    sysmail.title       = mail_format.title;
    sysmail.content     = mail_format.content;
    FillMailAttach(0, ptpl, sysmail);

    // 发邮件
    int count = 0, index = 0;
    DamageInfoMultimap::iterator it_multi = damage_multi.begin();
    for (; it_multi != damage_multi.end(); ++it_multi)
    {
        ++count;
        if (count > ptpl->ranking[index].value)
        {
            // 换下一个级别
            ++index;
            if (index >= (int)ptpl->ranking.size())
                break;

            sysmail.attach_list.clear();
            FillMailAttach(index, ptpl, sysmail);
        }
        ASSERT(count <= ptpl->ranking[index].value);

        PlayerInfoMap::const_iterator it_player = player_info_map_.find(it_multi->second);
        if (it_player == player_info_map_.end())
            continue;

        int32_t master_id = it_player->second.masterid;
        RoleID  role_id   = it_player->second.roleid;
        Player::SendSysMail(master_id, role_id, sysmail);
    }
}

void WorldBossImp::NotifyWorldBossDead()
{
    if (!is_in_worldboss_map_)
        return;

    // 广播给所有的player
    std::vector<XID> player_vec;
    npc_.world_plane()->GetAllPlayerInWorld(player_vec);
    for (size_t i = 0; i < player_vec.size(); ++i)
    {
	    npc_.SendMsg(GS_MSG_WORLD_BOSS_DEAD, player_vec[i], npc_.templ_id());
    }

    // 通知地图
    npc_.SendPlaneMsg(GS_PLANE_MSG_WORLD_BOSS_DEAD, npc_.templ_id());

    // 刷新globalData，更新世界BOSS战斗次数
    globalData::WorldBossInfo::TimeValue time_val;
    time_val.timestamp = timestamp_of_born_;
    time_val.cur_time  = g_timer->GetSysTime();
    for (MasterSet::iterator it_re = master_set_.begin(); it_re != master_set_.end(); ++it_re)
    {
        s_pGlobalData->Modify<globalData::WorldBossInfo>((*it_re), npc_.templ_id(), time_val);
    }
}

void WorldBossImp::HandleSyncPlayerInfo(const MSG& msg)
{
    // unmarshal
    msgpack_wb_player_info param;
    MsgContentUnmarshal(msg, param);
    ASSERT(param.roleid == msg.source.id);

    player_info_map_[param.roleid] = param;
    RetainMasterID(param.masterid);
}

int WorldBossImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_COMBAT_PVE_END:
			{
				CHECK_CONTENT_PARAM(msg, msg_combat_end);
				msg_combat_end* param = (msg_combat_end*)msg.content;
				HandleCombatEnd(*param);
			}
			break;

		case GS_MSG_COMBAT_WORLD_BOSS_END:
			{
				HandleBossCombatResult(msg);
			}
			break;

        case GS_MSG_WB_COLLECT_PLAYER_INFO:
            {
                HandleSyncPlayerInfo(msg);
            }
            break;

		default:
			if (MonsterImp::OnMessageHandler(msg) != 0)
			{
				ASSERT(false);
				return -1;
			}
			break;
	}

	return 0;
}

void WorldBossImp::RetainMasterID(int32_t master_id)
{
    master_set_.insert(master_id);
}

void WorldBossImp::SyncGlobalRecord(int32_t master_id)
{
    if (!is_in_worldboss_map_)
        return;

    globalData::WorldBossRecord::RecordValue global_record;
    global_record.timestamp = timestamp_of_born_;
    PlayerInfoMap::const_iterator it = player_info_map_.begin();
    for (; it != player_info_map_.end(); ++it)
    {
        globalData::WorldBossRecord::PInfo record_info;
        DamageInfoMap::iterator it_dam = damage_info_map_.find(it->first);
        if (it_dam == damage_info_map_.end())
            continue;

        BuildGlobalRecord(it_dam->second, it->second, record_info);
        global_record.info_vec.push_back(record_info);
    }

    s_pGlobalData->Modify<globalData::WorldBossRecord>(master_id, npc_.templ_id(), global_record);
}

} // namespace gamed
