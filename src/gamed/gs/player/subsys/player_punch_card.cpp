#include "player_punch_card.h"

#include "common/obj_data/gen/player/punch_card.pb.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/global/glogger.h"
#include "gs/global/date_time.h"
#include "gs/global/timer.h"
#include "gs/global/game_util.h"
#include "gs/global/dbgprt.h"
#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;

namespace {

    static const int kQueryCooldownSecs = 5;
    
    /*
    inline bool check_mday_valid(time_t now, int32_t day_of_month)
    {
        if (day_of_month < 1 || day_of_month > 31)
            return false;

        struct tm tt = GetLocalTimeTM(now);
        int mday     = GetMDay(tt.tm_year + 1900, tt.tm_mon);
        // 每月初
        if (tt.tm_mday == 1)
        {
            int last_month = tt.tm_mon - 1;
            last_month     = (last_month < 0) ? 11 : last_month;
            int last_mday  = GetMDay(tt.tm_year + 1900, last_month);
            // 不是当天也不是上月的最后一天
            if (day_of_month != tt.tm_mday && day_of_month != last_mday)
                return false;
        }
        else // 不是月初
        {
            if (day_of_month > mday)
                return false;

            if (day_of_month > tt.tm_mday)
                return false;
        }

        return true;
    }

    // 当天是指今天凌晨5点到次日凌晨五点，需要day_of_month是合法值
    inline bool at_the_same_day(time_t now, int32_t day_of_month)
    {
        struct tm tt   = GetLocalTimeTM(now);
        int last_month = tt.tm_mon - 1;
        last_month     = (last_month < 0) ? 11 : last_month;
        int last_mday  = GetMDay(tt.tm_year + 1900, last_month);
        int day_diff   = tt.tm_mday - day_of_month;
        if (day_diff == 0) // 当天
        {
            if (tt.tm_hour >= 5)
                return true;
        }
        else if (day_diff == 1) // 昨天
        {
            if (tt.tm_hour < 5)
                return true;
        }
        else
        {
            // 跨月
            if (day_of_month == last_mday && tt.tm_mday == 1)
            {
                if (tt.tm_hour < 5)
                    return true;
            }
        }
        return false;
    }
    */

    inline bool check_mday_valid(time_t now, int32_t day_of_month)
    {
        if (day_of_month < 1 || day_of_month > 31)
            return false;

        struct tm tt = GetLocalTimeTM(now);
        int mday     = GetMDay(tt.tm_year + 1900, tt.tm_mon);
        if (day_of_month > mday)
            return false;
        
        return true;
    }

    // 当天指过24点，需要day_of_month是合法值
    inline bool at_the_same_day(time_t now, int32_t day_of_month)
    {
        struct tm tt = GetLocalTimeTM(now);
        if (day_of_month == tt.tm_mday)
            return true;

        return false;
    }
    
    inline bool has_punched(int32_t mday, int32_t punch_mask)
    {
        if (punch_mask & (1 << mday))
            return true;

        return false;
    }

    inline void mask_month_punch(int32_t mday, int32_t& punch_mask)
    {
        punch_mask |= 1 << mday;
    }

    inline bool get_award_entry(int cumulative_num, int type, PlayerPunchCardConfig::Entry& ent)
    {
        const GlobalConfigTempl* ptpl = s_pDataTempl->QueryGlobalConfigTempl();
        if (type == C2G::PCAT_MONTHLY)
        {
            for (size_t i = 0; i < ptpl->punch_card_cfg.monthly.size(); ++i)
            {
                if (cumulative_num == ptpl->punch_card_cfg.monthly[i].cumulative_num)
                {
                    ent = ptpl->punch_card_cfg.monthly[i];
                    return true;
                }
            }
        }
        else if (type == C2G::PCAT_HISTORY)
        {
            for (size_t i = 0; i < ptpl->punch_card_cfg.history.size(); ++i)
            {
                if (cumulative_num == ptpl->punch_card_cfg.history[i].cumulative_num)
                {
                    ent = ptpl->punch_card_cfg.history[i];
                    return true;
                }
            }
        }

        return false;
    }

