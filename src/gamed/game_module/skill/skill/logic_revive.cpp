#include "logic_revive.h"
#include "effect_logic.h"
#include "obj_interface.h"
#include "skill_expr.h"
#include "skill_def.h"
#include "damage_msg.h"

namespace skill
{

void LogicRevive::FillDmg(Damage& dmg) const
{
	dmg.prop = PROP_INDEX_HP;
	if (trigger_->IsDead() || trigger_->IsDying())
	{
		dmg.type = CHANGE_SCALE;
		dmg.value = atoi(logic_->params[0].c_str());
	}
	else
	{
		dmg.type = CHANGE_POINT;
		dmg.value = SkillExpr(logic_->params[1], caster_).Calculate();
	}
}

static void Revise(CastParam* param, Damage& dmg)
{
	if (dmg.prop != PROP_INDEX_HP || dmg.value <= 0 || dmg.type != CHANGE_POINT)
	{
		return;
	}
	int32_t scale = 1 + param->revise[REVISE_HEAL].scale;
	if (scale <= 0)
	{
		scale = 0;
	}
	int32_t point = param->revise[REVISE_HEAL].point;
	dmg.value = dmg.value * scale + point;
	if (dmg.value <= 0)
	{
		dmg.value = 0;
	}
}

bool LogicRevive::Cast(InnerMsg& msg) const
{
	CastParam* param = static_cast<CastParam*>(msg.param);
	EffectDamage* effect_dmg = param->dmg;
	effect_dmg->dmgs.push_back(Damage());
	Damage& dmg = effect_dmg->dmgs[effect_dmg->dmgs.size() - 1];
	FillDmg(dmg);
	Revise(param, dmg);
	return true;
}

} // namespace skill
