#include "player_gather.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/mine_templ.h"
#include "gs/template/data_templ/fill_blank_cfg_tbl.h"
#include "gamed/game_module/task/include/task.h"
#include "gs/scene/world.h"
#include "gs/global/dbgprt.h"
#include "gs/global/game_util.h"
#include "gs/global/randomgen.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/player/psession.h"
#include "gs/player/task_if.h"


namespace gamed {

using namespace dataTempl;

PlayerGather::PlayerGather(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_GATHER, player),
	  cur_gather_seq_no_(0)
{
}
	
PlayerGather::~PlayerGather()
{
	cur_gather_seq_no_ = 0;
}

void PlayerGather::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::GatherMaterial, PlayerGather::CMDHandler_GatherMaterial);
	REGISTER_NORMAL_CMD_HANDLER(C2G::GatherMiniGameResult, PlayerGather::CMDHandler_GatherMiniGameResult);
}

void PlayerGather::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_GATHER_REPLY, PlayerGather::MSGHandler_GatherReply);
	REGISTER_MSG_HANDLER(GS_MSG_MINE_HAS_BEEN_ROB, PlayerGather::MSGHandler_MineHasBeenRob);
	REGISTER_MSG_HANDLER(GS_MSG_GATHER_RESULT, PlayerGather::MSGHandler_GatherResult);
	REGISTER_MSG_HANDLER(GS_MSG_DRAMA_GATHER, PlayerGather::MSGHandler_DramaGather);
	REGISTER_MSG_HANDLER(GS_MSG_DRAMA_GATHER_MG_RESULT, PlayerGather::MSGHandler_DramaGatherMiniGameResult);
}

void PlayerGather::CMDHandler_GatherMaterial(const C2G::GatherMaterial& cmd)
{
	// 查看状态
	if (!player_.state().CanGather())
	{
		__PRINTF("当前状态无法采集 C2G::GatherMaterial");
		return;
	}

	XID matter_xid;
	MAKE_XID(cmd.obj_id, matter_xid);
	if (!matter_xid.IsMatter())
	{
		// 不是matter不能采集
		return;
	}

	world::worldobj_base_info info;
	if (!player_.world_plane()->QueryObject(matter_xid, info))
	{
		// 找不到对象
		return;
	}

	GatherMaterialStart(matter_xid, info.tid);
}

void PlayerGather::GatherMaterialStart(const XID& target, int32_t tid)
{
	// 找到数据模板
	const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(tid);
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Matter的DataTempl，templ_id=" << tid;
		return;
	}

	if (!CheckRequireCondition(pdata))
	{
		// 不满足开启条件
		return;
	}
		
	// 添加session
	float gather_time;
	if (pdata->turn_on_mode == MineTempl::TM_PROGRESSING)
	{
		gather_time = millisecond_to_second(pdata->progressing_time) + 0.5;
	}
	else
	{
		gather_time = millisecond_to_second(pdata->nonprogress_timeout) + 0.5;
	}
	PGatherPrepareSession* psession = new PGatherPrepareSession(&player_);
	psession->SetTarget(target, GetNextGatherSeqNo(), gather_time);
	player_.AddSessionAndStart(psession);

	// 拒绝一次暗雷触发战斗
	player_.RejectLandmineOneTime();
}

void PlayerGather::CMDHandler_GatherMiniGameResult(const C2G::GatherMiniGameResult& cmd)
{
	// 查看状态
	if (!player_.state().CanGather())
	{
		__PRINTF("当前状态无法采集 C2G::GatherMiniGameResult");
		return;
	}

	XID matter_xid;
	MAKE_XID(cmd.obj_id, matter_xid);
	HandleGatherMiniGameResult(matter_xid, cmd.gather_seq_no, cmd.is_success);
}

void PlayerGather::HandleGatherMiniGameResult(const XID& target, int32_t gather_seq_no, bool is_success)
{
	if (gather_seq_no != cur_gather_seq_no_)
	{
		__PRINTF("擦图采集序列不对");
		return;
	}

	PGatherResultSession* psession = new PGatherResultSession(&player_);
	psession->SetTarget(target, gather_seq_no, is_success);
	player_.AddSessionAndStart(psession);
}

