#ifndef SKILL_DATATEMPL_SKILL_RANGE_H_
#define SKILL_DATATEMPL_SKILL_RANGE_H_

#include "skill_types.h"

namespace skill
{

class SkillRange
{
public:
	SkillRange()
		: type(RANGE_SINGLE), num(0), factor(10000), max_selected(0), chain_param(0)
	{
	}

	inline bool CheckDataValidity() const;
	int8_t type;
	int32_t num;
	int32_t factor;
	int32_t max_selected; // type为链式时，单个目标可以被选中的次数上限
    int32_t chain_param; // type为链式时才有效
	NESTED_DEFINE(type, num, factor, max_selected, chain_param);
};

inline bool SkillRange::CheckDataValidity() const
{
	CHECK_INRANGE(type, RANGE_SINGLE, RANGE_BULLET_LINE)
	return num >= 0 && factor >= 0 && max_selected >= 0;
}

} // namespace skill

#endif // SKILL_DATATEMPL_SKILL_RANGE_H_
