#ifndef SHARED_BASE_SPINLOCKQUEUE_H_
#define SHARED_BASE_SPINLOCKQUEUE_H_

#include <deque>

#include "shared/base/assertx.h"
#include "shared/base/noncopyable.h"
#include "shared/base/spinlock.h"

namespace shared {

template<typename T>
class SpinLockQueue : noncopyable
{
public:
	SpinLockQueue()
		: spinlock_(),
		  queue_()
	{
	}

	void put(const T& x)
	{
		SpinLockGuard lock(spinlock_);
		queue_.push_back(x);
	}

	T take()
	{
		SpinLockGuard lock(spinlock_);
		assert(!queue_.empty());
		T front(queue_.front());
		queue_.pop_front();
		return front;
	}

	void takeAll(std::deque<T>& emptyQueue)
	{
		SpinLockGuard lock(spinlock_);
		assert(emptyQueue.empty());
		queue_.swap(emptyQueue);
	}

	size_t size() const
	{
		SpinLockGuard lock(spinlock_);
		return queue_.size();
	}

	bool empty() const
	{
		return queue_.empty();
	}

	void clearAll()
	{
		SpinLockGuard lock(spinlock_);
		std::deque<T> tmp_empty_deq;
		queue_.swap(tmp_empty_deq);
	}

private:
	mutable SpinLock	spinlock_;
	std::deque<T>		queue_;
};

} // namespace shared

#endif // SHARED_BASE_SPINLOCKQUEUE_H_
