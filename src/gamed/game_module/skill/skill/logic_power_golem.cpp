#include "logic_power_golem.h"
#include "obj_interface.h"
#include "skill_expr.h"
#include "effect_logic.h"

namespace skill
{

bool LogicPowerGolem::Cast(InnerMsg& msg) const
{
	int32_t value = SkillExpr(logic_->params[0], caster_).Calculate();
	if (value > 0)
	{
		caster_->PlayerPowerGolem(value);
	}
	return true;
}

} // namespace skill
