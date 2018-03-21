#ifndef SKILL_LOGIC_POWER_GOLEM_H_
#define SKILL_LOGIC_POWER_GOLEM_H_

#include "logic_if.h"

namespace skill
{

// 恢复魔偶能量
class LogicPowerGolem : public LogicIf
{
public:
	virtual bool Cast(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_POWER_GOLEM_H_
