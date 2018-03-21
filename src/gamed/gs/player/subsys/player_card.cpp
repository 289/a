#include "player_card.h"


#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/player/subsys_if.h"
#include "gs/template/data_templ/card_templ.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/player/player_sender.h"
#include "gs/item/item_data.h"
#include "gs/item/item.h"

namespace gamed
{

using namespace std;
using namespace dataTempl;

#define GetCardTempl(id) s_pDataTempl->QueryDataTempl<dataTempl::CardTempl>(id)

PlayerCard::PlayerCard(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_CARD, player)
{
}

PlayerCard::~PlayerCard()
{
}

void PlayerCard::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::LevelUpCard, PlayerCard::CMDHandler_LevelUpCard);
	REGISTER_NORMAL_CMD_HANDLER(C2G::EquipCard, PlayerCard::CMDHandler_EquipCard);
}

void PlayerCard::RegisterMsgHandler()
{
}

bool PlayerCard::IsCardExist(int32_t card_item_idx) const
{
    return card_inv_.find(card_item_idx) != card_inv_.end();
}
/*
static void MakeCardEntry(G2C::CardEntry& entry, const playerdef::CardEntry& card)
{
    entry.id = card.id;
    entry.exp = card.exp;
    entry.star_id = card.star_id;
    entry.item_idx = card.item_idx;
}
*/
bool PlayerCard::GainCard(int32_t card_item_id, int32_t card_item_idx)
{
	///
	/// 调用本接口表示玩家游戏中真正获得卡牌
	/// 所以需要通知客户端
	///
    const CardTempl* templ = GetCardTempl(card_item_id);
    if (!templ)
    {
        return false;
    }

    playerdef::CardEntry& card = card_inv_[card_item_idx];
    card.id = card_item_id;
    card.exp = 0;
    card.star_id = 0;
    card.item_idx = card_item_idx;
    card.card_templ = templ;

    //G2C::GainCard packet;
    //MakeCardEntry(packet.card, card);
    //SendCmd(packet);
    return true;
}

static void UpdateProp(Player& player, const CardTempl* templ, bool inc)
{
    size_t size = templ->addon_props.size();
    for (size_t i = 0; i < size; ++i)
    {
        int32_t value = templ->addon_props[i];
        if (value == 0)
        {
            continue;
        }

        if (inc)
        {
            player.IncPropPoint(i, value);
        }
        else
        {
            player.DecPropPoint(i, value);
        }
    }
}

static void UpdateSkillLevel(Player& player, const CardTempl* templ, bool inc)
{
    size_t size = templ->lvlup_skills.size();
    for (size_t i = 0; i < size; ++i)
    {
        const CardTempl::skill_entry& entry = templ->lvlup_skills[i];
        int32_t gid = entry.skill_group_id;
        int32_t level = inc ? entry.tmp_skill_level : -1 * entry.tmp_skill_level;
        player.LevelUpSkill(gid, level, LVLUP_MODE_CARD);
    }
}

static void TriggerCardEffect(Player& player, const CardTempl* templ, bool active)
{
    UpdateProp(player, templ, active);
    UpdateSkillLevel(player, templ, active);
}

bool PlayerCard::RegisterCard(const playerdef::CardEntry& card)
{
	///
	/// 调用本接口并不是指玩家获得卡牌,
	/// 而是玩家上线时使用卡牌物品来初始化卡牌子系统,
	/// 所以这里不需要通知客户端
	///
	if (IsCardExist(card.item_idx))
	{
		LOG_ERROR << "玩家 " << player_.role_id() << " 添加卡牌失败，卡牌物品IDX：" << card.item_idx; 
		ASSERT(false);
		return false;
	}

    card_inv_[card.item_idx] = card;
    if (card.star_id != 0)
    {
        TriggerCardEffect(player_, static_cast<const CardTempl*>(card.card_templ), true);
    }
    return true;
}

bool PlayerCard::UnRegisterCard(int32_t card_item_idx)
{
	if (!IsCardExist(card_item_idx))
    {
		LOG_ERROR << "玩家 " << player_.role_id() << " 删除卡牌失败，卡牌物品IDX：" << card_item_idx;
		ASSERT(false);
		return false;
	}

    playerdef::CardEntry& card = card_inv_[card_item_idx];
    if (card.star_id != 0)
    {
        TriggerCardEffect(player_, static_cast<const CardTempl*>(card.card_templ), false);
    }
    card_inv_.erase(card_item_idx);

	//G2C::LostCard packet;
	//packet.item_inv_idx = card_item_idx;
	//SendCmd(packet);
    return true;
}

