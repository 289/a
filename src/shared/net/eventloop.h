#ifndef SHARED_NET_EVENTLOOP_H_
#define SHARED_NET_EVENTLOOP_H_

#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/thread.h"
#include "shared/net/timerid.h"
#include "shared/net/timer.h"

namespace shared {
namespace net {

class Channel;
class Poller;
class TimerQueue;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too mush details.
class EventLoop : noncopyable
{
public:
	typedef void (*Functor)(void*);

	EventLoop();
	~EventLoop();

	///
	/// Loops forever.
	///
	/// Must be called in the same thread as creation of the object.
	///
	void Loop();

	void Quit();

	///
	/// ++ when poll returns, record the number of times that data arrivial.
	///
	int64_t		iteration() const { return iteration_; }

	/// Runs callback immediately in the loop thread.
	/// It wakes up the loop, and run the cb.
	/// If in the same loop thread, cb is run within the function.
	/// Safe to call from other threads
	void	RunInLoop(const Functor& cb, void* pdata);
	/// Queues callback in the loop thread.
	/// Runs after finish pooling.
	/// Safe to call from other threads.
	void	QueueInLoop(const Functor& cb, void* pdata);

	// timers
	
	///
	/// Runs callback at 'time'
	/// Safe to call from other threads
	///
	TimerId RunAt(const Timestamp& time, const Timer::TimerCallback& cb, void* pdata);
	///
	/// Runs callback after @c delay seconds.
	/// Safe to call from other threads.
	///
	TimerId RunAfter(double delay, const Timer::TimerCallback& cb, void* pdata);
	///
	/// Runs callback after @c delay milliseconds.
	/// Safe to call from other threads
	///
	TimerId RunAfterMsecs(double delay, const Timer::TimerCallback& cb, void* pdata);
	///
	/// Runs callback every @c interval seconds.
	/// Safe to call from other threads.
	///
	TimerId RunEvery(double interval, const Timer::TimerCallback& cb, void* pdata);
	///
	/// Runs callback every @c interval milliseconds.
	/// Safe to call from other threads.
	///
	TimerId RunEveryMsecs(double interval, const Timer::TimerCallback& cb, void* pdata);
	///
	/// Cancels the timer.
	/// Safe to call from other threads.
	///
	void Cancel(TimerId timerId);


	// internal usage
	void	Wakeup();
	void	UpdateChannel(Channel* channel);
	void	RemoveChannel(Channel* channel);

	void	AssertInLoopThread()
	{
		if (!IsInLoopThread())
		{
			AbortNotInLoopThread();
		}
	}

	bool	IsInLoopThread() const { return threadId_ == CurrentThread::tid(); }
	bool	event_handling() const { return event_handling_; }

	static EventLoop* GetEventLoopOfCurrentThread();

private:
	class FunctorWrapper
	{
	public:
		FunctorWrapper()
			: func(NULL),
			  pdata_(NULL)
		{ }

		FunctorWrapper(const Functor& f, void* pdata)
			: func(f),
			  pdata_(pdata)
		{ }

		Functor func;
		void* pdata_;
	};

private:
	void	AbortNotInLoopThread();
	void	HandleWakeupRead();
	void	DoPendingFunctors();

	static void* RegisterWakeupRead(void*);
	void	PrintActiveChannels() const; // DEBUG

	typedef std::vector<Channel*> ChannelList;

	bool			looping_;	/* atomic */
	bool			quit_;		/* atomic */
	bool			event_handling_; /* atomic */
	bool			calling_pending_functors_; /* atomic */
	int64_t			iteration_;
	const			pid_t threadId_;
	Poller*			poller_;
	int				wakeup_fd_;
	ChannelList		active_channels_;
	Channel*		pwakeup_channel_;
	Channel*		pcurrent_active_channel_;
	MutexLock		mutex_;
	std::vector<FunctorWrapper> pending_functors_;

	//timers
	TimerQueue*		ptimerqueue_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_EVENTLOOP_H_
