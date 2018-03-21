#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_TALENT_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_TALENT_H__

#include "gs/item/item.h"

namespace gamed
{

/**
 * @class ItemTalent
 * @brief 卡牌实体
 */
class ItemTalent : public ItemBody
{
private:
    struct talent_data
    {
        uint8_t item_func_type;
        int32_t talent_group_id;
        int32_t talent_id;
    };

	talent_data data;

public:
	explicit ItemTalent(int id): ItemBody(id)
	{}
	virtual ~ItemTalent()
	{}

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual bool TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual Item::ITEM_USE OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
};

};

#endif // __GAMED_GS_ITEM_ITEM_BODY_ITEM_TALENT_H__
