#ifndef GAMED_GS_SUBSYS_PLAYER_PUNCH_CARD_H_
#define GAMED_GS_SUBSYS_PLAYER_PUNCH_CARD_H_

#include "gs/player/player_subsys.h"

// proto
#include "common/protocol/gen/global/player_punch_card.pb.h"


namespace gamed {

/**
 * @brief：player签到子系统
 */
class PlayerPunchCard : public PlayerSubSystem
{
public:
    PlayerPunchCard(Player& player);
	virtual ~PlayerPunchCard();

    bool SaveToDB(common::PlayerPunchCardData* pdata);
    bool LoadFromDB(const common::PlayerPunchCardData& data);

	virtual void OnHeartbeat(time_t cur_time);
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    void   GetPunchCardData() const;
    void   ResetPunchCard();


protected:
    void   CMDHandler_PlayerPunchCard(const C2G::PlayerPunchCard&);
    void   CMDHandler_GainPunchCardAward(const C2G::GainPunchCardAward&);
    void   CMDHandler_QueryPunchCardData(const C2G::QueryPunchCardData&);
    void   CMDHandler_RePunchCardHelpRe(const C2G::RePunchCardHelp_Re&);

	int    MSGHandler_RePunchCardHelp(const MSG&);
	int    MSGHandler_RePunchCardHelpReply(const MSG&);
	int    MSGHandler_RePunchCardHelpError(const MSG&);


private:
    void   PunchCardByMonthDay(time_t now, int day_of_month);
    void   RefreshMonthlyInfo(time_t now);
    int    CalcRePunchCount() const;
    void   SendQueryPunchCardDataRe();


private:
    int32_t last_punch_time_;     // 上次签到时间
    int32_t last_refresh_time_;   // 上次检查刷新的时间
    int32_t re_punch_count_;      // 自己帮好友补签的次数
    int32_t history_punch_;       // 所有签到次数
    int32_t history_award_got_;   // 历史奖励领取到哪一档，记录的是当前档的累计次数
    int32_t monthly_punch_mask_;  // 每月已经签到哪几天，是一个mask，标记[1,31]
    typedef std::set<int32_t> MonthlyAwardSet;
    MonthlyAwardSet monthly_award_got_; // 当月已经领取的奖励，记录的是累计次数

    // runtime data
    int32_t query_cooldown_;  // 查询冷却
    common::protocol::global::RePunchCardHelp re_punch_request_;
}; 

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_PUNCH_CARD_H_
