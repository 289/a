#ifndef SKILL_PLAYER_UTIL_H_
#define SKILL_PLAYER_UTIL_H_

#include "skill_types.h"

namespace skill
{

class Damage;

class PlayerUtil
{
public:
	static bool IsAlive(const Player* player);		// 非死亡，可以濒死
	static bool CanAction(const Player* player);	// 玩家血量必须大于0
	static bool CanAttacked(const Player* player);	// 攻击该玩家不会导致ATB暂停
	static bool IsDead(const Player* player);
	static bool IsDying(const Player* player);

	static int32_t GetHP(const Player* player);
	static int32_t GetMaxHP(const Player* player);

	static int32_t GetAllPlayers(const Player* player, PlayerVec& all);
	static int32_t GetMembers(const Player* player, int8_t team, PlayerVec& members);
	static Player* FindPlayer(const Player* player, int64_t roleid);
	static int32_t GetAlive(const PlayerVec& src, PlayerVec& alive);
	static int32_t GetCanAction(const PlayerVec& src, PlayerVec& action);
	static int32_t GetCanAttacked(const PlayerVec& src, PlayerVec& attacked);
	static int32_t GetDead(const PlayerVec& src, PlayerVec& des);
	static int32_t GetDying(const PlayerVec& src, PlayerVec& des);

	static int32_t ChangeProp(Player* attacker, Player* defender, Damage& dmg);
};

} // namespace skill

#endif // SKILL_PLAYER_UTIL_H_
