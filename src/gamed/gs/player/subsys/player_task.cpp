#include "player_task.h"

#include "common/protocol/gen/G2M/instance_msg.pb.h"
#include "common/protocol/gen/G2M/battleground_msg.pb.h"
#include "gamed/game_module/task/include/task.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/fill_blank_cfg_tbl.h"
#include "gs/template/data_templ/instance_templ.h"
#include "gs/template/data_templ/battleground_templ.h"
#include "gamed/client_proto/G2C_proto.h"

#include "gs/global/dbgprt.h"
#include "gs/global/global_counter.h"
#include "gs/global/timer.h"
#include "gs/scene/world.h"
#include "gs/scene/base_wm_imp.h"
#include "gs/player/task_if.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;

SHARED_STATIC_ASSERT((int8_t)playerdef::NT_FIRST_NAME  == (int8_t)task::NAME_FIRST);
SHARED_STATIC_ASSERT((int8_t)playerdef::NT_MIDDLE_NAME == (int8_t)task::NAME_MID);
SHARED_STATIC_ASSERT((int8_t)playerdef::NT_LAST_NAME   == (int8_t)task::NAME_LAST);

PlayerTask::PlayerTask(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_TASK, player),
      is_getalldata_complete_(false)
{
	SAVE_LOAD_REGISTER(common::PlayerTaskData, PlayerTask::SaveToDB, PlayerTask::LoadFromDB);
}

PlayerTask::~PlayerTask()
{
}

void PlayerTask::OnRelease()
{
    is_getalldata_complete_ = false;
    waiting_task_.clear();
}

void PlayerTask::OnLeaveWorld()
{
    if (gcounter_set_.size() > 0)
    {
        s_pGCounter->UnregisterListenerByID(player_.master_id(), player_.role_id());
    }
    gcounter_set_.clear();

    // map_counter_set_在换地图LeaveMap通知地图
}

void PlayerTask::OnHeartbeat(time_t cur_time)
{
    FlushWaitingTasks();
}

bool PlayerTask::LoadFromDB(const common::PlayerTaskData& data)
{
	int32_t now = g_timer->GetSysTime();
	const std::string* task_data = &(data.active_task);
	if (!task_data->empty())
	{
		active_task_.AppendBuffer(task_data->c_str(), task_data->length());
		active_task_.Unmarshal();
		active_task_.LoadComplete(now);
	}

	task_data = &(data.finish_task);
	if (!task_data->empty())
	{
		finish_task_.AppendBuffer(task_data->c_str(), task_data->length());
		finish_task_.Unmarshal();
		finish_task_.LoadComplete(now);
	}

	task_data = &(data.finish_time_task);
	if (!task_data->empty())
	{
		finish_time_task_.AppendBuffer(task_data->c_str(), task_data->length());
		finish_time_task_.Unmarshal();
		finish_time_task_.LoadComplete(now);
	}

	task_data = &(data.task_diary);
	if (!task_data->empty())
	{
		task_diary_.AppendBuffer(task_data->c_str(), task_data->length());
		task_diary_.Unmarshal();
		task_diary_.LoadComplete(now);
	}

	task_data = &(data.task_storage);
	if (!task_data->empty())
	{
		task_storage_.AppendBuffer(task_data->c_str(), task_data->length());
		task_storage_.Unmarshal();
		task_storage_.LoadComplete(now);
	}

	return true;
}

bool PlayerTask::SaveToDB(common::PlayerTaskData* pData)
{
	int32_t now = g_timer->GetSysTime();
	active_task_.PreSave(now);
	active_task_.ClearContent();
	active_task_.Marshal();
	size_t size = active_task_.GetSize();
	const char* content = (const char*)(active_task_.GetContent());
	if (size > 0)
	{
		pData->active_task.assign(content, size);
	}

	finish_task_.ClearContent();
	finish_task_.Marshal();
	size = finish_task_.GetSize();
	content = (const char*)(finish_task_.GetContent());
	if (size > 0)
	{
		pData->finish_task.assign(content, size);
	}

	finish_time_task_.ClearContent();
	finish_time_task_.Marshal();
	size = finish_time_task_.GetSize();
	content = (const char*)(finish_time_task_.GetContent());
	if (size > 0)
	{
		pData->finish_time_task.assign(content, size);
	}

	task_diary_.ClearContent();
	task_diary_.Marshal();
	size = task_diary_.GetSize();
	content = (const char*)(task_diary_.GetContent());
	if (size > 0)
	{
		pData->task_diary.assign(content, size);
	}

	task_storage_.ClearContent();
	task_storage_.Marshal();
	size = task_storage_.GetSize();
	content = (const char*)(task_storage_.GetContent());
	if (size > 0)
	{
		pData->task_storage.assign(content, size);
	}
	return true;
}

