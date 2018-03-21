#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_TASK_SCROLL_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_TASK_SCROLL_H__

#include "gs/item/item.h"
#include "gs/item/item_manager.h"


namespace dataTempl {
    class TaskScrollTempl;
} // namespace dataTempl


namespace gamed {

/**
 * @class ItemTaskScroll
 * @brief 任务卷轴物品的实体类
 */
class ItemTaskScroll : public ItemBody
{
public:
	explicit ItemTaskScroll(int id)
        : ItemBody(id),
          consume_on_use_(false),
          consume_item_id_(0),
          consume_item_count_(0),
          cd_group_id_(0),
          cd_group_time_(0),
          cd_time_(0),
          ptpl_(NULL)
	{ }

	virtual ~ItemTaskScroll()
	{ }

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual bool TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual Item::ITEM_USE OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;


private:
    struct TaskEntry
	{
		int32_t task_id;
		int32_t probability;
		TaskEntry(): task_id(0),probability(0) {}
	};
    
	bool    consume_on_use_;       // 使用后是否销毁
	int32_t consume_item_id_;      // 使用物品消耗的道具ID
	int32_t consume_item_count_;   // 使用物品消耗的道具个数
        
    int32_t cd_group_id_;     // 冷却组
    int32_t cd_group_time_;   // 冷却组冷却时间
    int32_t cd_time_;         // 物品自身的冷却时间

    const dataTempl::TaskScrollTempl* ptpl_; // 模板指针

	std::vector<TaskEntry> tasks_; // 使用物品的任务奖励列表
};

} // namespace gamed

#endif
