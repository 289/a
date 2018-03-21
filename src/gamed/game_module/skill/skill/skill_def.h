#ifndef SKILL_SKILL_DEF_H_
#define SKILL_SKILL_DEF_H_

#include "damage_msg.h"

namespace skill
{

class EffectTempl;
class Buff;

enum
{
	BUFF_NOTINSERT = 0x01,
	BUFF_ERASE = 0x10,
};

class InnerMsg
{
public:
	InnerMsg()
		: buffdmg_vec(NULL)
	{
	}

	InnerMsg(BuffDamageVec* dmg_vec, void* p)
		: buffdmg_vec(dmg_vec), param(p)
	{
	}

	BuffDamageVec* buffdmg_vec;
	void* param;
};

class ReviseParam
{
public:
	ReviseParam()
		: scale(0), point(0)
	{
	}

	ReviseParam(int32_t s, int32_t p)
		: scale(s), point(p)
	{
	}

	int32_t scale;
	int32_t point;
};

class CastParam
{
public:
	CastParam()
		: factor(0), defenders(0), crit(false), dmg(NULL)
	{
	}

	float factor;
	int32_t defenders;
	bool crit;
	EffectDamage* dmg;
	ReviseParam revise[3];
};

class AttackedParam
{
public:
	AttackedParam()
		: attacker(NULL), effect(NULL), dmg_vec(NULL), buff(NULL)
	{
	}

	AttackedParam(Player* player, const EffectTempl* e = NULL, DamageVec* dmg = NULL)
		: attacker(player), effect(e), dmg_vec(dmg), buff(NULL)
	{
	}

	Player* attacker;
	const EffectTempl* effect;
	DamageVec* dmg_vec;
	Buff* buff;
};

} // namespace skill

#endif // SKILL_SKILL_DEF_H_
