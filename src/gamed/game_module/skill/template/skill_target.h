#ifndef SKILL_DATATEMPL_SKILL_TARGET_H_
#define SKILL_DATATEMPL_SKILL_TARGET_H_

#include "skill_types.h"

namespace skill
{

class SkillTarget
{
public:
	SkillTarget()
		: type(TARGET_SELF)
	{
	}

	bool CheckDataValidity() const;
	int8_t type;
	ParamVec params; // 大小跟值类型由type决定
	NESTED_DEFINE(type, params);
};

} // namespace skill

#endif // SKILL_DATATEMPL_SKILL_TARGET_H_
