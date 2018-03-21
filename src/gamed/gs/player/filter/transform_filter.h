#ifndef GAMED_GS_PLAYER_FILTER_TRANSFORM_FILTER_H_
#define GAMED_GS_PLAYER_FILTER_TRANSFORM_FILTER_H_

#include "game_module/skill/include/world_buff.h"
#include "gs/player/filter.h"
#include "gs/player/player.h"


namespace gamed {

/**
 * @class TransformFilter - transform filter world buff
 * @brief 大世界变身
 */
class TransformFilter : public Filter
{
	enum 
    {
		MASK = FILTER_MASK_BUFF | FILTER_MASK_UNIQUE,
	};

    DECLARE_FILTER(TransformFilter, FILTER_TYPE_TRANSFORM, MASK);

public:
	TransformFilter(Player* player, int32_t effectid, int32_t timeout = 0)
        : Filter(player, FILTER_TYPE_TRANSFORM, MASK, effectid)
	{
        SetTimeout(timeout);

        if (CheckEffectID(effectid))
        {
            skill::TransformBuffInfo info;
            if (!skill::GetTransformBuffInfo(effect_id_, info))
            {
                SetFilterInit(false);
                return;
            }

            SetTimeout(info.duration);
        }
	}

protected:
	virtual void OnAttach()
	{
	}

	virtual void OnDetach()
	{
	}
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_FILTER_TRANSFORM_FILTER_H_
