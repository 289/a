#ifndef GAMED_GS_TEMPLATE_MAPDATA_SPOT_MONSTER_H_
#define GAMED_GS_TEMPLATE_MAPDATA_SPOT_MONSTER_H_

#include "base_mapdata.h"


namespace mapDataSvr {

/**
 * @brief 定点怪信息
 *    1.布定点怪、NPC都是用这个地图元素模板的数据，NPC没有战斗事件参数
 *    2.要在base_mapdata.cpp中添加INIT语句才能生效（Clone生效）
 */
class SpotMonster : public BaseMapData
{
	DECLARE_MAPDATA(SpotMonster, MAPDATA_TYPE_SPOT_MONSTER);
public:
	enum PatrolMode
	{
		REPEATING          = 0,
		STOP_AT_TERMINAL   = 1,
		CIRCILING          = 2,
	};

	enum NpcTemplType
	{
		NT_INVALID     = 0,
		NT_MONSTER     = 1,
		NT_SERVICE_NPC = 2,
	};
		
	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}


public:
// 0
	Coordinate   coord;                // 出生的坐标位置
	TemplID      associated_templ_id;  // 关联的数据模板id，可以是monster也可以是NPC
	uint8_t      dir;                  // 朝向 direction，默认值0
	bool         is_wandered;          // 是否随机走动、游荡，如果填了patrol_path_id，则该项失效。默认值false
    ElemID       patrol_path_id;       // 巡逻路径，默认值0，即没有巡逻路径

// 5
	uint8_t      patrol_mode;          // 对应PatrolMode枚举，巡逻方式，以enum方式记录
	uint8_t      ap_mode;              // 对应APMode枚举，passive, active mode
	uint8_t      combat_rule;          // 对应CombatRule枚举，战斗规则：不显示模型、显示模型不可战斗、显示模型世界boss
	int32_t      refresh_interval;     // 怪物消失后的刷新间隔，单位s。默认值为10，0表示不刷新
	TemplID      monster_group_id;     // 怪物组id，默认值0，表示没有对应的怪物组

// 10
	MapID        battle_scene_id;      // 战斗时切换到的场景id，默认值0，即使用场景或者区域的战斗场景配置
	float        aggro_view_radius;    // 仇恨视野半径，单位m，默认值2米，最小值0.5，最大值20
	uint16_t     aggro_time;           // 仇恨时间，超过该时间则解除仇恨，单位s，默认值20秒，最小值10，最大值65530
	float        max_chase_distance;   // 最大追击距离，离开始追击点的直线距离，单位m，默认值4，最小值2，最大值65530
	float        chase_speed;          // 追击速度，单位m/s，默认值3米每秒，最小值0.1，最大值10

// 15
    uint8_t      npc_templ_type;       // 对应NpcTemplType枚举，表明布的associated_templ_id是npc还是monster，用于做检查


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(coord, associated_templ_id, dir, is_wandered, patrol_path_id);
		MARSHAL_MAPDATA(patrol_mode, ap_mode, combat_rule, refresh_interval, monster_group_id);
		MARSHAL_MAPDATA(battle_scene_id, aggro_view_radius, aggro_time, max_chase_distance, chase_speed);
		MARSHAL_MAPDATA(npc_templ_type);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(coord, associated_templ_id, dir, is_wandered, patrol_path_id);
		UNMARSHAL_MAPDATA(patrol_mode, ap_mode, combat_rule, refresh_interval, monster_group_id);
		UNMARSHAL_MAPDATA(battle_scene_id, aggro_view_radius, aggro_time, max_chase_distance, chase_speed);
		UNMARSHAL_MAPDATA(npc_templ_type);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (associated_templ_id <= 0)
			return false;

		if (REPEATING != patrol_mode && STOP_AT_TERMINAL != patrol_mode && CIRCILING != patrol_mode)
			return false;

		if (patrol_path_id > 0 && is_wandered)
			return false;

		if (refresh_interval < 0)
			return false;

		if (npc_templ_type != NT_MONSTER && npc_templ_type != NT_SERVICE_NPC)
			return false;

		if (dir < 0 || dir > 7)
			return false;
		
		// 如果是怪，则做如下检查
		if (npc_templ_type == NT_MONSTER)
		{
			if (PASSIVE != ap_mode && ACTIVE != ap_mode)
				return false;

			if (NO_MODEL != combat_rule && 
                MODEL_NO_COMBAT != combat_rule && 
                MODEL_WORLD_BOSS != combat_rule &&
                NEVER_COMBAT != combat_rule)
				return false;

			if (monster_group_id <= 0 || battle_scene_id < 0)
				return false;

			if (aggro_view_radius < 0.5 || aggro_view_radius > 20)
				return false;

			if (aggro_time < 10 || aggro_time > 65530)
				return false;

			if (max_chase_distance < 2 || max_chase_distance > 65530)
				return false;

			if (chase_speed < 0.1 || chase_speed > 10)
				return false;
		}

		return true;
	}
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_SPOT_MONSTER_H_