int PlayerGather::MSGHandler_GatherReply(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_gather_reply);
	const msg_gather_reply& param = *(const msg_gather_reply*)msg.content;
	const XID& matter_xid         = msg.source;

	// 查看状态
	if (!player_.state().CanGather())
	{
		__PRINTF("当前状态无法采集 MSGHandler_GatherReply");
		player_.SendMsg(GS_MSG_GATHER_CANCEL, msg.source, param.gather_seq_no);
		return 0;
	}

	// 是否有后续操作已经执行
	if (player_.HasSession())
	{
		__PRINTF("有后续Session无法采集");
		player_.SendMsg(GS_MSG_GATHER_CANCEL, msg.source, param.gather_seq_no);
		return 0;
	}

	// 检查是不是当前的采集序列
	if (param.gather_seq_no == cur_gather_seq_no_)
	{
		const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(param.templ_id);
		if (!pdata) 
		{
			LOG_ERROR << "没有找到Matter的DataTempl，templ_id=" << param.templ_id;
			player_.SendMsg(GS_MSG_GATHER_CANCEL, msg.source, param.gather_seq_no);
			return 0;
		}

		// 添加session
		int32_t gather_time;
		if (pdata->turn_on_mode == MineTempl::TM_PROGRESSING)
		{
			gather_time = pdata->progressing_time;
		}
		else
		{
			gather_time = pdata->nonprogress_timeout;
		}
		PGatherSession* psession = new PGatherSession(&player_);
		psession->SetTarget(matter_xid, gather_time, cur_gather_seq_no_, param.templ_id);
        if (pdata->trigger_task_id > 0)
        {
            psession->SetTriggerTask(pdata->trigger_task_id, 
                                     pdata->trigger_task_prob, 
                                     pdata->trigger_task_interval, 
                                     pdata->trigger_task_times);
        }
		player_.AddSessionAndStart(psession);
	}
	else if (param.gather_seq_no < cur_gather_seq_no_)
	{
		player_.SendMsg(GS_MSG_GATHER_CANCEL, msg.source, param.gather_seq_no);
	}
	else
	{
		ASSERT(false);
	}

	return 0;
}

int PlayerGather::MSGHandler_MineHasBeenRob(const MSG& msg)
{
	int gather_seq_no     = msg.param;
	const XID& matter_xid = msg.source;
	if (gather_seq_no == cur_gather_seq_no_)
	{
		PGatherResultSession* psession = new PGatherResultSession(&player_);
		psession->SetTarget(matter_xid, gather_seq_no, false, 1);
		player_.AddSessionAndStart(psession);
	}
	else if (gather_seq_no > cur_gather_seq_no_)
	{
		ASSERT(false);
	}
	return 0;
}

