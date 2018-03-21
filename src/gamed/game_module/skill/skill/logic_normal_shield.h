#ifndef SKILL_LOGIC_NORMAL_SHIELD_H_
#define SKILL_LOGIC_NORMAL_SHIELD_H_

#include "logic_if.h"

namespace skill
{

// 吸收伤害护盾
class LogicNormalShield : public LogicIf
{
public:
	virtual bool Init(InnerMsg& msg) const;
	virtual bool NormalShield(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_NORMAL_SHIELD_H_
