#ifndef GAMED_GS_SUBSYS_PLAYER_CARD_H_
#define GAMED_GS_SUBSYS_PLAYER_CARD_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"

namespace gamed 
{

class PlayerCard : public PlayerSubSystem
{
public:
    PlayerCard(Player& player);
    virtual ~PlayerCard();

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    bool GainCard(int32_t card_item_id, int32_t card_item_idx);
	bool RegisterCard(const playerdef::CardEntry& card);
	bool UnRegisterCard(int32_t card_item_idx);
	bool QueryCardInfo(int32_t card_item_idx, playerdef::CardEntry& card) const;
	void QueryCardSkill(std::set<int32_t>& skills) const;
    int32_t GetEquipCardNum(int32_t rank, int32_t level) const;
protected:
	//
	// CMD处理函数
	//
	void CMDHandler_LevelUpCard(const C2G::LevelUpCard&);
	void CMDHandler_EquipCard(const C2G::EquipCard&);
private:
    bool IsCardExist(int32_t card_item_idx) const;
private:
    static const int32_t MAX_VICE_CARD_NUM = 10;
    typedef std::map<int32_t/*item_index*/, playerdef::CardEntry> CardMap;
    CardMap card_inv_;      // 卡牌列表
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_CARD_H_
