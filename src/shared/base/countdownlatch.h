#ifndef SHARED_BASE_COUNTDOWNLATCH_H_
#define SHARED_BASE_COUNTDOWNLATCH_H_

#include "shared/base/noncopyable.h"
#include "shared/base/condition.h"
#include "shared/base/mutex.h"


namespace shared {

class CountDownLatch : noncopyable
{
public:
	explicit CountDownLatch(int count);
	void Wait();
	void CountDown();
	int get_count() const;

private:
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};

}

#endif // SHARED_BASE_COUNTDOWNLATCH_H_
