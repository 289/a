#ifndef SKILL_DATATEMPL_RECAST_H_
#define SKILL_DATATEMPL_RECAST_H_

#include "skill_types.h"

namespace skill
{

class ReCast
{
public:
	ReCast()
		: type(RECAST_NONE), prob(0), skillid(0)
	{
	}

	inline bool CheckDataValidity() const;
	uint8_t type;
	int32_t prob;
	SkillID skillid;
	NESTED_DEFINE(type, prob, skillid);
};

inline bool ReCast::CheckDataValidity() const
{
	CHECK_INRANGE(type, RECAST_NONE, RECAST_CRIT)
	CHECK_INRANGE(prob, 0, 10000)

    // 获取比特位中1的个数
	uint32_t count1 = type - ((type >> 1) & 033333333333) - ((type >> 2) & 011111111111);
	count1 =  ((count1 + (count1 >> 3)) & 030707070707) % 63;

	if (count1 != 0 && count1 != 1)
	{
		return false;
	}
	return skillid >= 0;
}

} // namespace skill

#endif // SKILL_DATATEMPL_RECAST_H_
