#ifndef GAMED_GS_GLOBAL_RUNNABLE_POOL_H_
#define GAMED_GS_GLOBAL_RUNNABLE_POOL_H_

#include "shared/base/singleton.h"
#include "shared/base/threadpool.h"


namespace gamed {

class RunnablePool : public shared::Singleton<RunnablePool>
{
	friend class shared::Singleton<RunnablePool>;

public:
	typedef shared::ThreadPool::Task Task;

	static void CreatePool(int numthreads);
	static void StopPool();
	static void AddTask(Task* ptask);
	static void AddTickTask(Task* ptask);
	static void AddSpecTask(Task* ptask, int32_t thread_index);

	// internal use
	static uint32_t RR_CalcKey(uint32_t thread_nums, uint64_t key);

	
protected:
	RunnablePool();
	~RunnablePool();

	static inline RunnablePool* GetInstance() {
		return &(get_mutable_instance());
	}
	
	void    Start(int numthreads);
	void    Stop();
	void    AddRunnableTask(Task* ptask);
	void    AddSpecRunTask(int t_num, Task* ptask);


private:
	shared::ThreadPool thread_pool_;
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_RUNNABLE_POOL_H_
