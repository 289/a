#ifndef SKILL_LOGIC_CONTROL_H_
#define SKILL_LOGIC_CONTROL_H_

#include "logic_if.h"

namespace skill
{

// 控制逻辑
class LogicControl : public LogicIf
{
public:
	virtual bool Attach(InnerMsg& msg);
	virtual bool Detach(InnerMsg& msg);
};

} // namespace skill

#endif // SKILL_LOGIC_CONTROL_H_
