#include <zlib.h>
#include "player_inventory.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/global/timer.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/player/player_prop.h"
#include "gs/item/item_manager.h"

#include "gs/template/data_templ/equip_templ.h"
#include "gs/template/data_templ/normal_item_templ.h"
#include "gs/template/data_templ/templ_manager.h"

#include "fixed_item_def.h"


namespace gamed {

using namespace dataTempl;

// gain item mode check
SHARED_STATIC_ASSERT((int)playerdef::GIM_NORMAL    == (int)G2C::GIM_NORMAL);
SHARED_STATIC_ASSERT((int)playerdef::GIM_INS_AWARD == (int)G2C::GIM_INS_AWARD);
SHARED_STATIC_ASSERT((int)playerdef::GIM_TASK      == (int)G2C::GIM_TASK);
SHARED_STATIC_ASSERT((int)playerdef::GIM_MAIL      == (int)G2C::GIM_MAIL);
SHARED_STATIC_ASSERT((int)playerdef::GIM_CARD      == (int)G2C::GIM_CARD);
SHARED_STATIC_ASSERT((int)playerdef::GIM_MAX       == (int)G2C::GIM_MAX);

///
/// PlayerInventory
///
PlayerInventory::PlayerInventory(Player& player): PlayerSubSystem(SUB_SYS_TYPE_INVENTORY, player),
	  inventory_(Item::INVENTORY, 0),
	  equipment_(Item::EQUIPMENT, Item::EQUIP_INDEX_MAX),
	  hide_inv_(Item::HIDE_INV, MAX_HIDE_INV_SIZE),
	  task_inv_(Item::TASK_INV, MAX_TASK_INV_SIZE),
	  pet_inv_(Item::PET_INV, MAX_PET_INV_SIZE),
	  card_inv_(Item::CARD_INV, MAX_CARD_INV_SIZE),
	  mount_inv_(Item::MOUNT_INV, MAX_MOUNT_INV_SIZE)
{
	SAVE_LOAD_REGISTER(common::PlayerInventoryData, PlayerInventory::SaveToDB, PlayerInventory::LoadFromDB);
}

PlayerInventory::~PlayerInventory()
{
}

bool PlayerInventory::SaveToDB(common::PlayerInventoryData* pData)
{
	int32_t inventory_cap = 0;
	int32_t equipment_cap = 0;
	int32_t hide_inv_cap  = 0;
	int32_t task_inv_cap  = 0;
	int32_t pet_inv_cap   = 0;
	int32_t card_inv_cap   = 0;
	int32_t mount_inv_cap   = 0;

	inventory_.SaveForDB(pData->inventory, inventory_cap);
	equipment_.SaveForDB(pData->equipment, equipment_cap);
	hide_inv_.SaveForDB(pData->hide_inv, hide_inv_cap);
	task_inv_.SaveForDB(pData->task_inv, task_inv_cap);
	pet_inv_.SaveForDB(pData->pet_inv, pet_inv_cap);
	card_inv_.SaveForDB(pData->card_inv, card_inv_cap);
	mount_inv_.SaveForDB(pData->mount_inv, mount_inv_cap);

	pData->inventory_cap = inventory_cap;
	return true;
}

bool PlayerInventory::LoadFromDB(const common::PlayerInventoryData& data)
{
	inventory_.LoadFromDB(data.inventory, data.inventory_cap);
	equipment_.LoadFromDB(data.equipment, 0);
	hide_inv_.LoadFromDB(data.hide_inv, 0);
	task_inv_.LoadFromDB(data.task_inv, 0);
	pet_inv_.LoadFromDB(data.pet_inv, 0);
	card_inv_.LoadFromDB(data.card_inv, 0);
	mount_inv_.LoadFromDB(data.mount_inv, 0);
	return true;
}

void PlayerInventory::OnInit()
{
	inventory_.set_owner(&player_);
	equipment_.set_owner(&player_);
	hide_inv_.set_owner(&player_);
	task_inv_.set_owner(&player_);
	pet_inv_.set_owner(&player_);
	card_inv_.set_owner(&player_);
	mount_inv_.set_owner(&player_);
}

void PlayerInventory::OnHeartbeat(time_t cur_time)
{
    for (int32_t where = Item::INVENTORY; where < Item::INV_MAX; ++where)
    {
        std::vector<int32_t> item_index_list;
	    ItemList& inv = GetItemList(where);
        inv.GetExpireItem(cur_time, item_index_list);
        for (size_t i = 0; i < item_index_list.size(); ++i)
        {
            int32_t item_idx = item_index_list[i];
            if (inv.TakeOutItem(item_idx, inv[item_idx].Type(), 1))
            {
                SelfGetItemData(where, item_idx);
            }
        }
    }
}

void PlayerInventory::OnRelease()
{
	inventory_.Clear();
	equipment_.Clear();
	hide_inv_.Clear();
	task_inv_.Clear();
	pet_inv_.Clear();
	card_inv_.Clear();
	mount_inv_.Clear();
}

void PlayerInventory::RegisterCmdHandler()
{
	// 注册正常CMD处理函数
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetItemData, PlayerInventory::CMDHandler_GetItemData);
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetItemListBrief, PlayerInventory::CMDHandler_GetItemListBrief);
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetItemListDetail, PlayerInventory::CMDHandler_GetItemListDetail);
	REGISTER_NORMAL_CMD_HANDLER(C2G::DropInventoryItem, PlayerInventory::CMDHandler_DropInventoryItem);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MoveInventoryItem, PlayerInventory::CMDHandler_MoveInventoryItem);

	REGISTER_NORMAL_CMD_HANDLER(C2G::SellInventoryItem, PlayerInventory::CMDHandler_SellInventoryItem);
	REGISTER_NORMAL_CMD_HANDLER(C2G::ExchangeInventoryItem, PlayerInventory::CMDHandler_ExchangeInventoryItem);
	REGISTER_NORMAL_CMD_HANDLER(C2G::EquipItem, PlayerInventory::CMDHandler_EquipItem);
	REGISTER_NORMAL_CMD_HANDLER(C2G::UndoEquip, PlayerInventory::CMDHandler_UndoEquip);
	REGISTER_NORMAL_CMD_HANDLER(C2G::UseItem, PlayerInventory::CMDHandler_UseItem);

	REGISTER_NORMAL_CMD_HANDLER(C2G::RefineEquip, PlayerInventory::CMDHandler_RefineEquip);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TidyInventory, PlayerInventory::CMDHandler_TidyInventory);
    REGISTER_NORMAL_CMD_HANDLER(C2G::ItemDisassemble, PlayerInventory::CMDHandler_ItemDisassemble);

	// 注册调试CMD处理函数
	REGISTER_DEBUG_CMD_HANDLER(C2G::DebugCleanAllItem, PlayerInventory::DebugCleanAllItem);
}

