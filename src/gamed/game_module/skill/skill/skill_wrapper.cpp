#include "skill_wrapper.h"
#include "buff_wrapper.h"
#include "cooldown_wrapper.h"
#include "obj_interface.h"
#include "skill_templ_manager.h"
#include "skill_templ.h"
#include "effect_templ.h"
#include "skill_expr.h"
#include "player_util.h"
#include "target_util.h"
#include "range_util.h"
#include "cast_proc.h"
#include "skill_def.h"

namespace skill
{

using namespace std;

SkillWrapper::SkillWrapper(Player* player)
	: default_skill_(0), player_(player)
{
}

void SkillWrapper::AddSkill(SkillID id)
{
	const SkillTempl* skill = GetSkillTempl(id);
	Scope cur_scope = player_->IsInCombat() ? SCOPE_COMBAT : SCOPE_SCENE;
	if ((skill->scope & cur_scope) == 0)
	{
		return;
	}

	SkillMap& skill_map = skill->type == SKILL_ACTIVE ? active_skill_ : passive_skill_;
	skill_map[id] = skill;
}

const SkillTempl* SkillWrapper::GetSkill(SkillID id) const
{
	SkillMap::const_iterator it = active_skill_.find(id);
	if (it != active_skill_.end())
	{
		return it->second;
	}
	it = passive_skill_.find(id);
	return it == passive_skill_.end() ? NULL : it->second;
}

static bool IsMatch(int32_t own, int8_t op, int32_t needed)
{
	switch (op)
	{
	case OP_LESS:
		return own < needed;
	case OP_EQUAL:
		return own == needed;
	case OP_GREATER:
		return own > needed;
	case OP_GREATER_EQUAL:
		return own >= needed;
	default:
		assert(false);
		return false;
	}
}

static int32_t GetNeeded(Player* player, const string& data, const ReviseParam* param)
{
	SkillExpr expr(data, player);
	double value = expr.Calculate();
	if (param != NULL)
	{
		value = value * (1 + param->scale / 10000.0) + param->point;
	}
	return (int32_t)value > 0 ? (int32_t)value : 0;
}

static int32_t CheckCond(Player* player, const CondVec& cond, bool consume)
{
	ReviseParam param;
	if (consume)
	{
		InnerMsg msg(NULL, &param);
		BuffWrapper* buff = player->GetBuff();
		buff->ConsumeRevise(msg);
	}
	
	CondVec::const_iterator it = cond.begin();
	for (; it != cond.end(); ++it)
	{
		bool item = it->type == COND_ITEM;
		ReviseParam* pparam = item ? NULL : &param;
		int32_t needed = GetNeeded(player, it->value, pparam);
		int32_t own = item ? player->GetItemCount(it->id) : player->GetProperty(it->id);
		if (!IsMatch(own, it->op, needed))
		{
			return item ? ERR_ITEM : ERR_PROP;
		}
	}
	return ERR_SUCCESS;
}

static int32_t OnCanCast(Player* player, const SkillTempl* skill)
{
	CooldownWrapper* cooldown = player->GetCooldown();
	if (cooldown->IsCooldown(skill))
	{
		return ERR_COOLDOWN;
	}

	int32_t ret = ERR_SUCCESS;
    if ((ret = CheckCond(player, skill->premise, false)) != ERR_SUCCESS)
    {
        return ret;
    }
	if (skill->magician == MAGICIAN_PLAYER)
	{
        if ((ret = CheckCond(player, skill->consume, true)) != ERR_SUCCESS)
        {
            return ret;
        }
	}

	BuffWrapper* buff = player->GetBuff();
	int32_t golemid = buff->GetGolemId();
    int32_t summon_golemid = skill->summon_golemid();
    if (golemid == 0 || summon_golemid == 0)
    {
        return ERR_SUCCESS;
    }
    // 同一个魔偶不允许重复出战
    if (summon_golemid == golemid)
    {
        return ERR_GOLEM;
    }
    // 收回时能量耗尽的魔偶需要能量完全回满才能再次出战
    int32_t deactive_power = player->GetGolemDeActivePower(summon_golemid);
    if (deactive_power != 0)
    {
        return ERR_SUCCESS;
    }
    int32_t cur_power = player->GetGolemProp(summon_golemid, PROP_INDEX_CON1);
    int32_t max_power = player->GetGolemProp(summon_golemid, PROP_INDEX_MAX_CON1);
    return cur_power < max_power ? ERR_GOLEM : ERR_SUCCESS;
}

int32_t SkillWrapper::CanCast(SkillID id) const
{
	SkillMap::const_iterator it = active_skill_.find(id);
	assert(it != active_skill_.end());
	return OnCanCast(player_, it->second);
}

static const SkillTempl* GetRealCastSkill(Player* player, SkillID& id)
{
    SkillWrapper* skill = player->GetSkill();
    if (player->IsSilent())
    {
        id = skill->default_skillid();
        return skill->GetSkill(id);
    }
    //else if (!player->IsCharged())
    //{
        //return skill->GetSkill(id);
    //}

    SkillID real_skillid = 0;
	BuffWrapper* buff = player->GetBuff();
	if ((real_skillid = buff->GetChargedSkill()) != 0)
    {
        id = real_skillid;
    }
	const SkillTempl* templ = GetSkillTempl(id);
    if (templ == NULL)
    {
        id = skill->default_skillid();
        templ = skill->GetSkill(id);
    }
    return templ;
}

static bool CanChangeTarget(const SkillTempl* skill)
{
	return skill->team == TEAM_ENEMY && skill->range.type == RANGE_SINGLE;
}

static bool GetBuffTarget(Player* player, PlayerVec& target)
{
	InnerMsg msg(NULL, &target);
	BuffWrapper* buff = player->GetBuff();
	buff->GetTarget(msg);
	// 目前只有嘲讽能改变技能的目标
	// 而战斗中只能存在最多一个嘲讽对象
	assert(target.size() <= 1);
	return !target.empty();
}

static int8_t GetSkillTarget(Player* player, const SkillTempl* skill, PlayerVec& target)
{
	PlayerVec all;
	PlayerUtil::GetMembers(player, skill->team, all);
	int8_t pos = TargetUtil::GetCastPos(player, skill->team, skill->target, all);
	if (pos != POS_INVALID)
	{
		RangeUtil::GetAttackRange(skill, pos, all, target);
	}
	return pos;
}

int8_t SkillWrapper::GetTarget(SkillID& id, PlayerVec& target) const
{
	// 这几个状态下不应该调用技能接口，由战斗保证
	assert(!player_->IsDizzy() && !player_->IsStone() && !player_->IsCharging());

	// 获取本轮实际释放的技能
    const SkillTempl* skill = GetRealCastSkill(player_, id);

	// 检查Buff是否影响技能目标选择
	if (CanChangeTarget(skill) && GetBuffTarget(player_, target))
	{
		return target[0]->GetPos();
	}
	return GetSkillTarget(player_, skill, target);
}

static void Consume(Player* player, const SkillTempl* skill)
{
	if (skill->magician != MAGICIAN_PLAYER)
	{
		return;
	}

	ReviseParam param;
	InnerMsg msg(NULL, &param);
	BuffWrapper* buff = player->GetBuff();
	buff->ConsumeRevise(msg);
	
	CondVec::const_iterator it = skill->consume.begin();
	for (; it != skill->consume.end(); ++it)
	{
		bool item = it->type == COND_ITEM;
		ReviseParam* pparam = item ? NULL : &param;
		int32_t needed = GetNeeded(player, it->value, pparam);
		if (item)
		{
			player->TakeOutItem(it->id, needed);
		}
		else
		{
			__PRINTF("Skill Consume skillid=%d value=%d", skill->templ_id, needed);
			player->DecPropPoint(it->id, needed);
		}
	}
}

void SkillWrapper::CastSkill(SkillID id, const PlayerVec& target, SkillDamage& dmg, bool recast)
{
	const SkillTempl* skill = GetSkill(id);
    if (skill == NULL)
    {
        skill = GetSkillTempl(id); // 处理蓄力技能
    }
	CastSkill(skill, target, dmg);

	if (!recast)
	{
		CooldownWrapper* cooldown = player_->GetCooldown();
		cooldown->SetCooldown(skill);
	}
}

void SkillWrapper::Consume(SkillID id)
{
	const SkillTempl* skill = GetSkill(id);
    // 当skill是由蓄力触发的技能时可能找不到，这时不消耗
    if (skill != NULL) 
    {
        skill::Consume(player_, skill);
    }
}

void SkillWrapper::CastSkill(const SkillTempl* skill, const PlayerVec& target, SkillDamage& dmg)
{
	// 获取Buff技能伤害修正参数
	CastParam param;
	InnerMsg msg(NULL, &param);
	BuffWrapper* buff = player_->GetBuff();
	buff->CastRevise(msg);

	// 计算技能的效果
	dmg.skillid = skill->templ_id;
	dmg.attacker = player_->GetId();
	AttackFrameVec::const_iterator it = skill->attack_frame.begin();
	for (; it != skill->attack_frame.end(); ++it)
	{
		dmg.frames.push_back(FrameDamage());
		FrameDamage* frame_dmg = &(dmg.frames[dmg.frames.size() - 1]);
		CastProc proc(player_, player_, &(*it), frame_dmg, &msg);
		proc.Cast(skill, target);
	}
}

void SkillWrapper::CastPassiveSkill(SkillDamageVec& skilldmg_vec)
{
	BuffDamageVec buff_vec;
	SkillMap::const_iterator it = passive_skill_.begin();
	for (; it != passive_skill_.end(); ++it)
	{
		const SkillTempl* skill = it->second;
		if (OnCanCast(player_, skill) != ERR_SUCCESS)
		{
			continue;
		}

		PlayerVec target;
		int32_t attack_pos = GetSkillTarget(player_, skill, target);

		SkillDamage dmg;
		CastSkill(skill, target, dmg);

		BuffWrapper* buff = player_->GetBuff();
		buff->CastSkill(dmg, buff_vec);

        dmg.cast_pos = attack_pos;
        skilldmg_vec.push_back(dmg);
		// 被动技能与其他效果之间不应该产生作用
		assert(buff_vec.empty());

		CooldownWrapper* cooldown = player_->GetCooldown();
		cooldown->SetCooldown(skill);
	}
}

void SkillWrapper::GetPassiveBuff(EffectTemplVec& buff_vec) const
{
	SkillMap::const_iterator pit = passive_skill_.begin();
	for (; pit != passive_skill_.end(); ++pit)
	{
		const SkillTempl* skill = pit->second;
		const AttackFrameVec& frame_vec = skill->attack_frame;
		AttackFrameVec::const_iterator fit = frame_vec.begin();
		for (; fit != frame_vec.end(); ++fit)
		{
            const EffectTemplVec* templ_vec = NULL;
			for (int32_t i = 0; i <= 1; ++i)
			{
				templ_vec = i == 0 ? &(fit->effect_templ) : &(fit->redir_templ);
				EffectTemplVec::const_iterator tit = templ_vec->begin();
				for (; tit != templ_vec->end(); ++tit)
				{
					if ((*tit)->type == EFFECT_BUFF)
					{
					    buff_vec.push_back(*tit);
					}
				}
			}
		}
	}
}

} // namespace skill
