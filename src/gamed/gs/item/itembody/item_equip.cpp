#include "item_equip.h"

#include "gs/player/player.h"
#include "gs/template/data_templ/equip_templ.h"

namespace gamed {

int ItemEquip::GetRank(const Item* parent) const
{
	return data.quality;
}

int ItemEquip::GetEquipMask() const
{
	return data.equip_mask;
}

int ItemEquip::GetExtraPrice(const Item* parent) const
{
	int extra_price = 0;
	int refine_lvl = GetRefineLevel(parent);
	for (int lvl = 1; lvl <= refine_lvl; ++ lvl)
	{
		const RefineEntry& entry = GetRefineEntry(lvl);
		extra_price += entry.money;
	}

	return extra_price * 0.5f;
}

ClassMask ItemEquip::GetClsLimit() const
{
	return data.cls_limit;
}

void ItemEquip::Initialize(const dataTempl::ItemDataTempl* tpl)
{
	//初始化基本数据
	ASSERT(tpl);
	const EquipTempl* pTpl = dynamic_cast<const dataTempl::EquipTempl*>(tpl);
	data.quality           = pTpl->quality;
	data.equip_mask        = pTpl->equip_mask;
	data.cls_limit         = pTpl->cls_limit;
	data.lvl_limit         = pTpl->lvl_limit;
	data.refine_tid        = pTpl->refine_tpl_id;
	data.init_slot_count   = pTpl->init_slot_count;
	data.extend_slot_count = pTpl->extend_slot_count;
	for (size_t i = 0; i < pTpl->addon_list.size(); ++ i)
	{
		data.addon_list.push_back(pTpl->addon_list[i]);
	}

	//初始化其它数据
	if (data.refine_tid > 0)
	{
		if (s_pItemMan->IsValidRefineTable(data.refine_tid))
		{
			max_refine_lvl_ = GetRefineTable().size();
		}
		else
		{
			ASSERT(false && "非法精练模板ID");
		}
	}
}

bool ItemEquip::TestRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	if (!data.refine_tid)
	{
		return false;
	}

	Player* player = (Player*)obj;
    ItemEssence& item_ess = parent->GetItemEssence();
    const EssRefine* pRefine = item_ess.QueryEss<EssRefine>(ESS_ID_REFINE);
    if (pRefine == NULL)
    {
        // 首次精炼
        const RefineEntry& entry = GetRefineEntry(1);
        return player->GetMoney() >= entry.money;
    }
    if (pRefine->refine_lvl >= max_refine_lvl_)
    {
        return false;
    }
	const RefineEntry& entry = GetRefineEntry(pRefine->refine_lvl);
	if (entry.money > player->GetMoney())
	{
		//金钱不足
		return false;
	}

	return true;
}

bool ItemEquip::TestActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	Player* player = (Player*)obj;
	if (!(player->class_mask() & data.cls_limit))
		return false;

	if (player->level() < (int)(data.lvl_limit))
		return false;

	if (!(data.equip_mask & (1 << index)))
		return false;

	return true;
}

void ItemEquip::OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	int inc_prop_ratio = 0;
	int refine_lvl = GetRefineLevel(parent);
	for (int lvl = 1; lvl <= refine_lvl; ++ lvl)
	{
		const RefineEntry& entry = GetRefineEntry(lvl);
		inc_prop_ratio += entry.ratio;
	}

	Player* player = dynamic_cast<Player*>(obj);
	for (size_t prop_index = 0; prop_index < data.addon_list.size(); ++ prop_index)
	{
		/*prop为基础属性+精练提升的属性*/
		int32_t prop = data.addon_list[prop_index] * (1 + inc_prop_ratio / (float)10000);
		player->IncEquipPoint(prop_index, prop);
	}
}

void ItemEquip::OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	int inc_prop_ratio = 0;
	int refine_lvl = GetRefineLevel(parent);
	for (int lvl = 1; lvl <= refine_lvl; ++ lvl)
	{
		const RefineEntry& entry = GetRefineEntry(lvl);
		inc_prop_ratio += entry.ratio;
	}

	Player* player = dynamic_cast<Player*>(obj);
	for (size_t prop_index = 0; prop_index < data.addon_list.size(); ++ prop_index)
	{
		/*prop为基础属性+精练提升的属性*/
		int32_t prop = data.addon_list[prop_index] * (1 + inc_prop_ratio / (float)10000);
		player->DecEquipPoint(prop_index, prop);
	}
}

void ItemEquip::OnEnable(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	//装备处于生效状态，重新让装备生效
	//这里直接调用OnActivate函数把装备属性加到玩家身上
	OnActivate(l, index, obj, parent);
}

void ItemEquip::OnDisable(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	//让装备失效，但是不卸载状态
	//这里直接调用OnDeactivate函数来去除装备对玩家属性的影响。
	OnDeactivate(l, index, obj, parent);
}

void ItemEquip::OnLoadFromDB(Item* parent) const
{
    // 模板改了以后可能存盘等级大于模板等级上线
    AdjustRefineLevel(parent);
}

void ItemEquip::DoRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	///
	/// 调用本函数前默认调用过TestRefine
	///

	ASSERT(data.refine_tid > 0);

	Player* player  = (Player*)obj;
    ItemEssence& item_ess = parent->GetItemEssence();
    EssRefine* pRefine = item_ess.QueryEss<EssRefine>(ESS_ID_REFINE);
    if (pRefine == NULL)
    {
        // 首次添加动态属性
        pRefine = new EssRefine;
        pRefine->refine_lvl = 1;
        pRefine->refine_tid = data.refine_tid;
        item_ess.InsertEss(pRefine);
    }
    else
    {
        pRefine->refine_lvl += 1;
    }
	//扣钱
	const RefineEntry& entry = GetRefineEntry(pRefine->refine_lvl);
	player->SpendMoney(entry.money);
}

int ItemEquip::GetRefineLevel(const Item* item) const
{
    const ItemEssence& item_ess = item->GetItemEssence();
    const EssRefine* pRefine = item_ess.QueryEss<EssRefine>(ESS_ID_REFINE);
    return pRefine == NULL ? 0 : pRefine->refine_lvl;
}

const RefineTable& ItemEquip::GetRefineTable() const
{
	return s_pItemMan->GetRefineTable(data.refine_tid);
}

const RefineEntry& ItemEquip::GetRefineEntry(int refine_lvl) const
{
	if (refine_lvl < 0 || refine_lvl > max_refine_lvl_)
	{
		ASSERT(false && "精练等级超过上限");
	}
	return GetRefineTable()[refine_lvl - 1];
}

void ItemEquip::AdjustRefineLevel(Item* parent) const
{
    ItemEssence& item_ess = parent->GetItemEssence();
    EssRefine* pRefine = item_ess.QueryEss<EssRefine>(ESS_ID_REFINE);

    if (pRefine != NULL && pRefine->refine_lvl > max_refine_lvl_) // 可能是模板修改了
    {
        if (max_refine_lvl_ <= 0)
        {
            item_ess.DeleteEss(ESS_ID_REFINE);
        }
        else
        {
            pRefine->refine_lvl = max_refine_lvl_;
        }
    }
}

}; // namespace gamed
