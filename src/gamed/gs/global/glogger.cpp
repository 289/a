#include "glogger.h"


namespace gamed {

bool GLogger::Init(const char* confFile, GLog::GLogType type, int32_t id)
{
	return GLog::init(confFile, type, id);
}

} // namespace gamed
