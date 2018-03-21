#ifndef SKILL_LOGIC_REVISE_H_
#define SKILL_LOGIC_REVISE_H_

#include "logic_if.h"

namespace skill
{

// 修正逻辑
class LogicRevise : public LogicIf
{
public:
	virtual bool ConsumeRevise(InnerMsg& msg) const;
	virtual bool CastRevise(InnerMsg& msg) const;
	virtual bool AttackedRevise(InnerMsg& msg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_REVISE_H_
