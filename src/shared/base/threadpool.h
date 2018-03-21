#ifndef SHARED_BASE_THREADPOOL_H_
#define SHARED_BASE_THREADPOOL_H_

#include <deque>
#include <vector>
#include <map>

#include "shared/base/noncopyable.h"
#include "shared/base/thread.h"
#include "shared/base/mutex.h"
#include "shared/base/condition.h"
#include "shared/base/callback_bind.h"

namespace shared {

class WorkerThread;
class ThreadPool : noncopyable
{
public:
	class Task : noncopyable
	{
	public:
		Task() { }
		virtual ~Task() { }
		virtual void Run() = 0;

		//如果子类实现了Release，则需要调用者自己来delete相应的task对象
		virtual void Release() { delete this; }
	};

public:
	typedef bind::Callback<uint32_t (uint32_t, uint64_t)> CalckeyCallback;
	//typedef uint32_t AddTaskId;

	explicit	ThreadPool(const std::string& name = std::string(), 
			               const CalckeyCallback& cb = BIND_FREE_CB(&ThreadPool::RR_CalcKey));
	~ThreadPool();

	void	Start(int numThreads);
	void	Stop();

	// 默认平均分配到各个工作线程
	void    RunTask(Task* taskObj);
	// 根据key值(比如roleid)指定分配到某个工作线程
	void	RunTaskSpec(uint64_t key, Task* taskObj);
	// 指定到特定的线程里执行
	void    RunInSpecThread(int thread_id, Task* taskObj);

	static uint32_t RR_CalcKey(uint32_t thread_nums, uint64_t key);

private:
	MutexLock	mutex_;
    std::string	name_;
	bool		running_;
	CalckeyCallback  calckey_cb_;
	std::vector<WorkerThread*>      threads_;
};

class WorkerThread : noncopyable
{
	typedef ThreadPool::Task* TaskPtr;
public:
	WorkerThread(const std::string& namestr);
	~WorkerThread();

	void    Start();
	void    AddTask(TaskPtr taskObj);
	void    Stop();

private:
	static void* WorkerFunc(void* pdata);

	void    Run();
	TaskPtr take();

	inline void set_running(bool is_running) { 
		running_ = is_running; 
	}

private:
	MutexLock      mutex_;
	Condition	   cond_;
	bool		   running_;
	Thread*        thread_;
	std::deque<TaskPtr>   queue_;
};

} // namespace shared

#endif // SHARED_BASE_THREADPOOL_H_
