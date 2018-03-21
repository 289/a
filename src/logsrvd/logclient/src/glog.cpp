#include "glog.h"

#include <stdarg.h>

#include "shared/base/conf.h"
#include "shared/logsys/logging.h"
#include "shared/net/manager/network.h"
#include "logsrvd/protocol/log_message.pb.h"

#include "logsrv_client.h"


namespace GLog {

using namespace std;
using namespace shared;
using namespace shared::net;

static shared::Conf g_log_conf;
static const size_t kStackBufSize   = 1 << 16;
static const char* kAllowRemoteLog  = "allow_remote_log";
static const char* kAllowStdout     = "allow_stdout";
static const char* kDefaultLogLevel = "default_log_level";
static bool g_allow_remote_log      = false;
static bool g_allow_stdout          = false;
static int32_t g_caller_id          = 0;
static logsrv::LogMessage::MessageType g_caller_type;
static logsrv::LogMessage::LogLevel g_msg_log_level;

///
/// inline func
///
inline void set_log_level(GLogLevel level)
{
	g_logLevel = level;
}

inline GLogLevel log_level()
{   
	return g_logLevel;
}

inline bool GetLogMsgType(GLogType type, logsrv::LogMessage::MessageType& msg_type)
{
	switch (type)
	{
		case GLOG_TYPE_AU:
			msg_type = logsrv::LogMessage::AU_TYPE;
			break;

		case GLOG_TYPE_GW:
			msg_type = logsrv::LogMessage::GW_TYPE;
			break;

		case GLOG_TYPE_DB:
			msg_type = logsrv::LogMessage::DB_TYPE;
			break;

		case GLOG_TYPE_GS:
			msg_type = logsrv::LogMessage::GS_TYPE;
			break;

		case GLOG_TYPE_LINK:
			msg_type = logsrv::LogMessage::LINK_TYPE;
			break;

		case GLOG_TYPE_MASTER:
			msg_type = logsrv::LogMessage::MASTER_TYPE;
			break;

		case GLOG_TYPE_MATCH:
			msg_type = logsrv::LogMessage::MATCH_TYPE;
			break;

		default:
			return false;
	}

	return true;
}

inline bool GetMsgLogLevel(GLogLevel level, logsrv::LogMessage::LogLevel& msg_level)
{
	switch (level)
	{
		case GLOG_TRACE:
			msg_level = logsrv::LogMessage::TRACE;
			break;

		case GLOG_DEBUG:
			msg_level = logsrv::LogMessage::DEBUG;
			break;

		case GLOG_INFO:
			msg_level = logsrv::LogMessage::INFO;
			break;

		case GLOG_WARN:
			msg_level = logsrv::LogMessage::WARN;
			break;

		case GLOG_ERROR:
			msg_level = logsrv::LogMessage::ERROR;
			break;

		case GLOG_FATAL:
			msg_level = logsrv::LogMessage::FATAL;
			break;

		default:
			return false;
	}

	return true;
}

inline bool GetDefaultLogLevel(const string& name, GLogLevel& level)
{
	for (int i = 0; i < NUM_LOG_LEVELS; ++i)
	{
		string tmpstr = LogLevelName[i];
		size_t found = tmpstr.find(name);
		if (found == 0)
		{
			level = (GLogLevel)i;
			return true;
		}
	}
	return false;
}

bool init(const char* confFile, GLogType type, int32_t id)
{
	std::string section_name = LogsrvClient::Name();
	if (!g_log_conf.Init(confFile))
	{
		LOG_ERROR << "g_log_conf init error!";
		return false;
	}
			
	// default log level
	GLogLevel level = GLOG_INFO;
	string log_level_name = g_log_conf.find(section_name.c_str(), kDefaultLogLevel);
	if (!GetDefaultLogLevel(log_level_name, level))
	{
		fprintf(stderr, "GLog::init() - GetDefaultLogLevel error!");
		return false;
	}

	// msg type
	if (!GetLogMsgType(type, g_caller_type))
	{
		fprintf(stderr, "GLog类型不正确！type:%d\n", (int)type);
		return false;
	}

	// msg level
	if (!GetMsgLogLevel(level, g_msg_log_level))
	{
		fprintf(stderr, "GLog日志等级不正确！level:%d\n", (int)level);
		return false;
	}

	set_log_level(level);
	g_caller_id     = id;

	// allow stdout
	string allow_out_str = g_log_conf.find(section_name.c_str(), kAllowStdout);
	if (allow_out_str == "true")
	{
		g_allow_stdout = true;
		LOG_INFO << "GLog打印到stdout";
	}
	else
	{
		LOG_INFO << "GLog不打印到stdout,配置文件" << kAllowStdout << "为false";
	}

	// allow remote log
	string allow_str = g_log_conf.find(section_name.c_str(), kAllowRemoteLog);
	if (allow_str == "true")
	{
		LOG_INFO << "GLog生效，进行远程Logservice的日志记录";
		g_allow_remote_log = true;
	}
	else
	{
		LOG_INFO << "GLog没有生效，不进行远程Logservice的日志记录";
		return true;
	}

	LogsrvClient* pclient = LogsrvClient::GetInstance();
	NetWork::Client(pclient);
	return true;
}

static inline void DoStdOutput(logsrv::LogMessage::LogLevel level, 
		                       const std::string& str, 
							   const char* file, 
							   int line, 
							   const char* func)
{
	if (file != NULL && func != NULL)
	{
		shared::Logger::SourceFile srcFile(file);
		switch (level)
		{
			case logsrv::LogMessage::TRACE:
				shared::Logger(srcFile, line, shared::Logger::TRACE, func).stream() << str;
				break;

			case logsrv::LogMessage::DEBUG:
				shared::Logger(srcFile, line, shared::Logger::DEBUG, func).stream() << str;
				break;

			case logsrv::LogMessage::INFO:
				shared::Logger(srcFile, line).stream() << str;
				break;

			case logsrv::LogMessage::WARN:
				shared::Logger(srcFile, line, shared::Logger::WARN).stream() << str;
				break;

			case logsrv::LogMessage::ERROR:
				shared::Logger(srcFile, line, shared::Logger::ERROR).stream() << str;
				break;

			case logsrv::LogMessage::FATAL:
				shared::Logger(srcFile, line, shared::Logger::FATAL).stream() << str;
				break;

			default:
				break;
		}

	}
	else
	{
		switch (level)
		{
			case logsrv::LogMessage::TRACE:
				LOG_TRACE << "GLog:" << str;
				break;

			case logsrv::LogMessage::DEBUG:
				LOG_DEBUG << "GLog:" << str;
				break;

			case logsrv::LogMessage::INFO:
				LOG_INFO << "GLog:" << str;
				break;

			case logsrv::LogMessage::WARN:
				LOG_WARN << "GLog:" << str;
				break;

			case logsrv::LogMessage::ERROR:
				LOG_ERROR << "GLog:" << str;
				break;

			case logsrv::LogMessage::FATAL:
				LOG_FATAL << "GLog:" << str;
				break;

			default:
				break;
		}
	}
}

static void remote_logger(logsrv::LogMessage::LogLevel level, 
		                  const char * format, 
						  va_list ap, 
		                  const char* file = NULL, 
						  int line = 0, 
						  const char* func = NULL)
{
	char msg_buf[kStackBufSize] = {0};

	size_t msg_len = vsnprintf(&(msg_buf[0]), kStackBufSize, format, ap);
	if (msg_len <= 0)
	{
		fprintf(stderr, "vsnprintf error\n");
		return;
	}

	if (g_allow_remote_log)
	{
		logsrv::LogMessage proto;
		proto.set_type(g_caller_type);
		proto.set_id(g_caller_id);
		proto.set_level(level);
		proto.set_msg(&(msg_buf[0]), msg_len);
		LogsrvClient::GetInstance()->Send(proto);
	}

	if (g_allow_stdout)
	{
		std::string tmpbuf;
		tmpbuf.assign(&(msg_buf[0]), msg_len);
		DoStdOutput(level, tmpbuf, file, line, func);
	}
}

void logF(const char* format, ...)
{
	if (!g_allow_remote_log && !g_allow_stdout)
		return;

	va_list ap;
	va_start(ap,format);
	remote_logger(g_msg_log_level, format, ap);
	va_end(ap);
}

void logF(GLogLevel level, const char* format, ...)
{
	if (!g_allow_remote_log && !g_allow_stdout)
		return;

	if (level >= g_logLevel)
	{
		logsrv::LogMessage::LogLevel msg_level;
		if (!GetMsgLogLevel(level, msg_level))
			return;

		va_list ap;
		va_start(ap,format);
		remote_logger(msg_level, format, ap);
		va_end(ap);
	}
}

void logF(const char* file, int line, const char* func, const char* format, ...)
{
	if (!g_allow_remote_log && !g_allow_stdout)
		return;

	va_list ap;
	va_start(ap,format);
	remote_logger(g_msg_log_level, format, ap, file, line, func);
	va_end(ap);
}

void logF(const char* file, int line, const char* func, GLogLevel level, const char* format, ...)
{
	if (!g_allow_remote_log && !g_allow_stdout)
		return;

	if (level >= g_logLevel)
	{
		logsrv::LogMessage::LogLevel msg_level;
		if (!GetMsgLogLevel(level, msg_level))
			return;

		va_list ap;
		va_start(ap,format);
		remote_logger(msg_level, format, ap, file, line, func);
		va_end(ap);
	}
}

} // namespace GLog
