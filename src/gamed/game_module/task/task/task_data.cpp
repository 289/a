#include "task_data.h"
#include "task_manager.h"
#include "task_succ.h"
#include "task_templ.h"
#include "entry_util.h"
#include "storage_util.h"

namespace task
{

TaskEntry::TaskEntry()
	: id(0), state(0), deliver_time(0), finish_time(0), script_end(false), templ(NULL)
{
}

TaskEntry::TaskEntry(const TaskEntry& rhs) 
	: id(rhs.id), state(rhs.state), deliver_time(rhs.deliver_time),
	finish_time(rhs.finish_time), data(rhs.data), script_end(rhs.script_end), templ(rhs.templ)
{
}

TaskEntry& TaskEntry::operator=(const TaskEntry& rhs)
{
	id = rhs.id;
	state = rhs.state;
	deliver_time = rhs.deliver_time;
	finish_time = rhs.finish_time;
	data = rhs.data;
    script_end = rhs.script_end;
	templ = rhs.templ;
	return *this;
}

void TaskEntry::FinishGoal()
{
    int8_t goal = templ->first_child != 0 ? -1 : templ->succ_cond.goal;
    data[0] = goal;
}

bool TaskEntry::IsFinishGoal() const
{
    int8_t goal = templ->first_child != 0 ? -1 : templ->succ_cond.goal;
    // 这里要先转成int8_t，因为char并没有规定是有符号的还是无符号的
    // 所以直接比较data[0]与goal在android平台上会出现问题
    int8_t value = data[0];
    return value == goal;
}

void TaskEntry::SetScriptEnd()
{
    if (templ->op.exclusive_op != EXOP_SCRIPT && templ->op.exclusive_op != EXOP_GUIDE)
    {
        return;
    }
	//state |= TASK_STATE_SCRIPT;
    script_end = true;
}

bool TaskEntry::IsScriptEnd() const
{
    if (templ->op.exclusive_op != EXOP_SCRIPT && templ->op.exclusive_op != EXOP_GUIDE)
    {
        return true;
    }
	//return (state & TASK_STATE_SCRIPT) != 0;
    return script_end;
}

TaskData::TaskData(uint16_t type) 
	: shared::net::BasePacket(type)
{
}

TaskData::TaskData(const TaskData& rhs)
	: shared::net::BasePacket(rhs.typeid_)
{
}

void TaskData::AddEntry(const TaskEntry& entry)
{
	task_list[entry.id] = entry;
}

void TaskData::Erase(TaskID id)
{
	task_list.erase(id);
}

void TaskData::LoadComplete(int32_t now)
{
	EntryMap::iterator it = task_list.begin();
	for (; it != task_list.end();)
	{
		TaskEntry& entry = it->second;
		entry.templ = s_pTask->GetTask(it->first);
        if (entry.templ == NULL)
        {
            task_list.erase(it++);
            continue;
        }
        ++it;
	}
}

void ActiveTask::AddEntry(const TaskEntry& entry)
{
    // 只有顶层库任务占用5个库任务的限额
	if (entry.templ->sid != 0 && entry.templ->parent == 0)
	{
		++storage_num;
	}
	task_list[entry.id] = entry;
}

void ActiveTask::Erase(TaskID id)
{
	EntryMap::iterator it = task_list.find(id);
	if (it != task_list.end())
	{
		if (it->second.templ->sid != 0)
		{
			--storage_num;
		}
		task_list.erase(it);
	}
}

void ActiveTask::LoadComplete(int32_t now)
{
	storage_num = 0;
	EntryMap::iterator it = task_list.begin();
	for (; it != task_list.end();)
	{
		TaskEntry& entry = it->second;
		entry.templ = s_pTask->GetTask(it->first);
		if (entry.templ == NULL)
		{
			task_list.erase(it++);
			continue;
		}
		if (entry.templ->sid != 0)
		{
			++storage_num;
		}

		EntryUtil::Revise(now, entry.templ, entry);
		const TaskSucc& succ_cond = entry.templ->succ_cond;
		if (succ_cond.goal == GOAL_WAIT_TIME && !succ_cond.time.record_offline)
		{
			int32_t* data = (int32_t*)(&(entry.data[0]));
			data[1] = now;
		}
		++it;
	}
}

void ActiveTask::PreSave(int32_t now)
{
	EntryMap::iterator it = task_list.begin();
	for (; it != task_list.end(); ++it)
	{
		TaskEntry& entry = it->second;
		const TaskSucc& succ_cond = entry.templ->succ_cond;
		if (succ_cond.goal == GOAL_WAIT_TIME && !succ_cond.time.record_offline)
		{
			int32_t* data = (int32_t*)(&(entry.data[0]));
			data[0] += now - data[1];
		}
	}
}

void FinishTimeTask::AddEntry(const TaskEntry& entry)
{
    TaskEntry* tmp = NULL;
    EntryMap::iterator it = task_list.find(entry.id);
    if (it == task_list.end())
    {
        tmp = &(task_list[entry.id]);
        *tmp = entry;
        //tmp->deliver_time = entry.finish_time;
    }
    else
    {
        tmp = &(it->second);
        // 只更新发放时间，用于限制接取间隔
        // 不更新完成时间，第一次完成时间用于作为清除的判断标准
        //tmp->deliver_time = entry.deliver_time;
    }
    tmp->deliver_time = entry.finish_time;
    EntryUtil::IncFinishTime(*tmp);
}

DiaryEntry::DiaryEntry()
	: id(0), state(0), time(0), templ(NULL)
{
}

DiaryEntry::DiaryEntry(const TaskTempl* task, int8_t task_state, int32_t finish_time)
	: id(task->id), state(task_state), time(finish_time), templ(task)
{
}

const std::string& DiaryEntry::diary() const
{
	return succ() ? templ->succ_diary : templ->fail_diary;
}

void TaskDiary::AddDiary(const TaskTempl* task, int8_t state, int32_t time)
{
	diary_list.push_back(DiaryEntry(task, state, time));
}

void TaskDiary::LoadComplete(int32_t now)
{
	DiaryVec::iterator it = diary_list.begin();
	for (; it != diary_list.end(); ++it)
	{
		it->templ = s_pTask->GetTask(it->id);
	}
}

StorageEntry::StorageEntry()
	: time(0), templ(NULL)
{
}

int32_t StorageEntry::GetUpdateInterval(int32_t now) const
{
    return StorageUtil::GetUpdateInterval(now, *this);
}

void TaskStorage::LoadComplete(int32_t now)
{
	StorageMap::iterator it = storage_list.begin();
	for (; it != storage_list.end(); ++it)
	{
		it->second.templ = s_pTask->GetStorage(it->first);
	}
}

StorageEntry& TaskStorage::GetEntry(StorageID sid)
{
	StorageMap::iterator it = storage_list.find(sid);
	if (it != storage_list.end())
	{
		return it->second;
	}
	StorageEntry& entry = storage_list[sid];
	entry.templ = s_pTask->GetStorage(sid);
	return entry;
}

} // namespace task
