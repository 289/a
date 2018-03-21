#ifndef GAMED_GS_PLAYER_FILTER_INVICIBLE_FILTER_H_
#define GAMED_GS_PLAYER_FILTER_INVICIBLE_FILTER_H_

#include "game_module/skill/include/world_buff.h"
#include "gs/player/filter.h"
#include "gs/player/player.h"


namespace gamed {

/**
 * @class InvincibleFilter
 * @brief 无敌Buff(无敌状态不进入战斗)
 */
class InvincibleFilter : public Filter
{
	enum 
    {
		MASK = FILTER_MASK_BUFF | FILTER_MASK_UNIQUE,
	};

    DECLARE_FILTER(InvincibleFilter, FILTER_TYPE_INVINCIBLE, MASK);

public:
	InvincibleFilter(Player* player, int32_t effectid, int32_t timeout = 0)
        : Filter(player, FILTER_TYPE_INVINCIBLE, MASK, effectid)
	{
        SetTimeout(timeout);

        if (CheckEffectID(effectid))
        {
            skill::InvincibleBuffInfo info;
            if (!skill::GetInvincibleBuffInfo(effect_id_, info))
            {
                // 初始化失败
                SetFilterInit(false);
                return;
            }

            SetTimeout(info.duration);
        }
	}

protected:
	virtual void OnAttach()
	{
		pplayer_->SetImmuneActiveMonster();
		pplayer_->SetImmuneLandmine();
		pplayer_->SetImmuneTeamCombat();
	}

	virtual void OnDetach()
	{
		pplayer_->ClrImmuneActiveMonster();
		pplayer_->ClrImmuneLandmine();
		pplayer_->ClrImmuneTeamCombat();
	}
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_FILTER_INVICIBLE_FILTER_H_
