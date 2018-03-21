#include "shared/base/timestamp.h"

namespace combat
{

int64_t GetSysTime()
{
    shared::Timestamp now(shared::Timestamp::Now());
    return now.seconds_since_epoch();
}

int64_t GetSysTimeMsec()
{
    shared::Timestamp now(shared::Timestamp::Now());
    return now.milli_seconds_since_epoch();
}

};
