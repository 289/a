#ifndef SHARED_BASE_BLOCKINGQUEUE_H_
#define SHARED_BASE_BLOCKINGQUEUE_H_

#include <deque>
#include <vector>

#include "shared/base/assertx.h"
#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/condition.h"

namespace shared {

template<typename T>
class BlockingQueue : noncopyable
{
public:
	BlockingQueue()
		: mutex_(),
		  notEmpty_(mutex_),
		  queue_()
	{
	}

	void put(const T& x)
	{
		MutexLockGuard lock(mutex_);
		queue_.push_back(x);
		notEmpty_.Notify(); // TODO: move outside of lock
	}

	void putAll(std::vector<T>& dataQueue)
	{
		MutexLockGuard lock(mutex_);
		queue_.insert(queue_.end(), dataQueue.begin(), dataQueue.end());
		notEmpty_.Notify();
	}

	T take()
	{
		MutexLockGuard lock(mutex_);
		// always use a while-loop, due to spurious wakeup
		while (queue_.empty())
		{
			notEmpty_.Wait();
		}
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	void takeAll(std::deque<T>& emptyQueue)
	{
		MutexLockGuard lock(mutex_);
		// always use a while-loop, due to spurious wakeup
		while (queue_.empty())
		{
			notEmpty_.Wait();
		}
		assert(!queue_.empty() && emptyQueue.empty());
		queue_.swap(emptyQueue);
	}

	size_t size() const
	{
		MutexLockGuard lock(mutex_);
		return queue_.size();
	}

	bool empty() const
	{
		return queue_.empty();
	}

	void clearAll()
	{
		MutexLockGuard lock(mutex_);
		std::deque<T> tmp_empty_deq;
		queue_.swap(tmp_empty_deq);
	}


private:
	mutable MutexLock	mutex_;
	Condition			notEmpty_;
	std::deque<T>		queue_;
};

} // namespace shared

#endif // SHARED_BASE_BLOCKINGQUEUE_H_
