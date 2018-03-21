#ifndef SKILL_LOGIC_PURIFY_H_
#define SKILL_LOGIC_PURIFY_H_

#include "logic_if.h"

namespace skill
{

// 净化
class LogicPurify : public LogicIf
{
public:
	virtual bool Purify(InnerMsg& msg);
};

} // namespace skill

#endif // SKILL_LOGIC_POWER_FLAG_H_
