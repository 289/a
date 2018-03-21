#ifndef SKILL_LOGIC_TRIGGER_EFFECT_H_
#define SKILL_LOGIC_TRIGGER_EFFECT_H_

#include "logic_if.h"

namespace skill
{

// 触发效果
class LogicTriggerEffect : public LogicIf
{
public:
	virtual bool RoundStart(InnerMsg& msg) const;
	virtual bool RoundEnd(InnerMsg& msg) const;
private:
	bool Trigger(InnerMsg& msg, int32_t point) const;
};

} // namespace skill

#endif // SKILL_LOGIC_TRIGGER_EFFECT_H_
