#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_MOUNT_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_MOUNT_H__

#include "gs/item/item.h"

namespace dataTempl
{
class MountTempl;
}

namespace gamed
{

/**
 * @class ItemMount
 * @brief 坐骑实体
 */
class ItemMount : public ItemBody
{
    const dataTempl::MountTempl* templ;
public:
	explicit ItemMount(int id): ItemBody(id)
	{}
	virtual ~ItemMount()
	{}

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual void OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual void OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual bool DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
};

};

#endif // __GAMED_GS_ITEM_ITEM_BODY_ITEM_MOUNT_H__
