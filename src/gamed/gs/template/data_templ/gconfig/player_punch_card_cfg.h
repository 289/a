#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_PUNCH_CARD_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_PUNCH_CARD_H_

namespace dataTempl {

/**
 * @brief 玩家签到系统全局配置表
 */
class PlayerPunchCardConfig
{
public:
    static const size_t kMaxHistoryAccumulate = 64; // 历史累计最大级数
    static const size_t kMaxMonthlyAccumulate = 10; // 月度累计最大级数
    static const size_t kMaxAwardItemCount    = 3;

    struct ItemInfo
    {
        int32_t tid;    // 模板id
        int32_t count;  // 发放个数
        NESTED_DEFINE(tid, count);
    };

    struct Entry
    {
        int32_t cumulative_num;  // 累计次数
        int32_t award_exp;       // 奖励经验
        int32_t award_money;     // 奖励游戏币
        int32_t award_score;     // 奖励学分
        BoundArray<ItemInfo, kMaxAwardItemCount> award_item; // 奖励物品
        NESTED_DEFINE(cumulative_num, award_exp, award_money, award_score, award_item);

        bool CheckValidity() const
        {
            for (size_t i = 0; i < award_item.size(); ++i)
            {
                if (award_item[i].tid <= 0 || award_item[i].count <= 0)
                    return false;
            }
            return true;
        }
    };

public:
    BoundArray<Entry, kMaxHistoryAccumulate> history; // 历史累计配置
    BoundArray<Entry, kMaxMonthlyAccumulate> monthly; // 月度累计配置

    NESTED_DEFINE(history, monthly);

    bool CheckDataValidity() const
    {
        for (size_t i = 0; i < history.size(); ++i)
        {
            if (i > 1)
            {
                if (history[i-1].cumulative_num >= history[i].cumulative_num)
                    return false;
            }

            if (!history[i].CheckValidity())
                return false;
        }

        for (size_t i = 0; i < monthly.size(); ++i)
        {
            if (i > 1)
            {
                if (monthly[i-1].cumulative_num >= monthly[i].cumulative_num)
                    return false;
            }

            if (!monthly[i].CheckValidity())
                return false;
        }
        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PLAYER_PUNCH_CARD_H_