void PlayerTask::PlayerGetTaskData()
{
	PlayerTaskIf task_if(&player_);
	task::PlayerEnterMap(&task_if);

	active_task_.ClearContent();
	active_task_.Marshal();
	finish_task_.ClearContent();
	finish_task_.Marshal();
	finish_time_task_.ClearContent();
	finish_time_task_.Marshal();
	task_diary_.ClearContent();
	task_diary_.Marshal();
	task_storage_.ClearContent();
	task_storage_.Marshal();

	G2C::TaskData packet;
	const char* content = (const char*)(active_task_.GetContent());
	size_t size = active_task_.GetSize();
	packet.active_task.assign(content, size);

	content = (const char*)(finish_task_.GetContent());
	size = finish_task_.GetSize();
	packet.finish_task.assign(content, size);

	content = (const char*)(finish_time_task_.GetContent());
	size = finish_time_task_.GetSize();
	packet.finish_time_task.assign(content, size);

	content = (const char*)(task_diary_.GetContent());
	size = task_diary_.GetSize();
	packet.task_diary.assign(content, size);

	content = (const char*)(task_storage_.GetContent());
	size = task_storage_.GetSize();
	packet.task_storage.assign(content, size);

	player_.sender()->SendCmd(packet);
}

void PlayerTask::KillMonster(int32_t monster_id, int32_t monster_num, std::vector<ItemEntry>& items)
{
	PlayerTaskIf task_if(&player_);
	task::ItemMap item_map;
	task::KillMonster(&task_if, monster_id, monster_num, item_map);

	task::ItemMap::iterator it = item_map.begin();
	for (; it != item_map.end(); ++ it)
	{
		ItemEntry item;
		item.item_id = it->first;
		item.item_count = it->second;
		items.push_back(item);
	}
}

int PlayerTask::CanDeliverTask(int32_t taskid) const
{
	PlayerTaskIf task_if(&player_);
	return task::CanDeliverTask(&task_if, taskid);
}

int PlayerTask::DeliverTask(int32_t taskid)
{
	PlayerTaskIf task_if(&player_);
	return task::DeliverTask(&task_if, taskid, false);
}

void PlayerTask::DeliverAward(int32_t task_id, int32_t choice)
{
	PlayerTaskIf task_if(&player_);
	task::DeliverAward(&task_if, task_id, choice);
}

bool PlayerTask::MiniGameEnd(int32_t game_tid)
{
	const BaseDataTempl* ptempl = s_pDataTempl->QueryBaseDataTempl(game_tid);
	if (ptempl == NULL)
		return false;

	switch (ptempl->GetType())
	{
		case dataTempl::TEMPL_TYPE_FILL_BLANK_CFG_TBL:
			{
				const FillBlankCfgTblTempl* fillblank_templ = dynamic_cast<const FillBlankCfgTblTempl*>(ptempl);
				if (fillblank_templ == NULL)
				{
					LOG_ERROR << "玩家填图采集找不到填图模板fill_blank_tid:" << fillblank_templ->templ_id;
					return false;
				}

				if (fillblank_templ->consume_item)
				{
					for (size_t i = 0; i < fillblank_templ->point_list.size(); ++i)
					{
						if (!player_.TakeOutItem(fillblank_templ->point_list[i].item_id, 1))
						{
							LOG_ERROR << "玩家" << player_.role_id() << " 填图采集扣除物品时没有找到对应的point物品！item_id:"
								<< fillblank_templ->point_list[i].item_id;
							return false;
						}
					}
				}
			}
			break;

		default:
			return false;
	}

	return true;
}

