#ifndef SKILL_RANGE_UTIL_H_
#define SKILL_RANGE_UTIL_H_

#include "skill_types.h"

namespace skill
{

class SkillTempl;

// 效果目标选择
class RangeUtil
{
public:
	static void GetAttackRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target);
private:
	static void SingleRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target);
	static void RangeRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target);
	static void AllRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target);
	static void LineRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target);
	static void ChainRange(const SkillTempl* templ, int8_t pos, PlayerVec& players, PlayerVec& target);
};

} // namespace skill

#endif // SKILL_RANGE_UTIL_H_
