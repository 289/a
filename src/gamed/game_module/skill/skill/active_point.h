#ifndef SKILL_ACTIVE_POINT_H_
#define SKILL_ACTIVE_POINT_H_

#include "skill_types.h"

namespace skill
{

class EffectTempl;
class Buff;
class InnerMsg;

class ActivePoint
{
public:
	ActivePoint(Player* caster, Player* trigger, const EffectTempl* effect);
	ActivePoint(const Buff& buff);

	bool Init(InnerMsg& msg) const;
	bool GetTarget(InnerMsg& msg) const;
	bool ConsumeRevise(InnerMsg& msg) const;
	bool CastRevise(InnerMsg& msg) const;
	bool AttackedRevise(InnerMsg& msg) const;
	bool Cast(InnerMsg& msg) const;
	bool RoundStart(InnerMsg& msg) const;
	bool RoundEnd(InnerMsg& msg) const;
	bool BeAttacked(InnerMsg& msg) const;
	bool Purify(InnerMsg& msg);
	bool NormalShield(InnerMsg& msg);
	bool ReboundShield(InnerMsg& msg);
	bool LifeChain(InnerMsg& msg);
	bool Attach(InnerMsg& msg);
	bool Enhance(InnerMsg& msg);
	bool Detach(InnerMsg& msg);
    bool WorldBuffChangeProp(InnerMsg& msg);
public:
	int32_t overlap_;
	Player* caster_;
	Player* trigger_;
	const EffectTempl* effect_;
};

} // namespace skill

#endif // SKILL_ACTIVE_POINT_H_
