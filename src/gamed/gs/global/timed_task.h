#ifndef GAMED_GS_GLOBAL_TIMED_TASK_H_
#define GAMED_GS_GLOBAL_TIMED_TASK_H_

#include <set>

#include "shared/base/singleton.h"
#include "shared/base/copyable.h"
#include "shared/base/mutex.h"
#include "shared/base/atomic.h"

#include "game_types.h"
#include "runnable_pool.h"


namespace gamed {

typedef int64_t TimedTaskId;

class TimedTaskManager;
class TaskQueueRunable;
class TimedTaskInfo : public shared::copyable
{
	friend class TimedTaskManager;
	friend class TaskQueueRunable;
public:
    typedef int32_t ParamType;

	enum OpMode
	{
		OP_NULL = 0,
		OP_REPEAT,
		OP_END,
	};

	XID     xid;
	int32_t interval;
	int32_t times;
	int32_t start_time;
	ParamType param;

	TimedTaskInfo();
	~TimedTaskInfo();

	void   Release();
	inline bool HasBeenTimed() const;


protected:
	inline TimedTaskId task_id() const;
	inline void set_task_id(TimedTaskId id);
	inline OpMode op_mode() const;
	inline void set_op_mode(OpMode mode);

private:
	TimedTaskId task_id_;
	OpMode      op_mode_;  //operating mode
};

///
/// inline func
///
inline TimedTaskId TimedTaskInfo::task_id() const
{
	return task_id_;
}

inline void TimedTaskInfo::set_task_id(TimedTaskId id)
{
	task_id_ = id;
}

inline TimedTaskInfo::OpMode TimedTaskInfo::op_mode() const
{
	return op_mode_;
}

inline void TimedTaskInfo::set_op_mode(OpMode mode)
{
	op_mode_ = mode;
}

inline bool TimedTaskInfo::HasBeenTimed() const
{
	if (task_id_ > 0) return true;

	return false;
}


///
/// TimedTaskManager
///
class TimedTaskManager : public shared::Singleton<TimedTaskManager>
{
	friend class shared::Singleton<TimedTaskManager>;
	friend class TaskQueueRunable;
public:
	static inline TimedTaskManager* GetInstance() {
		return &(get_mutable_instance());
	}

	// ---- thread safe ----
	bool    AddTimedTask(TimedTaskInfo& info);
	void    Cancel(const TimedTaskInfo& info);

	// **** thread unsafe ****
	void    Tick(TickIndex cur_tick);


protected:
	TimedTaskManager();
	~TimedTaskManager();


private:
	struct Entry
	{
		TickIndex     tick_index;
		TimedTaskInfo task_info;

		Entry(const TickIndex& index, const TimedTaskInfo& info)
			: tick_index(index),
			  task_info(info)
		{ }

		bool operator<(const Entry& rhs) const
		{
			return tick_index <= rhs.tick_index;
		}
	};

	// **** thread unsafe ****
	void GetExpired(TickIndex tickindex, std::vector<Entry>& timeup_vec);
	inline bool HasExpiredTasks(TickIndex tickindex);
	inline void InsertToTaskSet(TickIndex when, const TimedTaskInfo& info);
	inline void InsertToCanceledSet(TimedTaskId id);
	

private:
	typedef std::set<TimedTaskId> TimedTaskIdSet;
	typedef TimedTaskIdSet CanceledTaskSet;
	typedef TimedTaskIdSet ActiveTaskSet;
	typedef std::vector<Entry> EntryVec;
	typedef std::multiset<Entry> TimedTaskSet;
	TimedTaskSet     tasks_set_;
	EntryVec         expired_tasks_;
	CanceledTaskSet  canceled_tasks_;
	ActiveTaskSet    active_tasks_;

	shared::AtomicInt64   cur_highest_id_;

	// 需要同时锁mutex_tasks_set_和mutex_canceled_tasks_时，锁的顺序是：
	// 先锁mutex_tasks_set_再锁mutex_canceled_tasks_
	shared::MutexLock     mutex_tasks_set_;
	shared::MutexLock     mutex_canceled_tasks_;
};

///
/// inline func
///
inline void TimedTaskManager::InsertToTaskSet(TickIndex when, const TimedTaskInfo& info)
{
	TimedTaskSet::iterator result
		= tasks_set_.insert(Entry(when, info));
	assert(result != tasks_set_.end()); (void)result;

	std::pair<ActiveTaskSet::iterator, bool> idset_res
		= active_tasks_.insert(info.task_id());
	assert(idset_res.second); (void)idset_res;
}

inline void TimedTaskManager::InsertToCanceledSet(TimedTaskId id)
{
	std::pair<CanceledTaskSet::iterator, bool> result 
		= canceled_tasks_.insert(id);
	assert(result.second); (void)result;
}

inline bool TimedTaskManager::HasExpiredTasks(TickIndex tickindex)
{
	if (tasks_set_.empty() || tickindex < tasks_set_.begin()->tick_index)
		return false;

	return true;
}

///
/// TimedTaskQueue
///
class TaskQueueRunable : public RunnablePool::Task
{
	typedef TimedTaskManager::Entry Entry;
public:
	inline void  Copy(std::vector<Entry>& tasks_vec);
	virtual void Run();

private:
	void SendRepeatMsg(const XID& obj, TimedTaskId task_id, int32_t param, int32_t times);
	void SendEndMsg(const XID& obj, TimedTaskId task_id, int32_t param);

private:
	std::vector<Entry> queue_;
};

///
/// inline func
///
inline void TaskQueueRunable::Copy(std::vector<Entry>& tasks_vec)
{
	std::copy(tasks_vec.begin(), tasks_vec.end(), back_inserter(queue_));
}


#define s_pTimedTask gamed::TimedTaskManager::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_TIMED_TASK_H_
