#include "player_auction.h"

#include "common/rpc/gen/masterd/auction_start.pb.h"
#include "common/rpc/gen/masterd/auction_cancel.pb.h"
#include "common/rpc/gen/masterd/auction_buyout.pb.h"
#include "common/rpc/gen/masterd/auction_bid.pb.h"
#include "common/protocol/gen/G2M/auction_msg.pb.h"
#include "gamed/client_proto/G2C_proto.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/player/player_sender.h"
#include "gs/global/auction_config.h"
#include "gs/global/dbgprt.h"
#include "gs/item/item.h"
#include "gs/player/subsys_if.h"


namespace gamed {

using namespace dataTempl;
using namespace masterRpc;
using namespace common::protocol;

SHARED_STATIC_ASSERT((int)masterRpc::ACT_SCORE == (int)C2G::ACT_SCORE);
SHARED_STATIC_ASSERT((int)masterRpc::ACT_CASH  == (int)C2G::ACT_CASH);
SHARED_STATIC_ASSERT((int)masterRpc::ACT_SCORE == (int)G2C::ACT_SCORE);
SHARED_STATIC_ASSERT((int)masterRpc::ACT_CASH  == (int)G2C::ACT_CASH);


///
/// PlayerAuction
///
PlayerAuction::PlayerAuction(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_AUCTION, player)
{
    SAVE_LOAD_REGISTER(common::PlayerAuctionData, PlayerAuction::SaveToDB, PlayerAuction::LoadFromDB);
    ResetAllCoolDown();
}

PlayerAuction::~PlayerAuction()
{
    ResetAllCoolDown();
}

bool PlayerAuction::SaveToDB(common::PlayerAuctionData* pdata)
{
    // 拍卖的个人数据不需要gs来存盘
    return true;
}

bool PlayerAuction::LoadFromDB(const common::PlayerAuctionData& data)
{
    for (size_t i = 0; i < data.auction_list.size(); ++i)
    {
        const common::PlayerAuctionData::auction_entry& ent = data.auction_list[i];
        if (ent.auction_type == common::PlayerAuctionData::PLAYER_AUCTION)
        {
            auction_item_set_.insert(ent.auction_id);
        }
        else if (ent.auction_type == common::PlayerAuctionData::PLAYER_BID)
        {
            bid_item_map_[ent.auction_id] = ent.bid_price;
        }
        else
        {
            ASSERT(false);
        }
    }
    return true;
}

void PlayerAuction::SaveSelfAuctionItem(int64_t auction_id)
{
    ASSERT(auction_item_set_.insert(auction_id).second);
}

void PlayerAuction::SaveSelfBidItem(int64_t auction_id, int32_t price)
{
    bid_item_map_[auction_id] = price;
}

void PlayerAuction::AuctionItemResult(int err, int64_t auction_id, const void* request)
{
    const masterRpc::AuctionStartProxyRequest* tmpstub 
        = static_cast<const masterRpc::AuctionStartProxyRequest*>(request);
    ASSERT(tmpstub);

    G2C::AuctionItem_Re packet;

    // success
    if (err == 0)
    {
        packet.err_code             = err;
        packet.detail.auction_id    = auction_id;
        packet.detail.currency_type = tmpstub->currency_type();
        packet.detail.buyout_price  = tmpstub->buyout_price();
        packet.detail.bid_price     = tmpstub->bid_price();
        packet.detail.item_id       = tmpstub->item_id();
        packet.detail.item_name     = tmpstub->item_name();
        packet.detail.item_info     = tmpstub->item_info();

        SaveSelfAuctionItem(auction_id);
    }
    else // failure
    {
        packet.err_code             = err;
        packet.detail.auction_id    = 0;
        packet.detail.currency_type = 0;
        packet.detail.buyout_price  = 0;
        packet.detail.bid_price     = 0;
        packet.detail.item_id       = 0;
        packet.detail.item_name     = "";
        packet.detail.item_info     = "";
    }

    player_.sender()->SendCmd(packet);
}

void PlayerAuction::AuctionCancelResult(int err_code, int64_t auction_id, const void* response) const
{
    const AuctionCancelProxyResponse* tmpres 
        = static_cast<const AuctionCancelProxyResponse*>(response);
    ASSERT(tmpres);

    G2C::AuctionCancel_Re packet;
    packet.auction_id   = auction_id;
    if (err_code == AuctionCancelProxyResponse::NO_ERROR)
    {
        packet.err_code = G2C::AuctionCancel_Re::IS_SUCCESS;
    }
    else
    {
        packet.err_code = err_code;
    }

    packet.bidder        = tmpres->has_bidder() ? tmpres->bidder() : 0;
    packet.cur_bid_price = tmpres->has_cur_bid_price() ? tmpres->cur_bid_price() : 0;

    player_.sender()->SendCmd(packet);
}

void PlayerAuction::AuctionBuyoutResult(int err_code, const void* request)
{
    const AuctionBuyoutProxyRequest* buyout_req = static_cast<const AuctionBuyoutProxyRequest*>(request);
    ASSERT(buyout_req);

    G2C::AuctionBuyout_Re packet;
    packet.auction_id = buyout_req->auction_id();
    if (err_code == AuctionBuyoutProxyResponse::NO_ERROR)
    {
        packet.err_code = G2C::AuctionBuyout_Re::IS_SUCCESS;
    }
    else
    {
        packet.err_code = err_code;
    }
    player_.sender()->SendCmd(packet);
}

void PlayerAuction::AuctionBidResult(int err_code, int32_t cur_price, int64_t cur_bidder, const void* request)
{
    const AuctionBidProxyRequest* bid_req = static_cast<const AuctionBidProxyRequest*>(request);
    ASSERT(bid_req);

    G2C::AuctionBid_Re packet;
    packet.auction_id = bid_req->auction_id();
    packet.cur_price  = cur_price;
    packet.cur_bidder = cur_bidder;
    if (err_code == AuctionBidProxyResponse::NO_ERROR)
    {
        packet.err_code = G2C::AuctionBid_Re::IS_SUCCESS;
        SaveSelfBidItem(bid_req->auction_id(), bid_req->price());
    }
    else
    {
        packet.err_code = err_code;
    }
    player_.sender()->SendCmd(packet);
}

void PlayerAuction::AuctionInvalidQuery(const std::vector<int64_t>& auctionid_list)
{
    if (auctionid_list.empty())
        return;

    for (size_t i = 0; i < auctionid_list.size(); ++i)
    {
        int64_t id = auctionid_list[i];
        SelfAuctionItemSet::iterator it_set = auction_item_set_.find(id);
        if (it_set != auction_item_set_.end())
        {
            auction_item_set_.erase(it_set);
        }

        SelfBidItemMap::iterator it_map = bid_item_map_.find(id);
        if (it_map != bid_item_map_.end())
        {
            bid_item_map_.erase(it_map);
        }
    }

    G2C::AuctionRevoke packet;
	std::copy(auctionid_list.begin(), auctionid_list.end(), back_inserter(packet.auctionid_list));
    player_.sender()->SendCmd(packet);
}

bool PlayerAuction::TakeoutCost(int currency_type, int32_t price)
{
    if (price <= 0)
        return false;

    // 涉及游戏币和元宝的操作，小心！
    if (currency_type == C2G::ACT_SCORE)
    {
        if (!player_.CheckScore(price))
            return false;
        
        player_.SpendScore(price);
        return true;
    }
    else if (currency_type == C2G::ACT_CASH)
    {
        if (!player_.CheckCash(price))
            return false;

        player_.UseCash(price);
        return true;
    }
    
    return false;
}

bool PlayerAuction::CheckQueryCoolDown(QueryCooldownType type, int64_t param) const
{
    if (type == QCT_QUERY_LIST)
    {
        QueryListCDMap::const_iterator it = querylist_cooldown_.find(param);
        if (it != querylist_cooldown_.end() && it->second > 0)
            return true;
    }
    else
    {
        if (cooldown_array_[type] > 0)
            return true;
    }

    return false;
}

void PlayerAuction::SetQueryCoolDown(QueryCooldownType type, int64_t param)
{
    if (type == QCT_QUERY_LIST)
    {
        querylist_cooldown_[param] = kQueryCoolDownTime;
    }
    else
    {
        cooldown_array_[type] = kQueryCoolDownTime;
    }
}

void PlayerAuction::HeartbeatQueryCoolDown()
{
    for (size_t i = 0; i < cooldown_array_.size(); ++i)
    {
        if (cooldown_array_[i] > 0)
        {
            cooldown_array_[i] -= 1;
        }
    }

    QueryListCDMap::iterator it_query = querylist_cooldown_.begin();
    for (; it_query != querylist_cooldown_.end(); ++it_query)
    {
        if (it_query->second > 0)
        {
            it_query->second -= 1;
        }
    }
}

void PlayerAuction::ResetAllCoolDown()
{
    for (size_t i = 0; i < cooldown_array_.size(); ++i)
    {
        cooldown_array_[i] = 0;
    }

    QueryListCDMap::iterator it_query = querylist_cooldown_.begin();
    for (; it_query != querylist_cooldown_.end(); ++it_query)
    {
        it_query->second = 0;
    }
}

void PlayerAuction::OnHeartbeat(time_t cur_time)
{
    // count down cooldown
    HeartbeatQueryCoolDown();
}

void PlayerAuction::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionItem, PlayerAuction::CMDHandler_AuctionItem);
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionCancel, PlayerAuction::CMDHandler_AuctionCancel);
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionBuyout, PlayerAuction::CMDHandler_AuctionBuyout);
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionBid, PlayerAuction::CMDHandler_AuctionBid);
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionQueryList, PlayerAuction::CMDHandler_AuctionQueryList);
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionQueryBidPrice, PlayerAuction::CMDHandler_AuctionQueryBidPrice);
    REGISTER_NORMAL_CMD_HANDLER(C2G::AuctionQueryDetail, PlayerAuction::CMDHandler_AuctionQueryDetail);
    REGISTER_NORMAL_CMD_HANDLER(C2G::GetSelfAuctionItem, PlayerAuction::CMDHandler_GetSelfAuctionItem);
    REGISTER_NORMAL_CMD_HANDLER(C2G::GetSelfBidItem, PlayerAuction::CMDHandler_GetSelfBidItem);
}

