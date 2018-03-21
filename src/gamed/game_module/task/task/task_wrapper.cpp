#include "task_wrapper.h"
#include "task_templ.h"
#include "storage_templ.h"
#include "task_manager.h"
#include "task_interface.h"
#include "task_data.h"
#include "entry_util.h"
#include "cond_util.h"
#include "op_util.h"
#include "util.h"
#include "notify_util.h"
#include "storage_util.h"
#include "task_succ.h"

namespace task
{

using namespace std;

static const int32_t MAX_ACTIVE_STORAGE = 5;

TaskWrapper::TaskWrapper(Player* player, const TaskTempl* task)
	: player_(player), task_(task)
{
}

static bool HasSameTask(Player* player, const TaskTempl* task)
{
	return player->GetActiveTask()->IsExist(task->id);
}

static bool CanRedoTask(Player* player, const TaskTempl* task)
{
	FinishTask* finish_task = player->GetFinishTask();
	EntryMap::const_iterator it = finish_task->task_list.find(task->id);
	if (it != finish_task->task_list.end())
	{
        bool succ = it->second.IsSuccess();
        bool can_redo_after_succ = task->can_redo_after_succ();
        bool can_redo_after_fail = task->can_redo_after_fail();
        return (succ && can_redo_after_succ) || (!succ && can_redo_after_fail);
    }

	FinishTimeTask* finish_time_task = player->GetFinishTimeTask();
	it = finish_time_task->task_list.find(task->id);
    if (it == finish_time_task->task_list.end())
    {
        return true;
    }

    int32_t now = player->GetCurTime();
    if (now - it->second.deliver_time < task->limit.interval)
    {
        return false;
    }
    int32_t* data = (int32_t*)(&(it->second.data[0]));
    return data[0] < task->limit.num;
}

static bool IsStorageTaskFull(Player* player, const TaskTempl* task)
{
	return task->sid != 0 &&  player->GetActiveTask()->GetStorageNum() >= MAX_ACTIVE_STORAGE; 
}

static int32_t CheckPremise(Player* player, const TaskTempl* task, bool child)
{
	// 只有顶层任务才可发放，非顶层任务都由父任务直接发放
	if (!child && task->parent != 0)
	{
		return ERR_TASK_NOT_ROOT;
	}
	// 如果已经有相同的任务则不可发放
	if (HasSameTask(player, task))
	{
		return ERR_TASK_SAME;
	}
	// 如果已经完成且不能完成多次则不可发放
	if (!CanRedoTask(player, task))
	{
		return ERR_TASK_CANT_REDO;
	}
	// 身上最多只允许存在一定数目的库任务
	if (IsStorageTaskFull(player, task))
	{
		return ERR_TASK_STORAGE;
	}

	// 检查任务模板限制条件
	return CondUtil::Check(player, task->premise);
}

typedef map<int8_t, int32_t> SlotMap;
static void SlotNeeded(const DeliverItem& deliver_item, SlotMap& slot)
{
	const ItemDeliveredVec& item_list = deliver_item.item_list;
	if (deliver_item.rand_deliver && !item_list.empty())
	{
		// 随机发放的情况下默认只会发普通物品
		slot[item_list[0].type] = 1;
		return;
	}

	ItemDeliveredVec::const_iterator it = item_list.begin();
	for (; it != item_list.end(); ++it)
	{
		++(slot[it->type]);
	}
}

static void CalcSlotNeeded(SlotMap& source, SlotMap& dest, bool merge)
{
	for (int32_t i = ITEM_COMMON; i <= ITEM_HIDE; ++i)
	{
		if (merge)
		{
			dest[i] += source[i];
		}
		else if (source[i] > dest[i])
		{
			dest[i] = source[i];
		}
	}
}

static void RecursiveDeliverSlotNeeded(const TaskTempl* task, SlotMap& slot)
{
	if (task == NULL)
	{
		return;
	}
	// 获取自身所需的包裹空间
	SlotMap self_needed;
	SlotNeeded(task->op.deliver_item, self_needed);

	// 获取所有子节点可能需要的包裹空间
	SlotMap child_all_needed;
	int8_t mode = task->subtask_deliver_mode;
	const TaskTempl* child = task->first_child_templ;
	if (mode == DELIVER_ALL || mode == DELIVER_RANDOM)
	{
		while (child != NULL)
		{
			SlotMap child_needed;
			RecursiveDeliverSlotNeeded(child, child_needed);
			CalcSlotNeeded(child_needed, child_all_needed, mode == DELIVER_ALL);
			child = child->next_sibling_templ;
		}
	}
	else
	{
		RecursiveDeliverSlotNeeded(child, child_all_needed);
	}
	
	// 计算发放任务所需要的最大包裹空间
	CalcSlotNeeded(child_all_needed, slot, true);
	CalcSlotNeeded(self_needed, slot, true);
}

static int32_t HasEnoughSlot(Player* player, SlotMap& slot)
{
	for (int32_t i = ITEM_COMMON; i <= ITEM_HIDE; ++i)
	{
		if (!player->CanDeliverItem(i, slot[i]))
		{
			return ERR_TASK_ITEM;
		}
	}
	return ERR_TASK_SUCC;
}

bool TaskWrapper::CanShow() const
{
	// 只有顶层任务才可见
	if (task_->parent != 0)
	{
		return false;
	}

	// 非库任务只有可发放才可见
	// 库任务只要满足模板条件就可见
	bool show = false;
	if (task_->sid == 0)
	{
		show = CanDeliver() == ERR_TASK_SUCC;
	}
	else
	{
		show = CondUtil::Check(player_, task_->premise) == ERR_TASK_SUCC;
	}
	// 如果本身不可见，但是配置了不满足条件对话则也可见
	return show || !task_->unqualified_talk.empty();
}

int32_t TaskWrapper::CanDeliver() const
{
	// 检查基础发放条件
	int32_t ret = CheckPremise(player_, task_, false);
	if (ret != ERR_TASK_SUCC)
	{
		return ret;
	}
	// 检查是否有足够的包裹空间存放发放的物品
	SlotMap slot;
	RecursiveDeliverSlotNeeded(task_, slot);
	return HasEnoughSlot(player_, slot);
}

void TaskWrapper::DeliverTask(TaskID sub_task)
{
	__PRINTF("DeliverTask taskid=%d", task_->id);
	NotifyUtil::SendNew(player_, task_->id);
	AddTask();
	TaskDeliverOp();
	RecursiveDeliverTask(sub_task);
}

void TaskWrapper::AddTask()
{
	__PRINTF("AddTask taskid=%d", task_->id);
    ActiveTask* active = player_->GetActiveTask();
    if (active->task_list.find(task_->id) == active->task_list.end())
    {
        TaskEntry entry;
        EntryUtil::Init(player_->GetCurTime(), task_, entry);
        active->AddEntry(entry);

        int8_t status = task_->show_deliver_tips() ? STATUS_NEW_TIPS : STATUS_NEW;
        player_->StatusChanged(task_->id, status, false, task_->hide_task());
        if (task_->prior_track() && player_->IsInMapArea(task_->track.id))
        {
            player_->ClearTaskTracking();
        }
    }
}

void TaskWrapper::TaskDeliverOp()
{
	__PRINTF("TaskDeliverOp taskid=%d", task_->id);
	// 先收取开启条件中指定的物品
	// 开启条件只有主任务生效
	if (task_->parent == 0)
	{
		CondUtil::TakeAway(player_, task_->premise);
	}

	// 执行任务开启中指定的操作
	OpUtil::ExecuteOp(player_, task_);
}

void TaskWrapper::RecursiveDeliverTask(TaskID sub_task)
{
	switch (task_->subtask_deliver_mode)
	{
	case DELIVER_MANUAL:
		return DeliverManual(sub_task);
	case DELIVER_RANDOM:
		return DeliverRand();
	case DELIVER_ORDER:
		return DeliverOrder();
	case DELIVER_ALL:
		return DeliverAll();
	default:
		return;
	}
}

void TaskWrapper::DeliverManual(TaskID sub_task)
{
	const TaskTempl* child_task = task_->first_child_templ;
	while (child_task != NULL)
	{
		if (child_task->id == sub_task)
		{
			break;
		}
		child_task = child_task->next_sibling_templ;
	}
	if (child_task != NULL)
	{
		TaskWrapper wrapper(player_, child_task);
		wrapper.DeliverTask();
	}
}

void TaskWrapper::DeliverRand()
{
	TaskVec task_vec;
	vector<int32_t> prob_vec;
	const TaskTempl* child_task = task_->first_child_templ;
	while (child_task != NULL)
	{
		prob_vec.push_back(child_task->deliver_prob);
		task_vec.push_back(child_task);
		child_task = child_task->next_sibling_templ;
	}

	int32_t rand_index = Util::Rand(prob_vec);
	if (rand_index >= 0)
	{
		TaskWrapper wrapper(player_, task_vec[rand_index]);
		wrapper.DeliverTask();
	}
}

void TaskWrapper::DeliverOrder()
{
	if (task_->first_child_templ != NULL)
	{
		TaskWrapper wrapper(player_, task_->first_child_templ);
		wrapper.DeliverTask();
	}
}

void TaskWrapper::DeliverAll()
{
	const TaskTempl* child_task = task_->first_child_templ;
	while (child_task != NULL)
	{
		TaskWrapper wrapper(player_, child_task);
		wrapper.DeliverTask();
		child_task = child_task->next_sibling_templ;
	}
}

static int32_t CheckFinishCond(Player* player, TaskEntry* entry, bool succ)
{
	const TaskTempl* task = entry->templ;
    if (succ && task->first_child != 0)
    {
        return entry->IsFinishGoal() ? ERR_TASK_SUCC : ERR_TASK_STATE;
    }
	if (succ && task->first_child == 0) // 子任务检查成功条件
	{
        if (entry->templ->op.exclusive_op == EXOP_SCRIPT && !entry->IsScriptEnd())
        {
            return ERR_TASK_SCRIPT;
        }
		return CondUtil::Check(player, entry, task->succ_cond);
	}
	else
	{
		return CondUtil::Check(player, entry, task->fail_cond);
	}
}

int32_t TaskWrapper::CheckTaskFinish(bool succ) const
{
	TaskEntry* entry = player_->GetActiveTask()->GetEntry(task_->id);
	return CheckFinishCond(player_, entry, succ);
}

void TaskWrapper::TaskFinish(bool succ)
{
	TaskEntry* entry = player_->GetActiveTask()->GetEntry(task_->id);
	if (!succ)
	{
		entry->ClearSuccess();
	}
    else if (succ && task_->first_child != 0)
    {
        entry->FinishGoal();
    }
    entry->SetFinish();
	OpUtil::Recycle(task_->id, player_, task_->op.monster);
    OpUtil::SubscribeCounter(player_, task_, false);
	if (entry->IsSkip())
	{
		OpUtil::ServerTransport(player_, task_->succ_award.script_trans);
	}
}

int32_t TaskWrapper::CanDeliverAward() const
{
	int32_t ret = ERR_TASK_STATE;
	TaskEntry* entry = player_->GetActiveTask()->GetEntry(task_->id);
    bool succ = entry->IsSuccess();
    if (succ && entry->templ->finish_mode == FINISH_DIRECT && !entry->IsFinish())
    {
        return ret;
    }
    else if (succ && entry->templ->finish_mode == FINISH_NPC && CheckFinishCond(player_, entry, true) != ERR_TASK_SUCC)
    {
        return ret;
    }

	SlotMap slot;
	// 计算本身奖励所需的包裹空间
	const TaskAward& award = entry->IsSuccess() ? task_->succ_award : task_->fail_award;
	SlotNeeded(award.deliver_item, slot);

	// 如果发放新任务，需要检查该任务是否满足发放条件
	const TaskTempl* templ = s_pTask->GetTask(award.deliver_task);
	if (templ != NULL && (ret = CheckPremise(player_, templ, false)) != ERR_TASK_SUCC)
	{
		return ret;
	}
	RecursiveDeliverSlotNeeded(templ, slot);

	// 如果父任务顺序发放子任务，需要检查兄弟任务能否发放
	const TaskTempl* parent = task_->parent_templ;
	templ = task_->next_sibling_templ;
	if (parent != NULL && parent->subtask_deliver_mode == DELIVER_ORDER && templ != NULL)
	{
		if ((ret = CheckPremise(player_, templ, true)) != ERR_TASK_SUCC)
		{
			return ret;
		}
	}
	RecursiveDeliverSlotNeeded(templ, slot);

	// 检查包裹空间是否足够
	return HasEnoughSlot(player_, slot);
}

void TaskWrapper::DeliverAward()
{
	// 完成任务并递归发奖
	NotifyUtil::SendComplete(player_, task_->id);
	bool succ = TaskComplete();
	__PRINTF("DeliverAward taskid=%d succ=%d", task_->id, succ);
	RecursiveDeliverAward(succ);
}

static void OnUpdateStorageTask(Player* player, const TaskTempl* task, bool succ)
{
	if (task->sid == 0)
	{
		return;
	}
    // 记录成功库任务的完成
    if (succ)
    {
        player->StorageTaskComplete(task->id, task->quality);
    }
	// 删除已经完成的库任务
	StorageEntry& storage_entry = player->GetTaskStorage()->GetEntry(task->sid);
	TaskIDVec& task_list = storage_entry.task_list;
	TaskIDVec::iterator tit = find(task_list.begin(), task_list.end(), task->id);
	// 不一定存在，因为库任务可能刷新，刷新后原来的就不一定在列表中了
	if (tit != task_list.end())
	{
		task_list.erase(tit);
	}
#ifndef CLIENT_SIDE
	// 检查是否需要补充新的库任务
	const StorageTempl* storage_templ = storage_entry.templ;
	if (storage_templ->refresh && (int32_t)task_list.size() < storage_templ->max_num)
	{
		StorageUtil::Update(player, storage_entry);
		__PRINTF("Send New Storage Task id=%d", *(task_list.rbegin()));
		NotifyUtil::SendStorageRes(player, task->sid, false, *(task_list.rbegin()));
	}
#endif
}

static void RecursiveTaskErase(Player* player, const TaskTempl* task)
{
	ActiveTask* active_task = player->GetActiveTask();
    TaskEntry* entry = active_task->GetEntry(task->id);
    if (entry == NULL)
    {
        return;
    }
    if (task->op.exclusive_op == EXOP_SCRIPT && !entry->IsScriptEnd())
    {
        player->ScriptEnd(task->id, false);
    }
    if (task->op.exclusive_op == EXOP_GUIDE && !entry->IsScriptEnd())
    {
        player->GuideEnd(task->id);
    }
    OpUtil::Recycle(task->id, player, task->op.monster);
    OpUtil::SubscribeCounter(player, task, false);
    active_task->Erase(task->id);

	int8_t status = task->show_complete_tips() ? STATUS_COMPLETE_TIPS : STATUS_COMPLETE;
	player->StatusChanged(task->id, status, entry->IsSuccess(), task->hide_task());
    if (task->close_npc_area())
    {
        player->CloseNpcArea();
    }

	// 如果存在子任务则将子任务从当前列表中删除
	const TaskTempl* child = task->first_child_templ;
	while (child != NULL)
	{
        RecursiveTaskErase(player, child);
        child = child->next_sibling_templ;
    }
}

bool TaskWrapper::TaskComplete()
{
	TaskEntry* entry = player_->GetActiveTask()->GetEntry(task_->id);
	assert(entry != NULL && entry->IsFinish());
	bool succ = entry->IsSuccess();
	__PRINTF("TaskComplete taskid=%d succ=%d", task_->id, succ);

	// 添加任务日记
	int32_t now = player_->GetCurTime();
	if (!task_->succ_diary.empty() || !task_->fail_diary.empty())
	{
		player_->GetTaskDiary()->AddDiary(task_, succ, now);
	}

	// 添加完成任务，删除当前任务及其子任务
    if (task_->record_result())
    {
        int8_t limit = task_->limit.limit;
        entry->finish_time = now;
        entry->data.clear();
        if (limit == LIMIT_NONE && (succ || !task_->can_redo_after_fail()))
        {
		    player_->GetFinishTask()->AddEntry(*entry);
        }
        else if ((limit == LIMIT_DELIVER) || (limit == LIMIT_COMPLETE && succ))
        {
            player_->GetFinishTimeTask()->AddEntry(*entry);
        }
    }

    // 发放奖励
	CondUtil::TakeAway(player_, task_->succ_cond);
	OpUtil::Recycle(task_->id, player_, task_->op.npc);
	OpUtil::Recycle(task_->id, player_, task_->op.mine);
    OpUtil::UndoCameraMask(task_->id, player_, task_->op.camera_mask);
    OpUtil::RecyclePlayerBuff(player_, task_->op.buff);
	OpUtil::ExecuteAward(player_, task_, succ);

    RecursiveTaskErase(player_, task_);

	// 更新库任务
	OnUpdateStorageTask(player_, task_, succ);
	return succ;
}

static void ParentTaskFinish(Player* player, const TaskTempl* task, bool succ)
{
	TaskEntry* entry = player->GetActiveTask()->GetEntry(task->parent);
    if (entry != NULL)
    {
	    TaskWrapper wrapper(player, entry->templ);
	    wrapper.TaskFinish(succ);
    }
	NotifyUtil::SendFinish(player, task->parent, succ);
}

static bool IsAllSubTaskFinish(Player* player, const TaskTempl* task)
{
	ActiveTask* active_task = player->GetActiveTask();
	FinishTask* finish_task = player->GetFinishTask();
	const TaskTempl* sub_task = task->parent_templ->first_child_templ;
	while (sub_task != NULL)
	{		
		bool record = sub_task->record_result();
		// 如果记录完成结果则在完成任务里查找
		if (record && !finish_task->IsExist(sub_task->id))
		{
			return false;
		}
		else if (!record && active_task->IsExist(sub_task->id))
		{
			return false;
		}
		sub_task = sub_task->next_sibling_templ;
	}
	return true;
}

void TaskWrapper::RecursiveDeliverAward(bool succ)
{
	// 没有父任务的直接跳过
	if (task_->parent == 0)
	{
		return;
	}
	// 检查是否触发父任务完成
	if (succ && task_->succ_trigger_parent_succ())
	{
		return ParentTaskFinish(player_, task_, succ);
	}
	else if (!succ && task_->fail_trigger_parent_fail())
	{
		return ParentTaskFinish(player_, task_, succ);
	}
	int8_t deliver_mode = task_->parent_templ->subtask_deliver_mode;
	if (deliver_mode == DELIVER_ORDER && !succ)
	{
		return ParentTaskFinish(player_, task_, succ);
	}
	else if (deliver_mode == DELIVER_ORDER && task_->next_sibling_templ != NULL)
	{
		TaskWrapper wrapper(player_, task_->next_sibling_templ);
		return wrapper.DeliverTask();
	}
	else if (deliver_mode != DELIVER_ALL || IsAllSubTaskFinish(player_, task_))
	{
		return ParentTaskFinish(player_, task_, succ);
	}
}

static bool IsMonsterNeeded(const MonsterKilled& monster, int32_t monster_id)
{
	if (monster.package_id != 0)
	{
		return s_pTask->IsInMonsterPackage(monster.package_id, monster_id);
	}
	else
	{
		return monster.monster_id == monster_id;
	}
}

static bool HasEnoughItem(Player* player, const MonsterKilled& monster)
{
	if (monster.drop_item_id == 0)
	{
		return true;
	}

	return player->GetItemCount(monster.drop_item_id) >= monster.monster_count;
}

void TaskWrapper::KillMonster(int32_t mob_id, int32_t mob_count, ItemMap& items)
{
	__PRINTF("KillMonster taskid=%d mid=%d mcount=%d", task_->id, mob_id, mob_count);
	TaskEntry* entry = player_->GetActiveTask()->GetEntry(task_->id);
	int32_t* data = (int32_t*)(&(entry->data[0]));
	const MonsterKilledVec& monster_wanted = task_->succ_cond.monster.monster_list;	
	for (size_t i = 0; i < monster_wanted.size(); ++i)
	{
		const MonsterKilled& monster = monster_wanted[i];
		if (!IsMonsterNeeded(monster, mob_id) || CheckTaskFinish(true) == ERR_TASK_SUCC)
		{
			continue;
		}

		if (monster.drop_item_id == 0 && data[i] < monster.monster_count)
		{
			data[i] += mob_count;
			NotifyUtil::SendMonsterKilled(player_, task_->id, i, mob_count);
		}
		else if (!HasEnoughItem(player_, monster))
		{
			int32_t num = 0;
			for (int32_t i = 0; i < mob_count; ++i)
			{
				if (Util::Rand() <= monster.drop_item_prob)
				{
					num += monster.drop_item_count;
				}
			}
			player_->DeliverItem(monster.drop_item_id, num, 0);
			items[monster.drop_item_id] += num;
		}
	}
}

} // namespace task
