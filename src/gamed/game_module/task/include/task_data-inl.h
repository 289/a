#ifndef TASK_TASK_DATA_INL_H_
#define TASK_TASK_DATA_INL_H_

inline void TaskEntry::SetSuccess()
{
	state |= TASK_STATE_SUCC;
}

inline void TaskEntry::ClearSuccess()
{
	state &= ~TASK_STATE_SUCC;
}

inline bool TaskEntry::IsSuccess() const
{
	return (state & TASK_STATE_SUCC) != 0;
}

inline void TaskEntry::SetFinish()
{
	state |= TASK_STATE_FINISH;
}

inline void TaskEntry::ClearFinish()
{
	state &= ~TASK_STATE_FINISH;
}

inline bool TaskEntry::IsFinish() const
{
	return (state & TASK_STATE_FINISH) != 0;
}

inline void TaskEntry::SetGiveUp()
{
	state |= TASK_STATE_GIVEUP;
}

inline bool TaskEntry::IsGiveUp() const
{
	return (state & TASK_STATE_GIVEUP) != 0;
}

inline void TaskEntry::SetSkip()
{
	state |= TASK_STATE_SKIP;
}

inline bool TaskEntry::IsSkip() const
{
	return (state & TASK_STATE_SKIP) != 0;
}

inline void TaskEntry::SetShowTalk()
{
	state |= TASK_STATE_TALK;
}

inline void TaskEntry::ClearShowTalk()
{
	state &= ~TASK_STATE_TALK;
}

inline bool TaskEntry::IsShowTalk() const
{
	return (state & TASK_STATE_TALK) != 0;
}

inline bool TaskData::IsExist(TaskID id) const
{
	return task_list.find(id) != task_list.end();
}

inline int32_t ActiveTask::GetStorageNum() const
{
	return storage_num;
}

inline TaskEntry* TaskData::GetEntry(TaskID id)
{
	EntryMap::iterator it = task_list.find(id);
	return it == task_list.end() ? NULL : &(it->second);
}

inline bool DiaryEntry::succ() const
{
	return state != 0;
}

#endif // TASK_TASK_DATA_INL_H_
