#include "shared/security/randomgen.h"
#include "task.h"
#include "task_templ.h"
#include "storage_templ.h"
#include "task_manager.h"
#include "task_wrapper.h"
#include "notify_util.h"
#include "op_util.h"
#include "task_interface.h"
#include "task_msg.h"
#include "task_data.h"
#include "entry_util.h"
#include "storage_util.h"
#include "util.h"

namespace task
{

using namespace shared;

bool InitTaskSys(const char* task_file)
{
	shared::net::RandomGen::Init();
	return s_pTask->ReadFromFile(task_file);
}

bool IsTaskNPC(TaskID taskid, int32_t npc_id)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
	if (task == NULL)
	{
		return false;
	}

	const NPCVec& npc_list = task->op.npc.npc_list;
	NPCVec::const_iterator it = npc_list.begin();
	for (; it != npc_list.end(); ++it)
	{
		if (it->id == npc_id)
		{
			return true;
		}
	}
	return false;
}

bool IsTaskMonster(TaskID taskid, int32_t mob_id, int32_t mob_group, int32_t& scene_id)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
	if (task == NULL)
	{
		return false;
	}

	const MonsterVec& monster_list = task->op.monster.monster_list;
	MonsterVec::const_iterator it = monster_list.begin();
	for (; it != monster_list.end(); ++it)
	{
		if (it->id == mob_id && it->group == mob_group && mob_group != 0)
		{
			scene_id = it->scene;
			return true;
		}
	}
	return false;
}

bool IsTaskMine(TaskID taskid, int32_t mine_id)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
	if (task == NULL)
	{
		return false;
	}

	const MineVec& mine_list = task->op.mine.mine_list;
	MineVec::const_iterator it = mine_list.begin();
	for (; it != mine_list.end(); ++it)
	{
		if (it->id == mine_id)
		{
			return true;
		}
	}
	return false;
}

bool IsTaskTransPos(Player* player, TaskID taskid, int32_t world_id, double x, double y)
{
	TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
    if (entry == NULL)
    {
        return false;
    }

    // 是否操作引起的传送
	const PositionVec& pos_list = entry->templ->op.trans.pos_list;
	PositionVec::const_iterator it = pos_list.begin();
	for (; it != pos_list.end(); ++it)
	{
		//if (it->IsEqual(world_id, x, y))
        if (Util::IsEqual(*it, world_id, x, y))
		{
			return true;
		}
	}

    // 是否跨地图寻路传送
    if (!entry->templ->cross_trans())
    {
        return false;
    }
    PathType path_type = entry->IsFinish() ? PATH_AWARD : PATH_COMPLETE;
    const PathInfo* info = entry->templ->path_finder.GetPathInfo(path_type);
    //return info->pos.IsEqual(world_id, x, y);
    return Util::IsEqual(info->pos, world_id, x, y);
}

bool IsChangeNameTask(TaskID taskid, int8_t name_type)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
	return task != NULL && task->succ_cond.goal == GOAL_UI && task->succ_cond.dlg_type == DLG_FIRSTNAME;
}

bool IsTransferGenderClsTask(TaskID taskid)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
	return task != NULL && task->succ_cond.goal == GOAL_UI && task->succ_cond.dlg_type == DLG_CLS;
}

bool IsStorageTask(StorageID sid, TaskID tid)
{
	const StorageTempl* templ = s_pTask->GetStorage(sid);
	if (templ == NULL)
	{
		return false;
	}

	const TaskInfoVec& task_list = templ->task_list;
	TaskInfoVec::const_iterator it = task_list.begin();
	for (; it != task_list.end(); ++it)
	{
		if (it->id == tid)
		{
			return true;
		}
	}
	return false;
}

bool IsEnterInsTask(TaskID taskid, int32_t ins_id)
{
	if (ins_id == 0)
	{
		return false;
	}
	const TaskTempl* task = s_pTask->GetTask(taskid);
	return task != NULL && task->op.ins_id == ins_id;
}

bool IsEnterBGTask(TaskID taskid, int32_t bg_id)
{
	if (bg_id == 0)
	{
		return false;
	}
	const TaskTempl* task = s_pTask->GetTask(taskid);
	return task != NULL && task->op.bg_id == bg_id;
}

bool IsUITask(TaskID taskid)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
	return task != NULL && (task->ui_task() || task->mag_task());
}

