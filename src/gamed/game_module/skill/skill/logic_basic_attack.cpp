#include "logic_basic_attack.h"
#include "obj_interface.h"
#include "effect_templ.h"
#include "effect_logic.h"
#include "skill_expr.h"
#include "skill_def.h"
#include "damage_msg.h"
#include "buff.h"

namespace skill
{

using namespace std;

static int32_t GetDefence(Player* player, int8_t dmg_type)
{
    switch (dmg_type)
    {
    case ATTACK_PHYSICAL:
        return player->GetProperty(PROP_INDEX_MAX_PHYSICAL_DEFENCE);
    case ATTACK_MAGIC:
        return player->GetProperty(PROP_INDEX_MAX_MAGIC_DEFENCE);
    case ATTACK_THIRD:
        return player->GetProperty(PROP_INDEX_MAX_MAGIC_ATTACK);
    default:
        return 0;
    }
}

int32_t LogicBasicAttack::CalcDmg(int8_t dmg_type, int32_t base_dmg) const
{
    base_dmg -= GetDefence(trigger_, dmg_type);
	return base_dmg < 1 ? 1 : base_dmg;
}

void LogicBasicAttack::FillDmg(float factor, int32_t defenders, Damage& dmg) const
{
	dmg.prop = PROP_INDEX_HP;
	dmg.type = atoi(logic_->params[0].c_str());
	// 计算攻击者的输出伤害
	dmg.value = (int32_t)(SkillExpr(logic_->params[1], caster_).Calculate()) * factor;

	// 检查是否需要进行输出伤害均摊
	if (logic_->params.size() == 3)
	{
		int32_t max_dmg = SkillExpr(logic_->params[2], caster_).Calculate();
		dmg.value /= defenders;
		if (dmg.value > max_dmg)
		{
			dmg.value = max_dmg;
		}
	}

	// 套伤害公式计算实际的伤害值
	if (dmg.type != ATTACK_VALUE)
	{
		dmg.value = CalcDmg(dmg.type, dmg.value);
	}
}

static void Revise(Player* caster, Player* trigger, CastParam* param, Damage& dmg)
{
    int32_t ppierce = caster->GetProperty(PROP_INDEX_MAX_PHYSICAL_PIERCE);
    int32_t mpierce = trigger->GetProperty(PROP_INDEX_MAX_MAGIC_PIERCE);
    float scale = 1;
    scale += 0.75 * (ppierce - mpierce) / (ppierce + mpierce + 100);  // 伤害修正
    scale += param->revise[REVISE_ATTACK].scale / 10000.0;          // 技能比例修正
	if (scale < 0)
	{
		scale = 0;
	}

	int32_t point = param->revise[REVISE_ATTACK].point;
	float crit_scale = 0.5 + param->revise[REVISE_CRIT].scale / 10000.0;
	if (scale <= 0 || !param->crit || crit_scale <= 0)
	{
		crit_scale = 0;
	}

	dmg.value = dmg.value * (scale + crit_scale) + point;
	if (dmg.value <= 0)
	{
		dmg.value = 1;
	}
}
/*
int32_t LogicBasicAttack::CalcDmg(bool physic, int32_t base_dmg) const
{
	int32_t level = trigger_->GetLevel();
	int8_t index = PROP_INDEX_MAX_PHYSICAL_DEFENCE;
	int8_t defence_index = physic ? index : index + 1;
	int8_t pierce_index = defence_index + 2;
	int32_t defence = trigger_->GetProperty(defence_index);
	int32_t pierce = caster_->GetProperty(pierce_index);
	float rate = (float)(1 - pierce / (1000.0 + level * 100));
	rate = rate < 0 ? 0 : rate;
	base_dmg -= (int32_t)(defence * rate);
	return base_dmg < 1 ? 1 : base_dmg;
}

void LogicBasicAttack::FillDmg(float factor, int32_t defenders, Damage& dmg) const
{
	dmg.prop = PROP_INDEX_HP;
	dmg.type = atoi(logic_->params[0].c_str());
	// 计算攻击者的输出伤害
	dmg.value = (int32_t)(SkillExpr(logic_->params[1], caster_).Calculate()) * factor;

	// 检查是否需要进行输出伤害均摊
	if (logic_->params.size() == 3)
	{
		int32_t max_dmg = SkillExpr(logic_->params[2], caster_).Calculate();
		dmg.value /= defenders;
		if (dmg.value > max_dmg)
		{
			dmg.value = max_dmg;
		}
	}

	// 套伤害公式计算实际的伤害值
	if (dmg.type != ATTACK_VALUE)
	{
		dmg.value = CalcDmg(dmg.type == ATTACK_PHYSICAL, dmg.value);
	}
}

static void Revise(CastParam* param, Damage& dmg)
{
	int32_t scale = 1 + param->revise[REVISE_ATTACK].scale / 10000.0;
	if (scale < 0)
	{
		scale = 0;
	}

	int32_t point = param->revise[REVISE_ATTACK].point;
	int32_t crit_scale = 1 + param->revise[REVISE_CRIT].scale / 10000.0;
	if (scale <= 0 || !param->crit || crit_scale <= 0)
	{
		crit_scale = 0;
	}

	dmg.value = dmg.value *(scale + crit_scale) + point;
	if (dmg.value <= 0)
	{
		dmg.value = 1;
	}
}
*/
bool LogicBasicAttack::Cast(InnerMsg& msg) const
{
	CastParam* param = static_cast<CastParam*>(msg.param);
	EffectDamage* effect_dmg = param->dmg;
	if (param->crit)
	{
		effect_dmg->status |= EFFECT_CRIT;
	}
	effect_dmg->dmgs.push_back(Damage());
	Damage& dmg = effect_dmg->dmgs[effect_dmg->dmgs.size() - 1];
	FillDmg(param->factor, param->defenders, dmg);
	Revise(caster_, trigger_, param, dmg);
	dmg.value *= -1;
	return true;
}

bool LogicBasicAttack::RoundStart(InnerMsg& msg) const
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
	FillDmg(1.0, 1, dmg);
	dmg.value /= -1 * effect_->buff.time;
	if (dmg.value == 0)
	{
		dmg.value = -1;
	}
	return true;
}

bool LogicBasicAttack::RoundEnd(InnerMsg& msg) const
{
	return RoundStart(msg);
}

bool LogicBasicAttack::BeAttacked(InnerMsg& msg) const
{
	// 攻击无来源，则不触发
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	if (param->attacker == NULL)
	{
		return false;
	}

	BuffDamageVec* buffdmg_vec = msg.buffdmg_vec;
	buffdmg_vec->push_back(BuffDamage());
	BuffDamage& buff_dmg = (*buffdmg_vec)[buffdmg_vec->size() - 1];

	Buff* buff = static_cast<Buff*>(msg.param);
	buff_dmg.buff_sn = buff->sn_;
	buff_dmg.effectid = effect_->templ_id;
	buff_dmg.attacker = 0;
	buff_dmg.defender = trigger_->GetId();
	buff_dmg.dmgs.push_back(Damage());
	Damage& dmg = buff_dmg.dmgs[buff_dmg.dmgs.size() - 1];
	FillDmg(1.0, 1, dmg);
	dmg.value /= -1 * effect_->buff.time;
	if (dmg.value == 0)
	{
		dmg.value = -1;
	}
	return true;
}

} // namespace skill