    inline bool check_history_award(int cumulative_num)
    {
        const GlobalConfigTempl* ptpl = s_pDataTempl->QueryGlobalConfigTempl();
        for (size_t i = 0; i < ptpl->punch_card_cfg.history.size(); ++i)
        {
            if (cumulative_num == ptpl->punch_card_cfg.history[i].cumulative_num)
            {
                return true;
            }
        }
        return false;
    }

} // Anonymous


///
/// PlayerPunchCard
///
PlayerPunchCard::PlayerPunchCard(Player& player)
    : PlayerSubSystem(SUB_SYS_TYPE_PUNCH_CARD, player),
      last_punch_time_(0),
      last_refresh_time_(0),
      re_punch_count_(1),
      history_punch_(0),
      history_award_got_(GS_INT32_MAX),
      monthly_punch_mask_(0),
      query_cooldown_(0)
{
    SAVE_LOAD_REGISTER(common::PlayerPunchCardData, PlayerPunchCard::SaveToDB, PlayerPunchCard::LoadFromDB);
}

PlayerPunchCard::~PlayerPunchCard()
{
}

bool PlayerPunchCard::SaveToDB(common::PlayerPunchCardData* pdata)
{
    pdata->last_punch_time    = last_punch_time_;
    pdata->last_refresh_time  = last_refresh_time_;
    pdata->re_punch_count     = re_punch_count_;
    pdata->history_punch      = history_punch_;
    pdata->history_award_got  = history_award_got_;
    pdata->monthly_punch_mask = monthly_punch_mask_;

    // monthly scalable
    common::scalable::PCMonthlyAwardGot monthly_scala;
    MonthlyAwardSet::iterator it = monthly_award_got_.begin();
    for (; it != monthly_award_got_.end(); ++it)
    {
        monthly_scala.add_cumulative(*it);
    }

    pdata->monthly_award_got.clear();
    std::string& bufRef = pdata->monthly_award_got;
    bufRef.resize(monthly_scala.ByteSize());
    if (!monthly_scala.SerializeToArray((void*)bufRef.c_str(), bufRef.size()))
    {
        GLog::log("PlayerPunchCard::SaveToDB error! roleid:%ld", player_.role_id());
        return false;
    }

    return true;
}

bool PlayerPunchCard::LoadFromDB(const common::PlayerPunchCardData& data)
{
    last_punch_time_    = data.last_punch_time;   
    last_refresh_time_  = data.last_refresh_time; 
    re_punch_count_     = data.re_punch_count;  
    history_punch_      = data.history_punch;     
    history_award_got_  = data.history_award_got; 
    monthly_punch_mask_ = data.monthly_punch_mask;

    if (data.monthly_award_got.size() > 0)
    {
        common::scalable::PCMonthlyAwardGot scalable_data;
        const std::string& bufRef = data.monthly_award_got;
        if (!scalable_data.ParseFromArray(bufRef.c_str(), bufRef.size()))
        {
            GLog::log("PlayerPunchCard::LoadFromDB error! roleid:%ld", player_.role_id());
            return false;
        }

        for (int i = 0; i < scalable_data.cumulative_size(); ++i)
        {
            monthly_award_got_.insert(scalable_data.cumulative(i));
        }
    }

    // 刚登陆要刷新一下信息，可能是刚创建角色
    time_t now = g_timer->GetSysTime();
    RefreshMonthlyInfo(now);
    return true;
} 

void PlayerPunchCard::OnHeartbeat(time_t cur_time)
{
    if (query_cooldown_ > 0)
    {
        --query_cooldown_;
    }
}

void PlayerPunchCard::GetPunchCardData() const
{
    G2C::PunchCardData packet;

    packet.history_punch     = history_punch_;
    packet.history_award_got = history_award_got_;
    packet.re_punch_count    = re_punch_count_;
    
    if (monthly_punch_mask_ != 0)
    {
        for (int i = 0; i < 32; ++i) {
            if (monthly_punch_mask_ & (1 << i)) {
                packet.monthly_punch.insert(i);
            }
        }
    }

    packet.monthly_award_got = monthly_award_got_; 
    player_.sender()->SendCmd(packet);
}

void PlayerPunchCard::PunchCardByMonthDay(time_t now, int day_of_month)
{
    ++history_punch_;
    last_punch_time_ = now;
    mask_month_punch(day_of_month, monthly_punch_mask_);
}

