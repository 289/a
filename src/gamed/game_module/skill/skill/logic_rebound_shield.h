#ifndef SKILL_LOGIC_REBOUND_SHIELD_H_
#define SKILL_LOGIC_REBOUND_SHIELD_H_

#include "logic_if.h"

namespace skill
{

// 反弹伤害护盾
class LogicReboundShield : public LogicIf
{
public:
	virtual bool ReboundShield(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_REBOUND_SHIELD_H_
