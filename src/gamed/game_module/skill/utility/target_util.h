#ifndef SKILL_TARGET_UTIL_H_
#define SKILL_TARGET_UTIL_H_

#include "skill_types.h"

namespace skill
{

class SkillTarget;

// 技能目标选择
class TargetUtil
{
public:
	static int8_t GetCastPos(Player* caster, int8_t team, const SkillTarget& target, const PlayerVec& players);
private:
	static int8_t Appoint(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t Random(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t Sequence(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t HP(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t Line(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t Class(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t Position(int8_t team, const ParamVec& params, const PlayerVec& players);
	static int8_t Dead(int8_t team, const ParamVec& params, const PlayerVec& players);
};

} // namespace skill

#endif // SKILL_TARGET_UTIL_H_
