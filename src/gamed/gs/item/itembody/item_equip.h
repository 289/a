#ifndef __GAMED_GS_ITEM_ITEM_BODY_ITEM_EQUIP_H__
#define __GAMED_GS_ITEM_ITEM_BODY_ITEM_EQUIP_H__

#include "gs/item/item.h"
#include "gs/item/item_manager.h"

namespace gamed
{

/**
 * @brief 装备静态数据
 */
struct equip_data
{
	int quality;
	int equip_mask;
	int lvl_limit;
	int refine_tid;
	int init_slot_count;
	int extend_slot_count;
	ClassMask cls_limit;
	std::vector<int> addon_list; //附加属性列表
};

/**
 * @brief: 装备实体基类，所有可以装备的物品实体均继承ItemEquip
 */
class ItemEquip : public ItemBody
{
	equip_data data;
	int max_refine_lvl_;//最大精练等级

public:
	explicit ItemEquip(int id) :
		ItemBody(id),
		max_refine_lvl_(0)
	{ }
	virtual ~ItemEquip()
	{ }

	virtual int  GetRank(const Item* parent) const;
	virtual int  GetEquipMask() const;
	virtual int  GetExtraPrice(const Item* parent) const;
	virtual ClassMask GetClsLimit() const;
	virtual int  GetRefineLevel(const Item* parent) const;
	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual bool TestRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;		//测试装备是否可以精练
	virtual bool TestActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;	//测试是否可以装备
	virtual void OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;		//激活装备
	virtual void OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;	//去激活装备
	virtual void OnEnable(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;       //装备生效
	virtual void OnDisable(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;      //装备失效
	virtual void DoRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;		//精练装备
	virtual void OnLoadFromDB(Item* parent) const;


private:
	const RefineTable& GetRefineTable() const;
	const RefineEntry& GetRefineEntry(int refine_lvl) const;
    void  AdjustRefineLevel(Item* parent) const;
};

};

#endif
