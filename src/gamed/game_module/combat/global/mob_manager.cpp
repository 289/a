#include <stdlib.h>
#include "mob_manager.h"
#include "data_templ/templ_manager.h"
#include "data_templ/monster_templ.h"
#include "data_templ/monster_drop_templ.h"
#include "data_templ/golem_templ.h"
#include "shared/security/randomgen.h"


namespace combat
{

bool DropTable::GenerateDrop(std::vector<ItemEntry>& items)
{
	std::vector<Entry> __table = drop_table;

	int count = 0;
	while (count ++ < drop_times)
	{
		int32_t rand_num = shared::net::RandomGen::RandUniform(1, 10000);
		for (size_t i = 0; i < __table.size(); ++ i)
		{
			if (__table[i].probability < rand_num)
				continue;

			//生成掉落物品
			ItemEntry item;
			item.item_id = __table[i].item_id;
			item.item_count = __table[i].item_count;
			items.push_back(item);

			//处理唯一性掉落
			if (__table[i].unique)
			{
				//删除唯一性掉落
				__table.erase(__table.begin() + i);

                if (__table.size() != 0)
                {
                    std::vector<int> __probs;
                    for (size_t k = 0; k < __table.size(); ++ k)
                    {
                        __probs.push_back(__table[k].drop_prob);
                    }

                    //重新归一化掉落概率
                    Normalization(__probs.data(), __probs.size());

                    //重置掉落概率
                    for (size_t k = 0; k < __table.size(); ++ k)
                    {
                        __table[k].drop_prob   = __probs[k];
                        __table[k].probability = __probs[k];
                    }

                    //预处理概率
                    for (size_t k = 1; k < __table.size(); ++ k)
                    {
                        __table[k].probability += __table[k-1].probability;
                    }
                }
			}

			break;
		}
	}

	return items.size() > 0;
}

/**************************************MobManager**********************************************/
/**************************************MobManager**********************************************/
/**************************************MobManager**********************************************/
/**************************************MobManager**********************************************/

MobManager::MobManager()
{
}

MobManager::~MobManager()
{
	Release();
}

void MobManager::Initialize()
{
	InitDropMap();
	InitMobMap();
}

void MobManager::Release()
{
	MobMap::iterator it = mob_map_.begin();
	for ( ; it != mob_map_.end(); ++ it)
	{
		delete it->second;
		it->second = NULL;
	}

	DropTblMap::iterator it2 = drop_map_.begin();
	for ( ; it2 != drop_map_.end(); ++ it2)
	{
		delete it2->second;
		it2->second = NULL;
	}

	mob_map_.clear();
	drop_map_.clear();
}

bool MobManager::IsValidMob(TemplID mob_id) const
{
	return mob_map_.find(mob_id) != mob_map_.end();
}

char MobManager::GetMobLevel(TemplID mob_id) const
{
	if (mob_id <= 0) return -1;

	MobMap::const_iterator it = mob_map_.find(mob_id);
	if (it == mob_map_.end())
	{
		assert(false);
		return -1;
	}
	return it->second->mob_lvl;
}

int32_t MobManager::GetMobFaction(TemplID mob_id) const
{
	if (mob_id <= 0) return -1;

	MobMap::const_iterator it = mob_map_.find(mob_id);
	if (it == mob_map_.end())
	{
		assert(false);
		return -1;
	}
	return it->second->mob_fac;
}

bool MobManager::GetMobSkillGroup(TemplID mob_id, SkillGroup& skills) const
{
	if (mob_id <= 0)
        return false;

	MobMap::const_iterator it = mob_map_.find(mob_id);
	if (it == mob_map_.end())
	{
		assert(false);
		return false;
	}

	skills = it->second->skill_group;
	return true;
}

TemplID MobManager::GetMobNormalAtkID(TemplID mob_id) const
{
	if (mob_id <= 0)
        return 0;

	MobMap::const_iterator it = mob_map_.find(mob_id);
	if (it == mob_map_.end())
	{
		assert(false);
		return 0;
	}

	return it->second->normal_atk_id;
}

bool MobManager::GetMobProps(TemplID mob_id, PropsVec& props) const
{
	if (mob_id <= 0)
        return false;

	props.clear();
	MobMap::const_iterator it = mob_map_.find(mob_id);
	if (it == mob_map_.end())
	{
		assert(false);
		return false;
	}

	props = it->second->props;
	return true;
}

std::string MobManager::GetMobModel(TemplID mob_id) const
{
	if (mob_id <= 0)
        return false;

	MobMap::const_iterator it = mob_map_.find(mob_id);
	if (it == mob_map_.end())
	{
		assert(false);
		return std::string();
	}

	return it->second->model_src_path;
}

bool MobManager::GenerateDropItem(TemplID mob_id, std::vector<ItemEntry>& list)
{
	list.clear();

	Mob* mob = GetMob(mob_id);
	if (!mob  || shared::net::RandomGen::RandUniform(1, 10000) > mob->comm_drop_prob)
	{
		return false;
	}

	DropTable* drop = mob->comm_drop_tbl;
	if (!drop || drop->drop_times == 0)
	{
		return false;
	}

	return drop->GenerateDrop(list);
}

bool MobManager::GenerateLotteryItem(TemplID mob_id, std::vector<ItemEntry>& list)
{
	Mob* mob = GetMob(mob_id);
	if (!mob)
	{
		return false;
	}

	DropTable* drop = mob->spec_drop_tbl;
	if (!drop || drop->drop_times == 0)
	{
		return false;
	}

	return drop->GenerateDrop(list);
}

int32_t MobManager::GenerateDropExp(TemplID mob_id)
{
	Mob* mob = GetMob(mob_id);
	if (!mob) return -1;
	return mob->drop_exp;
}

int32_t MobManager::GenerateDropMoney(TemplID mob_id)
{
	Mob* mob = GetMob(mob_id);
	if (!mob) return -1;
	return mob->drop_money;
}

void MobManager::InitDropMap()
{
	using namespace dataTempl;

	///
	/// 加载掉落表
	///
	std::vector<const MonsterDropTempl*> list;
	s_pDataTempl->QueryDataTemplByType(list);
	for (size_t i = 0; i < list.size(); ++ i)
	{
		const MonsterDropTempl* pTpl = list[i];
		DropTable* pTbl = new DropTable;
		pTbl->drop_times = pTpl->drop_times;

		for (size_t j = 0; j < pTpl->drop_table.size(); ++ j)
		{
			const MonsterDropTempl::drop_entry& de = pTpl->drop_table[j];

			DropTable::Entry    tmp;
			tmp.item_id	        = de.item_id;
			tmp.item_count      = de.item_count;
			tmp.probability     = de.drop_prob;
			tmp.drop_prob       = de.drop_prob;
			tmp.unique          = de.unique_drop;
			pTbl->drop_table.push_back(tmp);
		}

		//处理物品掉落概率，方便使用
		for (size_t k = 1; k < pTbl->drop_table.size(); ++ k)
		{
			pTbl->drop_table[k].probability += pTbl->drop_table[k-1].probability;
		}

		drop_map_[pTpl->templ_id] = pTbl;
	}
}

void MobManager::InitMobMap()
{
	using namespace dataTempl;

	///
	/// 加载怪物数据
	///
	std::vector<const MonsterTempl*> list;
	s_pDataTempl->QueryDataTemplByType(list);
	for (size_t i = 0; i < list.size(); ++ i)
	{
		const MonsterTempl* pTpl = list[i];
		Mob* mob                 = new Mob;
		mob->mob_id              = pTpl->templ_id;
		mob->mob_lvl             = pTpl->level;
		mob->mob_fac             = pTpl->faction;
		mob->drop_exp            = pTpl->killed_exp;
		mob->drop_money          = pTpl->killed_money;
		mob->normal_atk_id       = pTpl->normal_attack_id;

		mob->skill_group.resize(pTpl->skill_group.size());
		for (size_t j = 0; j < pTpl->skill_group.size(); ++ j)
		{
			SkillEntry& entry = mob->skill_group[j];
			entry.skill = pTpl->skill_group[j].skill_id;
			entry.prob  = pTpl->skill_group[j].probability;
		}

		for (size_t k = 0; k < pTpl->properties.size(); ++ k)
		{
			mob->props.push_back(pTpl->properties[k]);
		}

		mob->comm_drop_prob = pTpl->normal_drop_prob;
		mob->comm_drop_tbl  = GetDropTable(pTpl->normal_droptable_id);
		mob->spec_drop_tbl  = GetDropTable(pTpl->special_droptable_id);
        mob->model_src_path = pTpl->model_src.to_str();

		mob_map_[pTpl->templ_id] = mob;
	}
}

MobManager::Mob* MobManager::GetMob(TemplID mob_id)
{
	MobMap::iterator it = mob_map_.find(mob_id);
	return it != mob_map_.end() ? it->second : NULL;
}

DropTable* MobManager::GetDropTable(TemplID drop_tbl_id)
{
	DropTblMap::iterator it = drop_map_.find(drop_tbl_id);
	return it != drop_map_.end() ? it->second : NULL;
}

/**************************************GolemManager**********************************************/
/**************************************GolemManager**********************************************/
/**************************************GolemManager**********************************************/
/**************************************GolemManager**********************************************/
GolemManager::GolemManager()
{
}

GolemManager::~GolemManager()
{
	Release();
}

void GolemManager::Initialize()
{
	using namespace dataTempl;

	///
	/// 加载魔偶数据
	///
	std::vector<const GolemTempl*> list;
	s_pDataTempl->QueryDataTemplByType(list);
	for (size_t i = 0; i < list.size(); ++ i)
	{
		const GolemTempl* pTpl = list[i];

		/*Golem* golem = new Golem;
		golem->golem_id = pTpl->templ_id;
		for (size_t i = 0; i < pTpl->skills.size(); ++ i)
			golem->skills.push_back(pTpl->skills[i]);
		for (size_t i = 0; i < pTpl->properties.size(); ++ i)
			golem->props.push_back(pTpl->properties[i]);

		golem_map_[pTpl->templ_id] = golem;*/
		golem_map_[pTpl->templ_id] = pTpl;
	}
}

void GolemManager::Release()
{
	/*GolemMap::iterator it = golem_map_.begin();
	for ( ; it != golem_map_.end(); ++ it)
	{
		delete it->second;
		it->second = NULL;
	}*/

	golem_map_.clear();
}

bool GolemManager::IsGolemValid(TemplID golem_id) const
{
	GolemMap::const_iterator it = golem_map_.find(golem_id);
	return it != golem_map_.end();
}

void GolemManager::GetGolemSkill(TemplID golem_id, SkillVec& skills) const
{
	GolemMap::const_iterator it = golem_map_.find(golem_id);
	if (it != golem_map_.end())
	{
		skills.clear();
		//skills = it->second->skills;
		for (size_t i = 0; i < it->second->skills.size(); ++ i)
			skills.push_back(it->second->skills[i]);
	}
}

void GolemManager::GetGolemProps(TemplID golem_id, PropsVec& props) const
{
	GolemMap::const_iterator it = golem_map_.find(golem_id);
	if (it != golem_map_.end())
	{
		props.clear();
		//props = it->second->props;
		for (size_t i = 0; i < it->second->properties.size(); ++ i)
			props.push_back(it->second->properties[i]);
	}
}

void GolemManager::GetGolemModel(TemplID golem_id, std::string& model) const
{
	GolemMap::const_iterator it = golem_map_.find(golem_id);
	if (it != golem_map_.end())
	{
        model = it->second->model_resource_path.to_str();
    }
}

}; // namespace combat