void PlayerPunchCard::ResetPunchCard()
{
    last_punch_time_    = 0;
    last_refresh_time_  = 0;
    re_punch_count_     = CalcRePunchCount();
    history_punch_      = 0;
    history_award_got_  = 0;
    monthly_punch_mask_ = 0;
    monthly_award_got_.clear();
    query_cooldown_     = 0;
}

int PlayerPunchCard::CalcRePunchCount() const
{
    // vip会增加这个值
    return 10;
}

void PlayerPunchCard::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::PlayerPunchCard, PlayerPunchCard::CMDHandler_PlayerPunchCard);
    REGISTER_NORMAL_CMD_HANDLER(C2G::GainPunchCardAward, PlayerPunchCard::CMDHandler_GainPunchCardAward);
    REGISTER_NORMAL_CMD_HANDLER(C2G::QueryPunchCardData, PlayerPunchCard::CMDHandler_QueryPunchCardData);
    REGISTER_NORMAL_CMD_HANDLER(C2G::RePunchCardHelp_Re, PlayerPunchCard::CMDHandler_RePunchCardHelpRe);
}

void PlayerPunchCard::RegisterMsgHandler()
{
    REGISTER_MSG_HANDLER(GS_MSG_RE_PUNCH_CARD_HELP, PlayerPunchCard::MSGHandler_RePunchCardHelp);
    REGISTER_MSG_HANDLER(GS_MSG_RE_PUNCH_CARD_HELP_REPLY, PlayerPunchCard::MSGHandler_RePunchCardHelpReply);
    REGISTER_MSG_HANDLER(GS_MSG_RE_PUNCH_CARD_HELP_ERROR, PlayerPunchCard::MSGHandler_RePunchCardHelpError);
}

void PlayerPunchCard::CMDHandler_PlayerPunchCard(const C2G::PlayerPunchCard& cmd)
{
    if (cmd.day_of_month < 1 || cmd.day_of_month > 31)
        return;

    time_t now = g_timer->GetSysTime();
    if (!check_mday_valid(now, cmd.day_of_month))
        return;

    // 刷新月度信息
    RefreshMonthlyInfo(now);

    G2C::PunchCardResult packet;
    packet.day_of_month  = cmd.day_of_month;
    packet.friend_roleid = cmd.friend_roleid;

    // 已经签到
    if (has_punched(cmd.day_of_month, monthly_punch_mask_))
    {
        packet.result = G2C::PunchCardResult::RT_HAS_PUNCHED;
        player_.sender()->SendCmd(packet);
        return;
    }

    // 当天签到
    if (at_the_same_day(now, cmd.day_of_month))
    {
        PunchCardByMonthDay(now, cmd.day_of_month);
        packet.result = G2C::PunchCardResult::RT_SUCCESS;
        player_.AddCashByGame(50); // FIXME:临时代码
    }
    else // 补签
    {
        if (cmd.friend_roleid <= 0)
            return;

        if (!player_.IsFriend(cmd.friend_roleid))
            return;

        // 发给好友
        global::RePunchCardHelp proto;
        proto.set_requester(player_.role_id());
        proto.set_friend_roleid(cmd.friend_roleid);
        proto.set_req_link_id(player_.link_id());
        proto.set_req_sid_in_link(player_.sid_in_link());
        proto.set_day_of_month(cmd.day_of_month);
        proto.set_first_name(player_.first_name());
        proto.set_mid_name(player_.middle_name());
        proto.set_last_name(player_.last_name());
        player_.sender()->SendToMaster(proto);
        return;
    }

    player_.sender()->SendCmd(packet);
}

