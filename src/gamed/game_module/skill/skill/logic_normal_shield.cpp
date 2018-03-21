#include "logic_normal_shield.h"
#include "obj_interface.h"
#include "effect_templ.h"
#include "effect_logic.h"
#include "skill_def.h"
#include "skill_expr.h"
#include "buff.h"

namespace skill
{

bool LogicNormalShield::Init(InnerMsg& msg) const
{
	Buff* buff = static_cast<Buff*>(msg.param);
	SkillExpr exp(logic_->params[1], caster_);
	int32_t shield_hp = exp.Calculate();
	if (buff->hp_ < shield_hp)
	{
		buff->hp_ = shield_hp;
	}
	return false;
}

// 是否可以吸收伤害
static bool CanShield(int32_t shield, int32_t hp, Damage& dmg)
{
    if (hp <= 0 || dmg.prop != PROP_INDEX_HP || dmg.value >= 0 || dmg.type > SHIELD_VALUE)
    {
        return false;
    }
    if (shield == SHIELD_VALUE)
    {
        return true;
    }
    if (dmg.type == ATTACK_VALUE)
    {
        return false;
    }
    return dmg.type == shield;
	//return (hp > 0 && dmg.prop == PROP_INDEX_HP && dmg.value < 0 && (dmg.type & shield) != 0);
}

bool LogicNormalShield::NormalShield(InnerMsg& msg) const
{
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
    Player* attacker = param->attacker;

	BuffDamage buff_dmg;
	buff_dmg.effectid = param->effect->templ_id;
	buff_dmg.attacker = attacker == NULL ? 0 : attacker->GetId();
	buff_dmg.defender = param->buff->owner_->GetId();
	buff_dmg.buff_sn = param->buff->sn_;

	int32_t& hp = param->buff->hp_;
	int8_t shield_type = atoi(logic_->params[0].c_str());
	float prob = atoi(logic_->params[2].c_str()) / 10000.0;
	DamageVec::iterator dit = param->dmg_vec->begin();
	for (; dit != param->dmg_vec->end() && hp > 0; ++dit)
	{
		if (!CanShield(shield_type, hp, *dit))
		{
			continue;
		}

		int32_t shield_dmg = dit->value * prob;
		shield_dmg = (shield_dmg + hp)  > 0 ? shield_dmg : -1 * hp;
		hp += shield_dmg;
		dit->value -= shield_dmg;

		buff_dmg.dmgs.push_back(Damage());
		Damage& new_dmg = buff_dmg.dmgs[buff_dmg.dmgs.size() - 1];
		new_dmg.type = DAMAGE_SHIELD;
		new_dmg.prop = PROP_INDEX_HP;
		new_dmg.value = shield_dmg;
	}

	if (!buff_dmg.dmgs.empty())
	{
		msg.buffdmg_vec->push_back(buff_dmg);
		return true;
	}
	return false;
}

} // namespace skill
