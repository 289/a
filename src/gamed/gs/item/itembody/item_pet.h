#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_PET_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_PET_H__

#include "gs/item/item.h"

namespace gamed
{

struct pet_data
{
	int32_t pet_id;
	int16_t pet_level;
	int16_t pet_blevel;
};

/**
 * @class ItemPet
 * @brief 宠物实体
 */
class ItemPet : public ItemBody
{
private:
	pet_data data;

public:
	explicit ItemPet(int id): ItemBody(id)
	{ }
	virtual ~ItemPet()
	{ }

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual void OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual void OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual bool DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
};

}; // namespace gamed

#endif // __GAMED_GS_ITEM_ITEM_BODY_ITEM_PET_H__
