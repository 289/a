#ifndef SHARED_BASE_MUTEX_H_
#define SHARED_BASE_MUTEX_H_

/*
 * * compile needed: -lpthread -lrt
 * */

//Names and Order of Includes
//1.preferred location(dir2/foo2.h)
//2.C system files
//3.C++ system files
//4.Other libraries'.h files
//5.Your project's .h files

#include <pthread.h>
#include <time.h>

#include "shared/base/assertx.h"
#include "shared/base/current_thread.h"
#include "shared/base/noncopyable.h"


#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    __THROW __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // !CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE


namespace shared {

class MutexLock : noncopyable
{
    static const int kMutexLockTimeOutSecond = 30;
public:
	MutexLock() : holder_(0)
	{
		MCHECK(pthread_mutex_init(&mutex_, NULL));
	}

	~MutexLock()
	{
		assert(holder_ == 0);
		MCHECK(pthread_mutex_destroy(&mutex_));
	}

	bool IsLockedByThisThread()
	{
		return holder_ == CurrentThread::tid();
	}

	void AssertLocked()
	{
		assert(IsLockedByThisThread());
	}

	// internal usage

	void lock()
	{
		MCHECK(pthread_mutex_lock(&mutex_));
		assign_holder();
	}

	void unlock()
	{
		unassign_holder();
		MCHECK(pthread_mutex_unlock(&mutex_));
	}

	void timed_lock()
	{
		struct timespec abs_time;
		clock_gettime(CLOCK_REALTIME, &abs_time);
		abs_time.tv_sec += kMutexLockTimeOutSecond;
		MCHECK(pthread_mutex_timedlock(&mutex_, &abs_time));
		assign_holder();
	}

	pthread_mutex_t* get_pthread_mutex() /* non-const */
	{
		return &mutex_;
	}

private:
	friend class Condition;

	class UnassignGuard : noncopyable
	{
	public:
		UnassignGuard(MutexLock& owner)
			: owner_(owner)
		{
			owner_.unassign_holder();
		}

		~UnassignGuard()
		{
			owner_.assign_holder();
		}

	private:
		MutexLock& owner_;
	};

	void unassign_holder()
	{
		holder_ = 0;
	}

	void assign_holder()
	{
		holder_ = CurrentThread::tid();
	}


private:
	pthread_mutex_t mutex_;
	pid_t holder_;
}; 

class MutexLockGuard : noncopyable
{
public:
	explicit MutexLockGuard(MutexLock& mutex)
		: mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard()
	{
		mutex_.unlock();
	}

private:
	MutexLock& mutex_;
};

class MutexLockTimedGuard : noncopyable
{
public:
	explicit MutexLockTimedGuard(MutexLock& mutex)
		: mutex_(mutex)
	{
		mutex_.timed_lock();
	}

	~MutexLockTimedGuard()
	{
		mutex_.unlock();
	}

private:
	MutexLock& mutex_;
};

class MutexLockTimedDoubleGuard : noncopyable
{
public:
	explicit MutexLockTimedDoubleGuard(MutexLock& lhs, MutexLock& rhs)
		: lhs_mutex_(lhs), rhs_mutex_(rhs)
	{
		if (&lhs_mutex_ < &rhs_mutex_)
		{
			lhs_mutex_.timed_lock();
			rhs_mutex_.timed_lock();
		}
		else
		{
			assert(&lhs_mutex_ != &rhs_mutex_);
			rhs_mutex_.timed_lock();
			lhs_mutex_.timed_lock();
		}
	}

	~MutexLockTimedDoubleGuard()
	{
		if (&lhs_mutex_ < &rhs_mutex_)
		{
			rhs_mutex_.unlock();
			lhs_mutex_.unlock();
		}
		else
		{
			assert(&lhs_mutex_ != &rhs_mutex_);
			lhs_mutex_.unlock();
			rhs_mutex_.unlock();
		}
	}

private:
	MutexLock& lhs_mutex_;
	MutexLock& rhs_mutex_;
};

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) SHARED_STATIC_ASSERT(false); //missing guard var name
#define MutexLockTimedGuard(x) SHARED_STATIC_ASSERT(false); //missing guard var name
#define MutexLockTimedDoubleGuard(x, y) SHARED_STATIC_ASSERT(false); //missing guard var name

}// namespace shared

#endif // SHARE_BASE_MUTEX_H_