void PlayerPunchCard::CMDHandler_GainPunchCardAward(const C2G::GainPunchCardAward& cmd)
{
    if (cmd.cumulative_num <= 0)
        return;

    time_t now = g_timer->GetSysTime();
    RefreshMonthlyInfo(now);

    G2C::GainPunchCardAward_Re packet;
    packet.result         = G2C::GainPunchCardAward_Re::RT_SUCCESS;
    packet.award_type     = cmd.award_type;
    packet.cumulative_num = cmd.cumulative_num;

    // 检查是否已经领取
    if (cmd.award_type == C2G::PCAT_MONTHLY)
    {
        std::set<int32_t>::iterator it = monthly_award_got_.find(cmd.cumulative_num);
        if (it != monthly_award_got_.end())
        {
            packet.result = G2C::GainPunchCardAward_Re::RT_GAINED;
            player_.sender()->SendCmd(packet);
            return;
        }
    }
    else if (cmd.award_type == C2G::PCAT_HISTORY)
    {
        if (!check_history_award(cmd.cumulative_num))
        {
            packet.result = G2C::GainPunchCardAward_Re::RT_NOT_FOUND;
            player_.sender()->SendCmd(packet);
            return;
        }
    }
    else
    {
        return;
    }

    // 查找对应的级别
    PlayerPunchCardConfig::Entry ent;
    if (!get_award_entry(cmd.cumulative_num, cmd.award_type, ent))
    {
        __PRINTF("CMDHandler_GainPunchCardAward 没有获得对应的奖励等级");
        packet.result = G2C::GainPunchCardAward_Re::RT_NOT_FOUND;
        player_.sender()->SendCmd(packet);
        return;
    }
    ASSERT(cmd.cumulative_num == ent.cumulative_num);

    // get award
    if (cmd.award_type == C2G::PCAT_MONTHLY)
    {
        monthly_award_got_.insert(ent.cumulative_num);
    }
    else if (cmd.award_type == C2G::PCAT_HISTORY)
    {
        if (ent.cumulative_num <= history_award_got_)
        {
            packet.result = G2C::GainPunchCardAward_Re::RT_DATA_ERR;
            player_.sender()->SendCmd(packet);
            return;
        }
        history_award_got_ = ent.cumulative_num;
    }

    // success
    if (ent.award_exp > 0) {
        player_.IncExp(ent.award_exp);
    }

    if (ent.award_money > 0) {
        player_.GainMoney(ent.award_money);
    }

    if (ent.award_score > 0) {
        player_.GainScore(ent.award_score);
    }

    for (size_t i = 0; i < ent.award_item.size(); ++i) {
        player_.GainItem(ent.award_item[i].tid, ent.award_item[i].count);
    }

    player_.sender()->SendCmd(packet);
}

void PlayerPunchCard::CMDHandler_QueryPunchCardData(const C2G::QueryPunchCardData& cmd)
{
    time_t now = g_timer->GetSysTime();
    RefreshMonthlyInfo(now);

    SendQueryPunchCardDataRe();
}

void PlayerPunchCard::SendQueryPunchCardDataRe()
{
    G2C::QueryPunchCardData_Re packet;
    packet.re_punch_count = re_punch_count_;
    player_.sender()->SendCmd(packet);
}

void PlayerPunchCard::CMDHandler_RePunchCardHelpRe(const C2G::RePunchCardHelp_Re& cmd)
{
    if (cmd.requester != re_punch_request_.requester() ||
        cmd.day_of_month != re_punch_request_.day_of_month())
    {
        __PRINTF("CMDHandler_RePunchCardHelpRe 信息已变化！");
        return;
    }

    if (cmd.agreement && re_punch_count_ > 0)
    {
        --re_punch_count_;

        global::RePunchCardHelpReply proto;
        proto.set_requester(re_punch_request_.requester());
        proto.set_friend_roleid(player_.role_id());
        proto.set_day_of_month(re_punch_request_.day_of_month());
        player_.sender()->SendToMaster(proto);
    }
    else
    {
        G2C::PunchCardResult packet;
        if (re_punch_count_ <= 0) {
            packet.result = G2C::PunchCardResult::RT_FRIEND_NO_COUNT;
        }
        else {
            packet.result = G2C::PunchCardResult::RT_FRIEND_REJECT;
        }
        packet.day_of_month  = re_punch_request_.day_of_month();
        packet.friend_roleid = player_.role_id();
        player_.sender()->SendCmdByMaster(cmd.requester, packet);
    }
}

