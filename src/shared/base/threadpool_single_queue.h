#ifndef SHARED_BASE_THREADPOOL_H_
#define SHARED_BASE_THREADPOOL_H_

#include <deque>
#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/base/thread.h"
#include "shared/base/mutex.h"
#include "shared/base/condition.h"

namespace shared {
namespace single_task_queue {

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
	typedef void (*TaskFunc)();

	explicit	ThreadPool(const std::string& name = std::string());
	~ThreadPool();

	void	Start(int numThreads);
	void	Stop();

	void	Run(Task* taskObj);

protected:
	void	RunInThread();

private:
	Task*	take();
	static void	JoinThread(Thread*);
	static void* WorkerFunc(void* pdata);

	MutexLock	mutex_;
	Condition	cond_;
    std::string	name_;
	bool		running_;
	std::vector<Thread*>  threads_;
	std::deque<Task*>     queue_;
};

} // namespace single_task_queue
} // namespace shared

#endif // SHARED_BASE_THREADPOOL_H_
