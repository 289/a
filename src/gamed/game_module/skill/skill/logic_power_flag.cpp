#include "logic_power_flag.h"
#include "obj_interface.h"

namespace skill
{

bool LogicPowerFlag::Cast(InnerMsg& msg) const
{
	caster_->SetGenPowerFlag();
	return true;
}

} // namespace skill
