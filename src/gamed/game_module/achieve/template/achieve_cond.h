#ifndef ACHIEVE_ACHIEVE_COND_H_
#define ACHIEVE_ACHIEVE_COND_H_

#include "achieve_types.h"

namespace achieve
{

class AchieveCond
{
public:
    AchieveCond()
        : op(ACHIEVE_OP_GREATER_EQUAL), value(0)
    {
    }

	inline bool CheckDataValidity() const;

    std::string data;
    int8_t op;
    int32_t value;
    std::string tips;

    NESTED_DEFINE(data, op, value, tips);
};
typedef std::vector<AchieveCond> AchieveCondVec;

inline bool AchieveCond::CheckDataValidity() const
{
    CHECK_INRANGE(op, ACHIEVE_OP_GREATER_EQUAL, ACHIEVE_OP_LESS_EQUAL)
    return value >= 0;
}

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_COND_H_
