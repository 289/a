#ifndef LOGSRVD_LOGSERVER_SRC_GLOGGING_H_
#define LOGSRVD_LOGSERVER_SRC_GLOGGING_H_

#include <memory>

#include "shared/base/noncopyable.h"
#include "shared/logsys/logfile.h"

#include "logsrvd/common/glog_def.h"


namespace shared 
{
	class LogStream;
} // shared


namespace GLog {

/**
 * @brief GLogger
 */
class GLogger : shared::noncopyable
{
public:
	GLogger(const std::string& path, const std::string& basename, size_t rollSize);
	~GLogger();

	void writeLog(GLog::GLogLevel level, int32_t id, const std::string& str);

private:
	void formatTime(shared::LogStream& a_stream);

private:
	shared::LogFile log_file_;
	char		    buffer_[64*1024];
};

} // namespace GLog

#endif // LOGSRVD_LOGSERVER_SRC_GLOGGING_H_