void PlayerTask::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::TaskNotify, PlayerTask::CMDHandler_TaskNotify);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TaskRegionTransfer, PlayerTask::CMDHandler_TaskRegionTransfer);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TaskChangeName, PlayerTask::CMDHandler_TaskChangeName);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TaskTransferGenderCls, PlayerTask::CMDHandler_TaskTransferGenderCls);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TaskInsTransfer, PlayerTask::CMDHandler_TaskInsTransfer);
	REGISTER_NORMAL_CMD_HANDLER(C2G::UITaskRequest, PlayerTask::CMDHandler_UITaskRequest);
	REGISTER_NORMAL_CMD_HANDLER(C2G::UpdateTaskStorage, PlayerTask::CMDHandler_UpdateTaskStorage);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TaskBGTransfer, PlayerTask::CMDHandler_TaskBGTransfer);
}

void PlayerTask::RegisterMsgHandler()
{
    REGISTER_MSG_HANDLER(GS_MSG_WORLD_DELIVER_TASK, PlayerTask::MSGHandler_WorldDeliverTask);
    REGISTER_MSG_HANDLER(GS_MSG_GLOBAL_COUNTER_CHANGE, PlayerTask::MSGHandler_GlobalCounterChange);
    REGISTER_MSG_HANDLER(GS_MSG_MAP_COUNTER_CHANGE, PlayerTask::MSGHandler_MapCounterChange);
}

void PlayerTask::CMDHandler_TaskNotify(const C2G::TaskNotify& packet)
{
	if (packet.databuf.size())
	{
		PlayerTaskIf task_if(&player_);
		task::RecvClientNotify(&task_if, packet.type, packet.databuf.c_str(), packet.databuf.size());
	}
}

void PlayerTask::CMDHandler_TaskRegionTransfer(const C2G::TaskRegionTransfer& cmd)
{
	///
	/// check validity
	///
	/*if (!player_.HasActiveTask(cmd.task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能进行传送！", cmd.task_id);
		return;
	}*/

	PlayerTaskIf task_if(&player_);
	if (!task::IsTaskTransPos(&task_if, cmd.task_id, cmd.map_id, cmd.pos_x, cmd.pos_y))
	{
		__PRINTF("不是任务传送，不能进行传送！task_id=%d map_id=%d x=%f y=%f", 
				cmd.task_id, cmd.map_id, cmd.pos_x, cmd.pos_y);
		return;
	}

	if (IS_NORMAL_MAP(player_.world_id()) && !IS_NORMAL_MAP(cmd.map_id))
	{
		LOG_ERROR << "玩家试图从普通地图传送到副本地图，任务数据填错！！task_id=" << cmd.task_id 
			<< " world_id=" << player_.world_id() << " map_id=" << cmd.map_id;
		return;
	}

    if (!IS_NORMAL_MAP(player_.world_id()) && player_.world_id() != cmd.map_id)
    {
        LOG_WARN << "玩家试图从非普通地图往外传送，任务task_id=" << cmd.task_id 
            << " world_id=" << player_.world_id() << " map_id=" << cmd.map_id;
        return;
    }

	if (player_.world_id() != cmd.map_id)
	{
		if (IS_INS_MAP(player_.world_id()) && !IS_NORMAL_MAP(cmd.map_id))
		{
			LOG_ERROR << "玩家在副本地图，但要求传送到非普通地图，并且不是本地图，任务数据填错！！task_id=" 
				<< cmd.task_id << " world_id=" << player_.world_id() << " map_id=" << cmd.map_id;
			return;
		}
	}

	///
	/// success
	///
	msg_player_region_transport param;
	param.source_world_id = player_.world_id();
	param.target_world_id = cmd.map_id;
	param.target_pos.x    = cmd.pos_x;
	param.target_pos.y    = cmd.pos_y;
	player_.SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, player_.object_xid(), &param, sizeof(param));
}

void PlayerTask::CMDHandler_TaskChangeName(const C2G::TaskChangeName& cmd)
{
	if (!CheckChangeNameParam(cmd.task_id, cmd.name_type, cmd.name))
	{
		LOG_WARN << "玩家" << player_.role_id() << " 任务改名参数有误！";
		SendChangeNameError();
		return;
	}
	else
	{
		player_.TaskChangeName(cmd.task_id, (playerdef::NameType)cmd.name_type, cmd.name);
		return;
	}
}