void PlayerInventory::RegisterMsgHandler()
{
}

void PlayerInventory::OnTransferCls()
{
    //转职后有些装备可能失效,同时有些装备会生效
    for (size_t i = 0; i < Item::EQUIP_INDEX_MAX; ++ i)
    {
        Item& item = equipment_[i];
        if (item.Type() == -1) continue;
        if (item.TestDisabled())
        {
            if (item.GetClsLimit().test(player_.role_class()))
            {
                //让装备生效
                Item::LOCATION l = Item::EQUIPMENT;
                item.Enable(l, i, &player_);
                SelfGetItemData(l, i);
            }
        }
        else
        {
            if (!item.GetClsLimit().test(player_.role_class()))
            {
                //让装备失效
                Item::LOCATION l = Item::EQUIPMENT;
                item.Disable(l, i, &player_);
                SelfGetItemData(l, i);
            }
        }
    }
}

#define ERROR_RETURN(err) \
		{ \
			SendError(err); \
			return; \
		}

void PlayerInventory::PlayerGetInventory(int where)
{
	if (where < 0 || where >= Item::INV_MAX)
		return;

	itemdataVector list;
	ItemList& inv = GetItemList(where);
	inv.DetailSave(list);

	int32_t inv_cap = inv.Size();
	player_.sender()->SelfItemListBrief(where, inv_cap, list);
}

void PlayerInventory::SelfGetInventoryDetail(int where)
{
	if (where < 0 || where >= Item::INV_MAX)
		return;

	itemdataVector list;
	ItemList& inv = GetItemList(where);
	inv.DetailSave(list);

	int32_t inv_cap = inv.Size();
	player_.sender()->SelfItemListDetail(where, inv_cap, list);
}

bool PlayerInventory::GetItemListDetail(int where, std::vector<itemdata>& list) const
{
    if (where < 0 || where >= Item::INV_MAX)
		return false;

    const ItemList& inv = GetItemList(where);
	inv.DetailSave(list);
    return true;
}

int PlayerInventory::CountItem(int item_type) const
{
	if (item_type <= 0)
		return 0;

	const ItemList* inv = NULL;
	if (!(inv=QueryInventory(item_type)))
		return 0;

	return inv->CountItem(item_type);
}

