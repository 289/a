#ifndef SHARED_NET_TIMERID_H_
#define SHARED_NET_TIMERID_H_

#include "shared/base/copyable.h"
#include "shared/net/timer.h"

namespace shared {
namespace net {

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public shared::copyable
{
public:
	TimerId()
		: timer_(NULL),
		  sequence_(0)
	{
	}

	TimerId(Timer* timer, int64_t seq)
		: timer_(timer),
		  sequence_(seq)
	{
	}

	inline void set_to_cancel();
	inline bool valid(); 

	// default copy-ctor, dtor and assignment are okay

	friend class TimerQueue;

private:
	Timer*	timer_;
	int64_t sequence_;
};

///
/// inline function
///
inline bool TimerId::valid()
{
	if (NULL == timer_ || 0 == sequence_)
		return false;

	return true;
}

inline void TimerId::set_to_cancel()
{
	timer_->set_to_cancel();
}

} // namespace net
} // namespace shared

#endif  // SHARED_NET_TIMERID_H_
