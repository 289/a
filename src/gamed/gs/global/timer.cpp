#include "timer.h"

#include "shared/net/eventloop.h"

#include "timed_task.h"


namespace gamed {

using namespace shared;
using namespace shared::net;

void Timer::TickCallback(void* pdata)
{
	Timer* ptimer = static_cast<Timer*>(pdata);
	ptimer->Tick();
}

Timer::Timer()
	: evloop_thread_(NULL),
	  loop_(NULL),
	  tick_index_(0)
{
}

Timer::~Timer()
{
}

void Timer::TimerThread()
{
	ASSERT(NULL == evloop_thread_);
	evloop_thread_ = new EventLoopThread(BIND_MEM_CB(&Timer::LoopThreadRunStatusCB, this));
	evloop_thread_->StartLoop();
}

void Timer::StopThread()
{
	ASSERT(NULL != evloop_thread_);
	SAFE_DELETE(evloop_thread_);
}

// run in loop thread
void Timer::Tick()
{
	{
		MutexLockGuard lock(cb_set_mutex_);
		expired_timers_.clear();
		get_expired(++tick_index_, expired_timers_);
	}

	// timed tasks
	s_pTimedTask->Tick(tick_index_);

	for (size_t i = 0; i < expired_timers_.size(); ++i)
	{
		CallbackInfo cbinfo = expired_timers_[i].second;
		cbinfo.callback_func(tick_index_, cbinfo.cb_obj, --cbinfo.times);
		if (CallbackInfo::INFINITE == cbinfo.repeat_type)
		{
			MutexLockGuard lock(cb_set_mutex_);
			TickIndex when = tick_index_ + cbinfo.interval;
			InsertToTimerSet(when, cbinfo);
		}
		else if (cbinfo.times > 0)
		{
			MutexLockGuard lock(cb_set_mutex_);
			TickIndex when = tick_index_ + cbinfo.interval;
			InsertToTimerSet(when, cbinfo);
		}
	}
}

// run in loop thread
void Timer::LoopThreadRunStatusCB(EventLoop* loop, bool is_start_run)
{
	if (is_start_run)
	{
		loop_     = loop;
		timer_id_ = loop_->RunEveryMsecs(kTickTime, Timer::TickCallback, this);
	}
	else
	{
		loop_->Cancel(timer_id_);
		loop_ = NULL;
	}
}

int Timer::SetTimer(int interval, int start_time, int times, timer_callback routine, void* obj)
{
	if (interval <= 0 || start_time < 0 || times < 0)
	{
		ASSERT(false);
		return -1;
	}

	CallbackInfo cbinfo;
	cbinfo.repeat_type   = (0 == times) ? CallbackInfo::INFINITE : CallbackInfo::TIMES_REPEAT;
	cbinfo.interval      = interval;
	cbinfo.times         = times;
	cbinfo.cb_obj        = obj;
	cbinfo.callback_func = routine;

	MutexLockGuard lock(cb_set_mutex_);
	TickIndex when = tick_index_ + start_time + interval;
	InsertToTimerSet(when, cbinfo);

	return tick_index_;
}

void Timer::get_expired(TickIndex tickindex, std::vector<Entry>& timeup_vec)
{
	Entry sentry(tickindex, CallbackInfo());
	TimerCallbackSet::iterator end = timer_cb_set_.lower_bound(sentry);
	assert(end == timer_cb_set_.end() || tickindex <= end->first);
	std::copy(timer_cb_set_.begin(), end, back_inserter(timeup_vec));
	timer_cb_set_.erase(timer_cb_set_.begin(), end);
}

time_t Timer::GetSysTime()
{
	Timestamp now(Timestamp::Now());
	return now.seconds_since_epoch();
}

int64_t Timer::GetSysTimeMsecs()
{
	Timestamp now(Timestamp::Now());
	return now.milli_seconds_since_epoch();
}

} // namespace gamed
