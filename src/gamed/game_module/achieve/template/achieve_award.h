#ifndef ACHIEVE_ACHIEVE_AWARD_H_
#define ACHIEVE_ACHIEVE_AWARD_H_

#include "achieve_types.h"

namespace achieve
{

// 需要与包裹的类型匹配
enum ItemType
{
	ITEM_COMMON,
	ITEM_EQUIP,
	ITEM_TASK,
	ITEM_HIDE,
};

struct ItemInfo
{
public:
    ItemInfo()
        : type(ITEM_COMMON), id(0), count(0)
    {
    }

    inline bool CheckDataValidity() const;

    int8_t type;
    int32_t id;
    int32_t count;

    NESTED_DEFINE(type, id, count);
};

class AchieveAward
{
public:
    AchieveAward()
        : title_id(0)
    {
    }

    inline bool CheckDataValidity() const;

    ItemInfo item;
    int32_t title_id;

    NESTED_DEFINE(item, title_id);
};

inline bool ItemInfo::CheckDataValidity() const
{
    if (id < 0 || count < 0)
    {
        return false;
    }
	CHECK_INRANGE(type, ITEM_COMMON, ITEM_HIDE)
	return true;
}

inline bool AchieveAward::CheckDataValidity() const
{
    CHECK_VALIDITY(item)
    return true;
}

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_AWARD_H_