void PlayerAuction::RegisterMsgHandler()
{
}

void PlayerAuction::CMDHandler_AuctionItem(const C2G::AuctionItem& cmd)
{
    if (auction_item_set_.size() >= kMaxAuctionItemCount)
        return;

    if (cmd.item_id <= 0 || 
        cmd.item_count <= 0 || 
        cmd.buyout_price < 0 ||
        cmd.bid_price <= 0)
    {
        return;
    }

    // 拍卖物品的一口价可以等于0，代表不能一口价买该物品
    if (cmd.buyout_price > 0 &&
        cmd.buyout_price < cmd.bid_price)
    {
        return;
    }

    if (cmd.currency_type != C2G::ACT_SCORE && 
        cmd.currency_type != C2G::ACT_CASH)
    {
        return;
    }

    if (!s_pAuctionCfg->IsAuctionableItem(cmd.item_id))
    {
        __PRINTF("物品%d不在拍卖物品列表里", cmd.item_id);
        return;
    }

    const ItemDataTempl* ptmpl = s_pDataTempl->QueryDataTempl<ItemDataTempl>(cmd.item_id);
    if (ptmpl == NULL)
    {
        __PRINTF("CMDHandler_AuctionItem 没有找到物品的模板 id:%d", cmd.item_id);
        return;
    }

    itemdata tmpdata;
    if (!player_.QueryItem(cmd.where, cmd.index, tmpdata))
    {
        __PRINTF("CMDHandler_AuctionItem 没有找到该物品 where:%d index:%d itemid:%d",
                cmd.where, cmd.index, cmd.item_id);
        return;
    }
    if (cmd.item_id != tmpdata.id || tmpdata.count < cmd.item_count)
    {
        __PRINTF("CMDHandler_AuctionItem 所查到的物品id不一致或数量不满足，cli_id:%d,%d svr_id:%d,%d",
                cmd.item_id, cmd.item_count, tmpdata.id, tmpdata.count);
        return;
    }
    if (tmpdata.proc_type == Item::ITEM_PROC_TYPE_NOTRADE ||
        tmpdata.proc_type == Item::ITEM_PROC_TYPE_BIND)
    {
        __PRINTF("CMDHandler_AuctionItem 所查物品不能拍卖，roleid:%ld, item_id:%d",
                player_.role_id(), tmpdata.id);
        player_.sender()->ErrorMessage(G2C::ERR_ITEM_CAN_NOT_AUCTION);
        return;
    }
    // 修改成实际的数量
    tmpdata.count = cmd.item_count;

    // make item detail
	ByteBuffer buffer;
    G2C::ItemDetail detail;
    MakeItemDetail(detail, tmpdata);
    G2C::ItemDetailVec tmp_vec;
    tmp_vec.item_list.push_back(detail);
    tmp_vec.Pack(buffer); 
    ASSERT(buffer.size());

    // take out item
    ASSERT(player_.TakeOutItem(tmpdata.index, tmpdata.id, tmpdata.count));

    playerdef::AuctionItemData data;
    data.currency_type = cmd.currency_type; 
    data.buyout_price  = cmd.buyout_price; 
    data.bid_price     = cmd.bid_price;
    data.item_id       = cmd.item_id;
    data.item_name     = ptmpl->visible_name.to_str(); 
    data.item_info.assign((const char*)buffer.contents(), buffer.size());

    player_.AuctionItem(data);
}