void PlayerTask::CMDHandler_TaskTransferGenderCls(const C2G::TaskTransferGenderCls& cmd)
{
	if (!CheckGenderClsParam(cmd.taskid, cmd.gender, cmd.cls))
	{
		LOG_WARN << "玩家" << player_.role_id() << " 任务转职变性参数有误！";
		return;
	}
	// 顺序不能变，因为初始盖纳没有女性模型
	// 如果先变性客户端会出错
	player_.TransferCls(cmd.cls);
	player_.TransferGender(cmd.gender);
	PlayerTaskIf task_if(&player_);
	task::TransferGenderClsSucc(&task_if, cmd.taskid);
}

void PlayerTask::CMDHandler_TaskInsTransfer(const C2G::TaskInsTransfer& cmd)
{
	///
	/// check validity
	///
	if (!player_.HasActiveTask(cmd.taskid))
	{
		__PRINTF("玩家没有该任务taskid%d，不能进行副本传送！", cmd.taskid);
		return;
	}

	if (!task::IsEnterInsTask(cmd.taskid, cmd.ins_templ_id))
	{
		__PRINTF("不是任务副本传送，不能进行传送！taskid=%d ins_templ_id=%d", 
				cmd.taskid, cmd.ins_templ_id);
		return;
	}

	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(cmd.ins_templ_id);
	if (ptempl == NULL)
	{
		__PRINTF("CMDHandler_TaskInsTransfer()找不到对应的副本模板！ins_templ_id:%d", cmd.ins_templ_id);
		return;
	}
	if (ptempl->ins_type != InstanceTempl::IT_SOLO)
	{
		LOG_WARN << "任务传送副本：所填副本id是非单人副本，无法进入！";
		return;
	}

	msg_ins_transfer_prepare ins_param;
	ins_param.ins_templ_id = cmd.ins_templ_id;
	ins_param.request_type = G2M::IRT_TASK_SOLO;
	player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));
}

void PlayerTask::CMDHandler_TaskBGTransfer(const C2G::TaskBGTransfer& cmd)
{
	if (!player_.HasActiveTask(cmd.taskid))
	{
		__PRINTF("玩家没有该任务taskid%d，不能进行战场传送！", cmd.taskid);
		return;
	}

	if (!task::IsEnterBGTask(cmd.taskid, cmd.bg_templ_id))
	{
		__PRINTF("不是任务战场传送，不能进行传送！taskid=%d bg_templ_id=%d", 
				cmd.taskid, cmd.bg_templ_id);
		return;
	}

	const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(cmd.bg_templ_id);
	if (ptempl == NULL)
	{
		__PRINTF("CMDHandler_TaskBGTransfer()找不到对应的战场模板！bg_templ_id:%d", cmd.bg_templ_id);
		return;
	}

	msg_bg_transfer_prepare bg_param;
	bg_param.bg_templ_id = cmd.bg_templ_id;
    bg_param.request_type = G2M::BGRT_UI_PVE_RALLY;
	player_.SendMsg(GS_MSG_BG_TRANSFER_PREPARE, player_.object_xid(), &bg_param, sizeof(bg_param));
}

void PlayerTask::CMDHandler_UITaskRequest(const C2G::UITaskRequest& cmd)
{
	if (!task::IsUITask(cmd.task_id))
	{
		__PRINTF("任务:%d 不是UI任务，不能远程领取！", cmd.task_id);
		return;
	}

	PlayerTaskIf task_if(&player_);
	if (task::DeliverTask(&task_if, cmd.task_id) == 0)
	{
		__PRINTF("%ld接到UI任务..........task_id:%d", player_.role_id(), cmd.task_id);
	}
}

void PlayerTask::CMDHandler_UpdateTaskStorage(const C2G::UpdateTaskStorage& cmd)
{
	PlayerTaskIf task_if(&player_);
    int32_t err = task::UpdateTaskStorageByCash(&task_if, cmd.storage_id);
    if (err == task::ERR_TASK_STORAGE)
    {
        return;
    }

    G2C::UpdateTaskStorage_Re reply;
    reply.storage_id = cmd.storage_id;
    reply.err = err == task::ERR_TASK_SUCC ? 0 : 1;
    player_.sender()->SendCmd(reply);
	__PRINTF("%ld付费刷新库任务..........storage_id:%d err=%d", player_.role_id(), cmd.storage_id, reply.err);
}

