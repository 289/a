#include "active_point.h"
#include "effect_logic.h"
#include "effect_templ.h"
#include "logic_if.h"
#include "buff.h"

namespace skill
{

#define FOR_EACH(func) \
	int32_t active = 0; \
	const LogicVec& logic_vec = effect_->logic; \
	LogicVec::const_iterator vit = logic_vec.begin(); \
	for (; vit != logic_vec.end(); ++vit) \
	{ \
		const EffectLogic& logic = *vit; \
		LogicIf* logic_if = LogicIf::GetIf(this, &logic); \
        if (logic_if != NULL) \
        { \
		    active |= (logic_if->func)(msg); \
        } \
	} \
	return (bool)active; \

ActivePoint::ActivePoint(Player* caster, Player* trigger, const EffectTempl* effect)
	: overlap_(1), caster_(caster), trigger_(trigger), effect_(effect)
{
}

ActivePoint::ActivePoint(const Buff& buff)
	: overlap_(buff.overlap_), caster_(buff.caster_), trigger_(buff.owner_), effect_(buff.effect_)
{
}

bool ActivePoint::Init(InnerMsg& msg) const
{
	FOR_EACH(Init)
}

bool ActivePoint::GetTarget(InnerMsg& msg) const
{
	FOR_EACH(GetTarget)
}

bool ActivePoint::ConsumeRevise(InnerMsg& msg) const
{
	FOR_EACH(ConsumeRevise)
}

bool ActivePoint::CastRevise(InnerMsg& msg) const
{
	FOR_EACH(CastRevise)
}

bool ActivePoint::AttackedRevise(InnerMsg& msg) const
{
	FOR_EACH(AttackedRevise)
}

bool ActivePoint::Cast(InnerMsg& msg) const
{
	// Buff类效果释放时不产生Damage，在Attach时才生效
	if (effect_->type == EFFECT_BUFF)
	{
		return true;
	}
	FOR_EACH(Cast)
}

bool ActivePoint::RoundStart(InnerMsg& msg) const
{
	FOR_EACH(RoundStart)
}

bool ActivePoint::RoundEnd(InnerMsg& msg) const
{
	FOR_EACH(RoundEnd)
}

bool ActivePoint::Purify(InnerMsg& msg)
{
	FOR_EACH(Purify)
}

bool ActivePoint::NormalShield(InnerMsg& msg)
{
	FOR_EACH(NormalShield)
}

bool ActivePoint::ReboundShield(InnerMsg& msg)
{
	FOR_EACH(ReboundShield)
}

bool ActivePoint::LifeChain(InnerMsg& msg)
{
	FOR_EACH(LifeChain)
}

bool ActivePoint::Attach(InnerMsg& msg)
{
	FOR_EACH(Attach)
}

bool ActivePoint::Enhance(InnerMsg& msg)
{
	FOR_EACH(Enhance)
}

bool ActivePoint::Detach(InnerMsg& msg)
{
	FOR_EACH(Detach)
}

bool ActivePoint::BeAttacked(InnerMsg& msg) const
{
	FOR_EACH(BeAttacked)
}

bool ActivePoint::WorldBuffChangeProp(InnerMsg& msg)
{
	FOR_EACH(Cast)
}

} // namespace skill
