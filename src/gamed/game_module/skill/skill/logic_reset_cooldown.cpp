#include "logic_reset_cooldown.h"
#include "obj_interface.h"
#include "cooldown_wrapper.h"
#include "effect_logic.h"

namespace skill
{

bool LogicResetCooldown::Cast(InnerMsg& msg) const
{
	int32_t skill_group = atoi(logic_->params[0].c_str());
	CooldownWrapper* cooldown = caster_->GetCooldown();
	cooldown->AddResetGID(skill_group);
	return true;
}

} // namespace skill
