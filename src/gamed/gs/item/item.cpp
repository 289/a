#include <zlib.h>
#include "item.h"
#include "item_manager.h"


namespace gamed {

/************************************************************************************************
 *                                 Class Item
*************************************************************************************************/

void MakeItem(Item& it, const itemdata& data)
{
    ASSERT(data.count > 0);
	it.type = data.id;
	it.count = data.count;
	it.price = data.recycle_price;
	it.pile_limit = data.pile_limit;
	it.proc_type = data.proc_type;
	it.expire_date = data.expire_date;
	it.is_active = false;
    it.ess.LoadEss(data.content);

	it.LocateSelf();
}

void MakeItemdata(itemdata& data, const G2C::ItemDetail& detail)
{
    ASSERT(detail.count > 0);
	data.id = detail.type;
	data.index = detail.index;
	data.count = detail.count;
	data.pile_limit = s_pItemMan->GetItemPileLimit(detail.type);
	data.proc_type = detail.proc_type;
	data.recycle_price = s_pItemMan->GetItemPrice(detail.type);
	data.expire_date = detail.expire_date;
	data.item_cls = s_pItemMan->GetItemCls(detail.type);
    data.content = detail.content;
}

void MakeItemDetail(G2C::ItemDetail& detail, const itemdata& data)
{
	detail.type = data.id;
	detail.index = data.index;
	detail.count = data.count;
	detail.proc_type = data.proc_type;
	detail.expire_date = data.expire_date;

    detail.content = data.content;
    int32_t size = data.content.size();
    const char* buf = data.content.c_str();
    detail.content_crc = size ? adler32(1, (const Bytef*)buf, size) : 0;
}

///
/// Item
///
void Item::LocateSelf()
{
	pBody = s_pItemMan->GetItemBody(type);
	if (pBody)
	{
		equip_mask = pBody->GetEquipMask();
	}
	else
	{
		equip_mask = 0;
	}

	price      = s_pItemMan->GetItemPrice(type);
	item_cls   = s_pItemMan->GetItemCls(type);
	pile_limit = s_pItemMan->GetItemPileLimit(type);
}

void Item::LoadFromDB(const common::PlayerInventoryData::ItemData& data)
{
    ASSERT(type == -1);
    ASSERT(data.type != -1);
    ASSERT(data.count > 0);
    type = data.type;
    count = data.count;
    pile_limit = data.pile_limit;
    proc_type = data.proc_type;
    expire_date = data.expire_date;
    ess.LoadScalableEss(data.content);

    LocateSelf();

    if (pBody) {
        pBody->OnLoadFromDB(this);
    }
}

void Item::SaveForDB(common::PlayerInventoryData::ItemData& data) const
{
    ASSERT(type != -1);
    ASSERT(count > 0);
    data.type = type;
    data.count = count;
    data.pile_limit = pile_limit;
    data.proc_type = proc_type;
    data.expire_date = expire_date;
    data.content.clear();
    ess.SaveScalableEss(data.content);
}

void Item::SaveItem(itemdata& data) const
{
    ASSERT(count > 0);
    data.id = type;
    data.count = count;
    data.pile_limit = pile_limit;
    data.proc_type = proc_type;
    data.expire_date = expire_date;
    data.item_cls = item_cls;
    ess.SaveEss(data.content);
}

void Item::Clear()
{
    type = -1;
    count = 0;
    pile_limit = 0;
    proc_type = 0;
    expire_date = 0;
    price = 0;
    pBody = NULL;
    equip_mask = 0;
    item_cls = 0;
    is_active = false;
    ref = 0;
    ess.Clear();
}

void Item::Release()
{
    Clear();
}

int32_t Item::GetTotalPrice() const
{
    int32_t total = BasePrice() + GetExtraPrice();
    total = (total > 0) ? total : 0;
    return total;
}


///
/// ItemBody
///
bool ItemBody::CanUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    switch(l)
    {
        case Item::INVENTORY:
            return TestUse(l, index, obj, parent);
        default:
            break;
    };
    return false;
}

bool ItemBody::CanRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    switch(l)
    {
        case Item::INVENTORY:
        case Item::EQUIPMENT:
            return TestRefine(l, index, obj, parent);
        default:
            break;
    };
    return false;
}

bool ItemBody::CanActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    switch(l)
    {
        case Item::INVENTORY:
            return TestActivate(l, index, obj, parent);
        default:
            break;
    };
    return false;
}

bool ItemBody::CanComposite(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    switch(l)
    {
        case Item::INVENTORY:
            return TestComposite(l, index, obj, parent);
        default:
            break;
    };
    return false;
}

Item::ITEM_USE ItemBody::UseItem(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::INVENTORY:
            return OnUse(l, index, obj, parent);
        default:
            break;
    };
    return Item::ITEM_USE_FAILURE;
}

void ItemBody::RefineEquip(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::INVENTORY:
            {
                DoRefine(l, index, obj, parent);
            }
            break;
        case Item::EQUIPMENT:
            {
                OnDeactivate(l, index, obj, parent);
                DoRefine(l, index, obj, parent);
                OnActivate(l, index, obj, parent);
            }
            break;
        default:
            break;
    };
}

void ItemBody::OnPutIn(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::EQUIPMENT:
        case Item::HIDE_INV:
        case Item::PET_INV:
        case Item::CARD_INV:
        case Item::MOUNT_INV:
            {
                Activate(l, index, obj, parent);
            }
            break;
        default:
            break;
    };
}

void ItemBody::OnTakeOut(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::EQUIPMENT:
        case Item::HIDE_INV:
        case Item::PET_INV:
        case Item::CARD_INV:
        case Item::MOUNT_INV:
            {
                Deactivate(l, index, obj, parent);
            }
            break;
        default:
            break;
    };
}

bool ItemBody::Update(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::PET_INV:
        case Item::CARD_INV:
            {
                ASSERT(parent->IsActive());
                return DoUpdate(l, index, obj, parent);
            }
            break;
        default:
            ASSERT(false);
            break;
    };
    return false;
}

bool ItemBody::Composite(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::INVENTORY:
            {
                return DoComposite(l, index, obj, parent);
            }
            break;
        default:
            ASSERT(false);
            break;
    };
    return false;
}

void ItemBody::Enable(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::EQUIPMENT:
            {
                DoEnable(l, index, obj, parent);
            }
            break;
        default:
            ASSERT(false);
            break;
    };
}

void ItemBody::Disable(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    switch(l)
    {
        case Item::EQUIPMENT:
            {
                DoDisable(l, index, obj, parent);
            }
            break;
        default:
            ASSERT(false);
            break;
    };
}

void ItemBody::Activate(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    ASSERT(!parent->IsActive());
    parent->SetActive(true);
    if (!parent->TestDisabled())
    {
        OnActivate(l, index, obj, parent);
    }
}

void ItemBody::Deactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    ASSERT(parent->IsActive());
    parent->SetActive(false);
    if (!parent->TestDisabled())
    {
        OnDeactivate(l, index, obj, parent);
    }
    else
    {
        parent->ClrDisabled();
    }
}

void ItemBody::DoEnable(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    ASSERT(parent->IsActive());
    ASSERT(parent->TestDisabled());
    parent->ClrDisabled();
    OnEnable(l, index, obj, parent);
}

void ItemBody::DoDisable(Item::LOCATION l, size_t index, Unit* obj, Item* parent)
{
    ASSERT(parent->IsActive());
    ASSERT(!parent->TestDisabled());
    parent->SetDisabled();
    OnDisable(l, index, obj, parent);
}

} // namespace gamed
