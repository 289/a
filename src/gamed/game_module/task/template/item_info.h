#ifndef TASK_ITEM_INFO_H_
#define TASK_ITEM_INFO_H_

#include "basic_info.h"

namespace task
{

// 需要与包裹的类型匹配
enum ItemType
{
	ITEM_COMMON,
	ITEM_EQUIP,
	ITEM_TASK,
	ITEM_HIDE,
};

class ItemInfo
{
public:
	ItemInfo()
		: id(0), count(0)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t id;
	int32_t count;

	NESTED_DEFINE(id, count);
};
typedef std::vector<ItemInfo> ItemInfoVec;

class ItemDeliveredInfo : public ItemInfo
{	
public:
	ItemDeliveredInfo()
		: flag(0), type(ITEM_COMMON), prob(100), valid_time(0)
	{
	}

	inline bool CheckDataValidity() const;

    int8_t flag;
	int8_t type;
	int32_t prob;
	int32_t valid_time;

	NESTED_DEFINE(id, count, flag, type, prob, valid_time); 
};
typedef std::vector<ItemDeliveredInfo> ItemDeliveredVec;

inline bool ItemInfo::CheckDataValidity() const
{
	return id != 0 && count >= 0;
}

inline bool ItemDeliveredInfo::CheckDataValidity() const
{
	if (!ItemInfo::CheckDataValidity())
	{
		return false;
	}
	if (prob <= 0 || valid_time < 0)
	{
		return false;
	}
	CHECK_INRANGE(type, ITEM_COMMON, ITEM_HIDE)
	return true;
}

} // namespace task

#endif // TASK_ITEM_INFO_H_