int PlayerGather::MSGHandler_GatherResult(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_gather_result);
	const msg_gather_result& param = *(const msg_gather_result*)msg.content;

	if (param.gather_seq_no != cur_gather_seq_no_)
	{
		__PRINTF("MSGHandler_GatherResult() 采集序号居然不对！");
		return 0;
	}

	// 找到数据模板
	const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(param.templ_id);
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Matter的DataTempl，templ_id=" << param.templ_id;
		return 0;
	}

	if (!CheckRequireCondition(pdata))
	{
		__PRINTF("MSGHandler_GatherResult() 玩家不满足采集开启条件！");
		return 0;
	}

	if (!CheckMiniGameCondition(pdata))
	{
		__PRINTF("MSGHandler_GatherResult() 玩家不满足小游戏采集开启条件");
		return 0;
	}

	// 扣除所需物品
	if (pdata->is_consume_item)
	{
		if (!player_.TakeOutItem(pdata->required_item_id, pdata->required_item_count))
		{
			return 0;
		}
	}

	// 小游戏类检查及扣除
	ConsumeMiniGameCond(pdata);

	///
	/// 采集成功，执行采集后操作
	///
	
	// 发物品
	DeliverAwardItem(pdata);

	// 发任务
	DeliverTask(pdata);

	// 回收任务
	RecycleTask(pdata);

	// 获得喵类视觉经验
	if (pdata->cat_vision_exp > 0)
	{
		player_.CatVisionGainExp(pdata->cat_vision_exp);
	}

	// 获得经验
	if (pdata->exp_award > 0)
	{
		player_.IncExp(pdata->exp_award);
	}

	// 获得游戏币
	if (pdata->money_award > 0)
	{
		player_.GainMoney(pdata->money_award);
	}

    // 获得学分
    if (pdata->score_award > 0)
    {
        player_.GainScore(pdata->score_award);
    }

	// 发战斗
	if (pdata->monster_group_id > 0)
	{
		if (mrand::RandSelect((int32_t)pdata->trigger_combat_prob))
		{
			msg_sys_trigger_combat param;
			param.monster_group_id = pdata->monster_group_id;
			param.battle_scene_id  = player_.GetCombatSceneID();
			player_.SendMsg(GS_MSG_SYS_TRIGGER_COMBAT, player_.object_xid(), &param, sizeof(param));
		}
	}

	// 开启地图元素
	if (pdata->enable_map_elem_list.size() > 0)
	{
		for (size_t i = 0; i < pdata->enable_map_elem_list.size(); ++i)
		{
			player_.CtrlMapElement(pdata->enable_map_elem_list[i], true);
		}
	}

	// 关闭地图元素
	if (pdata->disable_map_elem_list.size() > 0)
	{
		for (size_t i = 0; i < pdata->disable_map_elem_list.size(); ++i)
		{
			player_.CtrlMapElement(pdata->disable_map_elem_list[i], false);
		}
	}

    // 如果在有采集事件的地图需要通知地图，玩家采矿成功
    if (player_.world_plane()->HasMapGather())
    {
        player_.SendPlaneMsg(GS_PLANE_MSG_MAP_PLAYER_GATHER_MINE, pdata->templ_id);
    }

	return 0;
}

int PlayerGather::MSGHandler_DramaGather(const MSG& msg)
{
	GatherMaterialStart(msg.source, static_cast<int32_t>(msg.param));
	return 0;
}

int PlayerGather::MSGHandler_DramaGatherMiniGameResult(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_drama_erase_result);
	msg_drama_erase_result& param = *(msg_drama_erase_result*)msg.content;
	HandleGatherMiniGameResult(msg.source, param.gather_seq_no, param.is_success);
	return 0;
}

bool PlayerGather::CheckRequireCondByTid(int32_t tid)
{
	// 找到数据模板
	const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(tid);
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Matter的DataTempl，templ_id=" << tid;
		return false;
	}

	return CheckRequireCondition(pdata);
}

bool PlayerGather::CheckRequireCondition(const MineTempl* pdata)
{
	// 有没有对应的任务
	if (pdata->required_task_id > 0 && 
		!player_.HasActiveTask(pdata->required_task_id))
	{
		__PRINTF("没有对应的任务，不能采集！task_id:%d", pdata->required_task_id);
		return false;
	}

	// 有没有对应的物品
	if (pdata->required_item_id > 0 && 
		!player_.CheckItem(pdata->required_item_id, pdata->required_item_count))
	{
		__PRINTF("没有对应的物品，不能采集！item_id%d", pdata->required_item_id);
		return false;
	}

	// 检查喵类视觉
	int32_t self_cat_vision = player_.cat_vision_level();	
	if (pdata->cat_vision > 0 &&
		self_cat_vision < pdata->cat_vision)
	{
		__PRINTF("喵类视觉等级不够");
		return false;
	}

	// 检查obj黑名单
	if (!player_.ObjectCanInteract(pdata->templ_id))
	{
		LOG_WARN << "矿在黑名单里，不能交互！tid=" << pdata->templ_id;
		return false;
	}

	return true;
}

