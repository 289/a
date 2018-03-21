#ifndef __GAME_MODULE_COMBAT_GLOBAL_DROP_H__
#define __GAME_MODULE_COMBAT_GLOBAL_DROP_H__

#include <map>
#include <vector>
#include <algorithm>
#include "combat_types.h"
#include "combat_def.h"
#include "shared/base/singleton.h"
#include "shared/base/time_entry.h"

namespace combat
{

/**
 * @class GlobalDropMan
 * @brief 全局掉落表
 * @brief 本类为单例类,是非线程安全的,在目前战斗系统由单一线程调度的情况下是不会有安全问题的。
 */
class GlobalDropMan : public shared::Singleton<GlobalDropMan>
{
private:
	typedef shared::TimeEntry TimeSwitch;
	/*原始掉落表数据*/
	struct GDropTable
	{
		/*掉落表对怪物和地图的限制信息*/
		struct DropLimit
		{
			std::vector<TemplID> mob_ids_limit;
			std::vector<MapID>   map_ids_limit;
			int32_t mob_lvl_lower_limit;
			int32_t mob_lvl_upper_limit;
			int32_t mob_faction_limit;
		};

		bool is_active;
		TemplID templ_id;
		int32_t drop_exp;
		int32_t drop_money;
		TemplID normal_droptable_id;
		TemplID special_droptable_id;
		DropLimit drop_limit;
		int32_t   drop_prob;

		TimeSwitch tm_switch;

		bool IsMobExist(TemplID mob_id) const
		{
			const std::vector<TemplID>& list = drop_limit.mob_ids_limit;
			return std::find(list.begin(), list.end(), mob_id) != list.end();
		}
		bool IsMapExist(MapID map_id) const
		{
			const std::vector<MapID>& list = drop_limit.map_ids_limit;
			return std::find(list.begin(), list.end(), map_id) != list.end();
		}
	};

	typedef std::vector<GDropTable> GlobalDropTableVec;
	GlobalDropTableVec global_drop_tbl_vec_;   //全局掉落表列表

private:
	/*全局掉落产生器*/
	struct DropGenerator
	{
		TemplID     mob_id;
		GDropTable* drop_tbl;
		int32_t     drop_exp;
		int32_t     drop_money;
		std::vector<ItemEntry> drop_items;

		DropGenerator(TemplID mobid, GDropTable* p): mob_id(mobid), drop_tbl(p), drop_exp(0), drop_money(0){}
		bool Generate();
	};

private:
	typedef std::vector<GDropTable*> GlobDropTableVec;
	typedef std::map<TemplID/*mob-id*/, GlobDropTableVec> Mob2DropTableMap;

	/*指定地图的掉落表*/
	struct WorldDropTable
	{
		GlobDropTableVec commdroptbl_vec; //对任何怪物都生效的掉落表
		Mob2DropTableMap mob2droptbl_map; //对指定怪物生效的掉落表
	};

	typedef std::map<MapID, WorldDropTable> WorldDropTableMap;
	WorldDropTableMap world_drop_tbl_map_;   //本gs管理的地图对应的掉落表

public:
	GlobalDropMan();
	~GlobalDropMan();

	static GlobalDropMan& GetInstance()
	{
		static GlobalDropMan instance;
		return instance;
	}

	void Initialize(std::vector<MapID>& mapid_vec);
	void Release();
	bool GenerateDrop(MapID mapid, TemplID mob_id, int32_t& drop_exp, int32_t& drop_money, std::vector<ItemEntry>& drop_items);
};

#define s_global_drop_man GlobalDropMan::GetInstance()

};

#endif // __GAME_MODULE_COMBAT_GLOBAL_DROP_H__