int PlayerInventory::CountEquipItem(int item_type) const
{
	if (item_type <= 0)
		return 0;
    return equipment_.CountItem(item_type);
}

bool PlayerInventory::HasSlot(int where, int empty_slot_count) const
{
	const ItemList& inv = GetItemList(where);
	return inv.empty_slot_count() >= (size_t)empty_slot_count;
}

bool PlayerInventory::CheckItem(int item_type, int item_count) const
{
	if (item_type <= 0 || item_count <= 0)
		return true;

	const ItemList* inv = NULL;
	if (!(inv=QueryInventory(item_type)))
		return false;
	return inv->IsItemExist(item_type, item_count);
}

bool PlayerInventory::CheckItem(int item_idx, int item_type, int item_count) const
{
	if (item_type <= 0 || item_count <= 0)
		return true;

	const ItemList* inv = NULL;
	if (!(inv=QueryInventory(item_type)))
		return false;
	return inv->IsItemExist(item_idx, item_type, item_count);
}

bool PlayerInventory::TakeOutItem(int item_type, int item_count)
{
	if (item_type <= 0 || item_count <= 0)
		return true;

	int item_cls = s_pItemMan->GetItemCls(item_type);
	if (item_cls <= 0)
		return false;

	int where = Item::LocateLocation(item_cls);
	if (where < 0)
		return false;

	//删除count个物品(不在任务或隐藏包裹)
	ItemList& inv = GetItemList(where);
	int count = item_count;
	while (count > 0)
	{
		int rst = inv.Find(item_type);
		if (rst < 0)
			return false;

		if (count > inv[rst].Amount())
		{
			count -= inv[rst].Amount();
			inv.DecAmount(rst, inv[rst].Amount());
			SelfGetItemData(where, rst);
		}
		else
		{
			inv.DecAmount(rst, count);
			SelfGetItemData(where, rst);
			return true;
		}
	};
	return false;
}

bool PlayerInventory::TakeOutItem(int item_idx, int item_type, int item_count)
{
	if (item_type <= 0 || item_count <= 0)
		return true;

	int item_cls = s_pItemMan->GetItemCls(item_type);
	if (item_cls <= 0)
		return false;

	int where = Item::LocateLocation(item_cls);
	if (where < 0)
		return false;

	ItemList& inv = GetItemList(where);
	bool rst = inv.TakeOutItem(item_idx, item_type, item_count);
	if (rst)
	{
		SelfGetItemData(where, item_idx);
	}
	return rst;
}

bool PlayerInventory::TakeOutAllItem(int item_type, int item_count)
{
	if (item_type <= 0 || item_count <= 0)
		return true;

	int item_cls = s_pItemMan->GetItemCls(item_type);
	if (item_cls <= 0)
		return false;

	int where = Item::LocateLocation(item_cls);
	if (where < 0)
		return false;

	if (where == Item::HIDE_INV || where == Item::TASK_INV)
	{
		//特殊处理任务和隐藏包裹
		//删除ID为item_type的所有物品
		ItemList& inv = GetItemList(where);
		for (size_t i = 0; i < inv.Size(); ++ i)
		{
			if (inv[i].Type() == item_type)
			{
				inv.Remove(i);
                SelfGetItemData(where, i);
			}
		}

		return true;
	};

	//删除count个物品(不在任务或隐藏包裹)
	ItemList& inv = GetItemList(where);
	int count = item_count;
	while (count > 0)
	{
		int rst = inv.Find(item_type);
		if (rst < 0)
			return false;

		if (count > inv[rst].Amount())
		{
			count -= inv[rst].Amount();
			inv.DecAmount(rst, inv[rst].Amount());
			SelfGetItemData(where, rst);
		}
		else
		{
			inv.DecAmount(rst, count);
			SelfGetItemData(where, rst);
			return true;
		}
	};
	return false;
}

bool PlayerInventory::QueryItem(int where, int item_idx, itemdata& item) const
{
	if (where < 0 || where >= Item::INV_MAX)
		return false;

	const ItemList& inv = GetItemList(where);
	if (inv.SaveItem(item_idx, item) <= 0)
		return false;
	return true;
}

void PlayerInventory::QueryItemlist(int where, int& inv_cap, std::vector<itemdata>& list) const
{
	if (where < 0 || where >= Item::INV_MAX)
		return;

	const ItemList& inv = GetItemList(where);
	for (size_t i = 0; i < inv.Size(); ++ i)
	{
		if (inv[i].Type() == -1)
			continue;

		itemdata data;
		inv.SaveItem(i, data);
		list.push_back(data);
	}
	inv_cap = inv.Size();
}

