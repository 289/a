#ifndef SHARED_NET_EVENTLOOPTHREAD_H_
#define SHARED_NET_EVENTLOOPTHREAD_H_

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/condition.h"
#include "shared/base/thread.h"
#include "shared/base/callback_bind.h"


namespace shared {
namespace net {

class EventLoop;

class EventLoopThread : noncopyable
{
public: 
	typedef bind::Callback<void (EventLoop*, bool)> ThreadRunStatusCallback;

	EventLoopThread(const ThreadRunStatusCallback& cb = bind::NullCallback());
	~EventLoopThread();

	EventLoop* StartLoop();

private:
	void ThreadFunc();
	static void* RegisterThreadFunc(void*);

	EventLoop*    loop_;
	bool          exiting_;
	Thread        thread_;
	MutexLock     mutex_;
	Condition     cond_;
	ThreadRunStatusCallback callback_;
};

} // namespace net
} // namespace shared


#endif // SHARED_NET_EVENTLOOPTHREAD_H_
