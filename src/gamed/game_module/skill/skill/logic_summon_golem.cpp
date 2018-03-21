#include "logic_summon_golem.h"
#include "obj_interface.h"
#include "effect_logic.h"

namespace skill
{

bool LogicSummonGolem::Attach(InnerMsg& msg)
{
	int32_t golem_id = atoi(logic_->params[0].c_str());
	trigger_->PlayerSummonGolem(golem_id);
	return false;
}

} // namespace skill