bool IsTaskFinish(Player* player, TaskID taskid)
{
	const TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
	if (entry == NULL)
	{
		return false;
	}
	TaskWrapper wrapper(player, entry->templ);
	if (wrapper.CheckTaskFinish(false) == ERR_TASK_SUCC ||
		wrapper.CheckTaskFinish(true) == ERR_TASK_SUCC)
	{
		return true;
	}
    // 下面临时添加解决序章，战斗失败的那个任务会重复战斗的问题
    // 解决方法应该是把发怪跟脚本放在一个任务里，然后战斗失败的任务增加战斗组选项
    if (entry->templ->parent_templ == NULL || entry->templ->parent_templ->subtask_deliver_mode != DELIVER_ALL)
    {
        return false;
    }
	const TaskTempl* child = entry->templ->parent_templ->first_child_templ;
	while (child != NULL)
	{
        if (child != entry->templ && player->GetActiveTask()->GetEntry(child->id) != NULL)
        {
            TaskWrapper wrapper(player, child);
            if (wrapper.CheckTaskFinish(false) == ERR_TASK_SUCC && child->fail_trigger_parent_fail())
            {
                return true;
            }
            if (wrapper.CheckTaskFinish(true) == ERR_TASK_SUCC && child->succ_trigger_parent_succ())
            {
                return true;
            }
        }
        child = child->next_sibling_templ;
    }
	return false;
}

static const TaskTempl* GetTopTask(const TaskTempl* task, TaskID& sub_taskid)
{
	const TaskTempl* parent_task = task->parent_templ;
	if (parent_task == NULL)
	{
		return task;
	}

	// 最多只能发放第一级子任务，且必须由NPC发放
	if (parent_task->parent_templ != NULL)
	{
		return NULL;
	}
	sub_taskid = parent_task->subtask_deliver_mode == DELIVER_MANUAL ? task->id : 0;
	return parent_task;
}

TaskID GetTopTaskID(TaskID taskid)
{
	const TaskTempl* task = s_pTask->GetTask(taskid);
    if (task == NULL)
    {
        return 0;
    }
    TaskID sub_taskid = taskid;
    const TaskTempl* parent_task = GetTopTask(task, sub_taskid);
    if (parent_task == NULL || sub_taskid == 0 || sub_taskid != taskid)
    {
        return 0;
    }
    return parent_task->id;
}

int32_t UpdateTaskStorage(Player* player, int32_t sid)
{
	TaskStorage* storage = player->GetTaskStorage();
	StorageEntry& entry = storage->GetEntry(sid);
	if (entry.templ == NULL)
	{
		storage->storage_list.erase(sid);
		return ERR_TASK_STORAGE;
	}
	entry.task_list.clear();
	StorageUtil::Update(player, entry);
	NotifyUtil::SendStorageRes(player, sid, true, entry.task_list);
    return ERR_TASK_SUCC;
}

int32_t UpdateTaskStorageByCash(Player* player, int32_t sid)
{
	TaskStorage* storage = player->GetTaskStorage();
	StorageEntry& entry = storage->GetEntry(sid);
	if (entry.templ == NULL)
	{
		storage->storage_list.erase(sid);
		return ERR_TASK_STORAGE;
	}

    if (player->GetCash() < entry.templ->cash)
    {
        return ERR_TASK_CASH;
    }

    player->TakeAwayCash(entry.templ->cash);

	entry.task_list.clear();
	StorageUtil::Update(player, entry, entry.templ->quality);
	NotifyUtil::SendStorageRes(player, sid, true, entry.task_list);
    return ERR_TASK_SUCC;
}

void PlayerEnterMap(Player* player)
{
	// 检查当前任务是否有需要执行的状态
	const EntryMap& task_list = player->GetActiveTask()->task_list;
	EntryMap::const_iterator tit = task_list.begin();
	for (; tit != task_list.end(); ++tit)
	{
        bool finish = false;
        if (!tit->second.IsSuccess())
        {
            finish = true;
        }
        else
        {
            TaskWrapper wrapper(player, tit->second.templ);
            if (wrapper.CheckTaskFinish(false) == ERR_TASK_SUCC ||
                    wrapper.CheckTaskFinish(true) == ERR_TASK_SUCC)
            {
                finish = true;
            }
        }
        OpUtil::ExecuteRepeatOp(player, tit->second.templ, finish, false, tit->second.IsScriptEnd());
	}
}

int32_t CanDeliverTask(Player* player, TaskID taskid)
{
	const TaskTempl* task = s_pTask->GetTopTask(taskid);
	if (task == NULL)
	{
		return ERR_TASK_ID;
	}

	TaskWrapper wrapper(player, task);
	return wrapper.CanDeliver();
}

