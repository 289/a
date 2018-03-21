#ifndef GAMED_GS_TEMPLATE_DATATEMPL_INSTANCE_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_INSTANCE_TEMPL_H_

#ifdef PLATFORM_WINDOWS
#include <time.h>
#endif // PLATFORM_WINDOWS

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class InstanceTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(InstanceTempl, TEMPL_TYPE_INSTANCE_TEMPL);
public:
	static const int kMaxVisibleNameLen     = UTF8_LEN(16);   // 最多支持16个中文字符
	static const int kMaxInsDescription     = UTF8_LEN(1024); // 最多支持1024个中文字符
	static const int kMaxAutoTaskCount      = 3; 
	static const int kMaxKeyMonsterCount    = 10;
	
	enum InstanceType
	{
		IT_SOLO = 0,  // 单人
		IT_TEAM,      // 组队
		IT_UNION,     // 公会
	};

	enum ItemProcType
	{
		IPT_INVALID = 0, // 默认为无效
		IPT_CHECK,       // 只做检查，玩家包裹里是否有该物品
		IPT_TAKE_OUT,    // 收取玩家包裹里该物品
	};

    enum UIStyle
    {
        UIS_NORMAL_COUNTDOWN = INS_UI_STYLE_SEG, // 普通副本倒计时
        UIS_NORMAL_TIMING,      // 普通副本计时
        UIS_GEVENT_TIMING,      // 活动副本计时
        UIS_GEVENT_COUNTDOWN,   // 活动副本倒计时
    };

	// 副本地图内的入口坐标
	struct InstanceCoord
	{
		float  x;
		float  y;
		NESTED_DEFINE(x, y);
	};

	// 副本通关奖励
	struct InsCompleteAward
	{
		int32_t exp;   // 副本通关后获得的经验奖励，默认值：0表示没有经验奖励
		int32_t money; // 副本通关后获得的游戏币，默认值：0表示没有游戏币奖励
		int32_t normal_droptable;  // 普通物品奖励，掉落包id。默认值：0表示没有普通物品
		int32_t special_droptable; // 普通抽奖物品奖励，掉落包id。默认值：0表示没有抽奖物品
		NESTED_DEFINE(exp, money, normal_droptable, special_droptable);
	};

	inline void set_templ_id(TemplID id) { templ_id = id; }

public:
// 0
	BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;    // 游戏里副本的显示名
	BoundArray<uint8_t, kMaxInsDescription> ins_description; // 副本描述
	MapID            ins_map_id;   // 副本使用的地图编号，必须填写范围是6000 ~ 6999，默认值为0，为0时编辑器不予导出
	InstanceCoord    entrance_pos; // 进入副本后的初始坐标位置，默认x、y都是0
	int32_t          ins_type;     // 默认值为0（单人），配置副本的归属类型，对应InstanceType枚举

// 5
	TemplID          create_require_item;        // 玩家创建副本所需要的物品id，默认为0，表示不需要物品
	int32_t          create_item_count;          // 创建副本所需物品数量
	int8_t           create_item_proc;           // 默认值为0，对应ItemProcType枚举，create_require_item为零时，该项无效，
	int32_t          require_create_money;       // 创建副本需要的金钱，默认值为0
	int32_t          require_create_cash;        // 创建副本需要的元宝，默认值为0

// 10
	int32_t          create_reputation_id;       // 创建所需声望id，默认值：0
	int32_t          create_reputation_value;    // 创建所需声望值，默认值：0
	TemplID          entrance_require_item;      // 进入所需物品id，默认值：0
	int32_t          entrance_item_count;        // 进入所需物品数量，默认值：0
	int32_t          entrance_item_proc;         /* 默认值为0，对应ItemProcType枚举，进入所需物品的处理方式，
											      * entrance_require_item为0时，此项无效
											      */
// 15
	int32_t          entrance_require_money;     // 进入所需金钱，默认值：0
	int32_t          entrance_require_cash;      // 进入所需的元宝，默认值：0
	int32_t          entrance_reputation_id;     // 进入所需声望id，默认值：0
	int32_t          entrance_reputation_value;  // 进入所需声望值，默认值：0
	int32_t          level_lower_limit;          // 玩家进入的等级下限，默认值：0 表示不限制

// 20
	int32_t          level_upper_limit;          // 玩家进入的等级上限，默认值：0 表示不限制 
	int32_t          ins_max_survival_time;      // 副本创建多少时间后自动重置（强制）,单位s，默认值：3600
	int32_t          ins_remain_time;            // 副本最后一名玩家离开多长时间后重置副本，单位s，默认值：600
	BoundArray<int32_t, kMaxAutoTaskCount> ins_auto_tasks; // 玩家进入副本后自动接到的任务id，默认值：0 表示没有
	TimeSegment      time_seg;                   // 副本开启时间段

// 25
	BoundArray<int32_t, kMaxKeyMonsterCount> ins_key_monster; // 副本关键怪物，只有key monster都被打死，副本才会主动关闭
	int32_t          town_portal_map_id;         // 回城地图号，通过传送进入副本的都会传送回这张地图
	InstanceCoord    town_portal_pos;            // 回城地图坐标，通过传送进入副本的都会传送回这个坐标
	int32_t          ins_script_id;              // 副本脚本id，0表示没有，有值则必须等于自己的模板id
	int32_t          entrance_require_task;      // 进入副本所需携带的任务，指ActiveTask。默认值：0 表示没有要求

// 30
    int32_t          entrance_req_finish_task;   // 进入副本所需已完成任务，指FinishTask。默认值：0 表示没有要求
	int32_t          bronze_record_time;         // 副本铜牌记录时间，单位：秒，默认值：0表示没有铜牌记录时间
	int32_t          silver_record_time;         // 副本银牌记录时间，单位默认值同上
	int32_t          gold_record_time;           // 副本金牌记录时间，单位默认值同上
	TimeSegment      record_reset_time_seg;      // 副本记录重置时间段，清空时间，暂时没有使用

