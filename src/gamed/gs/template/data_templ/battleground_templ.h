#ifndef GAMED_GS_TEMPLATE_DATATEMPL_BATTLEGROUND_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_BATTLEGROUND_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

class BattleGroundTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(BattleGroundTempl, TEMPL_TYPE_BATTLEGROUND);
public:
    static const int kMaxVisibleNameLen   = UTF8_LEN(16);   // 最多支持16个中文字符
	static const int kMaxBGDescription    = UTF8_LEN(1024); // 最多支持1024个中文字符
    static const int kMaxBGEntrancePos    = 6;              // 最多6个进入点坐标
    static const int kMaxFactionCount     = 4;              // 最大的阵营个数

    enum ItemProcType
	{
		IPT_INVALID = 0, // 默认为无效
		IPT_CHECK,       // 只做检查，玩家包裹里是否有该物品
		IPT_TAKE_OUT,    // 收取玩家包裹里该物品
	};

    enum BGType
	{
        BGT_INVALID = 0,    // 默认为无效
		BGT_PVE_RALLY,      // PVE活动战场，跨服战场
        BGT_PVE_WORLD_BOSS, // 世界BOSS战场，本服战场
	};

    enum UIStyle
    {
        UIS_NORMAL = BG_UI_STYLE_SEG, // 普通战场
        UIS_PVE_RALLY,   // 活动战场
        UIS_WORLD_BOSS,  // 世界BOSS战场
    };

    // 战场地图内的入口坐标
	struct BGCoord
	{
        BGCoord() : x(0.f), y(0.f) { }
		float  x;
		float  y;
		NESTED_DEFINE(x, y);
	};

    // 战场的阵营信息
    struct FactionInfo
    {
        BoundArray<BGCoord, kMaxBGEntrancePos> entrance_pos;
        NESTED_DEFINE(entrance_pos);
    };

    struct WorldBossBGData
    {
        int32_t wb_tid;              // 世界BOSS的怪物模板id
        int32_t enter_limit_per_day; // 每天能开启的BOSS的次数，默认值是1，表示一天一次
        NESTED_DEFINE(wb_tid, enter_limit_per_day);
    };

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
// 0
	BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;      // 游戏里战场的显示名
	BoundArray<uint8_t, kMaxBGDescription>  bg_description;    // 战场描述
	MapID            bg_map_id;    // 战场使用的地图编号，必须填写范围是7000 ~ 7999，默认值为0，为0时编辑器不予导出
    FixedArray<FactionInfo, kMaxFactionCount> faction_list;    /* 阵营信息，没有特别指定时玩家默认在阵营0，
                                                                * 阵营的敌对关系每种类型的战场自己定，
                                                                * 默认情况下，阵营0和1是敌对的，怪物默认在阵营1
                                                                */
    int8_t           bg_type;      // 战场类型，对应BGType枚举

// 5
    int32_t          bg_max_survival_time;       // 战场持续时间，默认值：3600，单位秒，不能低于300秒
    int32_t          bg_supplement_time;         // 战场开始后多长时间内还允许补人，单位s，默认值：300秒，0是非法值，不能低于60秒
    int32_t          bg_player_upper_limit;      // 战场玩家数量上限，默认值：30，0是非法值
    int32_t          bg_player_lower_limit;      // 战场玩家数量下限，默认值：1，0是非法值
    int32_t          entrance_limit_per_day;     // 战场每日进入次数限制，默认：0，表示没有限制

// 5
    TemplID          entrance_require_item;      // 进入所需物品id，默认值：0
	int32_t          entrance_item_count;        // 进入所需物品数量，默认值：0
	int32_t          entrance_item_proc;         /* 默认值为0，对应ItemProcType枚举，进入所需物品的处理方式，
											      * entrance_require_item为0时，此项无效
											      */
	int32_t          entrance_require_money;     // 进入所需金钱，默认值：0
	int32_t          entrance_require_cash;      // 进入所需的元宝，默认值：0

// 10
	int32_t          entrance_reputation_id;     // 进入所需声望id，默认值：0
	int32_t          entrance_reputation_value;  // 进入所需声望值，默认值：0
	int32_t          level_lower_limit;          // 玩家进入的等级下限，默认值：0 表示不限制
	int32_t          level_upper_limit;          // 玩家进入的等级上限，默认值：0 表示不限制 
	TimeSegment      time_seg;                   // 副本开启时间段

// 15
    int32_t          entrance_require_task;      // 进入副本所需携带的任务，指ActiveTask。默认值：0 表示没有要求
    int32_t          entrance_req_finish_task;   // 进入副本所需已完成任务，指FinishTask。默认值：0 表示没有要求
	int32_t          bg_script_id;               // 战场脚本id，0表示没有，有值则必须等于自己的模板id
    int8_t           auto_create_team;           // 战场是否有预组队功能，默认值：0 表示没有预组队，1表示需要预组队
    int8_t           ui_style;                   // 对应枚举UIStyle，默认值：UIS_NORMAL