int32_t DeliverTask(Player* player, TaskID taskid, bool npc)
{
	__PRINTF("DeliverTask roleid=%ld taskid=%d", player->GetId(), taskid);
	const TaskTempl* task = s_pTask->GetTask(taskid);
	if (task == NULL)
	{
		return ERR_TASK_ID;
	}

	// 只能发放主任务
	TaskID sub_taskid = 0;
	if ((task = GetTopTask(task, sub_taskid)) == NULL || (!npc && sub_taskid != 0))
	{
		return ERR_TASK_WRONG_SUB;
	}

	// 检查能否发放
	TaskWrapper wrapper(player, task);
	int32_t ret = wrapper.CanDeliver();
	if (ret != ERR_TASK_SUCC)
	{
		return ret;
	}

	wrapper.DeliverTask(sub_taskid);
	return ERR_TASK_SUCC;
}

static void OnTaskAutoDeliver(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyAutoDeliver msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskAutoDeliver roleid=%ld taskid=%d", player->GetId(), msg.id);
		const TaskMap& auto_task = s_pTask->GetAutoTask();
		TaskMap::const_iterator it = auto_task.find(msg.id);
		if (it == auto_task.end())
		{
			return;
		}
		const TaskTempl* task = it->second;
		TaskWrapper wrapper(player, task);
		if (wrapper.CanDeliver() == ERR_TASK_SUCC)
		{
			wrapper.DeliverTask();
		}
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskAutoDeliver Unmarshal Err %s", ex.What());
	}
}

static void OnTaskCheckFinish(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyCheckFinish msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskCheckFinish roleid=%ld taskid=%d", player->GetId(), msg.id);
		TaskEntry* entry = player->GetActiveTask()->GetEntry(msg.id);
		// 任务不存在，已经失败，直接完成并且已经完成
		if (entry == NULL || !entry->IsSuccess() || 
            (entry->templ->finish_mode == FINISH_DIRECT && entry->IsFinish()))
		{
			return;
		}

		// 检查是否满足完成条件
		TaskWrapper wrapper(player, entry->templ);
		if (wrapper.CheckTaskFinish(false) == ERR_TASK_SUCC)
		{
			wrapper.TaskFinish(false);
			NotifyUtil::SendFinish(player, msg.id, false);
		}
        else if (wrapper.CheckTaskFinish(true) == ERR_TASK_SUCC)
		{
			wrapper.TaskFinish(true);
			NotifyUtil::SendFinish(player, msg.id, true);
		}
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskCheckFinish Unmarshal Err %s", ex.What());
	}
}

static void OnTaskCheckAward(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyCheckAward msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskCheckAward roleid=%ld taskid=%d", player->GetId(), msg.id);
		TaskEntry* entry = player->GetActiveTask()->GetEntry(msg.id);
        // 只有任务已经设置完成标记才可以自动发放奖励
		if (entry == NULL || !entry->IsFinish())
		{
			return;
		}

		TaskWrapper wrapper(player, entry->templ);
		if (wrapper.CanDeliverAward() == ERR_TASK_SUCC)
		{
			wrapper.DeliverAward();
		}
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskCheckAward Unmarshal Err %s", ex.What());
	}
}

static void OnTaskGiveUp(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyGiveUp msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskGiveUp roleid=%ld taskid=%d", player->GetId(), msg.id);
		EntryMap& task_list = player->GetActiveTask()->task_list;
		EntryMap::iterator it = task_list.find(msg.id);
		if (it == task_list.end() || !it->second.templ->can_giveup())
		{
			return;
		}

        TaskWrapper wrapper(player, it->second.templ);
        wrapper.TaskFinish(false);
        NotifyUtil::SendFinish(player, msg.id, false);
    }
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskGiveUp Unmarshal Err %s", ex.What());
	}
}

static void OnTaskScriptEnd(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyScriptEnd msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();
		__PRINTF("OnTaskScriptEnd roleid=%ld taskid=%d", player->GetId(), msg.id);
		TaskEntry* entry = player->GetActiveTask()->GetEntry(msg.id);
		if (entry == NULL || entry->templ->op.exclusive_op != EXOP_SCRIPT)
		{
			return;
		}
		if (msg.skip)
		{
			entry->SetSkip();
		}
        entry->SetScriptEnd();
		player->ScriptEnd(msg.id, entry->IsSkip());
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskScriptEnd Unmarshal Err %s", ex.What());
	}
}

static void OnTaskGuideEnd(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyGuideEnd msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskGuideEnd roleid=%ld taskid=%d", player->GetId(), msg.id);
		TaskEntry* entry = player->GetActiveTask()->GetEntry(msg.id);
		if (entry == NULL || entry->templ->op.exclusive_op != EXOP_GUIDE)
		{
			return;
		}
        entry->SetScriptEnd();
		player->GuideEnd(msg.id);
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskGuideEnd Unmarshal Err %s", ex.What());
	}
}

