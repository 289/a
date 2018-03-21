#ifndef GAMED_GS_TEMPLATE_DATATEMPL_MINE_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_MINE_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 矿物模板
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class MineTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(MineTempl, TEMPL_TYPE_MINE_TEMPL);
public:
	static const int kMaxVisibleNameLen     = UTF8_LEN(16); // 最多支持16个中文字符
	static const int kMaxProgressNameLen    = UTF8_LEN(12); // 最多支持12个中文字符
	static const int kMaxErasingHintLen     = UTF8_LEN(12); // 最多支持12个中文字符
	static const int kMaxGatherActionName   = UTF8_LEN(16);
	static const int kMaxModelSrcLen        = 512;
	static const int kMaxAwardItemListSize  = 16;
	static const int kMaxTaskCount          = 3;
	static const int kMaxMapElemCount       = 5;
    static const int kMaxGatherFinishEffect = 128;

	inline void set_templ_id(TemplID id) { templ_id = id; }

	enum TurnonMode
	{
		TM_PROGRESSING = 1, // 读条方式
		TM_ERASING,         // 擦除方式
		TM_BLANK_FILLING,   // 填图方式
	};

	enum MineVisibleRule
	{
		MVR_VISIBLE = 1,    // 不满足采集条件时，矿可见
		MVR_INVISIBLE,      // 不满足采集条件时，矿不可见
		MVR_NAME_INVISIBLE, // 不满足采集条件时，矿只有名字不可见
	};

    enum InteractiveRule
    {
        IR_INTERACTIVE_NOT_HINT,  // 满足条件时正常，不满足条件时点击没有提示
        IR_INTERACTIVE_HINT,      // 满足条件时正常，不满足条件时点击有提示
        IR_NOT_COND_NOT_INTER,    // 满足条件时正常，不满足条件时点穿（不可交互） 
        IR_NOT_INTERACTIVE,       // 满足条件时点穿，不满足条件时点穿（不可交互）
    };

    enum ClientCollisionType
    {
        CCT_NONE = 0,  // 无阻挡
        CCT_1X1,       // 1乘1格子的阻挡
        CCT_3X3,       // 3乘3格子的阻挡
        CCT_5X5,       // 5乘5格子的阻挡
        CCT_MAX
    };

	struct AwardItemPair
	{
		int32_t    item_id;
		int32_t    item_count; 
		int32_t    probability; // 万分数
		NESTED_DEFINE(item_id, item_count, probability);
	};

	struct ErasingDisplayItem
	{
		int32_t    pixel_x;  // 图片显示的x坐标，单位：像素点
		int32_t    pixel_y;  // 图片显示的y坐标，单位：像素点
		BoundArray<uint8_t, kMaxModelSrcLen> pic_src; // 图片资源路径
		NESTED_DEFINE(pixel_x, pixel_y, pic_src);
	};


public:
// 0
	BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;   // 游戏里怪物的显示名
	BoundArray<uint8_t, kMaxModelSrcLen>    model_src;      // 模型资源
	BoundArray<uint8_t, kMaxModelSrcLen>    turn_on_action; // 开启后播放动作，矿的动作
	int16_t toa_residence_time;   // 开启后动作组停留时间，动作组播放的最后一帧停留的时间，默认0，单位：s
	int8_t  turn_on_mode;         // 对应枚举TurnonMode

// 5
	int32_t progressing_time;     // 读条时间，单位：ms
	int32_t nonprogress_timeout;  // 非读条开启方式的超时时间，默认值十分钟，单位：ms
	BoundArray<uint8_t, kMaxProgressNameLen> progressing_name;  // 读条名称
	BoundArray<uint8_t, kMaxModelSrcLen>     erasing_base_map;  // 擦图的底图
	BoundArray<uint8_t, kMaxModelSrcLen>     erasing_cover_map; // 擦图的面图

// 10
	BoundArray<uint8_t, kMaxErasingHintLen>  erasing_hint;      // 擦图提示文字
	int32_t required_task_id;     // 开启所需任务id
	int32_t required_item_id;     // 开启所需道具
	int32_t required_item_count;  // 开启所需道具的个数
	bool    is_consume_item;      // 开启是否消耗道具，默认false

