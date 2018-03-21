#ifndef GAMED_GS_TEMPLATE_MAPDATA_AREA_WITH_RULES_H_
#define GAMED_GS_TEMPLATE_MAPDATA_AREA_WITH_RULES_H_

#include "base_mapdata.h"


namespace mapDataSvr {

/**
 * @brief 带规则的区域
 *    1.需要在base_mapdata.cpp中添加INIT语句才能生效（Clone生效）
 */
class AreaWithRules : public BaseMapData
{
	DECLARE_MAPDATA(AreaWithRules, MAPDATA_TYPE_AREA_WITH_RULES);
public:
	static const int kMaxBattleSceneCount = 5; // 最多可以配置5个战斗场景信息

    enum AreaType
    {
        AT_NORMAL = 0, // 普通规则区域 
        AT_MAP_AREA,   // 标记为区域地图，对应客户端的一张地图,
                       // 一张服务器地图（一个地图id）内可能有多张区域地图
    };

	struct ResurrectCoord
	{
		MapID       map_id; // 为0时表示该区域没有复活点
		Coordinate  coord;

		NESTED_DEFINE(map_id, coord);

		ResurrectCoord()
			: map_id(0)
		{ }
	};

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
	BoundArray<Coordinate, kMaxVertexCount> vertexes; // 区域顶点数组
	ResurrectCoord     resurrect_coord;     // 复活坐标是一个全局坐标，即可以跨地图复活
	bool               is_allow_pk;         // 是否允许pk，默认值为false
	uint8_t            area_level;          /* 区域等级：在区域重叠时，决定哪个区域的配置生效。
										     * 默认值为1，数字越大等级越高
			         				         * 在多个区域重叠，并有区域配置重复时，以区域等级最高的为准，每个配置单独生效
											 * */
	BoundArray<BattleSceneID, kMaxBattleSceneCount> battle_scene_list; // 玩家在此区域发起战斗的战斗场景

// 5
	EventID            event_id;            // 该区域所挂的事件id，默认值0，表示没有关联区域事件
    int8_t             area_type;           // 对应枚举AreaType，默认是普通区域


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(vertexes, resurrect_coord, is_allow_pk, area_level, battle_scene_list);
		MARSHAL_MAPDATA(event_id, area_type);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(vertexes, resurrect_coord, is_allow_pk, area_level, battle_scene_list);
		UNMARSHAL_MAPDATA(event_id, area_type);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (vertexes.size() < 3)
			return false;

		if (area_level <= 0)
			return false;

		for (size_t i = 0; i < battle_scene_list.size(); ++i)
		{
			if (battle_scene_list[i] <= 0)
				return false;
		}

        if (area_type != AT_NORMAL && area_type != AT_MAP_AREA)
            return false;

		return true;
	}
};

///
/// inline func
///
inline bool AreaWithRules::has_evscript() const
{
	if (event_id == 0)
		return false;

	return true;
}

inline bool AreaWithRules::has_battle_scene() const
{
	return !battle_scene_list.empty();
}

inline bool AreaWithRules::has_resurrect_coord() const 
{ 
	return resurrect_coord.map_id != 0; 
}

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_AREA_WITH_RULES_H_
