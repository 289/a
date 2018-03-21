#include "item_manager.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/hidden_item_templ.h"

//特殊物品实体类头文件
#include "gs/item/itembody/item_equip.h"
#include "gs/item/itembody/item_team_npc.h"
#include "gs/item/itembody/item_task_scroll.h"
#include "gs/item/itembody/item_pet.h"
#include "gs/item/itembody/item_card.h"
#include "gs/item/itembody/item_skill.h"
#include "gs/item/itembody/item_talent.h"
#include "gs/item/itembody/item_mount.h"

namespace gamed
{

ItemBody * MakeItemBody()
{
	return NULL;
}

void ItemManager::Initialize()
{
	///
	/// 本函数在模板数据加载后调用
	///

	//初始化精炼表MAP
	BuildRefineMap();
	
	//从模板管理类DataTemplManager获取所有物品的模板指针
	ITEM_TEMPL_VEC list;
	CollectAllItemTempl(list);

	//初始化物品实体MAP
	BuildItembodyMap(list);

	//初始化物品数据MAP
	BuildItemdataMap(list);
}

void ItemManager::Release()
{
	ITEM_BODY_MAP::iterator body_it = body_map_.begin();
	for (; body_it != body_map_.end(); ++body_it)
	{
		SAFE_DELETE(body_it->second);
	}

	ITEM_DATA_MAP::iterator item_it = item_map_.begin();
	for (; item_it != item_map_.end(); ++item_it)
	{
		SAFE_DELETE(item_it->second);
	}

	RefineTable_MAP::iterator refine_it = refine_tbl_map_.begin();
	for (; refine_it != refine_tbl_map_.end(); ++refine_it)
	{
		refine_it->second.clear();
	}

	item_map_.clear();
	body_map_.clear();
	refine_tbl_map_.clear();
}

void ItemManager::CollectAllItemTempl(ITEM_TEMPL_VEC& list)
{
	s_pDataTempl->CollectAllItemTempl(list);
}

void ItemManager::BuildItembodyMap(const ITEM_TEMPL_VEC& list)
{
#define CASE_GEN_ITEMBODY(Name, NAME) \
		case dataTempl::TEMPL_TYPE_##NAME: \
		{ \
			int tpl_id = pTpl->templ_id; \
			ItemBody * pBody = new Item##Name(tpl_id); \
			pBody->Initialize(pTpl); \
			if (!body_map_.insert(std::pair<int,ItemBody*>(tpl_id, pBody)).second) \
				ASSERT(false); \
		} \
		break;

	for (size_t i = 0; i < list.size(); ++i )
	{
		const ItemDataTempl* pTpl  = list[i];
		switch(pTpl->GetType())
		{
			CASE_GEN_ITEMBODY(Equip, EQUIP);
			CASE_GEN_ITEMBODY(TaskScroll, TASK_SCROLL);
			CASE_GEN_ITEMBODY(Pet, PET_ITEM);
			CASE_GEN_ITEMBODY(Card, CARD);
			CASE_GEN_ITEMBODY(Skill, SKILL_ITEM);
			CASE_GEN_ITEMBODY(Talent, TALENT_ITEM);
            CASE_GEN_ITEMBODY(Mount, MOUNT);

			case dataTempl::TEMPL_TYPE_HIDDEN_ITEM:
			{
				int tpl_id = pTpl->templ_id;
				ItemBody* pBody = CreateSpecItemBody(pTpl);
				if (!pBody) break;

				pBody->Initialize(pTpl);
				if (!body_map_.insert(std::pair<int,ItemBody*>(tpl_id, pBody)).second)
					ASSERT(false);
			}
			break;
			default:
			break;
		}
	}
#undef CASE_GEN_ITEMBODY
}

void ItemManager::BuildItemdataMap(const ITEM_TEMPL_VEC& list)
{
	for (size_t i = 0; i < list.size(); ++ i)
	{
		const ItemDataTempl * pTpl = list[i];
		TemplID tpl_id  = pTpl->templ_id;
		itemdata *pData = new itemdata;

		pData->id             = tpl_id;
		pData->count          = 1;
		pData->pile_limit     = pTpl->pile_limit;
		pData->proc_type      = pTpl->proc_type;
		pData->recycle_price  = pTpl->recycle_price;
		pData->expire_date    = 0;
		pData->item_cls       = Item::LocateItemCls(pTpl->GetType());
		pData->content.clear();

		item_map_[tpl_id] = pData;
	}
}

void ItemManager::BuildRefineMap()
{
	REFINE_TEMPL_VEC list;
	s_pDataTempl->QueryDataTemplByType(list);
	for (size_t i = 0; i < list.size(); ++ i)
	{
		const dataTempl::RefineTempl* pTpl = list[i];
		RefineTable& table = refine_tbl_map_[pTpl->templ_id];
		table.resize(pTpl->refine_table.size());
		for (size_t j = 0; j < pTpl->refine_table.size(); ++ j)
		{
			table[j].money = pTpl->refine_table[j].money;
			table[j].ratio = pTpl->refine_table[j].ratio;
		}
	}
}

ItemBody* ItemManager::CreateSpecItemBody(const dataTempl::ItemDataTempl* tpl)
{
	if (!tpl || tpl->GetType() != TEMPL_TYPE_HIDDEN_ITEM)
		return NULL;

	const dataTempl::HiddenItemTempl* pTpl = dynamic_cast<const dataTempl::HiddenItemTempl*>(tpl);
	switch (pTpl->type)
	{
		case dataTempl::HiddenItemTempl::HIT_TEAM_NPC:
			return new ItemTeamNpc(pTpl->templ_id);
		    break;
		default:
		    break;
	}

	return NULL;
}

}; // namespace gamed
