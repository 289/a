#include "timed_task.h"

#include "message.h"
#include "gmatrix.h"
#include "timer.h"


namespace gamed {

// 不能超过32位，因为MSG.param的高32位被times占了
SHARED_STATIC_ASSERT(sizeof(TimedTaskInfo::ParamType) == sizeof(int32_t));

///
/// TimedTaskInfo
///
TimedTaskInfo::TimedTaskInfo()
	: interval(-1),
	  times(0),
	  start_time(0),
	  task_id_(0),
	  op_mode_(OP_NULL)
{
}

TimedTaskInfo::~TimedTaskInfo()
{
	Release();
}

void TimedTaskInfo::Release()
{
	xid.id     = -1;
	xid.type   = XID::TYPE_INVALID;
	interval   = -1;
	times      = 0;
	start_time = 0;
	task_id_   = 0;
	op_mode_   = OP_NULL;
}

///
/// TaskQueueRunable
///
void TaskQueueRunable::Run()
{
	for (size_t i = 0; i < queue_.size(); ++i)
	{
		TimedTaskInfo& info = queue_[i].task_info;
		if (TimedTaskInfo::OP_REPEAT == info.op_mode())
		{
			SendRepeatMsg(info.xid, info.task_id(), info.param, info.times - 1);
		}
		else if (TimedTaskInfo::OP_END == info.op_mode())
		{
			SendEndMsg(info.xid, info.task_id(), info.param);
		}
	}

	queue_.clear();
}

void TaskQueueRunable::SendRepeatMsg(const XID& obj, TimedTaskId task_id, int32_t param, int32_t times)
{
	MSG msg;
	XID src;
	src.id   = task_id;
	src.type = XID::TYPE_TIMED_TASK;
    int64_t send_param = makeInt64(times, param);
	BuildMessage(msg, GS_MSG_OBJ_SESSION_REPEAT, obj, src, send_param);
	Gmatrix::SendObjectMsg(msg);
}

void TaskQueueRunable::SendEndMsg(const XID& obj, TimedTaskId task_id, int32_t param)
{
	MSG msg;
	XID src;
	src.id   = task_id;
	src.type = XID::TYPE_TIMED_TASK;
	BuildMessage(msg, GS_MSG_OBJ_SESSION_END, obj, src, param);
	Gmatrix::SendObjectMsg(msg);
}

///
/// TimedTaskManager
///
TimedTaskManager::TimedTaskManager()
{
}

TimedTaskManager::~TimedTaskManager()
{
}

bool TimedTaskManager::AddTimedTask(TimedTaskInfo& info)
{
	if (info.interval < 0 || info.times <= 0 || info.start_time < 0)
	{
		return false;
	}

	TickIndex when = g_timer->get_tick() + info.start_time + info.interval; 
	info.set_task_id(cur_highest_id_.increment_and_get());

	MutexLockGuard lock(mutex_tasks_set_);
	InsertToTaskSet(when, info);

	return true;
}

void TimedTaskManager::Cancel(const TimedTaskInfo& info)
{
	MutexLockGuard lock(mutex_tasks_set_);
	ActiveTaskSet::iterator it = active_tasks_.find(info.task_id());
	if (it == active_tasks_.end()) 
		return;

	MutexLockGuard lock_cancel(mutex_canceled_tasks_);
	InsertToCanceledSet(info.task_id());
}

// needed lock outside
void TimedTaskManager::GetExpired(TickIndex tickindex, std::vector<Entry>& timeup_vec)
{
	Entry sentry(tickindex, TimedTaskInfo());
	TimedTaskSet::iterator end = tasks_set_.lower_bound(sentry);
	assert(end == tasks_set_.end() || tickindex < end->tick_index);
	std::copy(tasks_set_.begin(), end, back_inserter(timeup_vec));
	tasks_set_.erase(tasks_set_.begin(), end);

	for (size_t i = 0; i < timeup_vec.size(); ++i)
	{
		size_t n = active_tasks_.erase(timeup_vec[i].task_info.task_id());
		assert(n == 1); (void)n;
	}

	assert(active_tasks_.size() == tasks_set_.size());
}

void TimedTaskManager::Tick(TickIndex cur_tick)
{
	{
		MutexLockGuard lock(mutex_tasks_set_);
		if (!HasExpiredTasks(cur_tick)) return;

		expired_tasks_.clear();
		GetExpired(cur_tick, expired_tasks_);
	}

	EntryVec::iterator it_vec = expired_tasks_.begin();
	for (; it_vec != expired_tasks_.end(); )
	{
		TimedTaskInfo& task_info = it_vec->task_info;
		if (!canceled_tasks_.empty())
		{
			MutexLockGuard lock(mutex_canceled_tasks_);
			CanceledTaskSet::iterator it_cancel = canceled_tasks_.find(task_info.task_id());
			if (it_cancel != canceled_tasks_.end())
			{
				canceled_tasks_.erase(it_cancel);
				it_vec = expired_tasks_.erase(it_vec);
				continue;
			}
		}

		if (--task_info.times > 0)
		{
			task_info.set_op_mode(TimedTaskInfo::OP_REPEAT);
			TickIndex when = cur_tick + task_info.interval; 

			MutexLockGuard lock(mutex_tasks_set_);
			InsertToTaskSet(when, task_info);
		}
		else
		{
			task_info.set_op_mode(TimedTaskInfo::OP_END);
		}

		++it_vec;
	}

	if (!expired_tasks_.empty())
	{
		TaskQueueRunable* runable_queue = new TaskQueueRunable();
		runable_queue->Copy(expired_tasks_);
		RunnablePool::AddTask(runable_queue);
	}
}

} // namespace gamed
