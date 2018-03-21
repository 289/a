#ifndef SKILL_DAMAGE_MSG_H_
#define SKILL_DAMAGE_MSG_H_

#include "skill_types.h"

namespace skill
{

struct Damage
{
	Damage()
		: type(DAMAGE_POINT), prop(PROP_INDEX_HP), value(0)
	{
	}
	Damage(int8_t dmg_type, int8_t dmg_prop, int32_t dmg_value)
		: type(dmg_type), prop(dmg_prop), value(dmg_value)
	{
	}

	int8_t type;
	int8_t prop;
	int32_t value;
};
typedef std::vector<Damage> DamageVec;

struct BuffDamage
{
	BuffDamage() 
		: effectid(0), attacker(0), defender(0), buff_sn(0)
	{
	}
	BuffDamage(EffectID id, RoleID attackerid, RoleID defenderid)
		: effectid(id), attacker(attackerid), defender(defenderid), buff_sn(0)
	{
	}

	EffectID effectid;
	RoleID attacker;
	RoleID defender;
	int32_t buff_sn;
	DamageVec dmgs;
};
typedef std::vector<BuffDamage> BuffDamageVec;

struct EffectDamage
{
	EffectDamage()
		: effectid(0), status(0)
	{
	}
	EffectDamage(EffectID id, int8_t estatus)
		: effectid(id), status(estatus)
	{
	}

	EffectID effectid;
	int8_t status;
	DamageVec dmgs;
};
typedef std::vector<EffectDamage> EffectDamageVec;

struct PlayerDamage
{
	PlayerDamage()
		: defender(0)
	{
	}

	PlayerDamage(RoleID roleid)
		: defender(roleid)
	{
	}

	RoleID defender;
	EffectDamageVec dmgs;
};
typedef std::vector<PlayerDamage> PlayerDamageVec;

struct FrameDamage
{
	PlayerDamageVec players;
	PlayerDamageVec redir_players;
};
typedef std::vector<FrameDamage> FrameDamageVec;

struct SkillDamage
{
	SkillDamage() 
		: skillid(0), attacker(0), cast_pos(POS_INVALID)
	{
	}
	SkillDamage(SkillID id, RoleID attackerid)
		: skillid(id), attacker(attackerid), cast_pos(POS_INVALID)
	{
	}

	SkillID skillid;
	RoleID attacker;
	int8_t cast_pos;
	FrameDamageVec frames;
};
typedef std::vector<SkillDamage> SkillDamageVec;

} // namespace skill

#endif // SKILL_DAMAGE_MSG_H_