// 35
    InsCompleteAward normal_award;               // 副本通关普通奖励
	InsCompleteAward bronze_award;               // 副本通关铜牌奖励
	InsCompleteAward silver_award;               // 副本通关银牌奖励
	InsCompleteAward gold_award;                 // 副本通关金牌奖励
	InsCompleteAward srv_record_award;           // 打破服务器该副本通关记录的奖励

// 40
	int8_t           has_srv_record;             // 该副本有服务器记录，默认值：0表示没有记录; 值是1表示有记录
    int8_t           ui_style;                   // 对应枚举UIStyle，默认值：UIS_NORMAL_COUNTDOWN
    int32_t          gevent_group_id;            // 活动组模板id，客户端界面使用
    

protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(visible_name, ins_description, ins_map_id, entrance_pos, ins_type);
		MARSHAL_TEMPLVALUE(create_require_item, create_item_count, create_item_proc, require_create_money, require_create_cash);
		MARSHAL_TEMPLVALUE(create_reputation_id, create_reputation_value, entrance_require_item, entrance_item_count, entrance_item_proc);
		MARSHAL_TEMPLVALUE(entrance_require_money, entrance_require_cash, entrance_reputation_id, entrance_reputation_value, level_lower_limit);
		MARSHAL_TEMPLVALUE(level_upper_limit, ins_max_survival_time, ins_remain_time, ins_auto_tasks, time_seg);
		MARSHAL_TEMPLVALUE(ins_key_monster, town_portal_map_id, town_portal_pos, ins_script_id, entrance_require_task);
		MARSHAL_TEMPLVALUE(entrance_req_finish_task, bronze_record_time, silver_record_time, gold_record_time, record_reset_time_seg);
		MARSHAL_TEMPLVALUE(normal_award, bronze_award, silver_award, gold_award, srv_record_award);
		MARSHAL_TEMPLVALUE(has_srv_record, ui_style, gevent_group_id);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(visible_name, ins_description, ins_map_id, entrance_pos, ins_type);
		UNMARSHAL_TEMPLVALUE(create_require_item, create_item_count, create_item_proc, require_create_money, require_create_cash);
		UNMARSHAL_TEMPLVALUE(create_reputation_id, create_reputation_value, entrance_require_item, entrance_item_count, entrance_item_proc);
		UNMARSHAL_TEMPLVALUE(entrance_require_money, entrance_require_cash, entrance_reputation_id, entrance_reputation_value, level_lower_limit);
		UNMARSHAL_TEMPLVALUE(level_upper_limit, ins_max_survival_time, ins_remain_time, ins_auto_tasks, time_seg);
		UNMARSHAL_TEMPLVALUE(ins_key_monster, town_portal_map_id, town_portal_pos, ins_script_id, entrance_require_task);
		UNMARSHAL_TEMPLVALUE(entrance_req_finish_task, bronze_record_time, silver_record_time, gold_record_time, record_reset_time_seg);
		UNMARSHAL_TEMPLVALUE(normal_award, bronze_award, silver_award, gold_award, srv_record_award);
		UNMARSHAL_TEMPLVALUE(has_srv_record, ui_style, gevent_group_id);
	}
		
	virtual bool OnCheckDataValidity() const
	{
		// 6k表示本服副本
		if (ins_map_id < 6000 || ins_map_id > 6999)
			return false;

		if (ins_type != IT_SOLO && ins_type != IT_TEAM && ins_type != IT_UNION)
			return false;

		if (create_require_item < 0)
			return false;

		if (create_require_item > 0 && create_item_count <= 0)
			return false;

		if (create_require_item > 0 && (create_item_proc != IPT_CHECK && create_item_proc != IPT_TAKE_OUT))
			return false;

		if (create_reputation_id > 0 && create_reputation_value < 0)
			return false;

		if (require_create_cash < 0)
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

		if (ins_max_survival_time <= 0)
			return false;

		if (ins_remain_time <= 0)
			return false;

		if (ins_max_survival_time < ins_remain_time)
			return false;

		for (size_t i = 0; i < ins_auto_tasks.size(); ++i)
		{
			if (ins_auto_tasks[i] <= 0)
				return false;
		}

		if (time_seg.is_valid && !CheckTimeSegment(time_seg))
			return false;

		if (town_portal_map_id <= 0 || town_portal_map_id >= 3000)
			return false;

		if (town_portal_map_id >= 1000 && town_portal_map_id <= 1999)
			return false;

		if (ins_script_id != 0 && ins_script_id != templ_id)
			return false;

		if (entrance_require_task < 0)
			return false;

		if (entrance_req_finish_task < 0)
			return false;

		if (!((gold_record_time == 0 && silver_record_time == 0 && bronze_record_time == 0) || 
			  (gold_record_time > 0 && silver_record_time > 0 && bronze_record_time > 0)))
		{
			return false;
		}

		if (gold_record_time > 0)
		{
			if (silver_record_time < gold_record_time)
				return false;

			if (bronze_record_time < gold_record_time)
				return false;

			if (bronze_record_time < silver_record_time)
				return false;
		}

		if (has_srv_record && gold_record_time <= 0)
			return false;

        if (ui_style != UIS_NORMAL_COUNTDOWN &&
            ui_style != UIS_NORMAL_TIMING &&
            ui_style != UIS_GEVENT_TIMING &&
            ui_style != UIS_GEVENT_COUNTDOWN)
            return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_INSTANCE_TEMPL_H_
