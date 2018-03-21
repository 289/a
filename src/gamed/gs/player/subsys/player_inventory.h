#ifndef GAMED_GS_SUBSYS_PLAYER_INVENTORY_H_
#define GAMED_GS_SUBSYS_PLAYER_INVENTORY_H_

#include "gs/item/item_list.h"
#include "gs/player/player_subsys.h"


namespace gamed {

/*
 * @class PlayerInventory
 * @brief 物品子系统，包括包裹、装备栏、任务包裹、隐藏包裹、宠物包裹
 */

class PlayerInventory : public PlayerSubSystem
{
	ItemList inventory_;
	ItemList equipment_;
	ItemList hide_inv_;
	ItemList task_inv_;
	ItemList pet_inv_;
	ItemList card_inv_;
	ItemList mount_inv_;

public: 
	PlayerInventory(Player& player);
	virtual ~PlayerInventory();

	bool SaveToDB(common::PlayerInventoryData* pData);
	bool LoadFromDB(const common::PlayerInventoryData& data);

	virtual void OnInit();
	virtual void OnHeartbeat(time_t cur_time);
	virtual void OnRelease();
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();
	virtual void OnTransferCls();

	///
	/// 物品子系统对外接口
	///
	int32_t GetWeaponID() const;
	void PlayerGetInventoryAll() const;
	void PlayerGetInventory(int where);
	void SelfGetInventoryDetail(int where);
	bool GainItem(int32_t item_type, GainItemMode mode, int valid_time, int32_t& item_count);
	bool GainItem(int where, GainItemMode mode, itemdata& item);
	bool IsInvFull(int where) const;
	int  CountItem(int item_type) const;
	int  CountEquipItem(int item_type) const;
	bool HasSlot(int where, int empty_slot_count) const;
	bool CheckItem(int item_type, int count) const;
	bool CheckItem(int item_idx, int item_type, int count) const;
	bool TakeOutItem(int item_type, int count);
	bool TakeOutItem(int item_idx, int item_type, int count);
	bool TakeOutAllItem(int item_type, int count);
	bool QueryItem(int where, int item_idx, itemdata& item) const;
	void QueryItemlist(int where, int& inv_cap, std::vector<itemdata>& list) const;
	void UpdateEquipCRC();
	bool UpdatePetItem(int item_idx);
	bool UpdateCardItem(int item_idx);
	bool CanTrade(int where, int item_idx) const;
    int32_t GetEquipNum(int32_t refine_level, int32_t rank) const;
    int32_t GetCardNum(int32_t level, int32_t rank) const;

    // 获取某个包裹中所有物品的信息（现在用于debug命令）
    bool GetItemListDetail(int where, std::vector<itemdata>& list) const;


protected:
	bool  GainItem(int where, GainItemMode mode, itemdata& item, int& item_count);
	ItemList& GetItemList(int where);
	ItemList* QueryInventory(int32_t item_type);
	const ItemList& GetItemList(int where) const;
	const ItemList* QueryInventory(int32_t item_type) const;
	void  SelfGetItemData(int where, int index) const;

	// 玩家游戏中获得新物品
	// 获得的物品可以为简单物品也可以为复杂物品
	void  SelfGainItemData(int where,
			               int type,
						   int amount,
						   int expire_date,
						   size_t last_idx,
						   int last_amount,
                           int gain_mode,
						   const void* content=NULL,
						   size_t content_len=0,
						   uint16_t content_crc=0) const;

	///
	/// CMD处理函数
	///
	void CMDHandler_GetItemData(const C2G::GetItemData&);
	void CMDHandler_GetItemListBrief(const C2G::GetItemListBrief&);
	void CMDHandler_GetItemListDetail(const C2G::GetItemListDetail&);
	void CMDHandler_DropInventoryItem(const C2G::DropInventoryItem&);
	void CMDHandler_MoveInventoryItem(const C2G::MoveInventoryItem&);
	void CMDHandler_SellInventoryItem(const C2G::SellInventoryItem&);
	void CMDHandler_ExchangeInventoryItem(const C2G::ExchangeInventoryItem&);
	void CMDHandler_TidyInventory(const C2G::TidyInventory&);
	void CMDHandler_EquipItem(const C2G::EquipItem&);
	void CMDHandler_UndoEquip(const C2G::UndoEquip&);
	void CMDHandler_UseItem(const C2G::UseItem&);
	void CMDHandler_RefineEquip(const C2G::RefineEquip&);
    void CMDHandler_ItemDisassemble(const C2G::ItemDisassemble&);

	///
	/// 调试CMD处理函数
	///
	void DebugCleanAllItem(const C2G::DebugCleanAllItem&);
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_INVENTORY_H_
