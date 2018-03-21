#ifndef __GAMED_GS_TEMPLATE_DATA_TEMPL_TASK_SCROLL_TEMPL_H__
#define __GAMED_GS_TEMPLATE_DATA_TEMPL_TASK_SCROLL_TEMPL_H__

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @class TaskScrollTempl
 * @brief 任务卷轴模板
 */
class TaskScrollTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(TaskScrollTempl, TEMPL_TYPE_TASK_SCROLL);
public:
	static const int kMaxErrorMsgLen = UTF8_LEN(16);

	struct TaskEntry
	{
		int32_t task_id;
		int32_t probability;
		NESTED_DEFINE(task_id, probability);
	};

    enum ForbidMapType
    {
        FMT_NONE = 0,  // 全地图都可以使用
        FMT_NONNORMAL, // 仅普通地图可以使用(非普通地图禁止使用)
    };

    struct SpecifyMap
    {
        int32_t map_id;        // 默认值：0表示没有指定地图
        Coordinate min_coord;  // 默认值：(0,0), 最小坐标
        Coordinate max_coord;  // 默认值：(1,1)，最大坐标
        NESTED_DEFINE(map_id, min_coord, max_coord);
    };
	
    inline void set_templ_id(TemplID id) { templ_id = id; }
    

public:
    // 0
	bool    consume_on_use;             // 使用后是否销毁
	TemplID consume_item_id;            // 使用物品消耗的道具ID
	int32_t consume_item_count;         // 使用物品消耗的道具个数
	BoundArray<TaskEntry,10> task_list; // 使用物品的任务奖励列表，没有归一化，小于10000不归一化
	BoundArray<uint8_t, kMaxErrorMsgLen> error_msg;// 玩家接收任务失败时的错误提示

    // 5
    ItemCoolDownData cooldown_data;     // 冷却数据
    int8_t forbid_map_type;             // 禁止使用的地图类型，默认值：FMT_NONE，对应ForbidMapType枚举
    SpecifyMap specify_map;             // 指定地图可以使用


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(consume_on_use, consume_item_id, consume_item_count, task_list, error_msg);
        MARSHAL_TEMPLVALUE(cooldown_data, forbid_map_type, specify_map);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(consume_on_use, consume_item_id, consume_item_count, task_list, error_msg);
        UNMARSHAL_TEMPLVALUE(cooldown_data, forbid_map_type, specify_map);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (consume_item_id < 0 ||
            consume_item_count < 0)
			return false;

		for (size_t i = 0; i < task_list.size(); ++ i)
		{
			const TaskEntry& task = task_list[i];
			if (task.task_id <= 0 ||
				task.probability <= 0 ||
				task.probability > 10000)
				return false;
		}

        if (!cooldown_data.is_valid())
            return false;

        if (forbid_map_type != FMT_NONE && 
            forbid_map_type != FMT_NONNORMAL)
            return false;

        if (specify_map.map_id > 0)
        {
            if (specify_map.min_coord.x == specify_map.max_coord.x &&
                specify_map.min_coord.y == specify_map.max_coord.y)
                return false;

            if (specify_map.min_coord.x > specify_map.max_coord.x ||
                specify_map.min_coord.y > specify_map.max_coord.y)
                return false;
        }

		return true;
	}
};

}; // namespace dataTempl

#endif
