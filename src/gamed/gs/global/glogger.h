#ifndef GAMED_GS_GLOBAL_GLOGGER_H_
#define GAMED_GS_GLOBAL_GLOGGER_H_

#include "logsrvd/logclient/src/glog.h"


namespace gamed {

class GLogger
{
public:
	static bool Init(const char* confFile, GLog::GLogType type, int32_t id);
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GLOGGER_H_