void PlayerInventory::UpdateEquipCRC()
{
	shared::net::ByteBuffer buffer;

	for (size_t i = 0; i < equipment_.Size(); ++ i)
	{
		Item& item = equipment_[i];
		if (item.Type() == -1)
			continue;

		buffer << item.Type();
		buffer << item.ProcType();
		buffer << item.ExpireDate();

        std::string content;
        item.GetItemEssence().SaveEss(content);
        buffer << content;
		//size_t content_len = 0;
		//const char* content = (const char*)(item.GetContent(content_len));
		//if (content_len > 0)
		//{
			//buffer.append(content, content_len);
		//}
	}

	/*adler32算法相对CRC算法更高效，这里使用adler32码代替CRC码*/
	player_.set_equip_crc(adler32(player_.equip_crc(), static_cast<const Bytef*>(buffer.contents()), buffer.size()));
}

bool PlayerInventory::UpdatePetItem(int item_idx)
{
	ItemList& inv = GetItemList(Item::PET_INV);
	if (!inv.Update(item_idx))
		return false;

	SelfGetItemData(Item::PET_INV, item_idx);
	return true;
}

bool PlayerInventory::UpdateCardItem(int item_idx)
{
	ItemList& inv = GetItemList(Item::CARD_INV);
	if (!inv.Update(item_idx))
		return false;

	SelfGetItemData(Item::CARD_INV, item_idx);
	return true;
}

bool PlayerInventory::CanTrade(int where, int item_idx) const
{
	if (where < 0 || where >= Item::INV_MAX)
		return false;

	const ItemList& inv = GetItemList(where);
	const Item& it = inv[item_idx];
	if (it.Type() <= 0)
		return false;

	return it.CanTrade();
}

bool PlayerInventory::GainItem(int32_t item_type, GainItemMode mode, int valid_time, int32_t& item_count)
{
	if (item_type <= 0 || item_count <= 0 || valid_time < 0)
	{
		return false;
	}

	itemdata tmpItem;
    if (!s_pItemMan->GenerateItem(item_type, tmpItem))
		return false;

	int item_cls = s_pItemMan->GetItemCls(item_type);
	if (item_cls <= 0)
		return false;

	int where = Item::LocateLocation(item_cls);
	if (where < 0)
		return false;

    if (tmpItem.pile_limit == 1 && valid_time != 0)
    {
        tmpItem.expire_date = g_timer->GetSysTime() + valid_time;
    }
	bool ret = GainItem(where, mode, tmpItem, item_count);
	return ret;
}

bool PlayerInventory::GainItem(int where, GainItemMode mode, itemdata& item)
{
	if (where < 0)
		return false;

	int item_count = item.count;
	return GainItem(where, mode, item, item_count);
}

bool PlayerInventory::GainItem(int where, GainItemMode mode, itemdata& item, int& item_count)
{
	if (item.id <= 0 || item_count <= 0)
		return false;

	if (item.proc_type & Item::ITEM_PROC_TYPE_BIND_ON_GAIN)
		item.proc_type |= Item::ITEM_PROC_TYPE_BIND;
    if (item.pile_limit != 1 && item.expire_date != 0)
        item.expire_date = 0;

	ItemList& inv = GetItemList(where);

	if (item.content.size() > 0)
	{
		// 获得复杂物品
		if (item.count != 1 || item.pile_limit != 1)
		{
			item_count = item.count;
			LOG_ERROR << "玩家 " << player_.role_id() << " 获得复杂物品失败，物品个数或者堆叠上限非法";
			return false;
		}

		if (inv.IsFull())
		{
			LOG_INFO << "玩家 " << player_.role_id() << " 获得物品失败，包裹满!!!";
			return false;
		}

		int last_index = inv.PushInEmpty(0, item);
		if (last_index < 0)
		{
			LOG_ERROR << "玩家 " << player_.role_id() << " 获得物品失败，last_index: " << last_index;
			return false;
		}

		size_t content_len = item.content.size();
		const void* content = item.content.data();
		uint16_t content_crc = adler32(1, static_cast<const Bytef*>(content), content_len);

		item_count = 0;
        item.index = last_index;
		SelfGainItemData(where, item.id, 1, item.expire_date, last_index, 1, mode, content, content_len, content_crc);
		//LOG_INFO << "玩家 " << player_.role_id() << " 获得复杂物品(" << item.id << ")";
        GLog::log("玩家 %ld 获得复杂物品(%d), gain_mode:%d", player_.role_id(), item.id, mode);
		return true;
	}

	// 获得简单物品，物品个数不限制
	int remain = item_count;
	while (remain > 0)
	{
		//确保放入物品数不超过堆叠上限
		if (remain > item.pile_limit)
		{
			item.count = item.pile_limit;
		}
		else
		{
			item.count = remain;
		}

		//剩余物品个数
		int __count = item.count;
		remain -= __count;

		//往包裹放
		int last_index = -1;
		if (!inv.Push(item, last_index))
		{
			//放入失败
			if (last_index >= 0)
			{
				//部分放入
				SelfGainItemData(where, item.id, __count-item.count, item.expire_date, last_index, inv[last_index].Amount(), mode);
			}
			else
			{
				//一个未放入
				//不用通知客户端
			}

			SendError(G2C::ERR_INV_FULL);

			//LOG_ERROR << "玩家 " << player_.role_id() << " 获得 " << __count - item.count << " 个物品(" << item.id << "), 包裹满了!!!";
            GLog::log("玩家 %ld 获得 %d 个物品(%d), 包裹满了!!! gain_mode:%d", player_.role_id(), __count - item.count, item.id, mode);

			//未放入的还回去
			remain += item.count;
			break;
		}
		else
		{
			//放入成功
            item.index = last_index;
			SelfGainItemData(where, item.id, __count, item.expire_date, last_index, inv[last_index].Amount(), mode);

			//LOG_INFO << "玩家 " << player_.role_id() << " 获得 " << __count - item.count << " 个物品(" << item.id << ")";
            GLog::log("玩家 %ld 获得 %d 个物品(%d), gain_mode:%d", player_.role_id(), __count - item.count, item.id, mode);
		}
	};

	item_count = remain;
	return true;
}

