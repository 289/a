#include "logic_change_prop.h"
#include "obj_interface.h"
#include "effect_templ.h"
#include "effect_logic.h"
#include "skill_expr.h"
#include "skill_def.h"
#include "damage_msg.h"
#include "buff.h"
#include "player_util.h"

namespace skill
{

using namespace std;

static void FillDmg(Player* caster, const EffectLogic* logic, float factor, int32_t defenders, Damage& dmg)
{
	dmg.type = CHANGE_POINT;
	dmg.prop = atoi(logic->params[0].c_str());
	dmg.value = (int32_t)(SkillExpr(logic->params[1], caster).Calculate()) * factor;
	// 是否需要均摊
	if (logic->params.size() == 3)
	{
		int32_t max_dmg = SkillExpr(logic->params[2], caster).Calculate();
		dmg.value /= defenders;
		if (dmg.value > max_dmg)
		{
			dmg.value = max_dmg;
		}
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

bool LogicChangeProp::Cast(InnerMsg& msg) const
{
	CastParam* param = static_cast<CastParam*>(msg.param);
	EffectDamage* effect_dmg = param->dmg;
	effect_dmg->dmgs.push_back(Damage());
	Damage& dmg = effect_dmg->dmgs[effect_dmg->dmgs.size() - 1];
	FillDmg(caster_, logic_, param->factor, param->defenders, dmg);
	Revise(param, dmg);
	return true;
}

// 当为节点类Buff时，只在生效时对外产生效果
bool LogicChangeProp::RoundStart(InnerMsg& msg) const
{
	BuffDamageVec* buffdmg_vec = msg.buffdmg_vec;
	buffdmg_vec->push_back(BuffDamage());
	BuffDamage& buff_dmg = (*buffdmg_vec)[buffdmg_vec->size() - 1];

	Buff* buff = static_cast<Buff*>(msg.param);
	buff_dmg.buff_sn = buff->sn_;
	buff_dmg.effectid = effect_->templ_id;
	buff_dmg.attacker = caster_->GetId();
	buff_dmg.defender = trigger_->GetId();
	buff_dmg.dmgs.push_back(Damage());
	Damage& dmg = buff_dmg.dmgs[buff_dmg.dmgs.size() - 1];
	FillDmg(caster_, logic_, 1.0, 1, dmg);
	return true;
}

bool LogicChangeProp::RoundEnd(InnerMsg& msg) const
{
    return RoundStart(msg);
}

bool LogicChangeProp::Attach(InnerMsg& msg)
{
	Damage dmg;
	FillDmg(caster_, logic_, 1.0, 1, dmg);
	PlayerUtil::ChangeProp(caster_, trigger_, dmg);
	return false;
}

bool LogicChangeProp::Enhance(InnerMsg& msg)
{
	Damage dmg;
	FillDmg(caster_, logic_, 1.0, 1, dmg);
	PlayerUtil::ChangeProp(caster_, trigger_, dmg);
	return false;
}

bool LogicChangeProp::Detach(InnerMsg& msg)
{
	Damage dmg;
	FillDmg(caster_, logic_, -1.0, 1, dmg);
	PlayerUtil::ChangeProp(caster_, trigger_, dmg);
	return false;
}

} // namespace skill
