#ifndef SKILL_LOGIC_LIFE_CHAIN_H_
#define SKILL_LOGIC_LIFE_CHAIN_H_

#include "logic_if.h"

namespace skill
{

// 生命链接
class LogicLifeChain : public LogicIf
{
public:
	virtual bool LifeChain(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_LIFE_CHAIN_H_
