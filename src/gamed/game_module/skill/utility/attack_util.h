#ifndef SKILL_ATTACK_UTIL_H_
#define SKILL_ATTACK_UTIL_H_

#include "damage_msg.h"

namespace skill
{

class AttackUtil
{
public:
	static void Attack(Player* player, BuffDamageVec& buffdmg_vec);
	static int32_t Attack(Player* player, SkillDamage& skill_dmg, BuffDamageVec& buffdmg_vec);
};

} // namespace skill

#endif // SKILL_ATTACK_UTIL_H_
