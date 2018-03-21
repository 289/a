#ifndef SKILL_DATATEMPL_HIT_RATE_H_
#define SKILL_DATATEMPL_HIT_RATE_H_

#include "skill_types.h"

namespace skill
{

// 技能命中
class HitRate
{
public:
	HitRate() 
		: type(HIT_REVISE), rate(0)
	{
	}

	inline bool CheckDataValidity() const;
	int8_t type;
	int32_t rate;
	NESTED_DEFINE(type, rate);
};

inline bool HitRate::CheckDataValidity() const
{
	CHECK_INRANGE(type, HIT_REVISE, HIT_ASSIGN)
	CHECK_INRANGE(rate, 0, 10000)
	return true;
}

} // namespace skill

#endif // SKILL_DATATEMPL_HIT_RATE_H_
