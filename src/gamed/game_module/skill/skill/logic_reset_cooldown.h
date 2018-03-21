#ifndef SKILL_LOGIC_RESET_COOLDOWN_H_
#define SKILL_LOGIC_RESET_COOLDOWN_H_

#include "logic_if.h"

namespace skill
{

// 重置CD效果逻辑
class LogicResetCooldown : public LogicIf
{
public:
	virtual bool Cast(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_RESET_COOLDOWN_H_
