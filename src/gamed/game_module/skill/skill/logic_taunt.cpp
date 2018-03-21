#include "logic_taunt.h"
#include "obj_interface.h"
#include "skill_def.h"

namespace skill
{

using namespace std;

bool LogicTaunt::GetTarget(InnerMsg& msg) const
{
	PlayerVec* target = static_cast<PlayerVec*>(msg.param);
	target->push_back(caster_->GetOwner());
	return true;
}

} // namespace skill