bool PlayerInventory::IsInvFull(int where) const
{
	const ItemList& inv = GetItemList(where);
	return inv.IsFull();
}

int32_t PlayerInventory::GetWeaponID() const
{
	return equipment_[Item::EQUIP_INDEX_WEAPON].Type();
}

ItemList& PlayerInventory::GetItemList(int where)
{
	ASSERT(where >= 0 && where < Item::INV_MAX);
	switch(where)
	{
		case Item::INVENTORY:
			return inventory_;
		case Item::EQUIPMENT:
			return equipment_;
		case Item::HIDE_INV:
			return hide_inv_;
		case Item::TASK_INV:
			return task_inv_;
		case Item::PET_INV:
			return pet_inv_;
		case Item::CARD_INV:
			return card_inv_;
		case Item::MOUNT_INV:
			return mount_inv_;
		default:
			ASSERT(false);
		return inventory_;
	};
}

const ItemList& PlayerInventory::GetItemList(int where) const
{
	ASSERT(where >= 0 && where < Item::INV_MAX);
	switch(where)
	{
		case Item::INVENTORY:
			return inventory_;
		case Item::EQUIPMENT:
			return equipment_;
		case Item::HIDE_INV:
			return hide_inv_;
		case Item::TASK_INV:
			return task_inv_;
		case Item::PET_INV:
			return pet_inv_;
		case Item::CARD_INV:
			return card_inv_;
		case Item::MOUNT_INV:
			return mount_inv_;
		default:
			ASSERT(false);
		return inventory_;
	};
}

void PlayerInventory::CMDHandler_GetItemData(const C2G::GetItemData& cmd)
{
	if (cmd.index < 0)
		return;

	int where = cmd.where;
	if (where < 0 || where >= Item::INV_MAX)
		return;

	SelfGetItemData(where, cmd.index);
}

void PlayerInventory::CMDHandler_GetItemListBrief(const C2G::GetItemListBrief& cmd)
{
	int where = cmd.where;
	if (where < 0 || where >= Item::INV_MAX)
		return;

	PlayerGetInventory(cmd.where);
}

void PlayerInventory::CMDHandler_GetItemListDetail(const C2G::GetItemListDetail& cmd)
{
	int where = cmd.where;
	if (where < 0 || where >= Item::INV_MAX)
		return;

	SelfGetInventoryDetail(where);
}

void PlayerInventory::CMDHandler_DropInventoryItem(const C2G::DropInventoryItem& cmd)
{
	size_t index = cmd.index;
	int count = cmd.count;
	if (index < 0 || count <= 0)
	{
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);
	}

	Item& it = inventory_[index];
	if (it.Type() == -1 || !it.CanDrop())
	{
		return;
	}

	int type = it.Type();
	if (inventory_.DecAmount(index, count) < 0)
	{
		return;
	}

	player_.sender()->DropInvItem(type, index, count);
}

