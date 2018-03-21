#ifndef TASK_TASK_LIMIT_H_
#define TASK_TASK_LIMIT_H_

#include "basic_cond.h"

namespace task
{

enum LimitType
{
    LIMIT_NONE,
    LIMIT_DELIVER,
    LIMIT_COMPLETE,
};

enum CleanUpType
{
    CLEANUP_NONE,
    CLEANUP_DAY,
    CLEANUP_WEEK,
    CLEANUP_MONTH,
    CLEANUP_TIME,
};

// 任务接取，清空限制
class TaskLimit
{
public:
    TaskLimit()
        : limit(LIMIT_NONE), interval(0), num(1), clean(CLEANUP_DAY)
    {
    }

    inline bool CheckDataValidity() const;

    int8_t limit;
    int32_t interval;
    int32_t num;
    int8_t clean;
    TimeSegmentCond segment;

    NESTED_DEFINE(limit, interval, num, clean, segment);
};

inline bool TaskLimit::CheckDataValidity() const
{
    CHECK_INRANGE(limit, LIMIT_NONE, LIMIT_COMPLETE)
    CHECK_INRANGE(clean, CLEANUP_NONE, CLEANUP_TIME)
    if (interval < 0 || num <= 0)
    {
        return false;
    }
    if (clean == CLEANUP_TIME && segment.segment.entries_.empty())
    {
        return false;
    }
    return true;
}

} // namespace task

#endif // TASK_TASK_LIMIT_H_
