#ifndef SHARED_NET_TIMER_H_
#define SHARED_NET_TIMER_H_

#include "shared/base/noncopyable.h"
#include "shared/base/atomic.h"
#include "shared/base/timestamp.h"

namespace shared {
namespace net {

///
/// Internal class for timer event.
///
class Timer : noncopyable
{
public:
	typedef void (*TimerCallback)(void*);

	enum RepeatUnits
	{
		NON_REPEAT = 0,
		SECS_INTERVAL,    // seconds interval
		MSECS_INTERVAL,   // milliseconds interval
	};

	Timer(const TimerCallback& cb, void* pdata, Timestamp when, double interval, RepeatUnits repeat)
		: callback_(cb),
		  pdata_(pdata),
		  expiration_(when),
		  interval_(interval),
		  repeat_(repeat),
		  sequence_(s_num_created_.increment_and_get()),
		  is_canceled_(false)
	{ }

	void run() const
	{
		if (!is_canceled())
		{
			callback_(pdata_);
		}
	}

	Timestamp	expiration() const  { return expiration_; }
	bool		repeat() const { return static_cast<bool>(repeat_); }
	int64_t		sequence() const { return sequence_; }

	void		restart(Timestamp now);
	void        set_to_cancel() { is_canceled_ = true; }
	bool        is_canceled() const { return is_canceled_; }

	static int64_t num_created() { return s_num_created_.get(); }

private:
	const TimerCallback	callback_;
	void*				pdata_;
	Timestamp			expiration_;
	const double		interval_;
	const RepeatUnits	repeat_;
	const int64_t		sequence_;

	static AtomicInt64  s_num_created_;

	bool                is_canceled_;
};

} // namespace net
} // namespace shared

#endif  // SHARED_NET_TIMER_H_
