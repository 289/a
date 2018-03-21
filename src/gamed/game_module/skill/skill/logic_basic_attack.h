#ifndef SKILL_LOGIC_BASIC_ATTACK_H_
#define SKILL_LOGIC_BASIC_ATTACK_H_

#include "logic_if.h"

namespace skill
{

class Damage;

// 基础伤害逻辑
class LogicBasicAttack : public LogicIf
{
public:
	virtual bool Cast(InnerMsg& msg) const;
	virtual bool RoundStart(InnerMsg& msg) const;
	virtual bool RoundEnd(InnerMsg& msg) const;
	virtual bool BeAttacked(InnerMsg& msg) const;
private:
	void FillDmg(float factor, int32_t defenders, Damage& dmg) const;
	//int32_t CalcDmg(bool physic, int32_t base_dmg) const;
	int32_t CalcDmg(int8_t dmg_type, int32_t base_dmg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_BASIC_ATTACK_H_
