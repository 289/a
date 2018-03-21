#ifndef SHARED_BASE_TIMEDLOCKQUEUE_H_
#define SHARED_BASE_TIMEDLOCKQUEUE_H_

#include <deque>

#include "shared/base/assertx.h"
#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"

namespace shared {

template<typename T>
class TimedLockQueue : noncopyable
{
public:
	TimedLockQueue()
		: mutex_(),
		  queue_()
	{
	}

	void put(const T& x)
	{
		MutexLockTimedGuard lock(mutex_);
		queue_.push_back(x);
	}

	T take()
	{
		MutexLockTimedGuard lock(mutex_);
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	void takeAll(std::deque<T>& emptyQueue)
	{
		MutexLockTimedGuard lock(mutex_);
		assert(emptyQueue.empty());
		queue_.swap(emptyQueue);
	}

	size_t size() const
	{
		MutexLockTimedGuard lock(mutex_);
		return queue_.size();
	}

	bool empty() const
	{
		return queue_.empty();
	}

	void clearAll()
	{
		MutexLockTimedGuard lock(mutex_);
		std::deque<T> tmp_empty_deq;
		queue_.swap(tmp_empty_deq);
	}

private:
	mutable MutexLock	mutex_;
	std::deque<T>		queue_;
};

} // namespace shared

#endif // SHARED_BASE_TIMEDLOCKQUEUE_H_
