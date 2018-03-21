#ifndef SKILL_LOGIC_POWER_FLAG_H_
#define SKILL_LOGIC_POWER_FLAG_H_

#include "logic_if.h"

namespace skill
{

// 攻击战士回复能力标记效果逻辑
class LogicPowerFlag : public LogicIf
{
public:
	virtual bool Cast(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_POWER_FLAG_H_
