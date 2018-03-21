#ifndef SHARED_NET_TIMERQUEUE_H_
#define SHARED_NET_TIMERQUEUE_H_

#include <set>
#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/timestamp.h"
#include "shared/net/channel.h"
#include "shared/net/timer.h"
#include "shared/net/timerid.h"

namespace shared {
namespace net {

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : noncopyable
{
public:
	TimerQueue(EventLoop* loop);
	~TimerQueue();

	///
	/// Schedules the callback to be run at given time,
	/// repeats if @c interval > 0.0.
	///
	/// Must be thread safe. Usually be called from other threads.
	TimerId AddTimer(const Timer::TimerCallback& cb,
			         void* pdata,
			         Timestamp when,
			         double interval,
					 Timer::RepeatUnits repeat_type);

	void	Cancel(TimerId timerId);

private:
	struct RegisterAddTimerWrapper
	{
		void*  pdata_;
		Timer* ptimer_;
	};

	struct RegisterCancelInLoopWrapper
	{
		void*	pdata_;
		TimerId	timerId_;
	};

	// FIXME: use unique_ptr<Timer> instead of raw pointers.
	typedef std::pair<Timestamp, Timer*> Entry;
	typedef std::set<Entry> TimerList;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::set<ActiveTimer> ActiveTimerSet;

	void	AddTimerInLoop(Timer* timer);
	void	CancelInLoop(TimerId timerId);

	// called when timerfd alarms
	void	HandleRead();

	static void* RegisterHandleRead(void* pdata);
	static void  RegisterAddTimerInLoop(void* pdata);
	static void  RegisterCancelInLoop(void* pdata);

	void	Reset(const std::vector<Entry>& expired, Timestamp now);
	bool	Insert(Timer* timer);

	// move out all expired timers
	std::vector<Entry> get_expired(Timestamp now);

	EventLoop*		loop_;
	const int		timerfd_;
	Channel			timerfd_channel_;
	// Timer list sorted by expiration
	TimerList		timers_;

	// for cancel()
	ActiveTimerSet	active_timers_;
	bool			calling_expired_timers_; /* atomic */
	ActiveTimerSet	canceling_timers_;
};

} // namespace net
} // namespace shared

#endif  // SHARED_NET_TIMERQUEUE_H_
