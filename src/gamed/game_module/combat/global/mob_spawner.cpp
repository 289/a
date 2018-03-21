#include "mob_spawner.h"
#include "mob_manager.h"
#include "extra_templ/extratempl_man.h"
#include "extra_templ/monster_group.h"
#include "shared/security/randomgen.h"


namespace combat
{

/**************************************MobSpawner**********************************************/
/**************************************MobSpawner**********************************************/
/**************************************MobSpawner**********************************************/
/**************************************MobSpawner**********************************************/

MobSpawner::MobSpawner()
{
}

MobSpawner::~MobSpawner()
{
	Release();
}

void MobSpawner::Initialize()
{
	///
	/// 加载怪物组
	///

	using namespace extraTempl;
	std::vector<const MonsterGroupTempl *> list;
	s_pExtraTempl->QueryExtraTemplByType(list);

	for (size_t i = 0; i < list.size(); ++ i)
	{
		const MonsterGroupTempl* pTpl = list[i];
		MobGroup& group = map_[pTpl->templ_id];

		group.group_id = pTpl->templ_id;
		group.group_type = pTpl->group_type;
		group.team_revise = pTpl->team_revise;
		group.next_group_id = pTpl->next_monster_group;
		group.combat_common_hint_id = pTpl->combat_common_hint_id;
		group.combat_start_hint_id = pTpl->combat_start_hint_id;
		group.sneak_attack_probability = pTpl->sneak_attack_probability;
		group.sneak_attacked_probability = pTpl->sneak_attacked_probability;

		//对出现几只怪物的概率进行特殊处理,方便使用
		group.probability[0] = pTpl->one_monster_prob;
		group.probability[1] = group.probability[0] + pTpl->two_monsters_prob;
		group.probability[2] = group.probability[1] + pTpl->three_monsters_prob;
		group.probability[3] = group.probability[2] + pTpl->four_monsters_prob;

		for (size_t j = 0; j < pTpl->monster_list.size(); ++ j)
		{
			const MonsterGroupTempl::MonsterInfo& mi = pTpl->monster_list[j];
			MobGroup::Mob	mob;
			mob.id			= mi.monster_tid;
			mob.gen_prob	= mi.monster_gen_prob;
			mob.pos			= mi.position;
			mob.unique		= mi.is_unique;
			group.candidates.push_back(mob);
		}

		//对出生概率进行特殊处理,方便使用
		for (size_t j = 1; j < group.candidates.size(); ++ j)
		{
			group.candidates[j].gen_prob += group.candidates[j-1].gen_prob;
		}
	}
	assert(map_.size() == list.size());
}

void MobSpawner::Release()
{
	map_.clear();
}

bool MobSpawner::GenerateMob(int mob_group_id, std::vector<MobInfo>& list, int min_mob_count)
{
	MobGroupMap::const_iterator it = map_.find(mob_group_id);
	if (it == map_.end())
	{
		__PRINTF("GenerateMob: 无效怪物群组ID(%d)", mob_group_id);
		return false;
	}
	
	list.clear();

	char group_type = it->second.group_type;
	if (group_type == MOB_GROUP_TYPE_RANDOM)
	{
		return GenerateRandomMob(it->second, list, min_mob_count);
	}
	else if (group_type == MOB_GROUP_TYPE_FIXED)
	{
		return GenerateFixedMob(it->second, list);
	}
	else
	{
		return false;
	}
}

int32_t MobSpawner::GetNextMobGroupID(TemplID mob_group_id) const
{
	MobGroupMap::const_iterator it = map_.find(mob_group_id);
	if (it == map_.end())
	{
		__PRINTF("GenerateMob: 无效怪物群组ID(%d)", mob_group_id);
		return 0;
	}
	return it->second.next_group_id;
}

int32_t MobSpawner::GetCombatCommonHintID(TemplID mob_group_id) const
{
	MobGroupMap::const_iterator it = map_.find(mob_group_id);
	if (it == map_.end())
	{
		__PRINTF("GenerateMob: 无效怪物群组ID(%d)", mob_group_id);
		return 0;
	}
	return it->second.combat_common_hint_id;
}

int32_t MobSpawner::GetCombatStartHintID(TemplID mob_group_id) const
{
	MobGroupMap::const_iterator it = map_.find(mob_group_id);
	if (it == map_.end())
	{
		__PRINTF("GenerateMob: 无效怪物群组ID(%d)", mob_group_id);
		return 0;
	}
	return it->second.combat_start_hint_id;
}

bool MobSpawner::TestSneakAttack(TemplID mob_group_id) const
{
	MobGroupMap::const_iterator it = map_.find(mob_group_id);
	if (it == map_.end())
	{
		__PRINTF("GenerateMob: 无效怪物群组ID(%d)", mob_group_id);
		return false;
	}

	int probability = it->second.sneak_attack_probability;
	return shared::net::RandomGen::RandUniform(1, 100) <= probability;
}

bool MobSpawner::TestSneakAttacked(TemplID mob_group_id) const
{
	MobGroupMap::const_iterator it = map_.find(mob_group_id);
	if (it == map_.end())
	{
		__PRINTF("GenerateMob: 无效怪物群组ID(%d)", mob_group_id);
		return false;
	}

	int probability = it->second.sneak_attacked_probability;
	return shared::net::RandomGen::RandUniform(1, 100) <= probability;
}

bool MobSpawner::GenerateRandomMob(const MobGroup& group, std::vector<MobInfo>& list, int min_mob_count)
{
	assert(min_mob_count <= MAX_COMBAT_UNIT_NUM);
	MobGroup __group = group;

	int count = 0;//随机产生怪物个数
	for (int n = 1; n <= MAX_COMBAT_UNIT_NUM; ++ n)
	{
		int rand_num = shared::net::RandomGen::RandUniform(1, 10000);
		if (rand_num <= __group.probability[n-1])
		{
			count = n;
			break;
		}
	}

	//当随机怪物个数小于玩家个数，需要修正产生的怪物个数，保证怪物数量大于等于玩家数量
	if (count < min_mob_count && __group.team_revise)
	{
		int32_t  __prob  = 0;
		for (int n = 1; n < min_mob_count; ++ n)
		{
			__prob += __group.probability[n-1];
		}

		for (int n = min_mob_count; n < MAX_COMBAT_UNIT_NUM; ++ n)
		{
			__group.probability[n-1] += __prob / (MAX_COMBAT_UNIT_NUM - min_mob_count);
		}

		int32_t lower_limit = __group.probability[min_mob_count-1];
		int32_t upper_limit = 10000;

		for (int n = min_mob_count; n <= MAX_COMBAT_UNIT_NUM; ++ n)
		{
			if (__group.probability[n-1] >= shared::net::RandomGen::RandUniform(lower_limit, upper_limit))
			{
				count = n;
				break;
			}
		}

		assert(count >= min_mob_count);
	}

	//随机1只怪
	if (count <= 0)
	{
		return false;
	}
	else if (count == 1)
	{
		//产生一只怪物
		MobInfo info;
		for (size_t i = 0; i < __group.candidates.size(); ++ i)
		{
			if (__group.candidates[i].gen_prob >= shared::net::RandomGen::RandUniform(1, 10000))
			{
				info.id  = __group.candidates[i].id;
				info.pos = __group.candidates[i].pos;
				info.lvl = s_mob_man.GetMobLevel(info.id);
				if (info.pos == COMBAT_POS_RANDOM)
				{
					info.pos = 3;
				}
				else
				{
					info.pos = info.pos-1;
				}
				list.push_back(info);
				break;
			}
		}

		assert(list.size() == 1);
	}
	else if (count > 1)
	{
		//产生多只怪物
		bool position[MAX_COMBAT_UNIT_NUM] = {false};
		MobGroup::MobVec candidates = __group.candidates;
		for (size_t n = 0; n < (size_t)count; ++ n)
		{
			MobInfo info;
			for (size_t i = 0; i < candidates.size(); ++ i)
			{
				if (candidates[i].gen_prob >= shared::net::RandomGen::RandUniform(1, 10000))
				{
					info.id  = candidates[i].id;
					info.lvl = s_mob_man.GetMobLevel(info.id);
	
					size_t pos = candidates[i].pos;
					if (pos != COMBAT_POS_RANDOM && !position[pos-1])
					{
						//指定默认位置
						//且默认位置为空位
						info.pos = pos-1;
						position[pos-1] = true;
					}
					else
					{
						//未指定默认位置或已占用
						//按4,3,2,1顺序分配站位
						size_t idx = 3;
						for (; idx >= 0; -- idx)
						{
							if (!position[idx])
							{
								info.pos = idx;
								position[idx] = true;
								break;
							}
						}
						assert(idx >= 0);
					}
	
					list.push_back(info);
					if (candidates[i].unique)
					{
						//本怪具有唯一性
						//删除以免再次随机到
						candidates.erase(candidates.begin() + i);
					}

					break;
				}
			}
		}
	}

	return list.size() > 0 && list.size() <= MAX_COMBAT_UNIT_NUM;
}

bool MobSpawner::GenerateFixedMob(const MobGroup& group, std::vector<MobInfo>& list)
{
	bool position[MAX_COMBAT_UNIT_NUM] = {false};
	for (size_t i = 0; i < group.candidates.size() && i < MAX_COMBAT_UNIT_NUM; ++ i)
	{
		MobInfo info;
		info.id  = group.candidates[i].id;
		info.lvl = s_mob_man.GetMobLevel(info.id);

		size_t pos = group.candidates[i].pos;
		if (pos != COMBAT_POS_RANDOM && !position[pos-1])
		{
			//指定默认位置
			//且默认位置为空位
			info.pos = pos-1;
			position[pos-1] = true;
		}
		else
		{
			//未指定默认位置或已占用
			//按4,3,2,1顺序分配站位
			size_t idx = 3;
			for (; idx >= 0; -- idx)
			{
				if (!position[idx])
				{
					info.pos = idx;
					position[idx] = true;
					break;
				}
			}
			assert(idx >= 0);
		}

		list.push_back(info);
	}

	return true;
}

};
