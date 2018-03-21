#ifndef SHARED_BASE_SPINLOCK_H_
#define SHARED_BASE_SPINLOCK_H_

#include <pthread.h>
#include <errno.h>

#include "shared/base/assertx.h"
#include "shared/base/noncopyable.h"
#include "shared/base/types.h"
#include "shared/base/base_define.h"
#include "shared/base/current_thread.h"

namespace shared {

class SpinLock : noncopyable
{
	static const int kSpinLockTimeOutSecond = 30;

public:
	SpinLock() : holder_(0)
	{
		int ret = pthread_spin_init(&spinlock_, 0); 
		assert(ret == 0);
	}

	~SpinLock()
	{
		assert(holder_ == 0);
		int ret = pthread_spin_destroy(&spinlock_);
		assert(ret == 0);
	}

	bool IsLockedByThisThread()
	{
		return holder_ == CurrentThread::tid();
	}

	void AssertLocked()
	{
		assert(IsLockedByThisThread());
	}

	void time_lock()
	{
		int counter = 0;
		while (true)
		{
			int err = pthread_spin_trylock(&spinlock_);
			if (0 == err) 
			{
				break;
			}
			else if (EBUSY == err)
			{
				assert(counter < (kSpinLockTimeOutSecond * TICK_PER_SEC));
				usleep((int)(sec_to_microsec(1) / TICK_PER_SEC));
				++counter;
				continue;
			}
			assert(0 == err);
		}
		holder_ = CurrentThread::tid();
	}

	void lock()
	{
		pthread_spin_lock(&spinlock_);
		holder_ = CurrentThread::tid();
	}

	void unlock()
	{
		holder_ = 0;
		pthread_spin_unlock(&spinlock_);
	}

	pthread_spinlock_t* get_pthread_spinlock() /* non-const */
	{
		return &spinlock_;
	}

private:
	pthread_spinlock_t spinlock_;
	pid_t holder_;
};

class SpinLockGuard : noncopyable
{
public:
	explicit SpinLockGuard(SpinLock& spinlock)
		: spinlock_(spinlock)
	{
		spinlock_.lock();
	}

	~SpinLockGuard()
	{
		spinlock_.unlock();
	}

private:
	SpinLock& spinlock_;
};

class SpinLockTimeGuard : noncopyable
{
public:
	explicit SpinLockTimeGuard(SpinLock& spinlock)
		: spinlock_(spinlock)
	{
		spinlock_.time_lock();
	}

	~SpinLockTimeGuard()
	{
		spinlock_.unlock();
	}

private:
	SpinLock& spinlock_;
};


} // namespace shared

#endif // SHARED_BASE_SPINLOCK_H_
