#ifndef SHARED_BASE_TIMESTAMP_H_
#define SHARED_BASE_TIMESTAMP_H_

#include "shared/base/copyable.h"
#include "shared/base/types.h"


namespace shared {

///
/// Time stamp in UTC, in microseconds resolution.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Timestamp : public shared::copyable
{
public:
	///
	/// Constucts an invalid Timestamp.
	///
	Timestamp()
		: micro_seconds_since_epoch_(0)
	{
	}

	///
	/// Constucts a Timestamp at specific time
	///
	/// @param microSecondsSinceEpoch
	explicit Timestamp(int64_t microSecondsSinceEpoch);

	void Swap(Timestamp& that)
	{
		std::swap(micro_seconds_since_epoch_, that.micro_seconds_since_epoch_);
	}

	// default copy/assignment/dtor are Okay

    std::string ToString() const;
    std::string ToFormattedString() const;

	bool valid() const { return micro_seconds_since_epoch_ > 0; }

	// To get micro_secs, milli_secs or secs
	int64_t micro_seconds_since_epoch() const { return micro_seconds_since_epoch_; }
	time_t seconds_since_epoch() const
	{ return static_cast<time_t>(micro_seconds_since_epoch_ / (int64_t)kMicroSecondsPerSecond); }
	int64_t milli_seconds_since_epoch() const 
	{ return static_cast<int64_t>(micro_seconds_since_epoch_ / (int64_t)kMicroSecondsPerMillisecond); }

	///
	/// Get time of now.
	///
	static Timestamp Now();
	static Timestamp invalid();

	static const int kMicroSecondsPerSecond      = 1000 * 1000;
	static const int kMicroSecondsPerMillisecond = 1000;

private:
	int64_t micro_seconds_since_epoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
	return lhs.micro_seconds_since_epoch() < rhs.micro_seconds_since_epoch();
}

inline bool operator>(Timestamp lhs, Timestamp rhs)
{
	return lhs.micro_seconds_since_epoch() > rhs.micro_seconds_since_epoch();
}

inline bool operator>=(Timestamp lhs, Timestamp rhs)
{
	return lhs.micro_seconds_since_epoch() >= rhs.micro_seconds_since_epoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
	return lhs.micro_seconds_since_epoch() == rhs.micro_seconds_since_epoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microseciond
/// resolution for next 100 years.
inline double TimeDifference(Timestamp high, Timestamp low)
{
	int64_t diff = high.micro_seconds_since_epoch() - low.micro_seconds_since_epoch();
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline double TimeMsecsDifference(Timestamp high, Timestamp low)
{
	int64_t diff = high.micro_seconds_since_epoch() - low.micro_seconds_since_epoch();
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerMillisecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp AddTimestampSecs(Timestamp timestamp, double seconds)
{
	int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
	return Timestamp(timestamp.micro_seconds_since_epoch() + delta);
}

/// 
/// Add @c milliseconds to given timestamp
///
/// @return timestamp+milliseconds as Timestamp
inline Timestamp AddTimestampMsecs(Timestamp timestamp, double milliseconds)
{
	int64_t delta = static_cast<int64_t>(milliseconds * Timestamp::kMicroSecondsPerMillisecond);
	return Timestamp(timestamp.micro_seconds_since_epoch() + delta);
}

} // namespace shared

#endif  // SHARED_BASE_TIMESTAMP_H_
