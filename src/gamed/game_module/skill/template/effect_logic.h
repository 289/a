#ifndef SKILL_DATATEMPL_EFFECT_LOGIC_H_
#define SKILL_DATATEMPL_EFFECT_LOGIC_H_

#include "skill_types.h"

namespace skill
{

class EffectLogic
{
public:
	EffectLogic()
		: type(LOGIC_BASIC_ATTACK)
	{
	}

	inline bool CheckDataValidity() const;
	int8_t type;
	typedef std::vector<std::string> ParamVec;
	ParamVec params; // 个数以及值类型由type决定
	NESTED_DEFINE(type, params);
};

inline bool EffectLogic::CheckDataValidity() const
{
	return type >= LOGIC_BASIC_ATTACK && type < LOGIC_MAX;
}
typedef std::vector<EffectLogic> LogicVec;

} // namespace skill

#endif // SKILL_DATATEMPL_EFFECT_LOGIC_H_