void PlayerAuction::CMDHandler_AuctionCancel(const C2G::AuctionCancel& cmd)
{
    if (cmd.auction_id <= 0)
        return;

    player_.AuctionCancel(cmd.auction_id);
}

void PlayerAuction::CMDHandler_AuctionBuyout(const C2G::AuctionBuyout& cmd)
{
    if (cmd.auction_id <= 0 || cmd.price <= 0)
        return;

    if (!TakeoutCost(cmd.currency_type, cmd.price))
        return; 

    player_.AuctionBuyout(cmd.auction_id, cmd.currency_type, cmd.price);
}

void PlayerAuction::CMDHandler_AuctionBid(const C2G::AuctionBid& cmd)
{
    if (bid_item_map_.size() >= kMaxBidItemCount)
        return;

    if (cmd.auction_id <= 0 || cmd.price <= 0)
        return;

    if (!TakeoutCost(cmd.currency_type, cmd.price))
        return; 
    
    player_.AuctionBid(cmd.auction_id, cmd.currency_type, cmd.price);
}

void PlayerAuction::CMDHandler_AuctionQueryList(const C2G::AuctionQueryList& cmd)
{
    if (cmd.category < 0 || cmd.category >= kMaxQueryListType)
    {
        __PRINTF("CMDHandler_AuctionQueryList category invalid!!");
        return;
    }

    if (cmd.currency_type != C2G::ACT_SCORE &&
        cmd.currency_type != C2G::ACT_CASH)
        return;

    int64_t cooldown_key = makeInt64(cmd.category, cmd.currency_type);
    if (CheckQueryCoolDown(QCT_QUERY_LIST, cooldown_key))
        return;
    
    G2M::QueryAuctionList proto;
    proto.set_roleid(player_.role_id());
    proto.set_category(cmd.category);
    proto.set_currency_type(cmd.currency_type);
    player_.sender()->SendToMaster(proto);

    SetQueryCoolDown(QCT_QUERY_LIST, cooldown_key);
}