// 15
	int16_t deliver_count;        // 发放物品次数
	BoundArray<AwardItemPair, kMaxAwardItemListSize> award_item_list; // 发放物品列表
	BoundArray<int32_t, kMaxTaskCount> deliver_task_list; // 发放任务id	
	BoundArray<int32_t, kMaxTaskCount> recycle_task_list; // 回收任务id（完成任务）
	int32_t skill_id;             // 对开启玩家是否技能（效果id）

// 20
	int32_t exp_award;            // 经验奖励
	int32_t money_award;          // 游戏币奖励
	int32_t monster_group_id;     // 发放战斗怪物组id，有一定概率触发：trigger_combat_prob
	BoundArray<int32_t, kMaxMapElemCount> enable_map_elem_list;  // 开启地图元素id列表
	BoundArray<int32_t, kMaxMapElemCount> disable_map_elem_list; // 关闭地图元素id列表


// 25
	int8_t  cat_vision;           /* 喵类视觉可见等级，只有玩家的喵类视觉等级大于等于该值，
	                               * 才能看见该mine，默认值0，最大值32
								   * */
	int32_t cat_vision_exp;       // 采集成功后，获得的喵类视觉经验
	int8_t  client_collision;     // 是否客户端阻挡，默认0表示否，对应枚举ClientCollisionType
	int8_t  is_client_hide;       // 采集后是否客户端隐藏，默认0表示否，其他表示是
	BoundArray<uint8_t, kMaxModelSrcLen> mineral_icon; // 矿物图标，默认为空，使用默认的图标


// 30
	BoundArray<uint8_t, kMaxGatherActionName> gather_action; // 采集动作名称，默认为：空，表示：播放默认采矿动作
	int8_t  interactive_rule;     // 对应InteractiveRule枚举
	int8_t  info_display_pos;     // 头顶信息显示位置，默认为0         
	int8_t  can_see_me;           // 对应MineVisibleRule枚举，不满足条件时的可见规则
    int32_t fillblank_templ_id;   // 填图模板id，如果是填图开启方式这项必须填一个有效模板

// 35
    ErasingDisplayItem erasing_display_item; // 擦图完成后显示获得的物品图标
	int32_t trigger_combat_prob;  // 采集完成后触发战斗概率, 默认值10000，即百分之百触发战斗
    int32_t trigger_task_id;      // 采集过程中发的任务，默认值0，表示没有任务
    int32_t trigger_task_prob;    // 采集过程中发任务的概率，默认值10000，表示百分之百的概率发任务
    int32_t trigger_task_interval;// 采集过程中发任务的间隔，默认值2000毫秒，表示2秒发一次

