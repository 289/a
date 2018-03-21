#include <string.h>
#include <stdint.h>
#include "prop_policy.h"
#include "combat_unit.h"
#include "combat_def.h"
#include "data_templ/templ_manager.h"

namespace combat
{

/****************************PlayerPropRuler*************************/
/****************************PlayerPropRuler*************************/
/****************************PlayerPropRuler*************************/
/****************************PlayerPropRuler*************************/
PlayerPropRuler::PropRuleVec PlayerPropRuler::prop_rule_vec;

void PlayerPropRuler::InitClsPropSyncRule(const std::vector<cls_prop_sync_rule>& list)
{
	prop_rule_vec.resize(list.size());
	for (size_t i = 0; i < list.size(); ++ i)
	{
		prop_rule_vec[i].sync_rule[0] = list[i].hp_sync_rule;
		prop_rule_vec[i].sync_rule[1] = list[i].mp_sync_rule;
		prop_rule_vec[i].sync_rule[2] = list[i].ep_sync_rule;
		prop_rule_vec[i].power_gen_rule = list[i].power_gen_rule;
	}
}

int PlayerPropRuler::GetPlayerPropSyncRule(int cls, int prop_index)
{
	return prop_rule_vec[(int)cls].sync_rule[(int)prop_index];
}

int PlayerPropRuler::GetPlayerPowerGenRule(int cls)
{
	return prop_rule_vec[(int)cls].power_gen_rule;
}

int32_t PlayerPropRuler::UpdatePlayerProp(int ruler, int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
#define CASE_UPDATE_PLAYER_PROP(num, curval, old_maxval, new_maxval) \
		case num: \
		{ \
			rst = UpdatePropByRule_##num(curval, old_maxval, new_maxval); \
		} \
		break;

	int32_t rst = 0;
	switch (ruler)
	{
		CASE_UPDATE_PLAYER_PROP(1, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(2, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(3, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(4, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(5, curval, old_maxval, new_maxval);
		default:
		{
			assert(false);
		}
		break;
	};
	return rst;

#undef CASE_UPDATE_PLAYER_PROP
}

int32_t PlayerPropRuler::UpdatePropByRule_1(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	return new_maxval;
}

int32_t PlayerPropRuler::UpdatePropByRule_2(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t offset = new_maxval - old_maxval;
	if (offset == 0)
	{
		return curval;
	}

	if (offset > 0)
	{
		//最大值变大
		int32_t new_prop = curval + offset;
		if (new_prop > new_maxval) new_prop = new_maxval;
		return new_prop;
	}
	else
	{
		//最大值变小
		if (curval > new_maxval) return new_maxval;
		return curval;
	}
}

int32_t PlayerPropRuler::UpdatePropByRule_3(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t offset = new_maxval - old_maxval;
	if (offset == 0)
	{
		return curval;
	}

	if (offset > 0)
	{
		//最大值变大
		int32_t new_prop = curval + offset;
		if (new_prop > new_maxval) new_prop = new_maxval;
		return new_prop;
	}
	else
	{
		//最大值变小
		if (curval > new_maxval) return new_maxval;
		return curval;
	}
}

int32_t PlayerPropRuler::UpdatePropByRule_4(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	if (curval > new_maxval) return new_maxval;
	return curval;
}

int32_t PlayerPropRuler::UpdatePropByRule_5(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t offset = new_maxval - old_maxval;
	if (offset == 0)
	{
		return curval;
	}

	if (offset > 0)
	{
		//最大值变大
		int32_t new_prop = curval + offset;
		if (new_prop > new_maxval) new_prop = new_maxval;
		return new_prop;
	}
	else
	{
		//最大值变小
		if (curval > new_maxval) return new_maxval;
		return curval;
	}
}

int32_t PlayerPropRuler::GenPowerByRule_1()
{
	assert(false);
	return 0;
}

int32_t PlayerPropRuler::GenPowerByRule_2(int32_t dmg, int32_t max_hp, int32_t speed_gen_power)
{
	int32_t tmp = (speed_gen_power * 9 * dmg) / (float)max_hp;
	return tmp > speed_gen_power ? tmp : speed_gen_power;
}

int32_t PlayerPropRuler::GenPowerByRule_3(int32_t dmg, int32_t max_hp, int32_t speed_gen_power)
{
	int32_t tmp = (speed_gen_power * 25 * dmg) / (float)max_hp;
	return tmp > speed_gen_power ? tmp : speed_gen_power;
}

int32_t PlayerPropRuler::GenPowerByRule_4(int32_t speed_gen_power)
{
	return speed_gen_power;
}

int32_t PlayerPropRuler::GenPowerByRule_5(int32_t speed_gen_power)
{
	return speed_gen_power;
}

/**********************************PropPolicy***************************************/
/**********************************PropPolicy***************************************/
/**********************************PropPolicy***************************************/
/**********************************PropPolicy***************************************/
void PropPolicy::UpdateProperty(CombatUnit* obj)
{
	/**
	 * 这里必须先更新扩展属性,因为基本属性依赖于扩展属性
	 */
	assert(obj);

	ExtendProp old_props[PROP_INDEX_HIGHEST] = {0};
	memcpy(old_props, obj->cur_prop_, sizeof(ExtendProp) * PROP_INDEX_HIGHEST);

	UpdateExtendProp(obj);
	UpdateBasicProp(obj, old_props);
}

/**
 * @func  UpdatePower
 * @brief 恢复角色能量
 * @brief 初心者职业不恢复能量；
 * @brief 攻战类角色在攻击并且造成伤害时恢复能量；
 * @brief 防战类角色在被攻击时恢复能量；
 * @brief 法师在每个回合结束时恢复能量；
 * @brief 技师在每个回合结束时恢复能量；
 */
void PropPolicy::UpdatePower(CombatUnit* obj, int call_time, int32_t damage)
{
	if (!obj->IsPlayer())
		return;

	int rule = PlayerPropRuler::GetPlayerPowerGenRule(obj->GetCls());
	switch (rule)
	{
		case GEN_POWER_RULE_1:
		{
			//do nothing
		}
		break;
		case GEN_POWER_RULE_2:
		{
			if (call_time == CT_ATTACK)
			{
				int32_t power_gen = PlayerPropRuler::GenPowerByRule_2(damage, obj->GetMaxHP(), obj->GetProp(PROP_INDEX_POWER_GEN_SPEED));
				obj->IncMP(PROP_INDEX_MP, power_gen);
			}
		}
		break;
		case GEN_POWER_RULE_3:
		{
			if (call_time == CT_DAMAGED)
			{
				int32_t power_gen = PlayerPropRuler::GenPowerByRule_3(damage, obj->GetMaxHP(), obj->GetProp(PROP_INDEX_POWER_GEN_SPEED));
				obj->IncMP(PROP_INDEX_MP, power_gen);
			}
		}
		break;
		case GEN_POWER_RULE_4:
		{
			if (call_time == CT_ROUND_END)
			{
				int32_t power_gen = PlayerPropRuler::GenPowerByRule_4(obj->GetProp(PROP_INDEX_POWER_GEN_SPEED));
				obj->IncMP(PROP_INDEX_MP, power_gen);
			}
		}
		break;
		case GEN_POWER_RULE_5:
		{
			if (call_time == CT_ROUND_END)
			{
				int32_t power_gen = PlayerPropRuler::GenPowerByRule_5(obj->GetProp(PROP_INDEX_POWER_GEN_SPEED));
				obj->IncMP(PROP_INDEX_MP, power_gen);
			}
		}
		break;
		default:
			assert(false);
		break;
	};
}

void PropPolicy::UpdateBasicProp(CombatUnit* obj, const ExtendProp* old_props)
{
	if (!(obj->GetRefreshExtPropMask() & MUTABLE_EXTPROP_MASK))
	{
		return;
	}

	if (obj->IsPlayer())
	{
		UpdatePlayerBasicProp(obj, old_props);
	}
	else if (obj->IsMob() || obj->IsTeamNpc())
	{
		UpdateMobBasicProp(obj);
	}
	else
	{
		assert(false);
	}
}

void PropPolicy::UpdateExtendProp(CombatUnit* obj)
{
	int32_t (&base_prop)[PROP_INDEX_HIGHEST] = obj->base_prop_;
	int32_t (&cur_prop) [PROP_INDEX_HIGHEST] = obj->cur_prop_;
	int32_t (&enh_point)[PROP_INDEX_HIGHEST] = obj->enh_point_;
	int32_t (&enh_scale)[PROP_INDEX_HIGHEST] = obj->enh_scale_;

	int32_t mask = obj->GetRefreshExtPropMask();
	int32_t index = 0;
	while (mask)
	{
		while (index < PROP_INDEX_HIGHEST)
		{
			if (mask & (1 << index))
				break;
			else
				++ index;
		}

		//这里必须检查index值
		//否则将出现写内存越界
		if (index >= PROP_INDEX_HIGHEST)
		{
			break;
		}

		assert(index < PROP_INDEX_HIGHEST);

		//更新属性
		cur_prop[index] = 0;
		cur_prop[index] += base_prop[index];
		cur_prop[index] *= 0.0001 * (10000 + enh_scale[index]);
		cur_prop[index] += enh_point[index];

		if (cur_prop[index] < 0)
		{
			//纠正负值
            if (index >= PROP_INDEX_HP)
            {
			    cur_prop[index] = 0;
            }
            else
            {
			    cur_prop[index] = 1;
            }
		}

		//消去最低位1
		mask &= mask-1;
	};
}

void PropPolicy::UpdateMobBasicProp(CombatUnit* obj)
{
	int32_t (&cur_prop)[PROP_INDEX_HIGHEST] = obj->cur_prop_;
	if (obj->GetHP() > cur_prop[PROP_INDEX_MAX_HP]) obj->SetHP(cur_prop[PROP_INDEX_MAX_HP]);
	if (obj->GetMP() > cur_prop[PROP_INDEX_MAX_MP]) obj->SetMP(cur_prop[PROP_INDEX_MAX_MP]);
	if (obj->GetEP() > cur_prop[PROP_INDEX_MAX_EP]) obj->SetEP(cur_prop[PROP_INDEX_MAX_EP]);
}

void PropPolicy::UpdatePlayerBasicProp(CombatUnit* obj, const ExtendProp* old_props)
{
	int cls          = obj->GetCls();
	int hp_sync_rule = PlayerPropRuler::GetPlayerPropSyncRule(cls, 0);
	int mp_sync_rule = PlayerPropRuler::GetPlayerPropSyncRule(cls, 1);
	int ep_sync_rule = PlayerPropRuler::GetPlayerPropSyncRule(cls, 2);

	int32_t new_hp = PlayerPropRuler::UpdatePlayerProp(hp_sync_rule, obj->GetHP(), old_props[PROP_INDEX_MAX_HP], obj->GetMaxHP());
	int32_t new_mp = PlayerPropRuler::UpdatePlayerProp(mp_sync_rule, obj->GetMP(), old_props[PROP_INDEX_MAX_MP], obj->GetMaxMP());
	int32_t new_ep = PlayerPropRuler::UpdatePlayerProp(ep_sync_rule, obj->GetEP(), old_props[PROP_INDEX_MAX_EP], obj->GetMaxEP());

	obj->SetHP(new_hp);
	obj->SetMP(new_mp);
	obj->SetEP(new_ep);
}

}; // namespace combat
