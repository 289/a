#ifndef GAMED_GS_TEMPLATE_MAPDATA_MAP_DEFAULT_AREA_H_
#define GAMED_GS_TEMPLATE_MAPDATA_MAP_DEFAULT_AREA_H_

#include "base_mapdata.h"
#include "area_with_rules.h"


namespace mapDataSvr {

/**
 * @brief MapDefaultArea
 *    1.MapDefaultArea是每张地图只有一份，默认区域覆盖整张地图
 *    2.要在base_mapdata.cpp中添加INIT语句才能生效（Clone生效）
 */
class MapDefaultArea : public BaseMapData
{
	DECLARE_MAPDATA(MapDefaultArea, MAPDATA_TYPE_MAP_DEFAULT_AREA);
public:
	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}

	inline bool has_evscript() const;
	inline bool has_battle_scene() const;
	inline bool has_resurrect_coord() const;


public:
// 0
	AreaWithRules::ResurrectCoord resurrect_coord;     // 复活坐标是一个全局坐标，即可以跨地图复活
	bool            is_allow_pk;         // 是否允许pk，默认值为false
	uint8_t         area_level;          /* 区域等级：在区域重叠时，决定哪个区域的配置生效。
										  * 默认值为1，数字越大等级越高，在多个区域重叠，并有区域配置重复时，
										  * 以区域等级最高的为准，每个配置单独生效
										  * */
	BoundArray<BattleSceneID, AreaWithRules::kMaxBattleSceneCount> battle_scene_list; // 玩家在此区域发起战斗的战斗场景
	EventID         event_id;            // 该区域所挂的事件id，默认值0，表示没有关联区域事件


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(resurrect_coord, is_allow_pk, area_level, battle_scene_list, event_id);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(resurrect_coord, is_allow_pk, area_level, battle_scene_list, event_id);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (area_level != 1)
			return false;

		if (resurrect_coord.map_id <= 0)
			return false;

		if (battle_scene_list.size() == 0)
			return false;

		for (size_t i = 0; i < battle_scene_list.size(); ++i)
		{
			if (battle_scene_list[i] <= 0)
				return false;
		}
		return true;
	}
};

///
/// inline func
///
inline bool MapDefaultArea::has_evscript() const
{
	if (event_id == 0)
		return false;

	return true;
}

inline bool MapDefaultArea::has_battle_scene() const
{
	return !battle_scene_list.empty();
}

inline bool MapDefaultArea::has_resurrect_coord() const 
{ 
	return resurrect_coord.map_id != 0; 
}


} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_MAP_DEFAULT_AREA_H_