static void OnTaskMiniGameEnd(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyMiniGameEnd msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskMiniGameEnd roleid=%ld taskid=%d succ=%d", player->GetId(), msg.id, msg.succ);
		TaskEntry* entry = player->GetActiveTask()->GetEntry(msg.id);
		if (entry == NULL || entry->templ->succ_cond.goal != GOAL_MINIGAME)
		{
			return;
		}
		if (msg.succ)
		{
            if (!player->MiniGameEnd(entry->templ->succ_cond.dlg_type))
            {
                return;
            }
            entry->FinishGoal();
            NotifyUtil::SendModify(player, msg.id);
        }
        else
        {
            TaskWrapper wrapper(player, entry->templ);
            wrapper.TaskFinish(false);
            NotifyUtil::SendFinish(player, msg.id, false);
        }
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskMiniGameEnd Unmarshal Err %s", ex.What());
	}
}

static void OnTaskStorageReq(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyStorageReq msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskStorageReq roleid=%ld sid=%d", player->GetId(), msg.id);
		StorageEntry& entry = player->GetTaskStorage()->GetEntry(msg.id);
		if (!StorageUtil::NeedUpdate(player, entry))
		{
			return;
		}
		entry.task_list.clear();
		StorageUtil::Update(player, entry);
		NotifyUtil::SendStorageRes(player, msg.id, true, entry.task_list);
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskStorageReq Unmarshal Err %s", ex.What());
	}
}

static void OnTaskLimitReq(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyLimitReq msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskLimitReq roleid=%ld taskid=%d", player->GetId(), msg.id);
        FinishTimeTask* finish_time = player->GetFinishTimeTask();
        TaskEntry* entry = finish_time->GetEntry(msg.id);
		if (entry == NULL || !EntryUtil::NeedUpdate(player, *entry))
		{
			return;
		}
        finish_time->Erase(msg.id);
		NotifyUtil::SendLimitRes(player, msg.id, 0);
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskLimitReq Unmarshal Err %s", ex.What());
	}
}

static void OnTaskPlayerRevive(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyRevive msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskPlayerRevive roleid=%ld taskid=%d", player->GetId(), msg.id);
		EntryMap& task_list = player->GetActiveTask()->task_list;
		EntryMap::iterator it = task_list.find(msg.id);
		if (it == task_list.end() || !it->second.templ->fail_cond.fail_dead())
		{
			return;
		}
        TaskWrapper wrapper(player, it->second.templ);
        wrapper.TaskFinish(false);
        NotifyUtil::SendFinish(player, msg.id, false);
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskPlayerRevive Unmarshal Err %s", ex.What());
	}
}

static void OnTaskPlayerLogin(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyLogin msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskPlayerLogin roleid=%ld sid=%d", player->GetId(), msg.id);
		EntryMap& task_list = player->GetActiveTask()->task_list;
		EntryMap::iterator it = task_list.find(msg.id);
		if (it == task_list.end() || !it->second.templ->fail_cond.fail_logout())
		{
			return;
		}
        TaskWrapper wrapper(player, it->second.templ);
        wrapper.TaskFinish(false);
        NotifyUtil::SendFinish(player, msg.id, false);
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskPlayerLogin Unmarshal Err %s", ex.What());
	}
}

static void OnTaskUITask(Player* player, const char* buff, size_t size)
{
	try
	{
		TaskNotifyUITask msg;
		msg.AppendBuffer(buff, size);
		msg.Unmarshal();

		__PRINTF("OnTaskUITask roleid=%ld sid=%d", player->GetId(), msg.id);
        const TaskTempl* task = s_pTask->GetTopTask(msg.id);
        if (task == NULL || (!task->ui_task() && !task->mag_task()))
        {
            return;
        }
		TaskWrapper wrapper(player, task);
		if (wrapper.CanDeliver() == ERR_TASK_SUCC)
		{
			wrapper.DeliverTask();
		}
	}
	catch (const Exception& ex)
	{
		__PRINTF("OnTaskUITask Unmarshal Err %s", ex.What());
	}
}