void PlayerInventory::CMDHandler_MoveInventoryItem(const C2G::MoveInventoryItem& cmd)
{
	if (cmd.src_idx < 0 || cmd.dst_idx < 0 || cmd.count <= 0)
	{
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);
	}

	if (!inventory_.MoveItem(cmd.dst_idx, cmd.src_idx, cmd.count))
	{
		return;
	}

	player_.sender()->MoveInvItem(cmd.dst_idx, cmd.src_idx, cmd.count);
}

void PlayerInventory::CMDHandler_SellInventoryItem(const C2G::SellInventoryItem& cmd)
{
    if (cmd.where != Item::INVENTORY && cmd.where != Item::PET_INV && cmd.where != Item::CARD_INV && cmd.where != Item::MOUNT_INV)
    {
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);
    }
    if (cmd.index < 0 || cmd.count < 0)
    {
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);
    }

	ItemList& inv = GetItemList(cmd.where);
	if (!inv.SellItem(cmd.index, cmd.count))
	{
		return;
	}

	SelfGetItemData(cmd.where, cmd.index);
	player_.sender()->SellInvItem(cmd.where, cmd.index, cmd.count);
}

void PlayerInventory::CMDHandler_ExchangeInventoryItem(const C2G::ExchangeInventoryItem& cmd)
{
	if (cmd.index1 < 0 || cmd.index2 < 0)
	{
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);
	}

	if (!inventory_.ExchangeItem(cmd.index1, cmd.index2))
	{
		return;
	}

	player_.sender()->ExchangeInvItem(cmd.index1, cmd.index2);
}

void PlayerInventory::CMDHandler_TidyInventory(const C2G::TidyInventory& cmd)
{
    /*
	ASSERT(cmd.where == Item::INVENTORY);
	if (!player_.TestCoolDown(PCD_INDEX_TIDY_INV))
	{
		//冷却中
		return;
	}

	inventory_.TidyUp();
	SelfGetInventoryDetail(cmd.where);
	player_.SetCoolDown(PCD_INDEX_TIDY_INV, COOL_TIME_TIDY_INV, );
	LOG_INFO << "玩家" << player_.role_id() << "整理包裹";
    */
}

void PlayerInventory::CMDHandler_EquipItem(const C2G::EquipItem& cmd)
{
	size_t idx_inv = cmd.idx_inv;
	size_t idx_equip = cmd.idx_equip;

	if (idx_inv < 0 || idx_inv >= inventory_.Size())
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);

	if (idx_equip < 0 || idx_equip >= equipment_.Size())
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);

	if (inventory_.IsEmptySlot(idx_inv))
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);

	if (!inventory_[idx_inv].CanActivate(Item::INVENTORY, idx_equip, &player_))
	{
		LOG_ERROR << "玩家 " << player_.role_id() << " 装备物品失败, item_id: " << inventory_[idx_inv].Type();
		return;
	}

	//检查通过，开始装备
	Item it1;
	inventory_.Remove(idx_inv, it1);

	if (!equipment_.IsEmptySlot(idx_equip))
	{
		//装备栏非空
		Item it2;
		equipment_.Remove(idx_equip, it2);
		int rst = inventory_.PushInEmpty(idx_inv, it2);
		ASSERT(rst >= 0);

		if (rst >= 0)
		{
			it2.Clear();
		}
		else
		{
			it2.Release();
		}
	}
	int rst = equipment_.PushInEmpty(idx_equip, it1);
	ASSERT(rst >= 0);

	//装备完成
	bool equip_changed = false;
	Item & it = equipment_[idx_equip];
	if (it.TestBindOnEquip())
	{
		it.Bind();
		equip_changed = true;
	}

	player_.sender()->EquipItem(idx_inv, idx_equip);

	if (equip_changed)
	{
		//通知装备更新
		SelfGetItemData(Item::EQUIPMENT, idx_equip);
	}

	//更新装备CRC
	UpdateEquipCRC();

	if (idx_equip == Item::EQUIP_INDEX_WEAPON)
	{
		//武器更新, 广播给周围玩家
		int64_t visible_mask = G2C::PlayerVisibleInfoChange::VM_WEAPON;
		std::vector<int32_t> visible_list;
		visible_list.push_back(it.Type());
		player_.sender()->PlayerVisibleInfoChange(visible_mask, visible_list);
	}
}

