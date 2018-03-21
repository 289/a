#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_CARD_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_CARD_H__

#include "gs/item/item.h"

namespace dataTempl
{
class CardTempl;
}

namespace gamed
{

/**
 * @class ItemCard
 * @brief 卡牌实体
 */
class ItemCard : public ItemBody
{
	//card_data data;
    const dataTempl::CardTempl* templ;

public:
	explicit ItemCard(int id): ItemBody(id)
	{}
	virtual ~ItemCard()
	{}

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual void OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual void OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual bool DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
};

};

#endif // __GAMED_GS_ITEM_ITEM_BODY_ITEM_CARD_H__
