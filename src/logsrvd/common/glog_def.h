#ifndef LOGSRVD_COMMON_GLOG_DEF_H_
#define LOGSRVD_COMMON_GLOG_DEF_H_


namespace GLog {

	enum GLogType
	{
		GLOG_TYPE_AU,
		GLOG_TYPE_GW,
		GLOG_TYPE_DB,
		GLOG_TYPE_GS,
		GLOG_TYPE_LINK,
		GLOG_TYPE_MASTER,
		GLOG_TYPE_MATCH,
		NUM_LOG_TYPES,
	};

	enum GLogLevel
	{
		GLOG_TRACE,
		GLOG_DEBUG,
		GLOG_INFO,
		GLOG_WARN,
		GLOG_ERROR,
		GLOG_FATAL,
		NUM_LOG_LEVELS,
	};

	extern const char*  LogFileBaseName[GLog::NUM_LOG_TYPES];
	extern const int    LogFileRollSize[GLog::NUM_LOG_TYPES];
	extern const char*  LogLevelName[GLog::NUM_LOG_LEVELS];
	extern GLogLevel    g_logLevel;

	
} // namespace GLog

#endif // LOGSRVD_COMMON_GLOG_DEF_H_