void PlayerPunchCard::RefreshMonthlyInfo(time_t now)
{
    if (query_cooldown_ > 0)
        return;

    struct tm last_tt = GetLocalTimeTM(last_refresh_time_);
    // 跨天刷新补签次数
    if (!at_the_same_day(now, last_tt.tm_mday))
    {
        last_refresh_time_ = now;
        re_punch_count_    = CalcRePunchCount();

        struct tm tt = GetLocalTimeTM(now);
        // 跨月刷新月度奖励，要在跨天之后检查
        if (last_tt.tm_year != tt.tm_year || last_tt.tm_mon != tt.tm_mon)
        {
            monthly_punch_mask_ = 0;
            monthly_award_got_.clear();
        }
    }
    
    query_cooldown_ = kQueryCooldownSecs;
}

int PlayerPunchCard::MSGHandler_RePunchCardHelp(const MSG& msg)
{
    // parse proto
    global::RePunchCardHelp proto;
    ASSERT(proto.ParseFromArray(msg.content, msg.content_len));

    if (re_punch_count_ > 0)
    {
        G2C::RePunchCardHelp packet;
        packet.role_id      = proto.requester();
        packet.day_of_month = proto.day_of_month();
        packet.first_name   = proto.first_name();
        packet.mid_name     = proto.mid_name();
        packet.last_name    = proto.last_name();
        player_.sender()->SendCmd(packet);

        // save requester
        re_punch_request_.CopyFrom(proto);
    }
    else
    {
        G2C::PunchCardResult packet;
        packet.result        = G2C::PunchCardResult::RT_FRIEND_NO_COUNT;
        packet.day_of_month  = proto.day_of_month();
        packet.friend_roleid = player_.role_id();
        player_.sender()->PunchCardPacket(proto.requester(), proto.req_link_id(), proto.req_sid_in_link(), packet);
    }
    return 0;
}

int PlayerPunchCard::MSGHandler_RePunchCardHelpReply(const MSG& msg)
{
    global::RePunchCardHelpReply proto;
    ASSERT(proto.ParseFromArray(msg.content, msg.content_len));

    if (!player_.IsFriend(proto.friend_roleid()))
    {
        __PRINTF("MSGHandler_RePunchCardHelpReply此人已经不是好友，不能帮忙补签!");
        return 0;
    }

    time_t now       = g_timer->GetSysTime();
    int day_of_month = proto.day_of_month();

    // 刷新月度信息
    RefreshMonthlyInfo(now);

    // 当天
    if (at_the_same_day(now, day_of_month))
    {
        __PRINTF("MSGHandler_RePunchCardHelpReply当天是不能好友补签的！");
        return 0;
    }

    if (!check_mday_valid(now, day_of_month))
    {
        __PRINTF("MSGHandler_RePunchCardHelpReply补签的日子不对？？mday:%d", day_of_month);
        return 0;
    }
    
    G2C::PunchCardResult packet;
    packet.result        = G2C::PunchCardResult::RT_SUCCESS;
    packet.day_of_month  = day_of_month;
    packet.friend_roleid = proto.friend_roleid();

    G2C::RePunchCardHelp_Re re_pack;
    re_pack.is_success   = true;
    re_pack.first_name   = player_.first_name();
    re_pack.middle_name  = player_.middle_name();
    re_pack.last_name    = player_.last_name();

    // 已经签到
    if (has_punched(day_of_month, monthly_punch_mask_))
    {
        packet.result = G2C::PunchCardResult::RT_HAS_PUNCHED;
        player_.sender()->SendCmd(packet);

        // master, 告诉该好友已经有人补签
        global::RePunchCardHelpError send_proto;
        send_proto.set_requester(proto.requester());
        send_proto.set_friend_roleid(proto.friend_roleid());
        player_.sender()->SendToMaster(send_proto);

        // 告诉好友结果
        re_pack.is_success = false;
        player_.sender()->SendCmdByMaster(proto.friend_roleid(), re_pack);
        return 0;
    }
    
    PunchCardByMonthDay(now, day_of_month);
    player_.sender()->SendCmd(packet);

    // 告诉好友结果
    player_.sender()->SendCmdByMaster(proto.friend_roleid(), re_pack);
    return 0;
}

int PlayerPunchCard::MSGHandler_RePunchCardHelpError(const MSG& msg)
{
    if (re_punch_count_ < CalcRePunchCount())
    {
        ++re_punch_count_;
        SendQueryPunchCardDataRe();
    }
    return 0;
}

} // namespace gamed
