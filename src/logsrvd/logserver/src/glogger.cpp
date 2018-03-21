#include "glogger.h"

#include <string.h>
#include <stdio.h>

#include "shared/base/timestamp.h"
#include "shared/logsys/logstream.h"


namespace GLog {

using namespace std;
using namespace shared;

__thread char   t_time[32];
__thread time_t t_lastSecond;


// helper class for known string length at compile time
class T
{
public:
	T(const char* str, unsigned len)
		: str_(str),
		  len_(len)
	{
		assert(strlen(str) == len_);
	}

	const char* str_;
	const unsigned len_;
};

inline shared::LogStream& operator<<(shared::LogStream& s, T v)
{
	s.Append(v.str_, v.len_);
	return s;
}


///
/// GLogger
///
GLogger::GLogger(const string& path, const string& basename, size_t rollSize)
	: log_file_(path, basename, rollSize, false, 3)
{
}

GLogger::~GLogger()
{
}

void GLogger::formatTime(shared::LogStream& a_stream)
{
	int64_t microSecondsSinceEpoch = Timestamp::Now().micro_seconds_since_epoch();
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
	int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
	if (seconds != t_lastSecond)
	{
		t_lastSecond = seconds;
		struct tm tm_time;
		//::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
		::localtime_r(&seconds, &tm_time); // FIXME TimeZone::from user’s specified timezone

		int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
				           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
				           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17); (void)len;
	}
	Fmt us(".%06dZ ", microseconds);
	assert(us.length() == 9);
	a_stream << T(t_time, 17) << T(us.data(), 9);
}

void GLogger::writeLog(GLog::GLogLevel level, int32_t id, const std::string& str)
{
	// format log
	LogStream a_stream;
	formatTime(a_stream);
	a_stream << id;
	a_stream << " ";
	a_stream << T(LogLevelName[level], 6);
	a_stream << str;
	a_stream << '\n';

	// append to file
	const LogStream::Buffer& buf(a_stream.buffer());
	log_file_.Append(buf.data(), buf.length());
#ifdef FLUSH_EVERY_LINE
	log_file_.Flush();
#endif // FLUSH_EVERY_LINE
}

} // namespace GLog
