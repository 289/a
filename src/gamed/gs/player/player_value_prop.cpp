#include "player_value_prop.h"

#include "gs/global/gmatrix.h"


namespace gamed {

PlayerValueProp::PlayerValueProp() 
{
    Reset();
}

PlayerValueProp::~PlayerValueProp()
{
    Reset();
}

void PlayerValueProp::Reset()
{
    base_cat_vision_gen_speed_        = 0;
    base_cat_vision_gen_interval_     = 0;
    base_dying_time_milliseconds_     = 0;
    enh_cat_vision_gen_speed_         = 0;
    enh_cat_vision_gen_interval_      = 0;
    enh_dying_time_milliseconds_      = 0;
    combat_award_exp_adjust_factor_   = 0;
    combat_award_money_adjust_factor_ = 0;
}

bool PlayerValueProp::Init()
{
    base_cat_vision_gen_speed_    = Gmatrix::GetCatVisionEPGenSpeed();
    base_cat_vision_gen_interval_ = Gmatrix::GetCatVisionEPGenInterval();
    base_dying_time_milliseconds_ = 3000;
    return true;
}

int32_t PlayerValueProp::GetCatVisionGenSpeed() const
{
    return base_cat_vision_gen_speed_ + enh_cat_vision_gen_speed_;
}

int32_t PlayerValueProp::GetCatVisionGenInterval() const
{
    return base_cat_vision_gen_interval_ + enh_cat_vision_gen_interval_;
}

void PlayerValueProp::IncCatVisionGenSpeed(int value)
{
    if (value <= 0)
        return;

    enh_cat_vision_gen_speed_ += value;
}

void PlayerValueProp::DecCatVisionGenSpeed(int value)
{
    if (value <= 0)
        return;

    enh_cat_vision_gen_speed_ -= value;
}

void PlayerValueProp::IncCatVisionGenInterval(int value)
{
    if (value <= 0)
        return;

    enh_cat_vision_gen_interval_ += value;
}

void PlayerValueProp::DecCatVisionGenInterval(int value)
{
    if (value <= 0)
        return;

    enh_cat_vision_gen_interval_ -= value;
}

int32_t PlayerValueProp::GetPlayerDyingTime() const
{
    return base_dying_time_milliseconds_ + enh_dying_time_milliseconds_;
}

void PlayerValueProp::IncPlayerDyingTime(int value)
{
    if (value <= 0)
        return;

    enh_dying_time_milliseconds_ += value;
}

void PlayerValueProp::DecPlayerDyingTime(int value)
{
    if (value <= 0)
        return;

    enh_dying_time_milliseconds_ -= value;
}

void PlayerValueProp::IncAwardExpAdjustFactor(int value)
{
    combat_award_exp_adjust_factor_ += value;
}

void PlayerValueProp::DecAwardExpAdjustFactor(int value)
{
    combat_award_exp_adjust_factor_ -= value;
}

double PlayerValueProp::GetCombatAwardExpAdjustFactor() const
{
    return (double)(combat_award_exp_adjust_factor_) / 10000.f;
}

void PlayerValueProp::IncAwardMoneyAdjustFactor(int value)
{
    combat_award_money_adjust_factor_ += value;
}

void PlayerValueProp::DecAwardMoneyAdjustFactor(int value)
{
    combat_award_money_adjust_factor_ -= value;
}

double PlayerValueProp::GetCombatAwardMoneyAdjustFactor() const
{
    return (double)(combat_award_money_adjust_factor_) / 10000.f;
}

} // namespace gamed