void PlayerInventory::CMDHandler_UndoEquip(const C2G::UndoEquip& cmd)
{
	size_t idx_equip = cmd.idx_equip;
	if (idx_equip < 0 || idx_equip >= equipment_.Size())
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);

	if (equipment_.IsEmptySlot(idx_equip))
		return;

	if (inventory_.IsFull())
		ERROR_RETURN(G2C::ERR_INV_FULL);

	Item it;
	equipment_.Remove(idx_equip, it);
	int rst = inventory_.PushInEmpty(0, it);
	ASSERT(rst >= 0);
	if (rst >= 0)
	{
		it.Clear();
	}
	else
	{
		it.Release();
	}

	player_.sender()->UndoEquip(rst, idx_equip);

	//更新装备CRC
	UpdateEquipCRC();

	if (idx_equip == Item::EQUIP_INDEX_WEAPON)
	{
		//武器更新, 广播给周围玩家
		int64_t visible_mask = G2C::PlayerVisibleInfoChange::VM_WEAPON;
		std::vector<int32_t> visible_list;
		visible_list.push_back(0);
		player_.sender()->PlayerVisibleInfoChange(visible_mask, visible_list);
	}
}

void PlayerInventory::CMDHandler_UseItem(const C2G::UseItem& cmd)
{
	int where = cmd.where;
	if (where < 0 || where >= Item::INV_MAX)
		return;

	size_t index = cmd.index;
	if (index < 0) return;

	ItemList& inv = GetItemList(where);
	Item& item = inv[index];
	if (!item.CanUse((Item::LOCATION)(where), index, &player_))
	{
        GLog::log("玩家 %ld 尝试使用物品失败，item_id:%d", player_.role_id(), inv[index].Type());
		return;
	}

	int type  = item.Type();
	int count = inv.UseItem(index);
	if (count > 0)
    {
	    player_.sender()->UseItem(where, type, index, count);
        GLog::log("玩家 %ld 使用物品:%d 个数:%d", player_.role_id(), type, count);
    }
    else if (count == 0)
	{
        GLog::log("玩家 %ld 使用物品:%d，使用后不消耗", player_.role_id(), type);
	}
    else
    {
        GLog::log("玩家 %ld 使用物品失败，item_id:%d", player_.role_id(), inv[index].Type());
    }
}

void PlayerInventory::CMDHandler_RefineEquip(const C2G::RefineEquip& cmd)
{
	int where = cmd.where;
	int level = cmd.level;
	int index = cmd.index;

	if (index < 0 || level <= 0)
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);

	if (where != Item::INVENTORY && where != Item::EQUIPMENT)
		ERROR_RETURN(G2C::ERR_FATAL_ERROR);

	ItemList& list = GetItemList(where);
	if (list[index].Type() <= 0)
		return;

	Item& it = list[index];
	if (it.GetRefineLevel() >= level || !it.CanRefine((Item::LOCATION)(where), index, &player_))
	{
		LOG_ERROR << "玩家 " << player_.role_id() << " 精练装备失败，item_id: " << it.Type();
		return;
	}

	for (;;)
	{
		if (!it.CanRefine((Item::LOCATION)(where), index, &player_))
			break;

		it.RefineEquip((Item::LOCATION)(where), index, &player_);
        player_.RefineAchieve(it.GetRefineLevel());
		LOG_INFO << "玩家 " << player_.role_id() << " 精练装备 " << it.Type() << " 到 " << it.GetRefineLevel() << " 级";
		if (it.GetRefineLevel() >= level)
			break;
	}

	//通知客户端物品信息更新
	itemdata data;
	data.index = index;
	if (cmd.where == Item::INVENTORY)
	{
		inventory_.SaveItem(index, data);
	}
	else if (cmd.where == Item::EQUIPMENT)
	{
		equipment_.SaveItem(index, data);
	}

    player_.sender()->SelfItemData(where, data, G2C::SelfItemDetail::UT_REFINE);
	//player_.sender()->RefineEquipReply(where, index, data);

	if (where == Item::EQUIPMENT)
	{
		//更新装备CRC
		UpdateEquipCRC();
	}
}

