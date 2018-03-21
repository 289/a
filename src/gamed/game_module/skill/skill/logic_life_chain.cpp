#include "logic_life_chain.h"
#include "obj_interface.h"
#include "buff_wrapper.h"
#include "effect_templ.h"
#include "effect_logic.h"
#include "skill_def.h"
#include "buff.h"

namespace skill
{

static bool CanChain(Damage& dmg)
{
	return dmg.type != DAMAGE_SHIELD && dmg.prop == PROP_INDEX_HP && dmg.value < 0;
}

bool LogicLifeChain::LifeChain(InnerMsg& msg) const
{
	// 攻击无来源，则不触发生命链接
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	if (param->attacker == NULL)
	{
		return false;
	}

	BuffDamage buff_dmg;
	buff_dmg.effectid = param->effect->templ_id;
	buff_dmg.attacker = 0;
	buff_dmg.buff_sn = param->buff->sn_;

	float prob = atoi(logic_->params[0].c_str()) / 10000.0;
	DamageVec::iterator dit = param->dmg_vec->begin();
	for (; dit != param->dmg_vec->end(); ++dit)
	{
		if (!CanChain(*dit))
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

	bool active = false;
	PlayerVec enemies, chain;
	trigger_->GetEnemy(enemies);
	PlayerVec::iterator it = enemies.begin();
	for (; it != enemies.end(); ++it)
	{
		if (*it == NULL)
		{
			continue;
		}

		BuffWrapper* buff = (*it)->GetBuff();
		if (!buff->InLifeChain(msg))
		{
			continue;
		}
		buff_dmg.defender = (*it)->GetId();
		msg.buffdmg_vec->push_back(buff_dmg);
		active = true;
	}
	return active;
}

} // namespace skill