void PlayerAuction::CMDHandler_AuctionQueryBidPrice(const C2G::AuctionQueryBidPrice& cmd)
{
    if (CheckQueryCoolDown(QCT_QUERY_BID_PRICE))
        return;

    G2M::QueryAuctionBidPrice proto;
    proto.set_roleid(player_.role_id());
    for (size_t i = 0; i < cmd.auctionid_list.size(); ++i)
    {
        int64_t auction_id = cmd.auctionid_list[i];
        if (auction_id <= 0) {
            continue;
        }
        if (auction_item_set_.find(auction_id) == auction_item_set_.end() &&
            bid_item_map_.find(auction_id) == bid_item_map_.end()) {
            continue;
        }
        proto.add_auction_id(auction_id); 
    }

    if (proto.auction_id_size() > 0)
    {
        player_.sender()->SendToMaster(proto);
        SetQueryCoolDown(QCT_QUERY_BID_PRICE);
    }
}

void PlayerAuction::CMDHandler_AuctionQueryDetail(const C2G::AuctionQueryDetail& cmd)
{
    if (CheckQueryCoolDown(QCT_QUERY_DETAIL))
        return;

    G2M::QueryAuctionDetail proto;
    proto.set_roleid(player_.role_id());
    for (size_t i = 0; i < cmd.auctionid_list.size(); ++i)
    {
        int64_t auction_id = cmd.auctionid_list[i];
        if (auction_id <= 0) {
            continue;
        }
        if (auction_item_set_.find(auction_id) == auction_item_set_.end() &&
            bid_item_map_.find(auction_id) == bid_item_map_.end()) {
            continue;
        }
        proto.add_auction_id(auction_id); 
    }

    if (proto.auction_id_size() > 0)
    {
        player_.sender()->SendToMaster(proto);
        SetQueryCoolDown(QCT_QUERY_DETAIL);
    }
}

void PlayerAuction::CMDHandler_GetSelfAuctionItem(const C2G::GetSelfAuctionItem& cmd)
{
    if (CheckQueryCoolDown(QCT_GET_SELF_AUCTION_ITEM))
        return;

    G2M::QueryAuctionDetail proto;
    proto.set_roleid(player_.role_id());
    SelfAuctionItemSet::const_iterator it = auction_item_set_.begin();
    for (; it != auction_item_set_.end(); ++it)
    {
        proto.add_auction_id(*it);
    }
    player_.sender()->SendToMaster(proto);

    SetQueryCoolDown(QCT_GET_SELF_AUCTION_ITEM);
}

void PlayerAuction::CMDHandler_GetSelfBidItem(const C2G::GetSelfBidItem& cmd)
{
    if (CheckQueryCoolDown(QCT_GET_SELF_BID_ITEM))
        return;

    G2M::QueryPlayerBidItem proto;
    proto.set_roleid(player_.role_id());
    SelfBidItemMap::const_iterator it = bid_item_map_.begin();
    for (; it != bid_item_map_.end(); ++it)
    {
        G2M::QueryPlayerBidItem::Info* info = proto.add_info_list();
        info->set_auction_id(it->first);
        info->set_self_bid_price(it->second);
    }
    player_.sender()->SendToMaster(proto);

    SetQueryCoolDown(QCT_GET_SELF_BID_ITEM);
}

} // namespace gamed
