#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_BOSS_CHALLENGE_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_BOSS_CHALLENGE_CONFIG_H_

namespace dataTempl
{

class BossChallengeConfig
{
public:
    static const int32_t kMaxTaskNum            = 3;
    static const int32_t kMaxChallengeIdNum     = 3;
    static const int32_t kMaxChallengeCondNum   = 64;

    struct BossChallengeInfo
    {
        BossChallengeInfo()
            : boss_challenge_id(0), min_level(0), max_level(0)
        {
        }

        int32_t boss_challenge_id;                                      // 挑战组ID
        int32_t min_level;                                              // 可见所需玩家下限
        int32_t max_level;                                              // 可见所需玩家上限
        BoundArray<int32_t, kMaxTaskNum> complete_task;                 // 可见所需完成任务列表
        BoundArray<int32_t, kMaxChallengeIdNum> complete_challenge;     // 可见所需通关挑战组ID

        NESTED_DEFINE(boss_challenge_id, min_level, max_level, complete_task, complete_challenge);

        bool CheckDataValidity() const
        {
            if (boss_challenge_id <= 0 || min_level < 0 || min_level > max_level)
            {
                return false;
            }
            for (size_t i = 0; i < complete_task.size(); ++i)
            {
                if (complete_task[i] <= 0)
                {
                    return false;
                }
            }
            for (size_t i = 0; i < complete_challenge.size(); ++i)
            {
                if (complete_challenge[i] <= 0)
                {
                    return false;
                }
            }
            return true;
        }
    };

    BoundArray<BossChallengeInfo, kMaxChallengeCondNum> challenge_list;  // 所有的界面BOSS挑战列表

    NESTED_DEFINE(challenge_list);

    bool CheckDataValidity() const
    {
        for (size_t i = 0; i < challenge_list.size(); ++i)
        {
            if (!challenge_list[i].CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_BOSS_CHALLENGE_CONFIG_H_
