#ifndef GAMED_GS_GLOBAL_TIMER_H_
#define GAMED_GS_GLOBAL_TIMER_H_

#include <vector>
#include <set>

#include "shared/base/singleton.h"
#include "shared/base/base_define.h"

#include "shared/net/eventloopthread.h"
#include "shared/net/timerid.h"

#include "game_types.h"


namespace gamed {

class Timer : public shared::Singleton<Timer>
{
	friend class shared::Singleton<Timer>;

	enum 
	{
		TICKTIME = 1000/TICK_PER_SEC, // Units are millisecond
		MINTIME  = 10,
	};

public:
	typedef void (*timer_callback)(TickIndex index, void *object, int remain);

	static inline Timer* GetInstance() {
		return &(get_mutable_instance());
	}

	void    TimerThread();
	void    StopThread();

	// ---- thread safe ----
	// SetTimer暂时只用来产生tick
	int     SetTimer(int interval, int start_time, int times, timer_callback routine, void* obj);
	time_t  GetSysTime();       // get seconds since epoch
	int64_t GetSysTimeMsecs();  // get milli-seconds since epoch

	inline TickIndex get_tick() const;


protected:
	Timer();
	~Timer();

	static void TickCallback(void*);


private:
	void    LoopThreadRunStatusCB(shared::net::EventLoop* loop, bool is_start_run);
	void    Tick();

	struct CallbackInfo
	{
		enum RepeatType
		{
			INFINITE = 0,
			TIMES_REPEAT,
		};
		RepeatType     repeat_type;
		int32_t        interval;
		int32_t        times;
		timer_callback callback_func;
		void*          cb_obj;
	};
	typedef std::pair<TickIndex, CallbackInfo> Entry;

	struct Comparator
	{
		bool operator()(const Entry& lhs, const Entry& rhs)
		{
			return lhs.first <= rhs.first;
		}
	};

	typedef std::multiset<Entry, Comparator> TimerCallbackSet;
	TimerCallbackSet      timer_cb_set_;
	std::vector<Entry>    expired_timers_;

	void get_expired(TickIndex tickindex, std::vector<Entry>& timeup_vec);
	inline void InsertToTimerSet(TickIndex when, CallbackInfo& cbinfo);


private:
	static const int kTickTime = TICKTIME;
	static const int kMinTime  = MINTIME;

	shared::net::EventLoopThread*  evloop_thread_;
	shared::net::EventLoop*        loop_;
	shared::net::TimerId           timer_id_;

	TickIndex           tick_index_;
	shared::MutexLock   cb_set_mutex_;
};

///
/// inline func
///
// needed lock outside
inline void Timer::InsertToTimerSet(TickIndex when, CallbackInfo& cbinfo)
{
	TimerCallbackSet::iterator result
		= timer_cb_set_.insert(Entry(when, cbinfo));
	assert(result != timer_cb_set_.end()); (void)result;
}

inline TickIndex Timer::get_tick() const
{
	return tick_index_;
}

#define g_timer gamed::Timer::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_TIMER_H_