// 20
    int32_t          gevent_group_id;            // 活动组模板id，客户端界面使用
    WorldBossBGData  worldboss_data;             // 世界BOSS战场的数据

    
protected:
	virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(visible_name, bg_description, bg_map_id, faction_list, bg_type);
        MARSHAL_TEMPLVALUE(bg_max_survival_time, bg_supplement_time, bg_player_upper_limit, bg_player_lower_limit, entrance_limit_per_day);
        MARSHAL_TEMPLVALUE(entrance_require_item, entrance_item_count, entrance_item_proc, entrance_require_money, entrance_require_cash);
        MARSHAL_TEMPLVALUE(entrance_reputation_id, entrance_reputation_value, level_lower_limit, level_upper_limit, time_seg);
        MARSHAL_TEMPLVALUE(entrance_require_task, entrance_req_finish_task, bg_script_id, auto_create_team, ui_style);
        MARSHAL_TEMPLVALUE(gevent_group_id, worldboss_data);
    }
	
    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(visible_name, bg_description, bg_map_id, faction_list, bg_type);
        UNMARSHAL_TEMPLVALUE(bg_max_survival_time, bg_supplement_time, bg_player_upper_limit, bg_player_lower_limit, entrance_limit_per_day);
        UNMARSHAL_TEMPLVALUE(entrance_require_item, entrance_item_count, entrance_item_proc, entrance_require_money, entrance_require_cash);
        UNMARSHAL_TEMPLVALUE(entrance_reputation_id, entrance_reputation_value, level_lower_limit, level_upper_limit, time_seg);
        UNMARSHAL_TEMPLVALUE(entrance_require_task, entrance_req_finish_task, bg_script_id, auto_create_team, ui_style);
        UNMARSHAL_TEMPLVALUE(gevent_group_id, worldboss_data);
    }
	
	virtual bool OnCheckDataValidity() const
    {
        if (bg_map_id < 7000 || bg_map_id > 7999)
            return false;

        if (bg_type != BGT_PVE_RALLY && bg_type != BGT_PVE_WORLD_BOSS)
            return false;

        // 分类型检查入口点
        if (bg_type == BGT_PVE_RALLY || bg_type == BGT_PVE_WORLD_BOSS)
        {
            // 活动战场、世界BOSS战场只考虑阵营A的入口
            if (faction_list[0].entrance_pos.size() <= 0)
                return false;
        }

        for (size_t i = 0; i < faction_list.size(); ++i)
        {
            for (size_t j = 0; j < faction_list[i].entrance_pos.size(); ++j)
            {
                if (faction_list[i].entrance_pos[j].x < -129 ||
                    faction_list[i].entrance_pos[j].y < -129)
                {
                    return false;
                }
            }
        }

        if (bg_supplement_time < 60 || bg_max_survival_time < 300 || bg_max_survival_time <= bg_supplement_time)
            return false;

        if (bg_player_upper_limit <= 0 || bg_player_lower_limit <= 0)
            return false;

        if (bg_player_upper_limit < bg_player_lower_limit)
            return false;

        if (entrance_limit_per_day < 0)
            return false;

        if (entrance_require_item < 0)
			return false;

		if (entrance_require_item > 0 && entrance_item_count <= 0)
			return false;

		if (entrance_require_item > 0 && (entrance_item_proc != IPT_CHECK && entrance_item_proc != IPT_TAKE_OUT))
			return false;

		if (entrance_require_money < 0 || entrance_require_cash < 0)
			return false;

		if (entrance_reputation_id > 0 && entrance_reputation_value < 0)
			return false;

		if (level_lower_limit < 0 || level_upper_limit < 0)
			return false;

		if (level_upper_limit > 0)
		{
			if (level_upper_limit < level_lower_limit)
				return false;
		}

        if (entrance_require_task < 0)
			return false;

		if (entrance_req_finish_task < 0)
			return false;

        if (bg_script_id != 0 && bg_script_id != templ_id)
			return false;

        if (bg_type == BGT_PVE_WORLD_BOSS)
        {
            if (worldboss_data.wb_tid <= 0)
                return false;
        }

        if (ui_style != UIS_NORMAL &&
            ui_style != UIS_PVE_RALLY &&
            ui_style != UIS_WORLD_BOSS)
            return false;

        if (ui_style == UIS_PVE_RALLY && gevent_group_id <= 0)
            return false;

        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_BATTLEGROUND_TEMPL_H_