void RecvClientNotify(Player* player, uint16_t type, const char* buff, size_t size)
{
	switch (type)
	{
	case TASK_CLT_NOTIFY_AUTO_DELV:
		return OnTaskAutoDeliver(player, buff, size);
	case TASK_CLT_NOTIFY_CHECK_FINISH:
		return OnTaskCheckFinish(player, buff, size);
	case TASK_CLT_NOTIFY_CHECK_AWARD:
		return OnTaskCheckAward(player, buff, size);
	case TASK_CLT_NOTIFY_GIVE_UP:
		return OnTaskGiveUp(player, buff, size);
	case TASK_CLT_NOTIFY_SCRIPT_END:
		return OnTaskScriptEnd(player, buff, size);
	case TASK_CLT_NOTIFY_GUIDE_END:
		return OnTaskGuideEnd(player, buff, size);
	case TASK_CLT_NOTIFY_MINIGAME_END:
		return OnTaskMiniGameEnd(player, buff, size);
	case TASK_CLT_NOTIFY_STORAGE_REQ:
		return OnTaskStorageReq(player, buff, size);
	case TASK_CLT_NOTIFY_LIMIT_REQ:
		return OnTaskLimitReq(player, buff, size);
	case TASK_CLT_NOTIFY_REVIVE:
		return OnTaskPlayerRevive(player, buff, size);
	case TASK_CLT_NOTIFY_LOGIN:
		return OnTaskPlayerLogin(player, buff, size);
	case TASK_CLT_NOTIFY_UITASK:
		return OnTaskUITask(player, buff, size);
	default:
		return;
	}
}

void ChangeNameSucc(Player* player, TaskID taskid, int8_t name_type)
{
	TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
    entry->FinishGoal();
    NotifyUtil::SendModify(player, taskid);
}

void TransferGenderClsSucc(Player* player, TaskID taskid)
{
	TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
    entry->FinishGoal();
    NotifyUtil::SendModify(player, taskid);
}

void KillMonster(Player* player, int32_t mob_id, int32_t mob_count, ItemMap& items)
{
	EntryMap& task_list = player->GetActiveTask()->task_list;
	EntryMap::iterator it = task_list.begin();
	for (; it != task_list.end(); ++it)
	{
		if (it->second.templ->succ_cond.goal != GOAL_KILL_MONSTER)
		{
			continue;
		}
		TaskWrapper wrapper(player, it->second.templ);
		wrapper.KillMonster(mob_id, mob_count, items);
	}
}

bool CombatFail(Player* player, TaskID taskid)
{
    TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
    if (entry == NULL || entry->templ->succ_cond.goal != GOAL_COMBAT_FAIL)
    {
        return false;
    }
    entry->FinishGoal();
    NotifyUtil::SendModify(player, taskid);
    return true;
}
/*
void NPCDead(Player* player, int32_t npc_id)
{
	std::vector<TaskEntry*> fail_task;
	EntryMap& task_list = player->GetActiveTask()->task_list;
	EntryMap::iterator it = task_list.begin();
	for (; it != task_list.end(); ++it)
	{
		TaskEntry& entry = it->second;
		const TaskTempl* task = entry.templ;
		if (task->fail_cond.npc_dead != 0 && task->fail_cond.npc_dead == npc_id)
		{
			fail_task.push_back(&entry);
		}
	}

	std::vector<TaskEntry*>::const_iterator fit = fail_task.begin();
	for (; fit != fail_task.end(); ++fit)
	{
		ForceTaskFinish(player, *fit, false);
	}
}
*/
void FinishPlayerTask(Player* player, TaskID taskid, bool succ)
{
	TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
    if (entry == NULL/* || entry->templ->succ_cond.goal != GOAL_TASK*/)
    {
        return;
    }
    if (succ)
    {
        entry->FinishGoal();
        NotifyUtil::SendModify(player, taskid);
    }
    else
    {
        TaskWrapper wrapper(player, entry->templ);
        wrapper.TaskFinish(false);
        NotifyUtil::SendFinish(player, taskid, false);
    }
}

void DeliverAward(Player* player, TaskID taskid, int32_t choice)
{
	int64_t roleid = player->GetId();
	__PRINTF("DeliverAward roleid=%ld taskid=%d choice=%d", roleid, taskid, choice);
	// 待发奖任务必须处于活动任务列表中，且必须处于成功完成状态
	TaskEntry* entry = player->GetActiveTask()->GetEntry(taskid);
    // 只有找NPC完成任务并且任务没有失败才检查是否可以发放奖励
	if (entry == NULL || !entry->IsSuccess() || entry->templ->finish_mode != FINISH_NPC)
	{
		return;
	}

	TaskWrapper wrapper(player, entry->templ);
	if (wrapper.CanDeliverAward() == ERR_TASK_SUCC)
	{
        wrapper.TaskFinish(true);
        NotifyUtil::SendFinish(player, taskid, true);
		wrapper.DeliverAward();
	}
}

} // namespace task
