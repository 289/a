#include "runnable_pool.h"


#define s_pRunnablePool gamed::RunnablePool::GetInstance()

namespace gamed {

using namespace shared;

uint32_t RunnablePool::RR_CalcKey(uint32_t thread_nums, uint64_t key)
{
	static uint64_t roundcount = 0;
	// 0号线程专做Heartbeat不在此分配
	uint32_t tmpround = ((++roundcount) % (thread_nums - 1)) + 1;
	return tmpround;
}

void RunnablePool::CreatePool(int numthreads)
{
	ASSERT(numthreads >= 4);
	s_pRunnablePool->Start(numthreads);
}

void RunnablePool::StopPool()
{
	s_pRunnablePool->Stop();
}

void RunnablePool::AddTask(Task* ptask)
{
	s_pRunnablePool->AddRunnableTask(ptask);
}

void RunnablePool::AddTickTask(Task* ptask)
{
	// 在0号线程做tick
	s_pRunnablePool->AddSpecRunTask(0, ptask);
}

void RunnablePool::AddSpecTask(Task* ptask, int32_t thread_index)
{
	s_pRunnablePool->AddSpecRunTask(thread_index, ptask);
}

RunnablePool::RunnablePool()
	: thread_pool_("gs runnable threadpool", BIND_FREE_CB(&RunnablePool::RR_CalcKey))
{
}

RunnablePool::~RunnablePool()
{
}

void RunnablePool::Start(int numthreads)
{
	thread_pool_.Start(numthreads);
}

void RunnablePool::Stop()
{
	thread_pool_.Stop();
}

void RunnablePool::AddRunnableTask(Task* ptask)
{
	thread_pool_.RunTask(ptask);
}

void RunnablePool::AddSpecRunTask(int t_num, Task* ptask)
{
	thread_pool_.RunInSpecThread(t_num, ptask);
}

} // namespace gamed
