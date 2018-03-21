#include "player_landmine.h"

#include <limits.h>

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/randomgen.h"
#include "gs/global/gmatrix.h"
#include "gs/player/subsys_if.h"


namespace gamed {

const float PlayerLandmine::kMinCalcDistanceSquare = 0.25*0.25; // 暗雷最小的计算距离

///
/// PlayerLandmine
///
PlayerLandmine::PlayerLandmine(Player& player)
    : PlayerSubSystem(SUB_SYS_TYPE_LANDMINE, player)
{
}

PlayerLandmine::~PlayerLandmine()
{
}

void PlayerLandmine::OnRelease()
{
    ResetTriggerData();
}

void PlayerLandmine::ResetTriggerData()
{
    prev_pos_ = player_.pos();
    landmine_map_.clear();
}

void PlayerLandmine::OnHeartbeat(time_t cur_time)
{
    if (landmine_map_.size() <= 0)
        return;

    if (prev_pos_.squared_distance(player_.pos()) > kMinCalcDistanceSquare)
    {
        // update pos
        prev_pos_ = player_.pos();

        // foreach landmine
        LandmineInfoMap::iterator it = landmine_map_.begin();
        for (; it != landmine_map_.end(); ++it)
        {
            entry_t& ent = it->second;

            if (--ent.encounter_timer > 0)
                continue;

            if (!player_.CanCombat())
                continue;
            
            TriggerCombat(ent);
        }
    }
}

void PlayerLandmine::TriggerCombat(entry_t& ent)
{
    ///
    /// 三个概率是完全独立的
    ///
    if (ent.pelemdata->active_task_cond.task_list.size() > 0)
    {
        // 检查活跃任务
        for (size_t i = 0; i < ent.pelemdata->active_task_cond.task_list.size(); ++i)
        {
            int32_t task_id = ent.pelemdata->active_task_cond.task_list[i];
            if (player_.HasActiveTask(task_id))
            {
                RandTriggerCombat(ent, TT_ACTIVE_TASK);
                return;
            }
        }
    }
    
    if (ent.pelemdata->finish_task_cond.task_list.size() > 0)
    {
        // 检查已完成任务
        for (size_t i = 0; i < ent.pelemdata->finish_task_cond.task_list.size(); ++i)
        {
            int32_t task_id = ent.pelemdata->finish_task_cond.task_list[i];
            if (player_.HasFinishTask(task_id))
            {
                RandTriggerCombat(ent, TT_FINISH_TASK);
                return;
            }
        }
    }

    // 普通概率
    RandTriggerCombat(ent, TT_NORMAL);
    return;
}

bool PlayerLandmine::RandTriggerCombat(entry_t& ent, TriggerType type)
{
    int32_t prob, interval, mg_id, bs_id;

    switch (type)
    {
        case TT_NORMAL:
            prob     = ent.pelemdata->encounter_enermy_prob;
            interval = ent.pelemdata->encounter_interval;
            mg_id    = ent.pelemdata->monster_group_id;
            bs_id    = ent.pelemdata->battle_scene_id;
            break;

        case TT_ACTIVE_TASK:
            prob     = ent.pelemdata->active_task_cond.encounter_enermy_prob;
            interval = ent.pelemdata->active_task_cond.encounter_interval;
            mg_id    = ent.pelemdata->active_task_cond.monster_group_id;
            bs_id    = ent.pelemdata->active_task_cond.battle_scene_id;
            break;

        case TT_FINISH_TASK:
            prob     = ent.pelemdata->finish_task_cond.encounter_enermy_prob;
            interval = ent.pelemdata->finish_task_cond.encounter_interval;
            mg_id    = ent.pelemdata->finish_task_cond.monster_group_id;
            bs_id    = ent.pelemdata->finish_task_cond.battle_scene_id;
            break;

        default:
            ASSERT(false);
            return false;
    }

    if (mrand::RandSelect(prob))
    {
        if (!player_.CheckLandmineAccumulate(interval))
            return false;

		msg_obj_trigger_combat param;
		param.monster_group_id  = mg_id;
		param.battle_scene_id   = bs_id;
		param.require_combatend_notify = true;
		param.landmine_interval = interval;
        
        MSG msg;
        BuildMessage(msg, GS_MSG_OBJ_TRIGGER_COMBAT, player_.object_xid(), ent.object_xid, 
                     0, &param, sizeof(param), player_.pos());
        Gmatrix::SendObjectMsg(msg);

		ent.encounter_timer = INT_MAX;
        return true;
	}
    return false;
}

void PlayerLandmine::RegisterMsgHandler()
{
    REGISTER_MSG_HANDLER(GS_MSG_NOTIFY_LANDMINE_INFO, PlayerLandmine::MSGHandler_NotifyLandmineInfo);
}

int PlayerLandmine::MSGHandler_NotifyLandmineInfo(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, msg_notify_landmine_info);
	const msg_notify_landmine_info& param = *(msg_notify_landmine_info*)msg.content;

    if (param.is_leave_area)
    {
        landmine_map_.erase(msg.source.id);
        return 0;
    }

    LandmineInfoMap::iterator it = landmine_map_.find(msg.source.id);
    if (it == landmine_map_.end())
    {
        entry_t tmpent;
        tmpent.pelemdata = s_pMapData->QueryMapDataTempl<mapDataSvr::AreaMonster>(param.elem_id);
        if (tmpent.pelemdata == NULL) 
        {
            LOG_ERROR << "暗雷区域地图元素id：" << param.elem_id << " 没有找到！";
            return 0;
        }
        landmine_map_[msg.source.id] = tmpent;
    }

    entry_t& ent        = landmine_map_[msg.source.id];
    ASSERT(ent.pelemdata != NULL);
    ent.object_xid      = msg.source;
    ent.encounter_timer = param.encounter_timer;
    return 0;
}

} // namespace gamed
