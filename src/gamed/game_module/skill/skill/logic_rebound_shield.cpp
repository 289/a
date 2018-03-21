#include "logic_rebound_shield.h"
#include "obj_interface.h"
#include "effect_templ.h"
#include "effect_logic.h"
#include "skill_def.h"
#include "buff.h"

namespace skill
{

static bool CanRebound(Damage& dmg)
{
	return dmg.type != DAMAGE_SHIELD && dmg.prop == PROP_INDEX_HP && dmg.value < 0;
}

bool LogicReboundShield::ReboundShield(InnerMsg& msg) const
{
	// 如果攻击无来源，则不反弹
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	if (param->attacker == NULL)
	{
		return false;
	}

	BuffDamage buff_dmg;
	buff_dmg.effectid = param->effect->templ_id;
	buff_dmg.attacker = 0;
    // 如果攻击者为魔偶，伤害反弹给主人
	buff_dmg.defender = param->buff->owner_->GetId();
	buff_dmg.buff_sn = param->buff->sn_;

	float prob = atoi(logic_->params[0].c_str()) / 10000.0;
	DamageVec::iterator dit = param->dmg_vec->begin();
	for (; dit != param->dmg_vec->end(); ++dit)
	{
		if (!CanRebound(*dit))
		{
			continue;
		}

		buff_dmg.dmgs.push_back(Damage());
		Damage& new_dmg = buff_dmg.dmgs[buff_dmg.dmgs.size() - 1];
		new_dmg.type = ATTACK_VALUE;
		new_dmg.prop = PROP_INDEX_HP;		
		int32_t dmg_value = dit->value * prob;
		new_dmg.value = dmg_value != 0 ? dmg_value : -1;
	}

	if (!buff_dmg.dmgs.empty())
	{
		msg.buffdmg_vec->push_back(buff_dmg);
		return true;
	}
	return false;
}

} // namespace skill
