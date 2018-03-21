#ifndef GAMED_GS_PLAYER_VALUE_PROPERTY_H_
#define GAMED_GS_PLAYER_VALUE_PROPERTY_H_

#include <stdint.h>


namespace gamed {

/**
 * @brief 记在玩家身上的一些数值属性
 *  （1）简单数值属性不好划分到子系统里的都放在这里
 */
class PlayerValueProp
{
public:
    PlayerValueProp();
    ~PlayerValueProp();

    bool Init();
    void Reset();

    // 喵类视觉
    int32_t GetCatVisionGenSpeed() const;
    int32_t GetCatVisionGenInterval() const;
    void    IncCatVisionGenSpeed(int value);
    void    DecCatVisionGenSpeed(int value);
    void    IncCatVisionGenInterval(int value);
    void    DecCatVisionGenInterval(int value);

    // 濒死时间 
    int32_t GetPlayerDyingTime() const;
    void    IncPlayerDyingTime(int value);
    void    DecPlayerDyingTime(int value);

    // 战斗奖励
    void    IncAwardExpAdjustFactor(int value);
    void    DecAwardExpAdjustFactor(int value);
    double  GetCombatAwardExpAdjustFactor() const;
    void    IncAwardMoneyAdjustFactor(int value);
    void    DecAwardMoneyAdjustFactor(int value);
    double  GetCombatAwardMoneyAdjustFactor() const;


private:
    int32_t    base_cat_vision_gen_speed_;        // 瞄类视觉恢复速度的基础值
    int32_t    base_cat_vision_gen_interval_;     // 瞄类视觉恢复周期的基础值
    int32_t    base_dying_time_milliseconds_;     // 玩家濒死时间的基础值
    int32_t    enh_cat_vision_gen_speed_;         // 瞄类视觉恢复速度的强化值
    int32_t    enh_cat_vision_gen_interval_;      // 瞄类视觉恢复周期的强化值
    int32_t    enh_dying_time_milliseconds_;      // 玩家濒死时间的强化值
    int32_t    combat_award_exp_adjust_factor_;   // 战斗奖励经验值的校正(万分数)
    int32_t    combat_award_money_adjust_factor_; // 战斗奖励金钱值的校正(万分数)
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_VALUE_PROPERTY_H_
