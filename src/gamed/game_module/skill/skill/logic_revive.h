#ifndef SKILL_LOGIC_REVIVE_H_
#define SKILL_LOGIC_REVIVE_H_

#include "logic_if.h"

namespace skill
{

class Damage;

// 复活
class LogicRevive : public LogicIf
{
public:
	virtual bool Cast(InnerMsg& msg) const;
private:
	void FillDmg(Damage& dmg) const;
};

} // namespace skill

#endif // SKILL_LOGIC_REVIVE_H_