void PlayerInventory::CMDHandler_ItemDisassemble(const C2G::ItemDisassemble& cmd)
{
    // 目前只能分解普通包裹里的装备和武器
    if (cmd.index < 0 || cmd.index >= (int)inventory_.Size() )
        return;

    const EquipTempl* pequip = s_pDataTempl->QueryDataTempl<EquipTempl>(cmd.item_id);
    if (pequip == NULL)
    {
        // 必须是装备或武器
        player_.sender()->ErrorMessage(G2C::ERR_ITEM_CANNOT_DISASSEMBLE);
        return;
    }

    // 检查物品是否存在
    if (!CheckItem(cmd.index, cmd.item_id, 1))
        return;

    // 计算分解后的个数
    const NormalItemTempl* pitem = s_pDataTempl->QueryDataTempl<NormalItemTempl>(FIXED_ITEM_DISASSEMBLE_PRODUCT);
    if (pitem == NULL)
        return;

    const Item& it = inventory_[cmd.index];
    if (it.Type() != cmd.item_id)
        return;

    int calc_count = (int)(((float)it.GetTotalPrice() / 50.f) + 0.5f);
    calc_count     = (calc_count > 0) ? calc_count : 1;
    int slot_count = calc_count / pitem->pile_limit + 1;

    // 包裹是否还有空间
    if (!HasSlot(Item::INVENTORY, slot_count))
    {
        player_.sender()->ErrorMessage(G2C::ERR_INV_FULL_CANNOT_DISASSEMBLE);
        return;
    }

    // 把装备或武器消耗掉
    if (!TakeOutItem(cmd.index, cmd.item_id, 1))
    {
        player_.sender()->ErrorMessage(G2C::ERR_DISASSEMBLE_FAILURE);
        return;
    }

    // 获得分解后的物品
    int count = calc_count;
	if (!GainItem(FIXED_ITEM_DISASSEMBLE_PRODUCT, playerdef::GIM_NORMAL, 0, count))
    {
        player_.sender()->ErrorMessage(G2C::ERR_GAIN_ITEM_FAILURE);
        GLog::log("玩家 %ld 分解物品后，获得物品:%d失败，应获得%d个，实际获得%d个.", player_.role_id(), 
                FIXED_ITEM_DISASSEMBLE_PRODUCT, calc_count, count);
        return;
    }

    // 通知客户端
    G2C::ItemDisassemble_Re packet;
    packet.index = cmd.index;
    player_.sender()->SendCmd(packet);

    // 记录远程log
    GLog::log("玩家 %ld 分解物品:%d,获得分解后物品:%d count:%d", player_.role_id(), cmd.item_id, FIXED_ITEM_DISASSEMBLE_PRODUCT, count);
}

ItemList* PlayerInventory::QueryInventory(int32_t item_type)
{
	int item_cls = s_pItemMan->GetItemCls(item_type);
	if (item_cls <= 0)
		return NULL;

	int where = Item::LocateLocation(item_cls);
	if (where < 0)
		return NULL;

	ItemList* inv = &GetItemList(where);
	return inv;
}

const ItemList* PlayerInventory::QueryInventory(int32_t item_type) const
{
	int item_cls = s_pItemMan->GetItemCls(item_type);
	if (item_cls <= 0)
		return NULL;

	int where = Item::LocateLocation(item_cls);
	if (where < 0)
		return NULL;

	const ItemList* inv = &GetItemList(where);
	return inv;
}

void PlayerInventory::SelfGetItemData(int where, int index) const
{
	const ItemList& inv = GetItemList(where);

	itemdata data;

	data.index = index;
	inv.SaveItem(index, data);
	player_.sender()->SelfItemData(where, data, G2C::SelfItemDetail::UT_NORMAL);
}

void PlayerInventory::SelfGainItemData(int where,
									   int type,
									   int amount,
									   int expire_date,
									   size_t last_idx,
									   int last_amount,
                                       int gain_mode,
									   const void* content,
									   size_t content_len,
									   uint16_t content_crc) const
{
	player_.sender()->GainItem(where, type, amount, expire_date, last_idx, last_amount, gain_mode, content, content_len, content_crc);
}

int32_t PlayerInventory::GetEquipNum(int32_t refine_level, int32_t rank) const
{
    int32_t num = 0;
    const ItemList& item_list = GetItemList(Item::EQUIPMENT);
    for (size_t i = 0; i < item_list.Size(); ++i)
    {
        const Item& it = item_list[i];
        if (it.Type() > 0 && it.GetRefineLevel() >= refine_level && it.GetRank() >= rank)
        {
            ++num;
        }
    }
    return num;
}

int32_t PlayerInventory::GetCardNum(int32_t level, int32_t rank) const
{
    return 0;
}

/********************************GM DEBUG CMD*********************************/
/********************************GM DEBUG CMD*********************************/
/********************************GM DEBUG CMD*********************************/
/********************************GM DEBUG CMD*********************************/
void PlayerInventory::DebugCleanAllItem(const C2G::DebugCleanAllItem& cmd)
{
	int where = cmd.where;
	if (where < 0 || where >= Item::INV_MAX)
		return;

	ItemList& inv = GetItemList(where);
	inv.Clear();

	SelfGetInventoryDetail(where);
	LOG_INFO << "玩家" << player_.role_id() << "清除包裹" << where;
}

#undef ERROR_RETURN

} // namespace gamed
