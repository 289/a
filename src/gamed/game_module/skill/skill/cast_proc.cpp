#include "cast_proc.h"
#include "damage_msg.h"
#include "obj_interface.h"
#include "effect_templ.h"
#include "skill_templ.h"
#include "active_point.h"
#include "util.h"
#include "redir_util.h"
#include "skill_def.h"

namespace skill
{

typedef std::set<const EffectTempl*> EffectSet;

CastProc::CastProc(Player* attacker, Player* trigger, const AttackFrame* frame, FrameDamage* dmg, InnerMsg* msg)
	: attacker_(attacker), trigger_(trigger), frame_(frame), dmg_(dmg), msg_(msg)
{
}

void CastProc::Cast(const SkillTempl* skill, const PlayerVec& target)
{
	if (CastNormalEffect(skill, target))
	{
		CastRedirEffect();
	}
}

static bool IsEffective(const EffectTempl* effect)
{
	return Util::Rand() <= effect->prob;
}

static bool IsHit(Player* attacker, Player* defender, const SkillTempl* skill)
{
	if (skill->team == TEAM_MATE)
	{
		return true;
	}

	int32_t rate = skill->hit.rate;
	int32_t rand = Util::Rand();
	if (skill->hit.type != HIT_ASSIGN)
	{
        int32_t hit_rate = attacker->GetProperty(PROP_INDEX_MAX_HIT);;
        int32_t dodge_rate = defender->GetProperty(PROP_INDEX_MAX_DODGE);
        __PRINTF("****base_rate=%d attacker_hit=%d defender_dodge=%d", rate, hit_rate, dodge_rate);

		rate += attacker->GetProperty(PROP_INDEX_MAX_HIT);
		rate -= defender->GetProperty(PROP_INDEX_MAX_DODGE);
		rate *= 10;
	}
	return rand <= rate;
}

static bool IsCritHit(Player* attacker, Player* defender)
{
	int32_t crit_hit = attacker->GetProperty(PROP_INDEX_MAX_CRIT_HIT);
	int32_t crit_rate = crit_hit / (1000.0 + defender->GetLevel() * 100) * 10000;
	int32_t rand = Util::Rand();
 	return rand <= crit_rate;
}

bool CastProc::CastNormalEffect(const SkillTempl* skill, const PlayerVec& target)
{
	bool hit = false;
	float calc = skill->range.factor / 10000.0;
	PlayerDamageVec& dmg = dmg_->players;
	CastParam* param = static_cast<CastParam*>(msg_->param);
	param->factor = 1.0f;
	param->defenders = target.size();

	EffectSet life_chain_effect;
	PlayerVec::const_iterator tit = target.begin();
	for (; tit != target.end(); ++tit)
	{
		Player* defender = *tit;
		dmg.push_back(PlayerDamage(defender->GetId()));
		if (!IsHit(attacker_, defender, skill))
		{
			continue;
		}
		param->crit = IsCritHit(attacker_, defender);

		hit = true;
		PlayerDamage& player_dmg = dmg[dmg.size() - 1];

		EffectTemplVec::const_iterator eit = frame_->effect_templ.begin();
		for (; eit != frame_->effect_templ.end(); ++eit)
		{
			const EffectTempl* effect = *eit;
			if (!IsEffective(effect))
			{
				continue;
			}

			if (effect->life_chain())
			{
				life_chain_effect.insert(effect);
			}
			player_dmg.dmgs.push_back(EffectDamage(effect->templ_id, EFFECT_NORMAL));
			param->dmg = &(player_dmg.dmgs[player_dmg.dmgs.size() - 1]);
			ActivePoint point(attacker_, defender, effect);
			point.Cast(*msg_);
		}
		param->factor *= calc;
	}	
	if (!life_chain_effect.empty())
	{
		dmg.push_back(PlayerDamage(attacker_->GetId()));
		PlayerDamage& player_dmg = dmg[dmg.size() - 1];
		EffectSet::const_iterator lit = life_chain_effect.begin();
		for (; lit != life_chain_effect.end(); ++lit)
		{
			player_dmg.dmgs.push_back(EffectDamage((*lit)->templ_id, EFFECT_NORMAL));
		}
	}
	return hit;
}

void CastProc::CastRedirEffect()
{
	PlayerDamageVec& dmg = dmg_->redir_players;
	CastParam* param = static_cast<CastParam*>(msg_->param);
	param->factor = 1.0f;

	EffectSet life_chain_effect;
	EffectTemplVec::const_iterator eit = frame_->redir_templ.begin();
	for (; eit != frame_->redir_templ.end(); ++eit)
	{
		const EffectTempl* effect = *eit;
		if (!IsEffective(effect))
		{
			continue;
		}
		if (effect->life_chain())
		{
			life_chain_effect.insert(effect);
		}

		PlayerVec target;
		const Redirect& redir = effect->redir;
		RedirUtil::GetTarget(attacker_, trigger_, redir, target);
		param->defenders = target.size();
		PlayerVec::const_iterator tit = target.begin();
		for (; tit != target.end(); ++tit)
		{
			Player* defender = *tit;
			//param->crit = IsCritHit(attacker_, defender);
			dmg.push_back(PlayerDamage(defender->GetId()));
			PlayerDamage& player_dmg = dmg[dmg.size() - 1];
			player_dmg.dmgs.push_back(EffectDamage(effect->templ_id, EFFECT_REDIR));
			param->dmg = &(player_dmg.dmgs[player_dmg.dmgs.size() - 1]);
			ActivePoint point(attacker_, defender, effect);
			point.Cast(*msg_);
		}
	}
	if (!life_chain_effect.empty())
	{
		dmg.push_back(PlayerDamage(attacker_->GetId()));
		PlayerDamage& player_dmg = dmg[dmg.size() - 1];
		EffectSet::const_iterator lit = life_chain_effect.begin();
		for (; lit != life_chain_effect.end(); ++lit)
		{
			player_dmg.dmgs.push_back(EffectDamage((*lit)->templ_id, EFFECT_NORMAL));
		}
	}
}

} // namespace skill
