#ifndef SHARED_BASE_CONDITION_H_
#define SHARED_BASE_CONDITION_H_

#include <pthread.h>

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"

namespace shared {

class Condition : noncopyable
{
public:
	explicit Condition(MutexLock& mutex)
		: mutex_(mutex)
	{
		MCHECK(pthread_cond_init(&pcond_, NULL));
	}

	~Condition()
	{
		MCHECK(pthread_cond_destroy(&pcond_));
	}

	void Wait()
	{
		MutexLock::UnassignGuard ug(mutex_);
		MCHECK(pthread_cond_wait(&pcond_, mutex_.get_pthread_mutex()));
	}

	// returns true if time out, false otherwise
	bool WaitForSeconds(int seconds);

	void Notify()
	{
		MCHECK(pthread_cond_signal(&pcond_));
	}

	void NotifyAll()
	{
		MCHECK(pthread_cond_broadcast(&pcond_));
	}

private:
	MutexLock& mutex_;
	pthread_cond_t pcond_;
};

}

#endif // SHARED_BASE_CONDITION_H_
