#ifndef GAMED_GS_TEMPLATE_MAPDATA_AREA_MONSTER_H_
#define GAMED_GS_TEMPLATE_MAPDATA_AREA_MONSTER_H_

#include "base_mapdata.h"


namespace mapDataSvr {

/**
 * @brief 需要在base_mapdata.cpp中添加INIT语句才能生效（Clone生效）
 */
class AreaMonster : public BaseMapData
{
	DECLARE_MAPDATA(AreaMonster, MAPDATA_TYPE_AREA_MONSTER);
public:
    static const int kMaxConditionTask = 64;

	enum AppearType
	{
		AT_LANDMINE         = 0,
		AT_VISIBLE_LANDMINE = 1,
	};

    struct ConditionProb
    {
        int16_t encounter_enermy_prob; // 万分数
        int32_t encounter_interval;    // 遇敌间隔
        TemplID monster_group_id;      // 怪物组id
        MapID   battle_scene_id;       // 战斗时切换到的场景id，默认值0，即使用场景或者区域的战斗场景配置
        BoundArray<int32_t, kMaxConditionTask> task_list; // 任务列表
        NESTED_DEFINE(encounter_enermy_prob, encounter_interval, monster_group_id, battle_scene_id, task_list);
    };

	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}


public:
// 0
	BoundArray<Coordinate, kMaxVertexCount>    vertexes; // 区域顶点数组
	TemplID      monster_group_id;        // 怪物组id
	MapID        battle_scene_id;         // 战斗时切换到的场景id，默认值0，即使用场景或者区域的战斗场景配置
	uint8_t      appear_type;             // 对应AppearType枚举，分为：明雷、暗雷
	TemplID      associated_templ_id;     // 关联的数据模板id，可以是monster也可以是NPC

// 5
	int16_t      visible_num;             // 明雷时，怪物的可见数量
	uint8_t      ap_mode;                 // 对应APMode枚举，passive, active mode
	bool         is_wandered;             // 是否随机走动、游荡。默认值false
	uint8_t      combat_rule;             // 对应CombatRule枚举，默认值为NO_MODEL
	int32_t      refresh_interval;        // 怪物消失后的刷新间隔，单位s。默认值为10，0表示不刷新

// 10
	int16_t      encounter_enermy_prob;   // 遇敌概率，单位万分数。默认值为1000，即不遇敌
	int32_t      encounter_interval;      /* 遇敌间隔，单位秒。默认值为5，即没有保护时间。
	                                       * 玩家在一次遇敌战斗结束后，会进入暂时的保护状态，
										   * 在此状态下，不会进行遇敌概率计算
										   * */
	float        aggro_view_radius;       // 仇恨视野半径，单位m，默认值2米，最小值0.5，最大值20
	uint16_t     aggro_time;              // 仇恨时间，超过该时间则解除仇恨，单位s，默认值20秒，最小值10，最大值65530
	float        max_chase_distance;      // 最大追击距离，离开始追击点的直线距离，单位m，默认值4，最小值2，最大值65530

// 15
	float         chase_speed;            // 追击速度，单位m/s，默认值3米每秒，最小值0.1，最大值10
    ConditionProb active_task_cond;       // 活跃任务遇敌条件
    ConditionProb finish_task_cond;       // 完成任务遇到条件


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(vertexes, monster_group_id, battle_scene_id, appear_type, associated_templ_id);
		MARSHAL_MAPDATA(visible_num, ap_mode, is_wandered, combat_rule, refresh_interval);
		MARSHAL_MAPDATA(encounter_enermy_prob, encounter_interval, aggro_view_radius, aggro_time, max_chase_distance);
		MARSHAL_MAPDATA(chase_speed, active_task_cond, finish_task_cond);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(vertexes, monster_group_id, battle_scene_id, appear_type, associated_templ_id);
		UNMARSHAL_MAPDATA(visible_num, ap_mode, is_wandered, combat_rule, refresh_interval);
		UNMARSHAL_MAPDATA(encounter_enermy_prob, encounter_interval, aggro_view_radius, aggro_time, max_chase_distance);
		UNMARSHAL_MAPDATA(chase_speed, active_task_cond, finish_task_cond);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (vertexes.size() < 3)
			return false;

		if (monster_group_id <= 0 || battle_scene_id < 0)
			return false;

		if (appear_type != AT_LANDMINE && appear_type != AT_VISIBLE_LANDMINE)
			return false;

		if ( (appear_type == AT_VISIBLE_LANDMINE && visible_num == 0) ||
			 (appear_type == AT_LANDMINE && visible_num != 0) )
			return false;

		if (PASSIVE != ap_mode && ACTIVE != ap_mode)
			return false;

		if (refresh_interval < 0 || encounter_interval < 0)
			return false;

		if (!check_prob_valid(encounter_enermy_prob))
			return false;

        // 暗雷怪需要检查相关数值
        if (appear_type == AT_LANDMINE)
        {
            if (encounter_interval <= 0)
                return false;

            if (active_task_cond.task_list.size() > 0)
            {
                if (!check_prob_valid(active_task_cond.encounter_enermy_prob))
                    return false;

                if (active_task_cond.encounter_interval <= 0)
                    return false;

                if (active_task_cond.monster_group_id <= 0)
                    return false;

                if (active_task_cond.battle_scene_id < 0)
                    return false;
            }

            if (finish_task_cond.task_list.size() > 0)
            {
                if (!check_prob_valid(finish_task_cond.encounter_enermy_prob))
                    return false;

                if (finish_task_cond.encounter_interval <= 0)
                    return false;

                if (finish_task_cond.monster_group_id <= 0)
                    return false;

                if (finish_task_cond.battle_scene_id < 0)
                    return false;
            }
        }

		// 明雷怪需要检查相关数值
		if (appear_type == AT_VISIBLE_LANDMINE)
		{
			if (NO_MODEL != combat_rule && 
                MODEL_NO_COMBAT != combat_rule && 
                MODEL_WORLD_BOSS != combat_rule &&
                NEVER_COMBAT != combat_rule)
				return false;

			if (aggro_view_radius < 0.5 || aggro_view_radius > 20)
				return false;

			if (aggro_time < 10 || aggro_time > 65530)
				return false;

			if (max_chase_distance < 2 || max_chase_distance > 65530)
				return false;

			if (chase_speed < 0.1 || chase_speed > 10)
				return false;

			if (associated_templ_id <= 0)
				return false;
		}

		return true;
	}
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_AREA_MONSTER_H_
