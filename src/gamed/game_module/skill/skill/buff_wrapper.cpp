#include "buff_wrapper.h"
#include "obj_interface.h"
#include "active_point.h"
#include "skill_wrapper.h"
#include "cooldown_wrapper.h"
#include "skill_templ_manager.h"
#include "skill_templ.h"
#include "effect_templ.h"
#include "skill_def.h"
#include "attack_util.h"
#include "player_util.h"
#include "util.h"

namespace skill
{

#define FOR_EACH(func, msg) \
	for (BuffVec::iterator bit = buff_vec_.begin(); bit != buff_vec_.end();) \
	{ \
		Buff& buff = *bit; \
		buff.func(msg); \
		if (buff.IsTimeout()) \
		{ \
			buff.Detach(msg); \
			bit = buff_vec_.erase(bit); \
			continue; \
		} \
		++bit; \
	} \

void BuffWrapper::Show()
{
	BuffVec::const_iterator vit = buff_vec_.begin();
	for (; vit != buff_vec_.end(); ++vit)
	{
		const Buff& buff = *vit;
		int32_t buffid = buff.effect_->templ_id;
		int64_t duration = buff.duration_;
		int32_t overlap = buff.overlap_;
		int32_t num = buff.num_;
		int32_t hp = buff.hp_;
		int64_t attacher = buff.caster_->GetId();
		__PRINTF("buffid=%d attacher=%ld duration=%ld overlap=%d num=%d hp=%d", 
				buffid, attacher, duration, overlap, num, hp);
	}
}

BuffWrapper::BuffWrapper(Player* player)
	: player_(player)
{
}

void BuffWrapper::ClearBuff()
{
	InnerMsg msg;
	BuffVec::iterator bit = buff_vec_.begin();
	for (; bit != buff_vec_.end(); ++bit)
	{
		bit->Detach(msg);
	}
	buff_vec_.clear();
}

void BuffWrapper::ClearBuff(const EffectTempl* effect, Player* caster)
{
	InnerMsg msg;
	BuffVec::iterator bit = buff_vec_.begin();
	for (; bit != buff_vec_.end();)
	{
		Buff& buff = *bit;
		if (buff.effect_ != effect || buff.caster_ != caster)
		{
			++bit;
			continue;
		}
		buff.Detach(msg);
		bit = buff_vec_.erase(bit);
	}
}

void BuffWrapper::GetTarget(InnerMsg& msg)
{
	FOR_EACH(GetTarget, msg)
}

void BuffWrapper::ConsumeRevise(InnerMsg& msg)
{
	FOR_EACH(ConsumeRevise, msg)
}

void BuffWrapper::CastRevise(InnerMsg& msg)
{
	FOR_EACH(CastRevise, msg)
}

void BuffWrapper::CombatStart(SkillDamageVec& skilldmg_vec)
{
	SkillWrapper* skill = player_->GetSkill();
	skill->CastPassiveSkill(skilldmg_vec);
}

void BuffWrapper::CombatEnd()
{
	ClearBuff();
}

void BuffWrapper::RoundStart(BuffDamageVec& buffdmg_vec)
{
	InnerMsg msg(&buffdmg_vec, NULL);
	FOR_EACH(RoundStart, msg)
	AttackUtil::Attack(player_, buffdmg_vec);
	CooldownWrapper* cooldown = player_->GetCooldown();
    cooldown->HeartBeat();
}

void BuffWrapper::RoundEnd(SkillDamageVec& skilldmg_vec, BuffDamageVec& buffdmg_vec)
{
	if (PlayerUtil::CanAction(player_))
	{
		SkillWrapper* skill = player_->GetSkill();
		skill->CastPassiveSkill(skilldmg_vec);
	}

	InnerMsg msg(&buffdmg_vec, NULL);
    //FOR_EACH(RoundEnd, msg)
	for (BuffVec::iterator bit = buff_vec_.begin(); bit != buff_vec_.end();)
	{
		Buff& buff = *bit;
		buff.RoundEnd(msg);
		if (buff.IsTimeout())
		{
			buff.Detach(msg);
			bit = buff_vec_.erase(bit);
			continue;
		}
		++bit;
	}
	AttackUtil::Attack(player_, buffdmg_vec);
}

void BuffWrapper::Dying(SkillDamageVec& skilldmg_vec, BuffDamageVec& buffdmg_vec)
{
	// 清除自己身上的Buff效果
	ClearBuff();

	// 获取自己造成的被动效果
	EffectTemplVec passive_buff;
	SkillWrapper* skill = player_->GetSkill();
	skill->GetPassiveBuff(passive_buff);
	// 去除其他玩家身上的自己造成的被动效果
	PlayerVec all;
	PlayerUtil::GetAllPlayers(player_, all);
	PlayerVec::iterator it = all.begin();
	for (; it != all.end(); ++it)
	{
		if (!PlayerUtil::CanAction(*it) || (*it == player_))
		{
			continue;
		}

		BuffWrapper* buff = (*it)->GetBuff();
		EffectTemplVec::const_iterator pit = passive_buff.begin();
		for (; pit != passive_buff.end(); ++pit)
		{
			buff->ClearBuff(*pit, player_);
		}
	}

	// 所有人重新释放一次被动效果
	Revive(skilldmg_vec);
}

void BuffWrapper::Dead(BuffDamageVec& buffdmg_vec)
{
	// TODO
}

void BuffWrapper::Revive(SkillDamageVec& skilldmg_vec)
{
	PlayerVec all;
	PlayerUtil::GetAllPlayers(player_, all);
	PlayerVec::iterator it = all.begin();
	for (; it != all.end(); ++it)
	{
		if (!PlayerUtil::CanAction(*it))
		{
			continue;
		}
		SkillWrapper* skill = (*it)->GetSkill();
		skill->CastPassiveSkill(skilldmg_vec);
	}
}

int32_t BuffWrapper::CastSkill(SkillDamage& skill_dmg, BuffDamageVec& buffdmg_vec)
{
	int32_t status = RECAST_NONE;
	status |= AttackUtil::Attack(player_, skill_dmg, buffdmg_vec);
	AttackUtil::Attack(player_, buffdmg_vec);	

	// 检查是否进行追击
	//const SkillTempl* skill = player_->GetSkill()->GetSkill(skill_dmg.skillid);
	const SkillTempl* skill = GetSkillTempl(skill_dmg.skillid);
	if ((skill->recast.type & status) == RECAST_NONE)
	{
		return 0;
	}
	int32_t rand = Util::Rand();
	return rand <= skill->recast.prob ? skill->recast.skillid : 0;
}

static void UpdateBuff(Buff& buff, InnerMsg& msg)
{
    bool new_buff = false;
	const BuffInfo& info = buff.effect_->buff;
	buff.duration_ = buff.owner_->GetCurTime() + info.time;
	buff.num_ = info.num;
	if (buff.overlap_ < info.overlap)
	{
		if (buff.overlap_ == 0)
		{
			buff.Attach(msg);
            new_buff = true;
		}
		else
		{
			buff.Enhance(msg);
		}
		++buff.overlap_;
	}
    if (!new_buff)
    {
        buff.owner_->AttachBuffer(buff.sn_, buff.effect_->templ_id, buff.caster_->GetId());
    }

	InnerMsg init_msg(NULL, &buff);
	ActivePoint point(buff);
	point.Init(init_msg);
	if (buff.hp_ <= 0)
	{
		buff.hp_ = 1;
	}
}

static bool Replace(InnerMsg& msg, Buff& buff, bool& insert)
{
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	const EffectTempl* effect = param->effect;
	Player* attacker = param->attacker;

	const MutexInfo& new_mutex = effect->mutex;
	const MutexInfo& old_mutex = buff.effect_->mutex;
	if (old_mutex.gid == MUTEX_GID_INDEPENDENT || old_mutex.gid != new_mutex.gid)
	{
		return false;
	}

	insert = new_mutex.prior >= old_mutex.prior;
	if (new_mutex.prior == old_mutex.prior)
	{
		if (buff.caster_ != attacker)
		{
			return new_mutex.mutex;
		}
		if (buff.effect_ == effect)
		{
			UpdateBuff(buff, msg);
		    insert = false;
		}
	}
	return insert;
}

static void InsertNewBuff(BuffVec& buff_vec, Buff& buff, InnerMsg& msg)
{
	const BuffInfo& info = buff.effect_->buff;
	buff.duration_ = buff.owner_->GetCurTime() + info.time;
	buff.num_ = info.num;

    int32_t overlap = 0;
    BuffVec::iterator same_buff = buff_vec.end();
    BuffVec::iterator bit = buff_vec.begin();
    for (; bit != buff_vec.end(); ++bit)
    {
        if ((*bit).effect_ == buff.effect_)
        {
            ++overlap;
            if (same_buff == buff_vec.end())
            {
                same_buff = bit;
            }
        }
    }
    if (overlap == info.overlap)
    {
		(*same_buff).Detach(msg);
        buff_vec.erase(same_buff);
    }

	InnerMsg init_msg(NULL, &buff);
	ActivePoint point(buff);
	point.Init(init_msg);
	if (buff.hp_ <= 0)
	{
		buff.hp_ = 1;
	}
    buff.Attach(msg);
    buff_vec.push_back(buff);
}

void BuffWrapper::AttachBuff(InnerMsg& msg)
{
	// 濒死或者死亡的玩家将不再添加Buff
	if (!PlayerUtil::CanAction(player_))
	{
		return;
	}

	bool insert = true;
	BuffVec::iterator bit = buff_vec_.begin();
	for (; bit != buff_vec_.end();)
	{
		if (!Replace(msg, *bit, insert))
		{
			++bit;
			continue;
		}
		bit->Detach(msg);
		bit = buff_vec_.erase(bit);
	}

	if (insert)
	{
		AttackedParam* param = static_cast<AttackedParam*>(msg.param);
		Buff buff(param->attacker, player_, param->effect);
        InsertNewBuff(buff_vec_, buff, msg);
		//UpdateBuff(buff, msg);
		//buff_vec_.push_back(buff);
	}
}

void BuffWrapper::AttackedRevise(InnerMsg& msg)
{
	ReviseParam revise_param;
	InnerMsg revise_msg(msg.buffdmg_vec, &revise_param);
	FOR_EACH(AttackedRevise, revise_msg)
	if (revise_param.scale == 0 && revise_param.point == 0)
	{
		return;
	}
	int32_t scale = 1 + revise_param.scale / 10000.0;
	if (scale < 0)
	{
		scale = 0;
	}
	int32_t point = -1 * revise_param.point;

	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	DamageVec::iterator dit = param->dmg_vec->begin();
	for (; dit != param->dmg_vec->end(); ++dit)
	{
		if (dit->prop != PROP_INDEX_HP || dit->value >= 0)
		{
			continue;
		}
		dit->value = dit->value * scale + point;
		if (dit->value >= 0)
		{
			dit->value = -1;
		}
	}
}

static int32_t DoDamage(Player* attacker, Player* defender, DamageVec* dmg_vec)
{
	int32_t status = RECAST_NONE;
	DamageVec::iterator dit = dmg_vec->begin();
	for (; dit != dmg_vec->end();)
	{
		if (dit->type != DAMAGE_SHIELD)
		{
			status |= PlayerUtil::ChangeProp(attacker, defender, *dit);
		}
		if (dit->type == DAMAGE_SCALE)
		{
			dit = dmg_vec->erase(dit);
		}
		else
		{
			++dit;
		}
	}
	return status;
}

int8_t BuffWrapper::BeAttacked(InnerMsg& msg)
{
	int32_t status = RECAST_NONE;
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	if (param->effect->type == EFFECT_BUFF && param->dmg_vec->empty())
	{
		AttachBuff(msg);
		return status;
	}

	// 净化
	ActivePoint point(param->attacker, player_, param->effect);
	point.Purify(msg);
	// 伤害修正
	AttackedRevise(msg);
	// 吸收护盾
	FOR_EACH(NormalShield, msg)
	// 反伤护盾
	FOR_EACH(ReboundShield, msg)
	// 生命链接
	FOR_EACH(LifeChain, msg)
	// 属性修改
	status |= DoDamage(param->attacker, player_, param->dmg_vec);
	// 受击
    // 现在伤害应该正确，但是客户端表现没法跟受击配合上
    //point.BeAttacked(msg);
	return status;
}

bool BuffWrapper::InLifeChain(InnerMsg& msg) const
{
	AttackedParam* param = static_cast<AttackedParam*>(msg.param);
	const EffectTempl* effect = param->buff->effect_;
	Player* trigger = param->buff->owner_;

	BuffVec::const_iterator bit = buff_vec_.begin();
	for (; bit != buff_vec_.end(); ++bit)
	{
        // 魔偶放的生命链接等同于主人
		if (bit->effect_ == effect && bit->caster_->GetOwner() == trigger->GetOwner())
		{
			return true;
		}
	}
	return false;
}

int32_t BuffWrapper::GetChargedSkill() const
{
    int32_t skillid = 0;
	BuffVec::const_iterator bit = buff_vec_.begin();
	for (; bit != buff_vec_.end(); ++bit)
	{
		if (bit->duration_ == 1 && (skillid = bit->effect_->charged_skillid()) != 0)
		{
            break;
		}
	}
    return skillid;
}

int32_t BuffWrapper::GetGolemId() const
{
    int32_t golemid = 0;
	BuffVec::const_iterator bit = buff_vec_.begin();
	for (; bit != buff_vec_.end(); ++bit)
	{
		if ((golemid = bit->effect_->summon_golemid()) != 0)
		{
            break;
		}
	}
    return golemid;
}

} // namespace skill
