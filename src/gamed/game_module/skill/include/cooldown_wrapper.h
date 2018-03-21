#ifndef SKILL_COOLDOWN_WRAPPER_H_
#define SKILL_COOLDOWN_WRAPPER_H_

#include "skill_types.h"

namespace skill
{

class SkillTempl;

// 冷却信息
struct Cooldown
{
	Cooldown();

	CooldownGID gid;
	int32_t cooltime;
	int64_t duration;
};
typedef std::map<const SkillTempl*, Cooldown> CooldownMap;

class CooldownWrapper
{
	friend class SkillWrapper;
public:
	CooldownWrapper(Player* player);

	int64_t GetCooldown(SkillID id) const;
	int64_t GetCooldown(const SkillTempl* skill) const;
	bool IsCooldown(SkillID id) const;
	bool IsCooldown(const SkillTempl* skill) const;
	void HeartBeat();
    void GetCooldownInfo(std::map<int32_t, int32_t>& cooldown_info) const;

	inline void AddResetGID(SkillGID gid);
private:
	void SetCooldown(const SkillTempl* skill);
	void OnSetCooldown(const SkillTempl* skill);
	void ResetCooldown(const SkillTempl* skill);
	void ResetAll();
	void ResetSkill(const SkillTempl* skill);
	void ResetGroup(SkillGID gid);
private:
	Player* player_;
	CooldownMap cooldown_;
	SkillGroupSet reset_gid_;
};

inline void CooldownWrapper::AddResetGID(SkillGID gid)
{
	reset_gid_.insert(gid);
}

} // namespace skill

#endif // SKILL_SERVER_COOLDOWN_WRAPPER_H_
