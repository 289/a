#ifndef _GAMED_GS_ITEM_ITEM_LIST_H_
#define _GAMED_GS_ITEM_ITEM_LIST_H_

#include "item.h"

#include <set>
#include <vector>
#include <stdint.h>

#include "shared/base/assertx.h"
#include "shared/base/noncopyable.h"
#include "common/obj_data/player_attr_declar.h"

namespace gamed {

typedef std::vector<common::PlayerInventoryData::ItemData> ItemDataVector;
typedef std::vector<itemdata> itemdataVector;

class Item;
class Unit;

/**
 * @class ItemList
 * @brief 物品对象列表
 * @brief 包裹类管理管理器均通过该类实现
 */
class ItemList : public shared::noncopyable
{
private:
	struct AgingItem
	{
		int index;
		time_t expire_time;
		bool operator < (const AgingItem& rhs) const
		{
			return expire_time < rhs.expire_time;
		}
	};
	typedef std::set<AgingItem> AgingItemSet;

	Unit*					owner_;
	Item::LOCATION			location_;
	std::vector<Item>		list_;				//Item对象列表
	size_t                  capacity_;          //栏位大小
	size_t					empty_slot_count_;	//空闲栏位个数
	AgingItemSet			aging_items;		//时效物品集合

public:
	ItemList(Item::LOCATION l, size_t n):
		owner_(NULL),
		location_(l),
		list_(n),
		capacity_(n),
		empty_slot_count_(n)
	{ }
	virtual ~ItemList()
	{ }

	void   set_owner(Unit* obj)						{ ASSERT(!owner_); owner_ = obj; }
	size_t empty_slot_count() const					{ return empty_slot_count_; }
	size_t Size() const								{ return capacity_; }
	bool   IsFull() const							{ return !empty_slot_count_; }
	bool   IsEmptySlot(int index) const				{ return list_[index].Type() == -1; }
	Item&  operator[](size_t index)					{ return list_[index]; }
	const  Item& operator[](size_t index) const		{ return list_[index]; }
	int    Find(int type) const;
	int    CountItem(int type) const;
	bool   HasSlot(int type, int count) const;
	bool   IsItemExist(int type, int count) const;
	bool   IsItemExist(size_t index, int type, int count) const;
	bool   TakeOutItem(size_t index, int type, int count);
	bool   Update(size_t index);
	void   HeartBeat(time_t cur_time);
	void   Clear();
    bool   IsItemIndex(size_t index) const;

	void   LoadFromDB(const ItemDataVector& list, int32_t cap);
	void   SaveForDB(ItemDataVector& list, int32_t& cap) const;
	int    SaveItem(size_t index, itemdata& data) const;
	void   DetailSave(itemdataVector& list) const;
    void   GetExpireItem(time_t cur_time, std::vector<int32_t>& expire_index_list);


	/**
	 * @brief: 放入物品，堆叠优先，支持部分放入
	 * @param: 参数的count字段被修改，指示剩余物品个数(即未成功放入)。如果未完全放入的话，则传入对象不会被释放
	 * @param: last_index 保存最后一次成功放入物品的位置
	 * @ret  : 成功放入返回ture，失败或部分放入返回false
	 */
	bool Push(Item& it, int& last_index);
	bool Push(itemdata& data, int& last_index);

	/**
	 * @brief: 在指定位置放入物品，支持堆叠，超过堆叠上限则放入失败
	 *         注意此接口不会把叠加的物品切分成若干部分。
	 *         放入成功，参数it被清空
	 * @ret  : 放入成功返回true，否则返回false
	 */
	bool Push(int index, Item& it);

	/**
	 * @brief: 在空位放入物品，不会发生部分放入
	 * @ret  : 成功返回物品放入的位置，否则返回-1
	 */
	int PushInEmpty(int start, const Item& it);
	int PushInEmpty(int start, const itemdata& data);

	/**
	 * @brief: 在指定位置放入物品it，并且将该位置原来的物品换出保存在data中
	 */
	void Exchange(int index, Item& it);

	/**
	 * @brief: 交货两个位置的物品
	 */
	bool ExchangeItem(size_t index1, size_t index2);

	/**
	 * @brief: 从src位置移count个物品到dest位置上
	 */
	bool MoveItem(size_t dest, size_t src, int count);

	/**
	 * @brief: 删除并取出指定位置物品
	 */
	bool Remove(size_t index, Item& it);

	/**
	 * @brief: 删除指定位置物品
	 */
	bool Remove(size_t index);

	/**
	 * @brief: 将给定位置物品个数增加
	 * @ret  : 成功返回累加个数，失败返回-1
	 */
	int IncAmount(size_t index, int count);

	/**
	 * @brief: 从指定位置删除count个物品
	 * @ret  : 成功返回剩余物品个数，失败返回-1
	 */
	int DecAmount(size_t index, int count);

	/**
	 * @func: UseItem
	 * @func: 使用物品
	 *        1) 使用index位置的物品
     *        2) 返回-1表示使用失败，0表示使用但不消耗，大于0消耗对应数目的物品
	 */
	int UseItem(size_t index);
	bool SellItem(size_t index, int count);

	/**
	 * @func: TidyUp
	 * @func: 包裹整理原则
	 *        1) 按照物品分组类型集中存放，物品类型先后顺序暂时依赖定义顺序;
	 *        2) 同分组类型物品的内部顺序默认按照物品ID顺序从小到大依次存放;
	 *        3) 同分组类型物品如果有高级属性，则按照高级属性排序存放，如装备品质;
	 *        4) 其它分组物品的内部存放顺序待定;
	 */
	void TidyUp();//整理包裹

private:

	/**
	 * @brief: 试图堆叠存放，
	 * @param: type：堆叠物品类型
	 * @param: count: 堆叠物品个数
	 * @param: pEmpty: 第一个空闲位置的指针
	 * @ret  : 堆叠成功返回堆叠位置，发生多次堆叠时返回最后一次堆叠位置，失败返回-1
	 *         参数count调用后被修改为剩余物品个数
	 */
	int TryPile(int type, int& count, Item*& pEmpty);
	bool FindEmpty(Item*& pEmpty);	//寻找空位
	void OnPutIn(int index);		//放入物品后调用
	void OnTakeOut(int index);		//取出物品前调用
	int  TotalItemCount() const;    //物品总数
};

}; // namespace gamed

#endif // _GAMED_GS_ITEM_ITEM_LIST_H_
