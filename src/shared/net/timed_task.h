#ifndef SHARED_NET_TIMED_TASK_H_
#define SHARED_NET_TIMED_TASK_H_

#include <vector>
#include <queue>
#include <functional>
#include <map>

#include "shared/base/types.h"
#include "shared/base/noncopyable.h"
#include "shared/base/callback_bind.h"
#include "shared/base/atomic.h"
#include "shared/base/spinlock.h"
#include "shared/base/timestamp.h"

#include "shared/net/eventloop.h"

namespace shared {
namespace net {

typedef uint64_t TimedTaskId;

// TimedTask对象new出来后只能由TimedTaskManager来delete
// 外部只能调用TimedTaskManager::Cancel(task_id)来取消回调
class TimedTask : noncopyable
{
	friend class TimedTaskManager;
public:
	typedef bind::Callback<void (const TimedTask*)> TimedTaskCallback;

	enum RepeatTypes
	{
		RUN_ONCE = 0,
		RUN_REPEATED,
	};

	// interval 的单位是毫秒 millisecond
	TimedTask(int32_t interval, 
			  RepeatTypes repeat_type, 
			  const TimedTaskCallback& callback = bind::NullCallback());

	void    OnTimer();
	void    Cancel();
	void    UpdateTimeout();

	bool    is_repeated() const { return repeated_; }
	bool    is_canceled() const { return is_canceled_; }

	// return Msecs
	inline double get_next_timeout(Timestamp& now);

	void          set_task_id(TimedTaskId task_id) { task_id_ = task_id; }
	TimedTaskId   get_task_id() const { return task_id_; }
	Timestamp     get_timeout_ts() { return timeout_ts_; }


protected:
	//如果子类实现了Release，则需要调用者自己来delete相应的task对象
	virtual void Release() { delete this; }
	virtual ~TimedTask();


private:
	int32_t            interval_;
	RepeatTypes        repeated_;
	TimedTaskCallback  callback_;

	bool               is_canceled_;
	TimedTaskId        task_id_;
	Timestamp          timeout_ts_;
};

class TimedSecTask : public TimedTask
{
public:
	// interval 的单位是秒 second
	TimedSecTask(int32_t interval, 
			     RepeatTypes repeat_type, 
			     const TimedTaskCallback& callback = bind::NullCallback())
		: TimedTask(shared::sec_to_msec(interval), repeat_type, callback)
	{ }
};

class TimedTaskManager
{
public:
	// 目前只能在loop线程中使用
	TimedTaskManager(EventLoop* loop);
	~TimedTaskManager();

	// 注意：AddTimedTask()是把TimedTask及回调函数注册到
	// EventLoop所在的线程，由loop所在线程来调用timer的回调，
	// 所以回调函数要主要加锁，并且要注意回调的成员函数的生命周期
	// 目前只能在loop线程中使用
	TimedTaskId  AddTimedTask(TimedTask* ptask);
	void         Cancel(TimedTaskId task_id);


private:
	void        TimeOutHandler(void);
	// 调用前需要加锁
	inline void DeleteTask(TimedTask* pcur_task);
	static void RegisterTimerEvent(void*);

	struct Comparator
	{
		bool operator()(const TimedTask* lhs, const TimedTask* rhs)
		{
			return lhs->timeout_ts_ > rhs->timeout_ts_;
		}
	};

private:
	// min heap by priority-queue
	typedef std::priority_queue<TimedTask*, std::vector<TimedTask*>, Comparator> Min_PQ;
	Min_PQ       task_min_pq_;

	typedef std::map<TimedTaskId, TimedTask*> MapTask;
	MapTask      task_map_;

	EventLoop*   loop_;
	SpinLock     spin_lock_;
	TimedTaskId  timed_task_id_seq_; // needed lock
	TimerId      cur_timer_id_;

	std::vector<TimedTask*> expired_tasks_; 
};

///
/// inline function
///
inline void TimedTaskManager::DeleteTask(TimedTask* pcur_task)
{
	SpinLockGuard lock(spin_lock_);
	MapTask::iterator it = task_map_.find(pcur_task->get_task_id());
	task_map_.erase(it);
	pcur_task->Release();
}

inline double TimedTask::get_next_timeout(Timestamp& now)
{
	double next_ms = TimeMsecsDifference(timeout_ts_, now);
	return (next_ms > 0) ? next_ms : 0.0f; 
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_TIMED_TASK_H_
