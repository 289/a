#include "logic_charge.h"
#include "obj_interface.h"
#include "effect_logic.h"
#include "effect_templ.h"
#include "skill_def.h"
#include "buff.h"

namespace skill
{

bool LogicCharge::RoundEnd(InnerMsg& msg) const
{
	Buff* buff = static_cast<Buff*>(msg.param);
    if (buff->duration_ == 0)
    {
        trigger_->SetCharging(false);
        trigger_->SetCharged(false);
    }
    else if (buff->duration_ == 1)
    {
        trigger_->SetCharging(false);
        trigger_->SetCharged(true);
    }
    else if (buff->duration_ == effect_->buff.time - 1)
    {
        trigger_->SetCharging(true);
        trigger_->SetCharged(false);
    }
    return false;
}

} // namespace skill
