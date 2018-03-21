#ifndef SKILL_LOGIC_TAUNT_H_
#define SKILL_LOGIC_TAUNT_H_

#include "logic_if.h"

namespace skill
{

// 嘲讽逻辑
class LogicTaunt : public LogicIf
{
public:
	virtual bool GetTarget(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_TAUNT_H_