bool PlayerCard::QueryCardInfo(int32_t card_item_idx, playerdef::CardEntry& card) const
{
    CardMap::const_iterator it = card_inv_.find(card_item_idx);
    if (it == card_inv_.end())
    {
        return false;
    }
    card = it->second;
    return true;
}

void PlayerCard::CMDHandler_LevelUpCard(const C2G::LevelUpCard& cmd)
{
    // 检查升级类型
    if (cmd.lvlup_type < C2G::LevelUpCard::LVLUP_TYPE_CARD ||
        cmd.lvlup_type > C2G::LevelUpCard::LVLUP_TYPE_CASH)
    {
        return;
    }
    // 检查主卡有效性
    if (!IsCardExist(cmd.idx_main_card))
    {
        return;
    }
    playerdef::CardEntry& main_card = card_inv_[cmd.idx_main_card];
    const CardTempl* mtempl = GetCardTempl(main_card.id);
    if (mtempl == NULL || mtempl->card_gain_on_lvlup <= 0)
    {
        return;
    }
    bool use_cash = cmd.lvlup_type == C2G::LevelUpCard::LVLUP_TYPE_CASH;
    int32_t size = cmd.idx_vice_cards.size();
    if ((use_cash && size != 0) || (!use_cash && (size <= 0 || size > MAX_VICE_CARD_NUM)))
    {
        return;
    }
    int32_t old_star_id = main_card.star_id;
    // 计算升级提供的经验值以及需要的消耗
    const GlobalConfigTempl* global_templ = s_pDataTempl->QueryGlobalConfigTempl();
    std::map<int32_t, int32_t> takeout_cards;
    int32_t exp = use_cash ? mtempl->exp_need_on_lvlup : main_card.exp;
    int32_t consume = use_cash ? (exp - main_card.exp) * global_templ->card_config.cash_consume_per_exp : 0;
    for (int32_t i = 0; i < size; ++i)
    {
        int32_t vice_card_index = cmd.idx_vice_cards[i];
        if (!IsCardExist(vice_card_index))
        {
            return;
        }
        const playerdef::CardEntry& vice_card = card_inv_[vice_card_index];
        const CardTempl* vtempl = GetCardTempl(vice_card.id);
        if (vtempl == NULL)
        {
            return;
        }
        takeout_cards[vice_card.item_idx] = vice_card.id;
        exp += vtempl->exp_offer_on_consume;
        consume += vtempl->money_need_on_consume;
    }
    // 检查消耗是否足够
    if ((use_cash && !player_.CheckCash(consume)) || 
        (!use_cash && !player_.CheckMoney(consume)))
    {
        player_.sender()->LevelUpCard(0, cmd.lvlup_type);
        return;
    }
    // 计算升级后的卡牌
    const CardTempl* lvl_templ = mtempl;
    while (lvl_templ->card_gain_on_lvlup > 0 && exp >= lvl_templ->exp_need_on_lvlup)
    {
        exp -= lvl_templ->exp_need_on_lvlup;
        lvl_templ = GetCardTempl(lvl_templ->card_gain_on_lvlup);
    }
    if (lvl_templ != mtempl) // 升级了则消耗掉主卡牌
    {
        takeout_cards[main_card.item_idx] = main_card.id;
    }
    if (lvl_templ->card_gain_on_lvlup <= 0 || use_cash)
    {
        exp = 0;
    }
    // 扣除合成消耗
    if (use_cash)
    {
        player_.UseCash(consume);
    }
    else
    {
        player_.SpendMoney(consume);
    }
    std::map<int32_t, int32_t>::const_iterator it = takeout_cards.begin();
    for (; it != takeout_cards.end(); ++it)
    {
        if (!player_.TakeOutItem(it->first, it->second, 1))
        {
            return; // 前面检查了包裹按理不应出错，如果出错扣了白扣
        }
    }
    // 生成新的卡牌物品
    if (lvl_templ != mtempl)
    {
        itemdata data;
        data.id = lvl_templ->templ_id;
        data.count = 1;
        data.pile_limit = lvl_templ->pile_limit;
        data.proc_type = mtempl->proc_type;     // 延续原卡牌的处理方式
        data.recycle_price = lvl_templ->recycle_price;
        data.expire_date = 0;
        data.item_cls = Item::ITEM_CLS_CARD;
        if (!player_.GainItem(Item::CARD_INV, data, GIM_CARD))
        {
            return;
        }
        // 更新动态属性
        playerdef::CardEntry& new_card = card_inv_[data.index];
        new_card.exp = exp;
        new_card.star_id = old_star_id;
        player_.UpdateCardItem(new_card.item_idx);
        if (old_star_id != 0)
        {
            TriggerCardEffect(player_, static_cast<const CardTempl*>(new_card.card_templ), true);
        }
    }
    else
    {
        main_card.exp = exp;
        player_.UpdateCardItem(main_card.item_idx);
    }
    player_.sender()->LevelUpCard(1, cmd.lvlup_type);

    GLog::log("玩家%ld 合成卡牌成功，消耗%d%s，消耗卡牌：", player_.role_id(), consume, use_cash ? "元宝" : "游戏币");
}

