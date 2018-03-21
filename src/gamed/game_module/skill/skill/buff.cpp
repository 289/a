#include "buff.h"
#include "obj_interface.h"
#include "active_point.h"
#include "effect_templ.h"
#include "skill_def.h"

namespace skill
{

#define ACTIVE(func) \
	ActivePoint point(*this); \
	if (point.func(msg)) \
	{ \
		DecActiveNum(); \
	} \

uint32_t Buff::next_sn_ = 0;

Buff::Buff()
	: caster_(NULL), owner_(NULL), effect_(NULL), 
	sn_(0), num_(0), overlap_(0), hp_(0), duration_(0)
{
}


Buff::Buff(Player* attacker, Player* defender, const EffectTempl* effect)
	: caster_(attacker), owner_(defender), effect_(effect), sn_(++next_sn_), 
	num_(0), overlap_(0), hp_(0), duration_(0)
{
}

int32_t Buff::GetAttacherId() const
{
    return caster_->GetId();
}

int32_t Buff::GetTargetId() const
{
    return owner_->GetId();
}

int32_t Buff::GetBuffSeq() const
{
    return sn_;
}

int32_t Buff::GetBuffId() const
{
    return effect_->templ_id;
}

void Buff::Attach(InnerMsg& msg)
{
	owner_->AttachBuffer(sn_, effect_->templ_id, caster_->GetId());

	if (effect_->buff.type != BUFF_NORMAL)
	{
		return;
	}
	ACTIVE(Attach)
}

void Buff::Enhance(InnerMsg& msg)
{
	if (effect_->buff.type != BUFF_NORMAL)
	{
		return;
	}
	ACTIVE(Enhance)
}

void Buff::Detach(InnerMsg& msg)
{
	owner_->DetachBuffer(sn_, effect_->templ_id, caster_->GetId());
	
	if (effect_->buff.type != BUFF_NORMAL)
	{
		return;
	}
	ACTIVE(Detach)
}

void Buff::BeAttacked(InnerMsg& msg)
{
	const BuffInfo& info = effect_->buff;
	if (info.type == BUFF_NORMAL || info.update != UPDATE_BEATTACKED)
	{
		return;
	}
	msg.param = this;
	ACTIVE(BeAttacked)
}

void Buff::GetTarget(InnerMsg& msg)
{
	ACTIVE(GetTarget)
}

void Buff::ConsumeRevise(InnerMsg& msg)
{
	ACTIVE(ConsumeRevise)
}

void Buff::CastRevise(InnerMsg& msg)
{
	ACTIVE(CastRevise)
}

void Buff::AttackedRevise(InnerMsg& msg)
{
	ACTIVE(AttackedRevise)
}

void Buff::NormalShield(InnerMsg& msg)
{
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	param->buff = this;
	ACTIVE(NormalShield)
}

void Buff::ReboundShield(InnerMsg& msg)
{
	ACTIVE(ReboundShield)
}

void Buff::LifeChain(InnerMsg& msg)
{
	ACTIVE(LifeChain)
}

void Buff::RoundStart(InnerMsg& msg)
{
	const BuffInfo& info = effect_->buff;
	if (info.type == BUFF_NORMAL || info.update != UPDATE_ROUNDSTART)
	{
		return;
	}
	msg.param = this;
	ACTIVE(RoundStart)
	DecDuration();
}

void Buff::RoundEnd(InnerMsg& msg)
{
	const BuffInfo& info = effect_->buff;
	if (info.type == BUFF_POINT && info.update == UPDATE_ROUNDSTART)
	{
		return;
	}
	if (info.type == BUFF_POINT && info.update == UPDATE_ROUNDEND)
	{
        msg.param = this;
        ACTIVE(RoundEnd)
	}
	DecDuration();
}

} // namespace skill
