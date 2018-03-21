#ifndef SKILL_LOGIC_CHANGE_PROP_H_
#define SKILL_LOGIC_CHANGE_PROP_H_

#include "logic_if.h"

namespace skill
{

// 属性修改逻辑
class LogicChangeProp : public LogicIf
{
public:
	virtual bool Cast(InnerMsg& msg) const;
	virtual bool RoundStart(InnerMsg& msg) const;
	virtual bool RoundEnd(InnerMsg& msg) const;
	virtual bool Attach(InnerMsg& msg);
	virtual bool Enhance(InnerMsg& msg);
	virtual bool Detach(InnerMsg& msg);
};

} // namespace skill

#endif // SKILL_LOGIC_CHANGE_PROP_H_
