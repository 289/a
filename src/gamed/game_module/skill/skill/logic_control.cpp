#include "logic_control.h"
#include "obj_interface.h"
#include "effect_logic.h"

namespace skill
{

// 设置控制状态
static void SetStatus(Player* player, int8_t type, bool attach)
{
	switch (type)
	{
	case CONTROL_DIZZY:
		return player->SetDizzy(attach);
	case CONTROL_SILENCE:
		return player->SetSilent(attach);
	case CONTROL_SEAL:
		return player->SetSeal(attach);
	case CONTROL_STONE:
		return player->SetStone(attach);
	default:
		return;
	}
}

bool LogicControl::Attach(InnerMsg& msg)
{
	int8_t type = atoi(logic_->params[0].c_str());
	SetStatus(trigger_, type, true);
	return false;
}

bool LogicControl::Detach(InnerMsg& msg)
{
	int8_t type = atoi(logic_->params[0].c_str());
	SetStatus(trigger_, type, false);
	return false;
}

} // namespace skill
