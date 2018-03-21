#include "item_list.h"

#include "gs/player/player.h"
#include "gs/global/timer.h"

namespace gamed {

/************************************************************************************************
**                                 PUBLIC FUNCTION
*************************************************************************************************/

template<typename T>
struct IdxCMP
{
	int index;
	IdxCMP(int idx): index(idx) {}
	bool operator()(const T& t) const
	{
		return index == t.index;
	}
};

void ItemList::LoadFromDB(const ItemDataVector& list, int32_t cap)
{
	if (cap > 0)
	{
		capacity_ = cap;
		empty_slot_count_ = cap;
		list_.resize(capacity_);
	}

	ASSERT(capacity_ > 0);
	ASSERT(list.size() <= Size());
    
    int32_t now = g_timer->GetSysTime();
	for (size_t i = 0; i < list.size(); ++i)
	{
        if (list[i].expire_date == 0 || list[i].expire_date > now)
        {
            size_t index = list[i].index;
            list_[index].LoadFromDB(list[i]);
            OnPutIn(index);
        }
	}
}

void ItemList::SaveForDB(ItemDataVector& list, int32_t& cap) const
{
	list.clear();
	list.resize(Size() - empty_slot_count_);
	for (size_t i=0, idx=0; i < Size(); ++ i)
	{
		if (list_[i].Type() == -1)
			continue;

		list[idx].index = i;
		list_[i].SaveForDB(list[idx]);
		++idx;
	}

	cap = capacity_;
}

int ItemList::SaveItem(size_t index, itemdata& data) const
{
	if (index < 0 || index >= Size())
		return -1;

	if (list_[index].Type() == -1)
		return 0;

	list_[index].SaveItem(data);
	data.index = index;
	return index + 1;
}

void ItemList::DetailSave(itemdataVector& list) const
{
	list.clear();
	list.resize(Size() - empty_slot_count_);
	for (size_t i = 0, j = 0; i < list_.size(); ++i)
	{
		if (list_[i].Type() == -1)
			continue;

		list_[i].SaveItem(list[j]);
		list[j].index = i;
		++j;
	}
}

void ItemList::Clear()
{
	for (size_t i = 0; i < Size(); ++ i)
	{
		if (list_[i].Type() != -1)
		{
			OnTakeOut(i);
			list_[i].Release();
		}
	}
	ASSERT(empty_slot_count_ == Size());
}

void ItemList::HeartBeat(time_t cur_time)
{
	if (!aging_items.size())
		return;

	time_t now = cur_time;
	AgingItemSet::iterator it = aging_items.begin();
	for (; it != aging_items.end(); )
	{
		if (it->expire_time > now)
			break;

		///
		/// 时效物品超时被删除
		/// 这里禁止调用OnTakeOut函数
		/// 调用将导致删除失败
		///
		size_t index = it->index;
		list_[index].OnTakeOut(location_, index, owner_);
		list_[index].Release();
		++empty_slot_count_;
		aging_items.erase(it++);
	}
}

void ItemList::GetExpireItem(time_t cur_time, std::vector<int32_t>& expire_index__list)
{
	if (!aging_items.size())
		return;

	AgingItemSet::iterator it = aging_items.begin();
	for (; it != aging_items.end(); ++it)
	{
		if (it->expire_time <= cur_time)
        {
            expire_index__list.push_back(it->index);
        }
	}
}

int ItemList::Find(int type) const
{
	for (size_t i = 0; i < list_.size(); ++i)
	{
		if (list_[i].Type() != -1 && list_[i].Type() == type)
			return i;
	}
	return -1;
}

int ItemList::CountItem(int type) const
{
	int count = 0;
	for (size_t i = 0; i < list_.size(); ++i)
	{
		if (list_[i].Type() != -1 && list_[i].Type() == type)
		{
			count += list_[i].Amount();
		}
	}
	return count;
}

bool ItemList::HasSlot(int type, int count) const
{
	if (empty_slot_count_)
		return true;

	for (size_t i = 0; i < list_.size() && count > 0; i ++)
	{
		if (list_[i].Type() != -1 && list_[i].Type() == type)
		{
			count -= list_[i].PileLimit() - list_[i].Amount();
		}
	}
	return count <= 0;
}

bool ItemList::IsItemExist(int type, int count) const
{
	return CountItem(type) >= count;
}

bool ItemList::IsItemExist(size_t index, int type, int count) const
{
	if (index >= Size())
		return false;

	const Item& it = list_[index];
	if (it.Type() == -1 || it.Type() != type)
		return false;
	return it.Amount() >= count;
}

bool ItemList::TakeOutItem(size_t index, int type, int count)
{
	if (!IsItemExist(index, type, count))
		return false;
	return DecAmount(index, count) >= 0;
}

bool ItemList::Update(size_t index)
{
	if (index >= Size() || IsEmptySlot(index))
		return false;

	return list_[index].Update(location_, index, owner_);
}

bool ItemList::Push(Item& it, int& last_index)
{
	ASSERT(it.Type() != -1 && it.Amount() > 0);
	if (empty_slot_count_ == 0 && it.PileLimit() == 1)
		return false;

	Item* pEmpty = NULL;
	if (it.PileLimit() > 1)	//可以堆叠的物品
	{
		int remain = it.Amount();
		last_index = TryPile(it.Type(), remain, pEmpty);
		ASSERT(remain >= 0);
		if (remain == 0)
		{
			//全部放入，释放物品
			it.Release();
			return true;
		}

		it.SetAmount(remain);
	}
	else
	{
		FindEmpty(pEmpty);
	}

	if (pEmpty == NULL)
	{
		//部分放入或一个都没有放入
		return false;
	}

	*pEmpty = it;
	it.Clear();

	int index = pEmpty - list_.data();
	OnPutIn(index);
	last_index = index;
	return true;
}

bool ItemList::Push(itemdata& data, int& last_index)
{
	ASSERT(data.id > 0 && data.count > 0);

	last_index = -1;
	if (empty_slot_count_ == 0 && data.pile_limit == 1)
		return false;

	Item * pEmpty = NULL;
	if (data.pile_limit > 1)
	{	
		//可以堆叠的物品先尝试堆叠放入
		last_index = TryPile(data.id, data.count, pEmpty);
		ASSERT(data.count >=0);
		if (data.count == 0)
			return true;
	}
	else
	{
		FindEmpty(pEmpty);
	}

	if (pEmpty == NULL)
	{
		//部分放入或一个都没有放入
		return false;
	}

	Item it;
	MakeItem(it, data);
	*pEmpty = it;
	it.Clear();
	data.count = 0;//放入成功，置零

	int index = pEmpty - list_.data();
	OnPutIn(index);
	last_index = index;
	return true;
}

bool ItemList::Push(int index, Item& it)
{
	ASSERT(it.Type() != -1);
	Item& old = list_[index]; 
	if (old.Type() == -1)
	{
		//原来没有物品，直接放入
		old = it;
		it.Clear();
		OnPutIn(index);
		return true;
	}

	if (old.Type() == it.Type() && old.PileLimit() >= old.Amount() + it.Amount())
	{ 
		//原来有物品，且类型相同，不超过堆叠上限
		//注意：这里禁止将物品进行拆分
		ASSERT(it.PileLimit() == old.PileLimit());
		old.IncAmount(it.Amount());
		it.Release();
		return true;
	}

	return false;
}

int ItemList::PushInEmpty(int start, const Item& it)
{
	ASSERT(it.Type() != -1);
	for (size_t i = start; i < list_.size(); ++ i)
	{
		if (list_[i].Type() != -1)
			continue;

		//找到空位
		list_[i] = it;
		OnPutIn(i);
		return i;
	}
	return -1;
}

int ItemList::PushInEmpty(int start, const itemdata& data)
{
	ASSERT(data.id > 0);
	Item it;
	MakeItem(it, data);
	int rst = PushInEmpty(start, it);
	it.Clear();
	return rst;
}

void ItemList::Exchange(int index, Item& it)
{
	Item & src = list_[index];
	if (src.Type() != -1)
	{
		OnTakeOut(index);
	}

	Item tmp = it;
	it = src;
	src = tmp;
	tmp.Clear();

	if (src.Type() != -1)
	{
		OnPutIn(index);
	}
}

bool ItemList::ExchangeItem(size_t index1, size_t index2)
{
	if (index1 == index2 || index1 >= list_.size() || index2 >= list_.size())
		return false;

	Item& it1 = list_[index1];
	Item& it2 = list_[index2];
	Item tmp = it1;
	it1 = it2;
	it2 = tmp;
	tmp.Clear();
	return true;
}

bool ItemList::Remove(size_t index, Item& it)
{
	if (index >= Size() || list_[index].Type() == -1)
	{
		ASSERT(false);
		return false;
	}

	//OnTakeOut可能修改物品数据，所以需要在赋值it前调用
	OnTakeOut(index);

	it.Clear();
	it = list_[index];

	list_[index].Clear();
	return true;
}

bool ItemList::Remove(size_t index)
{
	if (index >= Size() || list_[index].Type() == -1)
	{
		ASSERT(false);
		return false;
	}

	OnTakeOut(index);
	list_[index].Release();
	return true;
}

bool ItemList::MoveItem(size_t dest, size_t src, int count)
{
	if (src == dest || src >= list_.size() || dest >= list_.size()) return false;
	if (!count) return false;
	Item& it_src = list_[src];
	Item& it_dest= list_[dest];
	if (it_src.Type() == -1 || it_src.Amount() < count) 
	{
		return false;
	}

	if (it_dest.Type() == -1)
	{
		//目标位置为空
		it_dest = it_src;
		it_dest.SetAmount(count);
		it_src.DecAmount(count);
		if (!it_src.Amount())
		{
			//源位置置空
			it_src.Clear();
		}
		else
		{
			OnPutIn(dest);
            std::string content;
            it_src.GetItemEssence().SaveEss(content);
            it_dest.GetItemEssence().LoadEss(content);
		}
		return true;
	}

	//目标位置非空
	if (it_dest.Type() != it_src.Type() || it_dest.Amount() + count > it_dest.PileLimit()) 
	{
		//源位置和目标位置物品类型不一致
		//超过堆叠上限
		return false;
	}

	it_dest.IncAmount(count);
	it_src.DecAmount(count);
	if (!it_src.Amount())
	{
		OnTakeOut(src);
		it_src.Release();
	}
	return true;
}

int ItemList::IncAmount(size_t index, int count)
{
	if (index >= Size() || list_[index].Type() == -1)
	{
		ASSERT(false);
		return -1;
	}

	Item& it = list_[index];
	if (it.Amount() >= it.PileLimit()) return 0;
	int delta = it.PileLimit() - it.Amount();
	if (delta > count) delta = count;
	it.IncAmount(delta);
	return delta;
}

int ItemList::DecAmount(size_t index, int count)
{
	if (index >= Size() || list_[index].Type() == -1)
	{
		ASSERT(false);
		return -1;
	}

	Item& it = list_[index];
	if (it.Amount() <= count)
	{
		OnTakeOut(index);
		it.Release();
	}
	else
	{
		it.DecAmount(count);
	}
	return it.Amount();
}

int ItemList::UseItem(size_t index)
{
	if (index < 0 || index >= Size())
		return -1;

	Item& it = list_[index];
	if (it.Type() == -1)
		return -1;

	if (!it.CanUse(location_, index, owner_))
		return -1;

    Item::ITEM_USE rst = it.UseItem(location_, index, owner_);
	if (rst != Item::ITEM_USE_CONSUME && rst != Item::ITEM_USE_RETAIN)
		return -1;

    if (rst == Item::ITEM_USE_CONSUME)
    {
        DecAmount(index, 1);
        return 1;
    }
	return 0;
}

bool ItemList::SellItem(size_t index, int count)
{
	if (index < 0 || index >= Size())
		return false;

	Item& it = list_[index];
	if (it.Type() == -1)
		return false;

	if (it.Amount() < count)
		return false;

	if (!it.CanSell())
		return false;

	//获得金币
    int32_t total_value = it.GetTotalPrice() * count;
    if (total_value > 0)
    {
        Player* player = dynamic_cast<Player*>(owner_);
        player->GainMoney(total_value);
    }

	//扣除物品
	it.DecAmount(count);
	if (!it.Amount())
	{
		//空栏位
		OnTakeOut(index);
		it.Release();
	}
	return true;
}

/*
 * @brief:包裹整理原则
 *        1) 按照物品分组类型顺序存放;
 *        2) 同分组物品按照ID顺序存放;
 *        3) 同ID物品按照内部属性存放,如装备品质;
 *        4) 其它分组物品的内部存放顺序待定;
 */
class ItemInsertor
{
public:
	static void Insert(std::vector<Item>& list, const Item& it)
	{
		//插入物品,考虑堆叠存放
		int count = it.Amount();
		std::vector<Item>::iterator iter = list.begin();
		for (; iter != list.end(); ++ iter)
		{
			if (iter->Type() == it.Type() && iter->Amount() < iter->PileLimit())
			{
				//堆叠存放
				int tmp = 0;
				if (count + iter->Amount() > iter->PileLimit())
				{
					//部分堆叠
					tmp =  iter->PileLimit() - it.Amount();
					iter->IncAmount(tmp);
					count -= tmp;
				}
				else
				{
					//全部堆叠
					iter->IncAmount(count);
					return;
				}
			}
		}

		//走到这里有2种可能：
		// 1) 部分堆叠成功；
		// 2) 一个也未堆叠；

		Item item = it;
		item.SetAmount(count);

		//将item插入空位
		//这里需要考虑插入位置：根据物品ID和Rank决定
		for (iter = list.begin(); iter != list.end(); ++ iter)
		{
			if (iter->Type() < it.Type())
			{
				continue;
			}
			else if (iter->Type() == it.Type() && iter->GetRank() >= it.GetRank())
			{
				continue;
			}
			else
			{
				//找到>=待插入物品ID的对象
				list.insert(iter, item);
				item.Clear();
				return;
			}
		}

		// 走到这里有2种情况
		// 1) list中物品的ID均<=待插入物品的ID
		// 2) list中物品的Rank均>=待插入物品的Rank
		// 出现这样的情况直接在尾部追加
		list.push_back(item);
		item.Clear();
	}
};

void ItemList::TidyUp()
{
	typedef std::vector<int/*idx*/>	IndexVec;
	typedef std::vector<IndexVec> ItemClsVec;

	int64_t total_count = TotalItemCount();

	//根据物品类别进行分组
	ItemClsVec item_cls_vec;
	item_cls_vec.resize(Item::ITEM_CLS_MAX);
	for (size_t i = 0; i < Size(); ++ i)
	{
		if (list_[i].Type() == -1) continue;
		int item_cls = list_[i].item_cls;
		item_cls_vec[item_cls].push_back(i);
	}

	//构造临时包裹
	std::vector<Item> tmp_list;
	for (size_t i = 0; i < item_cls_vec.size(); ++ i)
	{
		std::vector<Item> items;//保存同类物品
		const IndexVec& idx_vec = item_cls_vec[i];
		for (size_t j = 0; j < idx_vec.size(); ++ j)
		{
			ItemInsertor::Insert(items, list_[idx_vec[j]]);
		}

		tmp_list.insert(tmp_list.end(), items.begin(), items.end());
	}

	//临时包裹构造完毕
	//重置玩家包裹
	ASSERT(tmp_list.size() <= Size());
	Clear();

	for (size_t i = 0; i < tmp_list.size(); ++ i)
	{
		list_[i] = tmp_list[i];
		OnPutIn(i);
	}

	ASSERT(total_count == TotalItemCount());
	ASSERT(empty_slot_count_ == (Size() - tmp_list.size()));
}

/************************************************************************************************
**                                 PRIVATE FUNCTION
*************************************************************************************************/
int ItemList::TryPile(int type, int& count, Item*& pEmpty)
{
	int last_index = -1;
	for (size_t i = 0; i < list_.size(); i ++)
	{
		Item& it = list_[i];
		if (it.Type() == -1)
		{
			if (pEmpty == NULL) pEmpty = &(list_[i]);
		}
		else if (it.Type() == type && it.Amount() < it.PileLimit())
		{
			int tmp = count;
			if (tmp + it.Amount() > it.PileLimit())
			{
				tmp =  it.PileLimit() - it.Amount();
			}
			
			it.IncAmount(tmp);
			count -= tmp;
			last_index = i;
			if (count <= 0) break;
		}
	}
	return last_index;
}

bool ItemList::FindEmpty(Item *&pEmpty)
{
	for (size_t i = 0; i < list_.size(); i ++)
	{
		if (list_[i].Type() == -1)
		{
			pEmpty = &(list_[i]);
			return true;
		}
	}
	return false;
}

void ItemList::OnPutIn(int index)
{
	-- empty_slot_count_;
	Item& it = list_[index];
	it.OnPutIn(location_, index, owner_);
	if (it.ExpireDate())
	{
		//插入时效物品
		//加入时效物品索引表，提高HEART时效物品的效率
		ASSERT(it.PileLimit() == 1);
		AgingItemSet::iterator iter = std::find_if (aging_items.begin(), aging_items.end(), IdxCMP<AgingItem>(index));
		ASSERT(iter == aging_items.end());

		AgingItem ai;
		ai.index = index;
		ai.expire_time = it.ExpireDate();
		aging_items.insert(ai);
	}
}

void ItemList::OnTakeOut(int index)
{
	++ empty_slot_count_;
	list_[index].OnTakeOut(location_, index, owner_);
	if (list_[index].ExpireDate())
	{
		AgingItemSet::iterator it = std::find_if (aging_items.begin(), aging_items.end(), IdxCMP<AgingItem>(index));
        if (it != aging_items.end())
        {
		    aging_items.erase(it);
        }
	}
}

int ItemList::TotalItemCount() const
{
	int64_t total = 0; 
	for (size_t i = 0; i < Size(); ++ i)
	{
		if (list_[i].Type() == -1) continue;
		total += list_[i].Amount();
	}
	return total;
}

bool ItemList::IsItemIndex(size_t index) const
{
    return index < Size() && !IsEmptySlot(index);
}

};