void PlayerCard::CMDHandler_EquipCard(const C2G::EquipCard& cmd)
{
    // 卡牌不存在
    if (!IsCardExist(cmd.card_index))
    {
        return;
    }
    // 星盘是否可以镶嵌
    if (!player_.AllSparkActivate(cmd.star_id))
    {
        return;
    }
    // 是否已经有卡牌镶嵌在该星辰上
    CardMap::iterator it = card_inv_.begin();
    for (; it != card_inv_.end(); ++it)
    {
        if (it->second.star_id == cmd.star_id)
        {
            break;
        }
    }
    if (it != card_inv_.end())
    {
        it->second.star_id = 0;
        player_.UpdateCardItem(it->second.item_idx);
        TriggerCardEffect(player_, static_cast<const CardTempl*>(it->second.card_templ), false);
    }
    playerdef::CardEntry& card = card_inv_[cmd.card_index];
    card.star_id = cmd.star_id;
    player_.UpdateCardItem(card.item_idx);
    TriggerCardEffect(player_, static_cast<const CardTempl*>(card.card_templ), true);

    G2C::EquipCard_Re reply;
    reply.card_index = cmd.card_index;
    reply.star_id = cmd.star_id;
    player_.sender()->SendCmd(reply);
}

void PlayerCard::QueryCardSkill(std::set<int32_t>& skills) const
{
    typedef vector<const CardTempl*> CardTemplVec;
    typedef map<int16_t, CardTemplVec> CardGroupMap;
    CardGroupMap activate_cards;
    CardMap::const_iterator cit = card_inv_.begin();
    for (; cit != card_inv_.end(); ++cit)
    {
        const CardEntry& entry = cit->second;
        if (entry.star_id == 0)
        {
            continue;
        }

        const CardTempl* new_templ = static_cast<const CardTempl*>(entry.card_templ);
        CardTemplVec& templ_vec = activate_cards[new_templ->card_group_id];
        if (new_templ->card_group_id == -1 || templ_vec.empty())
        {
            templ_vec.push_back(new_templ);
        }
        else
        {
            const CardTempl* old_templ = templ_vec[0];
            if ((new_templ->quality > old_templ->quality) || (new_templ->quality == old_templ->quality || new_templ->level > old_templ->level))
            {
                templ_vec[0] = new_templ;
            }
        }
    }

    CardGroupMap::const_iterator ait = activate_cards.begin();
    for (; ait != activate_cards.end(); ++ait)
    {
        const CardTemplVec& templ_vec = ait->second;
        CardTemplVec::const_iterator cit = templ_vec.begin();
        for (; cit != templ_vec.end(); ++cit)
        {
            if ((*cit)->addon_skill_id != 0)
            {
                skills.insert((*cit)->addon_skill_id);
            }
        }
    }
}

int32_t PlayerCard::GetEquipCardNum(int32_t rank, int32_t level) const
{
    int32_t num = 0;
    CardMap::const_iterator it = card_inv_.begin();
    for (; it != card_inv_.end(); ++it)
    {
        const CardEntry& entry = it->second;
        if (entry.star_id == 0)
        {
            continue;
        }
        const CardTempl* templ = static_cast<const CardTempl*>(entry.card_templ);
        if (templ->quality >= rank && templ->level >= level)
        {
            num += 1;
        }
    }
    return num;
}

} // namespace gamed
