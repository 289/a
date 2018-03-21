#ifndef SHARED_BASE_RWLOCK_H_
#define SHARED_BASE_RWLOCK_H_

// in order to use pthread_rwlock_timedwrlock
//#define __USE_XOPEN2K

/*
 *  * * compile needed: -lpthread -lrt
 *   * */

//Names and Order of Includes
//1.preferred location(dir2/foo2.h)
//2.C system files
//3.C++ system files
//4.Other libraries'.h files
//5.Your project's .h files

#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "shared/base/assertx.h"
#include "shared/base/current_thread.h"
#include "shared/base/noncopyable.h"

namespace shared {

class RWLock : noncopyable
{
	static const int kRWLockTimeOutSecond = 30;
public:
	RWLock() : holder_(0)
	{
		int ret = pthread_rwlock_init(&rwlock_, NULL);
		assert(ret == 0); (void)ret;
	}

	~RWLock()
	{
		assert(holder_ == 0);
		int ret = -1;
		while ( (ret = pthread_rwlock_destroy(&rwlock_)) == EBUSY) 
		{
			wrlock();
			unlock();
		}
		assert(ret == 0);
	}

	// TODO: divide into rdholder and wrholder
	bool IsLockedByThisThread()
	{
		return holder_ == CurrentThread::tid();
	}

	void AssertLocked()
	{
		assert(IsLockedByThisThread());
	}

	void rdlock()
	{
		pthread_rwlock_rdlock(&rwlock_);
		holder_ = CurrentThread::tid();
	}

	void wrlock()
	{
		pthread_rwlock_wrlock(&rwlock_);
		holder_ = CurrentThread::tid();
	}

	void unlock()
	{
		holder_ = 0;
		pthread_rwlock_unlock(&rwlock_);
	}

	void timed_rdlock()
	{
		struct timespec abs_time;
		clock_gettime(CLOCK_REALTIME, &abs_time);
		abs_time.tv_sec += kRWLockTimeOutSecond;
		int ret = pthread_rwlock_timedrdlock(&rwlock_, &abs_time);
		assert(ret == 0); (void)ret; // ETIMEDOUT or other error happen

		holder_ = CurrentThread::tid();
	}

	void timed_wrlock()
	{
		struct timespec abs_time;
		clock_gettime(CLOCK_REALTIME, &abs_time);
		abs_time.tv_sec += kRWLockTimeOutSecond;
		int ret = pthread_rwlock_timedwrlock(&rwlock_, &abs_time);
		assert(ret == 0); (void)ret; // ETIMEDOUT or other error happen

		holder_ = CurrentThread::tid();
	}

	pthread_rwlock_t* get_pthread_rwlcok() /* non-const */
	{
		return &rwlock_;
	}

private:
	pthread_rwlock_t rwlock_;
	pid_t holder_;
};

class RWLockReadGuard : noncopyable
{
public:
	explicit RWLockReadGuard(RWLock& rwlock)
		: rwlock_(rwlock)
	{
		rwlock_.rdlock();
	}

	~RWLockReadGuard()
	{
		rwlock_.unlock();
	}

private:
	RWLock& rwlock_;
};

class RWLockWriteGuard : noncopyable
{
public:
	explicit RWLockWriteGuard(RWLock& rwlock)
		: rwlock_(rwlock)
	{
		rwlock_.wrlock();
	}

	~RWLockWriteGuard()
	{
		rwlock_.unlock();
	}

private:
	RWLock& rwlock_;
};

class RWLockTimedReadGuard : noncopyable
{
public:
	explicit RWLockTimedReadGuard(RWLock& rwlock)
		: rwlock_(rwlock)
	{
		rwlock_.timed_rdlock();
	}

	~RWLockTimedReadGuard()
	{
		rwlock_.unlock();
	}

private:
	RWLock& rwlock_;
};

class RWLockTimedWriteGuard : noncopyable
{
public:
	explicit RWLockTimedWriteGuard(RWLock& rwlock)
		: rwlock_(rwlock)
	{
		rwlock_.timed_wrlock();
	}

	~RWLockTimedWriteGuard()
	{
		rwlock_.unlock();
	}

private:
	RWLock& rwlock_;
};

} // namespace shared

//#undef __USE_XOPEN2K

#endif // SHARED_BASE_RWLOCK_H_