bool PlayerGather::CheckMiniGameCondition(const dataTempl::MineTempl* pdata)
{
	if (pdata->turn_on_mode == MineTempl::TM_BLANK_FILLING)
	{
		const FillBlankCfgTblTempl* fillblank_templ = s_pDataTempl->QueryDataTempl<FillBlankCfgTblTempl>(pdata->fillblank_templ_id);
		if (fillblank_templ == NULL)
		{
			LOG_ERROR << "玩家填图采集找不到填图模板，mine_tid:" << pdata->templ_id
				<< " fill_blank_tid:" << pdata->fillblank_templ_id;
			return false;
		}

		for (size_t i = 0; i < fillblank_templ->point_list.size(); ++i)
		{
			if (!player_.CheckItem(fillblank_templ->point_list[i].item_id, 1))
			{
				__PRINTF("玩家：%ld填图采集没有找到对应的point物品：%d", player_.role_id(), 
						 fillblank_templ->point_list[i].item_id);
				return false;
			}
		}
	}

	return true;
}
	
void PlayerGather::ConsumeMiniGameCond(const dataTempl::MineTempl* pdata)
{
	if (pdata->turn_on_mode == MineTempl::TM_BLANK_FILLING)
	{
		const FillBlankCfgTblTempl* fillblank_templ = s_pDataTempl->QueryDataTempl<FillBlankCfgTblTempl>(pdata->fillblank_templ_id);
		if (fillblank_templ == NULL)
		{
			LOG_ERROR << "玩家填图采集找不到填图模板，mine_tid:" << pdata->templ_id
				<< " fill_blank_tid:" << pdata->fillblank_templ_id;
			return;
		}

		if (fillblank_templ->consume_item)
		{
			for (size_t i = 0; i < fillblank_templ->point_list.size(); ++i)
			{
				if (!player_.TakeOutItem(fillblank_templ->point_list[i].item_id, 1))
				{
					LOG_ERROR << "玩家" << player_.role_id() << " 填图采集扣除物品时没有找到对应的point物品！item_id:"
						<< fillblank_templ->point_list[i].item_id;
					continue;
				}
			}
		}
	}
}

void PlayerGather::DeliverAwardItem(const dataTempl::MineTempl* pdata)
{
	if (pdata->award_item_list.empty())
	{
		return;
	}

	ASSERT(MineTempl::kMaxAwardItemListSize == (int)pdata->award_item_list.max_size());

	int32_t prob[MineTempl::kMaxAwardItemListSize] = {0};
	for (size_t i = 0; i < pdata->award_item_list.size(); ++i)
	{
		prob[i] = static_cast<int32_t>(pdata->award_item_list[i].probability);
	}

	int deliver_count = pdata->deliver_count;
	for (; deliver_count > 0; --deliver_count)
	{
		int index          = mrand::RandSelect(prob, pdata->award_item_list.size());
		int32_t item_id    = pdata->award_item_list[index].item_id;
		int32_t item_count = pdata->award_item_list[index].item_count;
		if (!player_.GainItem(item_id, item_count))
		{
			LOG_ERROR << "player GainItem error!";
			break;
		}
	}
}	

void PlayerGather::DeliverTask(const dataTempl::MineTempl* pdata)
{
	for (size_t i = 0; i < pdata->deliver_task_list.size(); ++i)
	{
		int32_t task_id = pdata->deliver_task_list[i];
		if (player_.DeliverTask(task_id) == 0)
		{
			__PRINTF("玩家%ld挖矿接到任务........task_id:%d", player_.role_id(), task_id);
		}
	}
}

void PlayerGather::RecycleTask(const dataTempl::MineTempl* pdata)
{
	for (size_t i = 0; i < pdata->recycle_task_list.size(); ++i)
	{
		int32_t task_id = pdata->recycle_task_list[i];
		player_.DeliverAward(task_id, 0);
	}
}

} // namespace gamed
