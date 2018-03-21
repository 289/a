#ifndef SHARED_BASE_RWLOCKQUEUE_H_
#define SHARED_BASE_RWLOCKQUEUE_H_

#include <deque>

#include "shared/base/assertx.h"
#include "shared/base/noncopyable.h"
#include "shared/base/rwlock.h"


namespace shared {

template<typename T>
class RWLockQueue : noncopyable
{
public:
	RWLockQueue()
		: rwlock_(),
		  queue_()
	{
	}

	void put(const T& x)
	{
		RWLockTimedWriteGuard wlock(rwlock_);
		queue_.push_back(x);
	}

	void putAll(std::vector<T>& dataQueue)
	{
		RWLockTimedWriteGuard wlock(rwlock_);
		queue_.insert(queue_.end(), dataQueue.begin(), dataQueue.end());
	}

	T take()
	{
		RWLockTimedReadGuard rlock(rwlock_);
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	void takeAll(std::deque<T>& emptyQueue)
	{
		RWLockTimedReadGuard rlock(rwlock_);
		assert(emptyQueue.empty());
		queue_.swap(emptyQueue);
	}

	size_t size() const
	{
		RWLockTimedReadGuard rlock(rwlock_);
		return queue_.size();
	}

	bool empty() const
	{
		return queue_.empty();
	}

	void clearAll()
	{
		RWLockTimedWriteGuard wlock(rwlock_);
		std::deque<T> tmp_empty_deq;
		queue_.swap(tmp_empty_deq);
	}


private:
	mutable RWLock    rwlock_;
	std::deque<T>	  queue_;
};

} // namespace shared

#endif // SHARED_BASE_RWLOCKQUEUE_H_
