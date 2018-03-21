#include "global_drop.h"
#include "mob_manager.h"
#include "combat_def.h"
#include "data_templ/templ_manager.h"
#include "data_templ/global_drop_templ.h"
#include "shared/security/randomgen.h"

namespace combat
{

/*****************************GlobalDropMan***************************/
/*****************************GlobalDropMan***************************/
/*****************************GlobalDropMan***************************/
/*****************************GlobalDropMan***************************/

GlobalDropMan::GlobalDropMan()
{
}

GlobalDropMan::~GlobalDropMan()
{
	Release();
}

void GlobalDropMan::Initialize(std::vector<int>& mapid_vec)
{
	/*获取全局掉落表的模板*/
	std::vector<const dataTempl::GlobalDropTempl*> tpl_list;
	s_pDataTempl->QueryDataTemplByType(tpl_list);
	if (tpl_list.empty())
		return;

	/*初始化全局掉落表列表*/
	GlobalDropTableVec& list = global_drop_tbl_vec_;
	list.resize(tpl_list.size());
	for (size_t i = 0; i < tpl_list.size(); ++ i)
	{
		const dataTempl::GlobalDropTempl* tpl = tpl_list[i];

		GDropTable& tbl                  = list[i];
		tbl.is_active                    = tpl->is_active;
		tbl.templ_id                     = tpl->templ_id;
		tbl.drop_exp                     = tpl->drop_exp;
		tbl.drop_money                   = tpl->drop_money;
		tbl.normal_droptable_id          = tpl->normal_droptable_id;
		tbl.special_droptable_id         = tpl->special_droptable_id;
		tbl.drop_prob                    = tpl->drop_probability;

		tbl.drop_limit.mob_lvl_lower_limit = tpl->drop_limit.monster_level_lower_limit;
		tbl.drop_limit.mob_lvl_upper_limit = tpl->drop_limit.monster_level_upper_limit;
		tbl.drop_limit.mob_faction_limit = tpl->drop_limit.monster_faction_limit;;
		for (size_t j = 0; j < tpl->drop_limit.map_ids_limit.size(); ++ j)
			tbl.drop_limit.map_ids_limit.push_back(tpl->drop_limit.map_ids_limit[j]);
		for (size_t j = 0; j < tpl->drop_limit.monster_ids_limit.size(); ++ j)
			tbl.drop_limit.mob_ids_limit.push_back(tpl->drop_limit.monster_ids_limit[j]);
	}

	/*缓存和本gs相关的掉落表*/
	for (size_t i = 0; i < global_drop_tbl_vec_.size(); ++ i)
	{
		GDropTable& tbl = global_drop_tbl_vec_[i];
		if (tbl.drop_limit.mob_ids_limit.empty())
		{
			//对任何怪物都有效的掉落表
			if (tbl.drop_limit.map_ids_limit.empty())
			{
				//对任何地图均有效
				for (size_t j = 0; j < mapid_vec.size(); ++ j)
				{
					TemplID mapid = mapid_vec[j];
					world_drop_tbl_map_[mapid].commdroptbl_vec.push_back(&tbl);
				}
			}
			else
			{
				//对部分地图有效
				for (size_t j = 0; j < mapid_vec.size(); ++ j)
				{
					int mapid = mapid_vec[j];
					if (tbl.IsMapExist(mapid))
					{
						world_drop_tbl_map_[mapid].commdroptbl_vec.push_back(&tbl);
					}
				}
			}
		}
		else
		{
			//对指定怪物生效的掉落表
			if (tbl.drop_limit.map_ids_limit.empty())
			{
				//对任意地图内的特定怪物生效
				for (size_t j = 0; j < mapid_vec.size(); ++ j)
				{
					//初始化怪物到掉落表的映射表
					TemplID mapid = mapid_vec[j];
					for (size_t k = 0; k < tbl.drop_limit.mob_ids_limit.size(); ++ k)
					{
						TemplID mob_id = tbl.drop_limit.mob_ids_limit[k];
						world_drop_tbl_map_[mapid].mob2droptbl_map[mob_id].push_back(&tbl);
					}
				}
			}
			else
			{
				//对特地地图特定怪物生效的掉落表
				for (size_t j = 0; j < mapid_vec.size(); ++ j)
				{
					int mapid = mapid_vec[j];
					if (tbl.IsMapExist(mapid))
					{
						//初始化怪物到掉落表的映射表
						for (size_t k = 0; k < tbl.drop_limit.mob_ids_limit.size(); ++ k)
						{
							TemplID mob_id = tbl.drop_limit.mob_ids_limit[k];
							world_drop_tbl_map_[mapid].mob2droptbl_map[mob_id].push_back(&tbl);
						}
					}
				}
			}
		}
	}
}

void GlobalDropMan::Release()
{
	global_drop_tbl_vec_.clear();
	world_drop_tbl_map_.clear();
}

bool GlobalDropMan::GenerateDrop(MapID mapid, TemplID mob_id, int32_t& drop_exp, int32_t& drop_money, std::vector<ItemEntry>& drop_items)
{
	WorldDropTableMap::iterator it = world_drop_tbl_map_.find(mapid);
	if (it == world_drop_tbl_map_.end())
	{
		return false;
	}

	GlobDropTableVec& commdroptbl_vec = it->second.commdroptbl_vec;
	Mob2DropTableMap& mob2droptbl_map = it->second.mob2droptbl_map;

	int32_t __exp = 0;
	int32_t __money = 0;
	std::vector<ItemEntry> __items;

	//处理对所有怪物生效的掉落表
	for (size_t i = 0; i < commdroptbl_vec.size(); ++ i)
	{
		DropGenerator generator(mob_id, commdroptbl_vec[i]);
		if (generator.Generate())
		{
			__exp += generator.drop_exp;
			__money += generator.drop_money;
			__items.insert(__items.end(), generator.drop_items.begin(), generator.drop_items.end());
		}
	}

	//处理对指定怪物生效的掉落表
	Mob2DropTableMap::iterator it2 = mob2droptbl_map.find(mob_id);
	if (it2 != mob2droptbl_map.end())
	{
		for (size_t i = 0; i < it2->second.size(); ++ i)
		{
			GDropTable* pdrop = it2->second[i];

			int32_t mob_lvl = s_mob_man.GetMobLevel(mob_id);
			if (mob_lvl >= pdrop->drop_limit.mob_lvl_lower_limit && mob_lvl <= pdrop->drop_limit.mob_lvl_upper_limit)
			{
				DropGenerator generator(mob_id, pdrop);
				if (generator.Generate())
				{
					__exp += generator.drop_exp;
					__money += generator.drop_money;
					__items.insert(__items.end(), generator.drop_items.begin(), generator.drop_items.end());
				}
			}
		}
	}

	drop_exp   = __exp;
	drop_money = __money;
	drop_items = __items;
	return true;
}

bool GlobalDropMan::DropGenerator::Generate()
{
	if (!drop_tbl->is_active)
		return false;

	if (mob_id <= 0 || drop_tbl == NULL)
		return false;

	//检查怪物在本掉落表有掉落
	if (drop_tbl->drop_limit.mob_ids_limit.empty())
	{
		//对所有怪物生效
	}
	else if (!drop_tbl->IsMobExist(mob_id))
	{
		//对此怪物不生效
		assert(false);
		return false;
	}

	int32_t mob_lvl = s_mob_man.GetMobLevel(mob_id);
	if (mob_lvl < drop_tbl->drop_limit.mob_lvl_lower_limit ||
		mob_lvl > drop_tbl->drop_limit.mob_lvl_upper_limit)
		return false;

	/*
	//TODO
	//阵营功能未做，暂不生效。
	int32_t mob_fac = s_mob_man.GetMobFaction(mob_id);
	if (!(mob_fac & drop_tbl->drop_limit.mob_faction_limit))
		return false;
	*/

	if (shared::net::RandomGen::RandUniform(1, 10000) >= drop_tbl->drop_prob)
		return false;

	//检查通过,产生掉落
	drop_exp   = drop_tbl->drop_exp;
	drop_money = drop_tbl->drop_money;

	DropTable* normal_drop_tbl = s_mob_man.GetDropTable(drop_tbl->normal_droptable_id);
	if (normal_drop_tbl)
	{
		std::vector<ItemEntry> __items;
		normal_drop_tbl->GenerateDrop(__items);
		drop_items.insert(drop_items.end(), __items.begin(), __items.end());
	}

	DropTable* spec_drop_tbl = s_mob_man.GetDropTable(drop_tbl->special_droptable_id);
	if (spec_drop_tbl)
	{
		std::vector<ItemEntry> __items;
		spec_drop_tbl->GenerateDrop(__items);
		drop_items.insert(drop_items.end(), __items.begin(), __items.end());
	}

	return true;
}

};
