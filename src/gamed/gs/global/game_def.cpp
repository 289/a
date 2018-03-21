#include "game_def.h"

#include <set>


namespace gamed {

	// check predefine game options
	bool CheckOptionsValidity()
	{
		if (kAggroNpcMaxCombatDisSquare > kMaxCombatDistanceSquare)
			return false;

		// 检查特殊的templ_id不能有重复项
		std::set<int32_t> tid_check_set;
		if (!tid_check_set.insert(kMapEffectsTemplID).second)
			return false;
		if (!tid_check_set.insert(kMapNpcZoneAnchorTemplID).second)
			return false;

        // 决斗的距离应该小于广播战斗的距离 
        if (kMaxPlayerDuelDistance > kBroadCastEnterCombatRange)
            return false;

		return true;
	}

} // namespace gamed
