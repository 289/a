#ifndef LOGSRVD_LOGCLIENT_SRC_GLOG_H_
#define LOGSRVD_LOGCLIENT_SRC_GLOG_H_

#include <stdint.h>

#include "logsrvd/common/glog_def.h"


namespace GLog {

/**
 * @brief Init
 *
 * @param confFile
 * @param type
 * @param id: 调用者的id，比如gsid，linkid
 *
 * @return true or false
 */
bool init(const char* confFile, GLogType type, int32_t id = 1);


/**
 * @brief logFormat
 * （1）写日志接口，没有填level则使用默认的level
 */
void logF(const char* format, ...);
void logF(GLogLevel level, const char* format, ...);
void logF(const char* file, int line, const char* func, const char* format, ...);
void logF(const char* file, int line, const char* func, GLogLevel level, const char* format, ...);


/**
 * @brief Log 
 * （1）写日志接口，没有填level则使用默认的level
 * （2）以下函数可以自动打出调用的函数、行号、文件名
 */
#define log(format, ...) \
	logF(__FILE__, __LINE__, __func__, format, ##__VA_ARGS__);

#define logL(level, format, ...) \
	logF(__FILE__, __LINE__, __func__, level, format, ##__VA_ARGS__);

} // namespace GLog

#endif // LOGSRVD_LOGCLIENT_SRC_GLOG_H_
