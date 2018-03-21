#include "logic_if.h"
#include "effect_logic.h"
#include "active_point.h"
#include "logic_basic_attack.h"
#include "logic_change_prop.h"
#include "logic_control.h"
#include "logic_life_chain.h"
#include "logic_normal_shield.h"
#include "logic_power_flag.h"
#include "logic_power_golem.h"
#include "logic_purify.h"
#include "logic_rebound_shield.h"
#include "logic_reset_cooldown.h"
#include "logic_revise.h"
#include "logic_revive.h"
#include "logic_summon_golem.h"
#include "logic_taunt.h"
#include "logic_trigger_effect.h"
#include "logic_charge.h"

namespace skill
{

LogicIf::LogicIfMap LogicIf::s_logic_map_;

void LogicIf::Init()
{
	static LogicBasicAttack s_attack;
	static LogicChangeProp s_change;
	static LogicControl s_control;
	static LogicTaunt s_taunt;
	static LogicNormalShield s_normal;
	static LogicReboundShield s_rebound;
	static LogicSummonGolem s_golem;
	static LogicPowerFlag s_flag;
	static LogicPowerGolem s_power;
	static LogicResetCooldown s_reset;
	static LogicPurify s_purify;
	static LogicLifeChain s_chain;
	static LogicRevive s_revive;
	static LogicRevise s_revise;
	static LogicTriggerEffect s_trigger;
	static LogicCharge s_charge;

	s_logic_map_[LOGIC_BASIC_ATTACK] = &(s_attack);
	s_logic_map_[LOGIC_CHANGE_PROP] = &(s_change);
	s_logic_map_[LOGIC_CONTROL] = &(s_control);
	s_logic_map_[LOGIC_TAUNT] = &s_taunt;
	s_logic_map_[LOGIC_NORMAL_SHIELD] = &s_normal;
	s_logic_map_[LOGIC_REBOUND_SHIELD] = &s_rebound;
	s_logic_map_[LOGIC_SUMMON_GOLEM] = &s_golem;
	s_logic_map_[LOGIC_POWER_FLAG] = &s_flag;
	s_logic_map_[LOGIC_POWER_GOLEM] = &s_power;
	s_logic_map_[LOGIC_RESET_CD] = &s_reset;
	s_logic_map_[LOGIC_PURIFY] = &s_purify;
	s_logic_map_[LOGIC_LIFE_CHAIN] = &s_chain;
	s_logic_map_[LOGIC_REVIVE] = &s_revive;
	s_logic_map_[LOGIC_REVISE] = &s_revise;
	s_logic_map_[LOGIC_TRIGGER_EFFECT] = &s_trigger;
	s_logic_map_[LOGIC_CHARGE] = &s_charge;
}

LogicIf* LogicIf::GetIf(const ActivePoint* point, const EffectLogic* logic)
{
	LogicIfMap::iterator it = s_logic_map_.find(logic->type);
	if (it == s_logic_map_.end())
	{
		return NULL;
	}
	LogicIf* logic_if = it->second;
	logic_if->caster_ = point->caster_;
	logic_if->trigger_ = point->trigger_;
	logic_if->effect_ = point->effect_;
	logic_if->overlap_ = point->overlap_;
	logic_if->logic_ = logic;
	return logic_if;
}

bool LogicIf::Init(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::GetTarget(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::ConsumeRevise(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::CastRevise(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::AttackedRevise(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::Cast(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::RoundStart(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::RoundEnd(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::NormalShield(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::ReboundShield(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::LifeChain(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::Dying(InnerMsg& msg) const
{
	return false;
}

bool LogicIf::Purify(InnerMsg& msg)
{
	return false;
}

bool LogicIf::Attach(InnerMsg& msg)
{
	return false;
}

bool LogicIf::Enhance(InnerMsg& msg)
{
	return false;
}

bool LogicIf::Detach(InnerMsg& msg)
{
	return false;
}

bool LogicIf::BeAttacked(InnerMsg& msg) const
{
    return false;
}

} // namespace skill
