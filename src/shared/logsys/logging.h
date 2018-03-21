#ifndef SHARED_LOGSYS_LOGGING_H_
#define SHARED_LOGSYS_LOGGING_H_

#include "shared/base/timestamp.h"
#include "shared/logsys/logstream.h"

namespace shared {

class Logger
{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,
	};

	// compile time calculation of basename of source file
	class SourceFile
	{
	public:
		template<int N>
		inline SourceFile(const char (&arr)[N])
			: data_(arr),
			  size_(N-1)
		{
			const char* slash = strrchr(data_, '/'); // builtin function
			if (slash)
			{
				data_ = slash + 1;
				size_ -= static_cast<int>(data_ - arr);
			}
		}

		explicit SourceFile(const char* filename)
			: data_(filename)
		{
			const char* slash = strrchr(filename, '/');
			if (slash)
			{
				data_ = slash + 1;
			}
			size_ = static_cast<int>(strlen(data_));
		}

		const char* data_;
		int size_;
	};

	Logger(SourceFile file, int line);
	Logger(SourceFile file, int line, LogLevel level);
	Logger(SourceFile file, int line, LogLevel level, const char* func);
	Logger(SourceFile file, int line, bool toAbort);
	~Logger();

	LogStream& stream() { return impl_.stream_; }

	static LogLevel log_level();
	static void SetLogLevel(LogLevel level);

	typedef void (*OutputFunc)(const char* msg, int len);
	typedef void (*FlushFunc)();
	static void SetOutput(OutputFunc);
	static void SetFlush(FlushFunc);

private:
	class Impl
	{
		public:
			typedef Logger::LogLevel LogLevel;
			Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
			void FormatTime();
			void Finish();

			Timestamp	time_;
			LogStream	stream_;
			LogLevel	level_;
			int			line_;
			SourceFile	basename_;
	};

	Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::log_level()
{
	return g_logLevel;
}

#define LOG_TRACE if (shared::Logger::log_level() <= shared::Logger::TRACE) \
	shared::Logger(__FILE__, __LINE__, shared::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (shared::Logger::log_level() <= shared::Logger::DEBUG) \
	shared::Logger(__FILE__, __LINE__, shared::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (shared::Logger::log_level() <= shared::Logger::INFO) \
	shared::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN shared::Logger(__FILE__, __LINE__, shared::Logger::WARN).stream()
#define LOG_ERROR shared::Logger(__FILE__, __LINE__, shared::Logger::ERROR).stream()
#define LOG_FATAL shared::Logger(__FILE__, __LINE__, shared::Logger::FATAL).stream()
#define LOG_SYSERR shared::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL shared::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
	::shared::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr) {
	if (ptr == NULL) {
		Logger(file, line, Logger::FATAL).stream() << names;
	}
	return ptr;
}

} // namespace shared

#endif  // SHARED_LOGSYS_LOGGING_H_
