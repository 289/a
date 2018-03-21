#ifndef ACHIEVE_ACHIEVE_PREMISE_H_
#define ACHIEVE_ACHIEVE_PREMISE_H_

#include "achieve_cond.h"

namespace achieve
{

class AchievePremise
{
public:
	inline bool CheckDataValidity() const;

    AchieveID premise;
    int8_t needed_cond;
    AchieveCondVec cond;

    NESTED_DEFINE(premise, needed_cond, cond);
};

// 内联函数
inline bool AchievePremise::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(cond)
    return premise >= 0 && needed_cond >= 1;
}

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_PREMISE_H_
