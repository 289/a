#include "cooldown_wrapper.h"
#include "cooldown_info.h"
#include "obj_interface.h"
#include "skill_wrapper.h"
#include "skill_templ.h"
#include "skill_templ_manager.h"

namespace skill
{

static const SkillGID SKILL_GROUP_ALL = -2;
static const SkillGID SKILL_GROUP_CURRENT = -1;

Cooldown::Cooldown()
	: gid(COOLDOWN_GID_INDEPENDENT), cooltime(0), duration(0)
{
}

CooldownWrapper::CooldownWrapper(Player* player)
	: player_(player)
{
}

int64_t CooldownWrapper::GetCooldown(SkillID id) const
{
	const SkillTempl* skill = player_->GetSkill()->GetSkill(id);
	return GetCooldown(skill);
}

int64_t CooldownWrapper::GetCooldown(const SkillTempl* skill) const
{
	CooldownMap::const_iterator it = cooldown_.find(skill);
	return it == cooldown_.end() ? 0 : it->second.duration;
}

bool CooldownWrapper::IsCooldown(SkillID id) const
{
	return IsCooldown(player_->GetSkill()->GetSkill(id));
}

bool CooldownWrapper::IsCooldown(const SkillTempl* skill) const
{
	return GetCooldown(skill) > player_->GetCurTime();
}

void CooldownWrapper::HeartBeat()
{
	int64_t now = player_->GetCurTime();
	bool combat = player_->IsInCombat();
	CooldownMap::iterator it = cooldown_.begin();
	for (; it != cooldown_.end();)
	{
		Cooldown& info = it->second;
		if (combat)
		{
			--info.duration;
		}

		if (info.duration > now)
		{
			++it;
		}
		else
		{
			cooldown_.erase(it++);
		}
	}
}

void CooldownWrapper::SetCooldown(const SkillTempl* skill)
{
	//HeartBeat();
	OnSetCooldown(skill);
	ResetCooldown(skill);
}

void CooldownWrapper::OnSetCooldown(const SkillTempl* skill)
{
	const CooldownInfo& info = skill->cooldown;
	int32_t cooltime = info.time;
	if (cooltime == 0)
	{
		return;
	}
	bool reset = info.reset;
	CooldownGID gid = info.gid;
	int64_t now = player_->GetCurTime();
	// 冷却更新规则
	if (gid != COOLDOWN_GID_INDEPENDENT)
	{
		CooldownMap::iterator it = cooldown_.begin();
		for (; it != cooldown_.end(); ++it)
		{
			Cooldown& cooldown = it->second;
			if (cooldown.gid == gid)
			{
				cooldown.duration = now;
				cooldown.duration += reset ? cooltime : cooldown.cooltime;
			}
		}
	}

	Cooldown& cooldown = cooldown_[skill];
	cooldown.duration = now + cooltime;
	cooldown.cooltime = cooltime;
	cooldown.gid = gid;
}

void CooldownWrapper::ResetCooldown(const SkillTempl* skill)
{
	SkillGroupSet::const_iterator it = reset_gid_.begin();
	for (; it != reset_gid_.end(); ++it)
	{
		switch (*it)
		{
		case SKILL_GROUP_ALL:
			ResetAll();
			break;
		case SKILL_GROUP_CURRENT:
			ResetSkill(skill);
			break;
		default:
			ResetGroup(*it);
			break;
		}
	}
	reset_gid_.clear();
}

void CooldownWrapper::ResetAll()
{
	cooldown_.clear();
}

void CooldownWrapper::ResetSkill(const SkillTempl* skill)
{
	cooldown_.erase(skill);
}

void CooldownWrapper::ResetGroup(SkillGID gid)
{
	CooldownMap::iterator it = cooldown_.begin();
	for (; it != cooldown_.end();)
	{
		const SkillTempl* skill = it->first;
		if (skill->skill_group.count(gid))
		{
			cooldown_.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

void CooldownWrapper::GetCooldownInfo(std::map<int32_t, int32_t>& cooldown_info) const
{
    CooldownMap::const_iterator it = cooldown_.begin();
    for (; it != cooldown_.end(); ++it)
    {
        cooldown_info[it->first->templ_id] = it->second.duration;
    }
}

} // namespace skill
