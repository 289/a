#ifndef SKILL_DATATEMPL_COOLDOWN_INFO_H_
#define SKILL_DATATEMPL_COOLDOWN_INFO_H_

#include "skill_types.h"

namespace skill
{

#define COOLDOWN_GID_INDEPENDENT -1

class CooldownInfo
{
public:
	CooldownInfo()
		: gid(COOLDOWN_GID_INDEPENDENT), time(0), reset(false)
	{
	}

	inline bool CheckDataValidity() const;
	CooldownGID gid; // 冷却组ID由技能编辑器生成
	int32_t time;
	bool reset;
	NESTED_DEFINE(gid, time, reset);
};

inline bool CooldownInfo::CheckDataValidity() const
{
	return gid >= COOLDOWN_GID_INDEPENDENT && time >= 0;
}

} // namespace skill

#endif // SKILL_DATATEMPL_COOLDOWN_INFO_H_
