#ifndef SKILL_LOGIC_SUMMON_GOLEM_H_
#define SKILL_LOGIC_SUMMON_GOLEM_H_

#include "logic_if.h"

namespace skill
{

// 召唤魔偶
class LogicSummonGolem : public LogicIf
{
public:
	virtual bool Attach(InnerMsg& msg);
};

} // namespace skill

#endif // SKILL_LOGIC_SUMMON_GOLEM_H_
