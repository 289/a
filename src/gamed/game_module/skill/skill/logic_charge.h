#ifndef SKILL_LOGIC_CHARGE_H_
#define SKILL_LOGIC_CHARGE_H_

#include "logic_if.h"

namespace skill
{

// 蓄力逻辑
class LogicCharge : public LogicIf
{
public:
	virtual bool RoundEnd(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_CHARGE_H_
