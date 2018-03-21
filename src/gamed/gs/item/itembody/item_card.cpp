#include "item_card.h"

#include "gs/player/player.h"
#include "gs/template/data_templ/card_templ.h"
#include "gs/template/data_templ/templ_manager.h"

namespace gamed
{

void ItemCard::Initialize(const dataTempl::ItemDataTempl* tpl)
{
    templ = dynamic_cast<const dataTempl::CardTempl*>(tpl);
}

void ItemCard::OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    ItemEssence& item_ess = parent->GetItemEssence();
    EssCard* pEss = item_ess.QueryEss<EssCard>(ESS_ID_CARD);
    if (pEss == NULL)
    {
		//玩家游戏中获得卡牌物品
		//为卡牌物品生成动态属性
        pEss = new EssCard;
        pEss->card_exp = 0;
        pEss->star_id = 0;
        item_ess.InsertEss(pEss);
        // 玩家获得卡牌
        Player* player = dynamic_cast<Player*>(obj);
        player->GainCard(parent->Type(), index);
		return;
    }
	playerdef::CardEntry entry;
	entry.id        = templ->templ_id;
	entry.exp       = pEss->card_exp;
	entry.star_id   = pEss->star_id;
	entry.item_idx  = index;
    entry.card_templ = templ;

	//向卡牌子系统注册
	Player* player = dynamic_cast<Player*>(obj);
	player->RegisterCard(entry);
}

void ItemCard::OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    ItemEssence& item_ess = parent->GetItemEssence();
    EssCard* pEss = item_ess.QueryEss<EssCard>(ESS_ID_CARD);
    if (pEss == NULL)
    {
        return;
    }
	//玩家失去卡牌
    Player* player = dynamic_cast<Player*>(obj);
    player->UnRegisterCard(index);
}

bool ItemCard::DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	///
	/// 卡牌子系统的数据更新，同步更新卡牌包裹中对应的卡牌物品
	///

	//从卡牌子系统获取卡牌信息
	playerdef::CardEntry entry;
	Player* player = dynamic_cast<Player*>(obj);
	if (!player->QueryCardInfo(index, entry))
		return false;

	if (entry.id != templ->templ_id)
		return false;

    ItemEssence& item_ess = parent->GetItemEssence();
    EssCard* pEss = item_ess.QueryEss<EssCard>(ESS_ID_CARD);
    if (pEss == NULL)
    {
        return false;
    }

	//更新卡牌属性
	pEss->card_exp  = entry.exp;
	pEss->star_id   = entry.star_id;
	return true;
}

};
