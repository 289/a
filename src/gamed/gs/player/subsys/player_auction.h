#ifndef GAMED_GS_SUBSYS_PLAYER_AUCTION_H_
#define GAMED_GS_SUBSYS_PLAYER_AUCTION_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player拍卖子系统
 */
class PlayerAuction : public PlayerSubSystem
{
    static const int32_t kQueryCoolDownTime   = 2; // 单位是秒
    static const int32_t kMaxQueryListType    = 32;
    static const size_t  kMaxAuctionItemCount = 8;
    static const size_t  kMaxBidItemCount     = 128;

public:
	PlayerAuction(Player& player);
	virtual ~PlayerAuction();

    bool SaveToDB(common::PlayerAuctionData* pdata);
    bool LoadFromDB(const common::PlayerAuctionData& data);

    void AuctionItemResult(int err, int64_t auction_id, const void* request);
    void AuctionCancelResult(int err_code, int64_t auction_id, const void* response) const;
    void AuctionBuyoutResult(int err_code, const void* request);
    void AuctionBidResult(int err_code, int32_t cur_price, int64_t cur_bidder, const void* request);
    void AuctionInvalidQuery(const std::vector<int64_t>& auctionid_list);

	virtual void OnHeartbeat(time_t cur_time);
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();


protected:
    void CMDHandler_AuctionItem(const C2G::AuctionItem&);
    void CMDHandler_AuctionCancel(const C2G::AuctionCancel&);
    void CMDHandler_AuctionBuyout(const C2G::AuctionBuyout&);
    void CMDHandler_AuctionBid(const C2G::AuctionBid&);
    void CMDHandler_AuctionQueryList(const C2G::AuctionQueryList&);
    void CMDHandler_AuctionQueryBidPrice(const C2G::AuctionQueryBidPrice&);
    void CMDHandler_AuctionQueryDetail(const C2G::AuctionQueryDetail&);
    void CMDHandler_GetSelfAuctionItem(const C2G::GetSelfAuctionItem&);
    void CMDHandler_GetSelfBidItem(const C2G::GetSelfBidItem&);


private:
    enum QueryCooldownType
    {
        QCT_QUERY_LIST = 0,
        QCT_QUERY_BID_PRICE,
        QCT_QUERY_DETAIL,
        QCT_GET_SELF_AUCTION_ITEM,
        QCT_GET_SELF_BID_ITEM,
        QCT_MAX_COUNT,
    };


private:
    void SaveSelfAuctionItem(int64_t auction_id);
    void SaveSelfBidItem(int64_t auction_id, int32_t price);
    bool TakeoutCost(int currency_type, int32_t price);
    bool CheckQueryCoolDown(QueryCooldownType type, int64_t param = 0) const;
    void SetQueryCoolDown(QueryCooldownType type, int64_t param = 0);
    void HeartbeatQueryCoolDown();
    void ResetAllCoolDown();


private:
    typedef std::set<int64_t/*auction_id*/> SelfAuctionItemSet;
    SelfAuctionItemSet auction_item_set_;

    typedef std::map<int64_t/*auction_id*/, int32_t/*price*/> SelfBidItemMap;
    SelfBidItemMap     bid_item_map_;

    shared::net::FixedArray<int, QCT_MAX_COUNT> cooldown_array_;

    typedef std::map<int64_t, int> QueryListCDMap;
    QueryListCDMap     querylist_cooldown_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_AUCTION_H_