// 40
    int32_t trigger_task_times;   // 采集过程中发任务的次数，默认值1，表示发一次任务
    int32_t score_award;          // 学分奖励，默认值0
    int8_t  cat_vision_hint;      // 是否提示玩家使用喵类视觉功能
    BoundArray<uint8_t, kMaxGatherFinishEffect> gather_finish_effect; // 采集完成音效，默认空表示没有音效


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(visible_name, model_src, turn_on_action, toa_residence_time, turn_on_mode);
		MARSHAL_TEMPLVALUE(progressing_time, nonprogress_timeout, progressing_name, erasing_base_map, erasing_cover_map);
		MARSHAL_TEMPLVALUE(erasing_hint, required_task_id, required_item_id, required_item_count, is_consume_item);
		MARSHAL_TEMPLVALUE(deliver_count, award_item_list, deliver_task_list, recycle_task_list, skill_id);
		MARSHAL_TEMPLVALUE(exp_award, money_award, monster_group_id, enable_map_elem_list, disable_map_elem_list);
		MARSHAL_TEMPLVALUE(cat_vision, cat_vision_exp, client_collision, is_client_hide, mineral_icon);
		MARSHAL_TEMPLVALUE(gather_action, interactive_rule, info_display_pos, can_see_me, fillblank_templ_id);
		MARSHAL_TEMPLVALUE(erasing_display_item, trigger_combat_prob, trigger_task_id, trigger_task_prob, trigger_task_interval);
        MARSHAL_TEMPLVALUE(trigger_task_times, score_award, cat_vision_hint, gather_finish_effect);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(visible_name, model_src, turn_on_action, toa_residence_time, turn_on_mode);
		UNMARSHAL_TEMPLVALUE(progressing_time, nonprogress_timeout, progressing_name, erasing_base_map, erasing_cover_map);
		UNMARSHAL_TEMPLVALUE(erasing_hint, required_task_id, required_item_id, required_item_count, is_consume_item);
		UNMARSHAL_TEMPLVALUE(deliver_count, award_item_list, deliver_task_list, recycle_task_list, skill_id);
		UNMARSHAL_TEMPLVALUE(exp_award, money_award, monster_group_id, enable_map_elem_list, disable_map_elem_list);
		UNMARSHAL_TEMPLVALUE(cat_vision, cat_vision_exp, client_collision, is_client_hide, mineral_icon);
		UNMARSHAL_TEMPLVALUE(gather_action, interactive_rule, info_display_pos, can_see_me, fillblank_templ_id);
		UNMARSHAL_TEMPLVALUE(erasing_display_item, trigger_combat_prob, trigger_task_id, trigger_task_prob, trigger_task_interval);
        UNMARSHAL_TEMPLVALUE(trigger_task_times, score_award, cat_vision_hint, gather_finish_effect);
	}

	virtual bool OnCheckDataValidity() const
	{
        if (model_src.size() <= 0)
            return false;

		if (toa_residence_time < 0)
			return false;

		if (turn_on_mode != TM_PROGRESSING && 
			turn_on_mode != TM_ERASING &&
			turn_on_mode != TM_BLANK_FILLING)
			return false;

        if (interactive_rule != IR_INTERACTIVE_NOT_HINT &&
            interactive_rule != IR_INTERACTIVE_HINT &&
            interactive_rule != IR_NOT_COND_NOT_INTER &&
            interactive_rule != IR_NOT_INTERACTIVE)
            return false;

		if (progressing_time < 0 || nonprogress_timeout < 0)
			return false;

		if (required_task_id < 0 || required_item_id < 0 || required_item_count < 0)
			return false;

		if (is_consume_item && (required_item_id <=0 || required_item_count <= 0))
			return false;

		if (deliver_count < 0)
			return false;

		int32_t prob_sum = 0;
		for (size_t i = 0; i < award_item_list.size(); ++i)	
		{
			if (award_item_list[i].item_id <= 0 ||
				award_item_list[i].probability <= 0 || 
				award_item_list[i].probability > 10000 || 
				award_item_list[i].item_count <= 0)
			{
				return false;
			}

			prob_sum += award_item_list[i].probability;
		}

		// 归一化检查
		if (award_item_list.size() && prob_sum != 10000)
			return false;

		for (size_t i = 0; i < deliver_task_list.size(); ++i)
		{
			if (deliver_task_list[i] <= 0)
				return false;
		}

		for (size_t i = 0; i < recycle_task_list.size(); ++i)
		{
			if (recycle_task_list[i] <= 0)
				return false;
		}

		if (skill_id < 0 || exp_award < 0 || money_award < 0 || 
            monster_group_id < 0 || score_award < 0)
			return false;

		for (size_t i = 0; i < enable_map_elem_list.size(); ++i)
		{
			if (enable_map_elem_list[i] <= 0)
				return false;
		}

		for (size_t i = 0; i < disable_map_elem_list.size(); ++i)
		{
			if (disable_map_elem_list[i] <= 0)
				return false;
		}

		if (cat_vision < 0 || cat_vision > 32 || cat_vision_exp < 0)
			return false;

		if (client_collision < CCT_NONE || client_collision >= CCT_MAX)
			return false;

		if (info_display_pos < 0)
			return false;

		if (can_see_me != MVR_VISIBLE && 
			can_see_me != MVR_INVISIBLE && 
			can_see_me != MVR_NAME_INVISIBLE)
		{
			return false;
		}

		if (turn_on_mode == TM_ERASING)
		{
			if (nonprogress_timeout < 1000)
				return false;
		}

		if (turn_on_mode == TM_BLANK_FILLING)
		{
			if (nonprogress_timeout < 1000 || fillblank_templ_id <= 0)
				return false;
		}

		if (trigger_combat_prob < 0 || trigger_combat_prob > 10000)
			return false;

        if (trigger_task_id < 0)
            return false;

        if (trigger_task_prob < 0 || trigger_task_prob > 10000)
            return false;

        if (trigger_task_interval < 0)
            return false;

        if (trigger_task_times < 0)
            return false;

        if (trigger_task_id > 0)
        {
            if (trigger_task_prob == 0 || trigger_task_interval == 0 || trigger_task_times == 0)
                return false;
        }

        if (cat_vision_hint != 0 && cat_vision_hint != 1)
            return false;

		return true;
	}
}; 

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_MINE_TEMPL_H_
