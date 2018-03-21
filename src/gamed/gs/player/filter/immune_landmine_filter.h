#ifndef GAMED_GS_PLAYER_FILTER_IMMUNE_LANDMINE_FILTER_H_
#define GAMED_GS_PLAYER_FILTER_IMMUNE_LANDMINE_FILTER_H_

#include "gs/player/filter.h"
#include "gs/player/player.h"


namespace gamed {

/**
 * @class ImmuneLandmineFilter
 * @brief 免疫暗雷区域触发的战斗
 */
class ImmuneLandmineFilter : public Filter
{
	enum
	{
		MASK = FILTER_MASK_BUFF | FILTER_MASK_UNIQUE | FILTER_MASK_SAVE_DB_DATA,
	};

    DECLARE_FILTER(ImmuneLandmineFilter, FILTER_TYPE_IMMUNE_LANDMINE, MASK);

public:
	ImmuneLandmineFilter(Player* player, int32_t effectid, int32_t timeout)
        : Filter(player, FILTER_TYPE_IMMUNE_LANDMINE, MASK, effectid)
	{
        SetTimeout(timeout);
	}

protected:
	virtual void OnAttach()
	{
		pplayer_->SetImmuneLandmine();
	}

	virtual void OnDetach()
	{
		pplayer_->ClrImmuneLandmine();
	}
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_FILTER_IMMUNE_LANDMINE_FILTER_H_