bool PlayerTask::CheckChangeNameParam(int32_t task_id, int8_t name_type, const std::string& name)
{
	///
	/// check validity 以下检查顺序不能乱
	///
	if (!player_.HasActiveTask(task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能进行改名！", task_id);
		return false;
	}

	if (name_type != task::NAME_FIRST && 
		name_type != task::NAME_MID &&
		name_type != task::NAME_LAST)
	{
		__PRINTF("玩家%ld改名的name_type不正确！", player_.role_id());
		return false;
	}

	if (!task::IsChangeNameTask(task_id, name_type))
	{
		__PRINTF("玩家%ld改名，任务%d不是改名任务！", player_.role_id(), task_id);
		return false;
	}

	if (name.size() > 64)
	{
		__PRINTF("玩家%ld改名，名字过长！", player_.role_id());
		return false;
	}

	if (name.size() != strlen(name.c_str()))
	{
		__PRINTF("玩家%ld改名，名字输入有问题！", player_.role_id());
		return false;
	}

	// ?????????????????????检查字符

	return true;
}

bool PlayerTask::CheckGenderClsParam(int32_t task_id, int8_t gender, int32_t cls)
{
	///
	/// check validity 以下检查顺序不能乱
	///
	if (!player_.HasActiveTask(task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能进行转职变性！", task_id);
		return false;
	}
	if (!task::IsTransferGenderClsTask(task_id))
	{
		__PRINTF("玩家%ld转职变性，任务%d不是对应任务！", player_.role_id(), task_id);
		return false;
	}

	if (gender < 0 || gender > 1)
	{
		return false;
	}

	if (cls < 0 || cls >= playerdef::CLS_MAXCLS_LABEL)
	{
		return false;
	}

	return true;
}

void PlayerTask::SendChangeNameError()
{
	std::string name;
	int8_t name_type = -1;	

	G2C::ChangeName_Re packet;
	packet.err_code  = (int8_t)G2C::ChangeName_Re::SVR_CHECK_FAIL;
	packet.name_type = name_type;
	packet.name      = name;
	player_.sender()->SendCmd(packet);
}

bool PlayerTask::HasActiveTask(int32_t task_id)
{
	return active_task_.IsExist(task_id);
}

bool PlayerTask::HasFinishTask(int32_t task_id)
{
	return finish_task_.IsExist(task_id);
}

int PlayerTask::MSGHandler_WorldDeliverTask(const MSG& msg)
{
    if (msg.source != player_.world_xid())
    {
        LOG_ERROR << "recv world_deliver_task msg from other map:" << MAP_ID(msg.source) 
            << " player in map:" << player_.world_id();
        return 0;
    }

    if (is_getalldata_complete_ && player_.state().IsNormal())
    {
        DeliverTask(msg.param);
    }
    else
    {
        waiting_task_.push_back(msg.param);
    }
    return 0;
}

int PlayerTask::MSGHandler_GlobalCounterChange(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, msg_global_counter_change);
    const msg_global_counter_change& param = *(msg_global_counter_change*)msg.content;
    SendGlobalCounterChange(param.index, param.value);
    return 0;
}

int PlayerTask::MSGHandler_MapCounterChange(const MSG& msg)
{
    if (player_.world_plane()->world_xid() != msg.source)
        return 0;

    CHECK_CONTENT_PARAM(msg, msg_map_counter_change);
    const msg_map_counter_change& param = *(msg_map_counter_change*)msg.content;

    G2C::MapCounterChange packet;
    packet.map_id = player_.world_id();
    packet.index  = param.index;
    packet.value  = param.value;
    player_.sender()->SendCmd(packet);
    return 0;
}

void PlayerTask::StartRecvWorldTask()
{
    is_getalldata_complete_ = true;
    FlushWaitingTasks();
}

void PlayerTask::FlushWaitingTasks()
{
    if (is_getalldata_complete_ && player_.state().IsNormal())
    {
        if (waiting_task_.size())
        {
            for (size_t i = 0; i < waiting_task_.size(); ++i)
            {
                DeliverTask(waiting_task_[i]);
            }
            waiting_task_.clear();
        }
    }
}

void PlayerTask::SubscribeGlobalCounter(int32_t index, bool is_subscribe)
{
    // 订阅
    if (is_subscribe)
    {
        CounterSet::iterator it = gcounter_set_.find(index);
        if (it == gcounter_set_.end())
        {
            s_pGCounter->RegisterListener(player_.master_id(), player_.role_id(), index);
            gcounter_set_.insert(index);

            int32_t value;
            if (s_pGCounter->GetGCounter(player_.master_id(), index, value))
            {
                SendGlobalCounterChange(index, value);
            }
        }
    }
    else // 退订
    {
        CounterSet::iterator it = gcounter_set_.find(index);
        if (it != gcounter_set_.end())
        {
            s_pGCounter->UnregisterListener(player_.master_id(), player_.role_id(), index);
            gcounter_set_.erase(index);
        }
    }
}

bool PlayerTask::GetGlobalCounter(int32_t index, int32_t& value) const
{
	bool ret = s_pGCounter->GetGCounter(player_.master_id(), index, value);
    if (!ret) {
        LOG_ERROR << "获取全局计数器失败！id:" << index;
    }
    return ret;
}

void PlayerTask::ModifyGlobalCounter(int32_t index, int8_t op, int32_t delta)
{
    switch (op)
    {
        case task::COUNTER_OP_ASSIGN:
            s_pGCounter->SetGCounter(player_.master_id(), index, delta);
            break;

        case task::COUNTER_OP_INC:
            s_pGCounter->ModifyGCounter(player_.master_id(), index, delta);
            break;

        case task::COUNTER_OP_DEC:
            s_pGCounter->ModifyGCounter(player_.master_id(), index, -delta);
            break;

        default:
            LOG_ERROR << "modify global counter error op:" << op;
            break;
    }
}

void PlayerTask::SubscribeMapCounter(int32_t world_id, int32_t index, bool is_subscribe)
{
    if (player_.world_id() != world_id)
        return;

    if (!player_.world_plane()->HasMapCounter())
        return;

    if (!BaseWorldManImp::CheckCounterIndex(index))
        return;

    // 订阅
    if (is_subscribe)
    {
        CounterSet::iterator it = map_counter_set_.find(index);
        if (it == map_counter_set_.end())
        {
            MapCounterSubscribe(index, is_subscribe);
            map_counter_set_.insert(index);
        }
    }
    else // 退订
    {
        CounterSet::iterator it = map_counter_set_.find(index);
        if (it != map_counter_set_.end())
        {
            // 现在只处理订阅的情况，退订不通知地图，多任务同时订阅时不会出现时序问题
            //MapCounterSubscribe(index, is_subscribe);
            map_counter_set_.erase(it);
        }
    }
}

void PlayerTask::ClearMapCounterList()
{
    if (map_counter_set_.size() > 0 && player_.world_plane()->HasMapCounter())
    {
        MapCounterSubscribe(-1, false);
    }
    map_counter_set_.clear();
}

void PlayerTask::MapCounterSubscribe(int32_t index, bool is_subscribe)
{
    plane_msg_map_counter_subscribe param;
    param.index        = index;
    param.is_subscribe = is_subscribe;
    player_.SendPlaneMsg(GS_PLANE_MSG_MAP_COUNTER_SUBSCRIBE, &param, sizeof(param));
}

void PlayerTask::SendGlobalCounterChange(int32_t index, int32_t value)
{
    G2C::GlobalCounterChange packet;
    packet.index = index;
    packet.value = value;
    player_.sender()->SendCmd(packet);
}

bool PlayerTask::CombatFail(int32_t taskid)
{
	PlayerTaskIf task_if(&player_);
	return task::CombatFail(&task_if, taskid);
}

bool PlayerTask::IsTaskFinish(int32_t taskid) const
{
	PlayerTaskIf task_if(&player_);
	return task::IsTaskFinish(&task_if, taskid);
}

} // namespace gamed

