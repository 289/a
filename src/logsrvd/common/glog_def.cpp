#include "glog_def.h"

#include <stdlib.h>


namespace GLog {

GLog::GLogLevel InitLogLevel()
{
	if (::getenv("GLOG_TRACE"))
		return GLog::GLOG_TRACE;
	else if (::getenv("GLOG_DEBUG"))
		return GLog::GLOG_DEBUG;
	else
		return GLog::GLOG_INFO;
}

GLog::GLogLevel g_logLevel = InitLogLevel();

const char* LogLevelName[GLog::NUM_LOG_LEVELS] =
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL ",
};

const char* LogFileBaseName[GLog::NUM_LOG_TYPES] =
{
	"Auth",
	"Gateway",
	"Gamedb",
	"Gameserver",
	"Link",
	"Master",
	"Match",
};

const int LogFileRollSize[GLog::NUM_LOG_TYPES] = 
{
	512*1024*1024, // 512Mbytes
	512*1024*1024,
	512*1024*1024,
	512*1024*1024,
	512*1024*1024,
	512*1024*1024,
	512*1024*1024,
};

} // namespace GLog
