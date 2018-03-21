#ifndef __GAMED_GS_ITEM_ITEM_MANAGER_H__
#define __GAMED_GS_ITEM_ITEM_MANAGER_H__

#include "item.h"
#include "item_data.h"

#include <map>

#include "shared/base/singleton.h"
#include "shared/logsys/logging.h"
#include "gs/template/data_templ/refine_templ.h"
#include "gs/template/data_templ/base_datatempl.h"

namespace gamed {

using namespace dataTempl;

/**
 * @class ItemManager
 * @brief 物品管理器,全局唯一,在模板数据加载成功后初始化
 * @brief 功能：
 *        1) 生成所有物品的itemdata;
 *        2) 为物品对象分配ItemBody;
 *        3) 缓存装备精炼表;
 */

class ItemBody;
class ItemManager : public shared::Singleton<ItemManager>
{
	typedef std::vector<const dataTempl::ItemDataTempl*>  ITEM_TEMPL_VEC;
	typedef std::vector<const dataTempl::RefineTempl*>    REFINE_TEMPL_VEC;

public:
	struct RefineEntry
	{
		int money;	//精练价格
		int ratio;	//属性提升比例(万分数)
	};
	typedef std::vector<RefineEntry> RefineTable;

private:
	typedef std::map<TemplID, itemdata*>   ITEM_DATA_MAP;
	typedef std::map<TemplID, ItemBody*>   ITEM_BODY_MAP;
	typedef std::map<TemplID, RefineTable> RefineTable_MAP;

	ITEM_BODY_MAP body_map_;
	ITEM_DATA_MAP item_map_;
	RefineTable_MAP refine_tbl_map_;

public:
	ItemManager() {}
	virtual ~ItemManager()
	{
		//暂时在这里释放吧
		Release();
	}
	static ItemManager* GetInstance()
	{
		return &(get_mutable_instance());
	}

	void Initialize();
	void Release();

	ItemBody* GetItemBody(TemplID id)
	{
		ITEM_BODY_MAP::iterator it = body_map_.find(id);
		if (it == body_map_.end())
		{
			return NULL;
		}
		return it->second;
	}

	const itemdata* GetItemData(TemplID id) const
	{
		ITEM_DATA_MAP::const_iterator it = item_map_.find(id);
		if (it == item_map_.end())
		{
			LOG_ERROR  << "未知的物品ID " << id;
			return NULL;
		}
		return it->second;
	}

	const RefineTable& GetRefineTable(TemplID id) const
	{
		RefineTable_MAP::const_iterator it = refine_tbl_map_.find(id);
		ASSERT(it != refine_tbl_map_.end());
		return it->second;
	}

	int GetItemCls(TemplID id) const
	{
		ITEM_DATA_MAP::const_iterator it = item_map_.find(id);
		if (it == item_map_.end())
		{
			LOG_ERROR  << "未知的物品ID " << id;
			return NULL;
		}
		return it->second->item_cls;
	}

	int GetItemPileLimit(TemplID id) const
	{
		ITEM_DATA_MAP::const_iterator it = item_map_.find(id);
		if (it == item_map_.end())
		{
			LOG_ERROR  << "未知的物品ID " << id;
			return NULL;
		}
		return it->second->pile_limit;
	}

    // 取基础价格
	int GetItemPrice(TemplID id) const
	{
		ITEM_DATA_MAP::const_iterator it = item_map_.find(id);
		if (it == item_map_.end())
		{
			LOG_ERROR  << "未知的物品ID " << id;
			return NULL;
		}
		return it->second->recycle_price;
	}

    /**
     * @brief GenerateItem 
     *  TODO:使用时必须检查返回值
     */
	bool GenerateItem(TemplID id, itemdata& data)
	{
		/**
		 * 产生物品时现在默认考虑了item_content字段
		 * 实际上item_map_缓存的itemdata的item_content均为空指针
		 */
		ITEM_DATA_MAP::const_iterator it = item_map_.find(id);
		if (it == item_map_.end())
		{
			LOG_ERROR  << "未知的物品ID " << id;
			return false;
		}
		it->second->Clone(data);
        return true;
	}

	bool IsValidItem(TemplID id) const
	{
		ITEM_DATA_MAP::const_iterator it = item_map_.find(id);
		return it != item_map_.end();
	}

	bool IsValidRefineTable(TemplID id) const
	{
		RefineTable_MAP::const_iterator it = refine_tbl_map_.find(id);
		return it != refine_tbl_map_.end();
	}

private:
	void CollectAllItemTempl(ITEM_TEMPL_VEC& list);
	void BuildItembodyMap(const ITEM_TEMPL_VEC& list);
	void BuildItemdataMap(const ITEM_TEMPL_VEC& list);
	void BuildRefineMap();

	ItemBody* CreateSpecItemBody(const dataTempl::ItemDataTempl* tpl);
};

#define s_pItemMan gamed::ItemManager::GetInstance()

typedef ItemManager::RefineEntry RefineEntry;
typedef ItemManager::RefineTable RefineTable;

}; // namespace gamed

#endif // __GAMED_GS_ITEM_ITEM_MANAGER_H__
