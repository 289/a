#include "combat_def.h"
#include "extra_templ/battle_scene.h"
#include "extra_templ/extratempl_man.h"
#include "data_templ/templ_manager.h"
#include "data_templ/global_config.h"
#include "data_templ/skill_tree_templ.h"


namespace combat
{

bool __COMBAT_PRINT_FLAG = true;

/****************************CombatAwardConfig*************************/
/****************************CombatAwardConfig*************************/
/****************************CombatAwardConfig*************************/
/****************************CombatAwardConfig*************************/
CombatAwardConfig::FactorVec CombatAwardConfig::exp_factor_vec;
CombatAwardConfig::FactorVec CombatAwardConfig::money_factor_vec;

void CombatAwardConfig::InitTeamAwardConfig()
{
	std::vector<const dataTempl::GlobalConfigTempl*> list;
	s_pDataTempl->QueryDataTemplByType(list);
	assert(list.size() == 1);

	const dataTempl::TeamAwardConfig& tpl = list.front()->team_award_config;
	for (size_t i = 0; i < tpl.exp_factor_array.size(); ++ i)
		exp_factor_vec.push_back(tpl.exp_factor_array[i]);

	for (size_t i = 0; i < tpl.money_factor_array.size(); ++ i)
		money_factor_vec.push_back(tpl.exp_factor_array[i]);
}

float CombatAwardConfig::GetTeamAwardExpFactor(int n)
{
	if (n < 0 || n >= (int)(exp_factor_vec.size()))
		return 0.0f;

	return exp_factor_vec[n-1];
}

float CombatAwardConfig::GetTeamAwardMoneyFactor(int n)
{
	if (n < 0 || n >= (int)(money_factor_vec.size()))
		return 0.0f;

	return money_factor_vec[n-1];
}

/****************************GlobalFunction*************************/
/****************************GlobalFunction*************************/
/****************************GlobalFunction*************************/
/****************************GlobalFunction*************************/
int32_t GetSkillByLevel(int32_t sk_tree_id, int32_t level)
{
	const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(sk_tree_id);
	assert(tpl);

	int32_t skill_id = tpl->lvlup_table[level].skill_id;
	assert(skill_id > 0);
	return skill_id;
}

int32_t GetCombatSceneEventID(int32_t combat_scene_id)
{
	std::vector<const extraTempl::BattleSceneTempl *> list;
	s_pExtraTempl->QueryExtraTemplByType(list);
	for (size_t i = 0; i < list.size(); ++ i)
	{
		const extraTempl::BattleSceneTempl* tpl = list[i];
		if (tpl->templ_id == combat_scene_id)
			return tpl->scene_event_id;
	}
	return 0;
}

void StackDump(const char* func, lua_State* state)
{
	for (int i = 1; i <= lua_gettop(state); ++ i)
	{
		__PRINTF("%s: %s", func, lua_typename(state, lua_type(state, i)));
	}
}

void Normalization(int32_t* prob, size_t size)
{
	int total = 0;
	for (size_t i = 0; i < size; ++i)
	{
		total += prob[i];
	}

	ASSERT(total > 0);
	int subTotal = 0;
	for (size_t i = 0; i < size - 1; ++i)
	{
		ASSERT(prob[i] > 0)
		prob[i]   = prob[i] * 10000 / total;
		subTotal += prob[i];
	}
	prob[size - 1] = 10000 - subTotal;
}

};
