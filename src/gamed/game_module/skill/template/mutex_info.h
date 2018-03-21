#ifndef SKILL_DATATEMPL_MUTEX_INFO_H_
#define SKILL_DATATEMPL_MUTEX_INFO_H_

#include "skill_types.h"

namespace skill
{

#define MUTEX_GID_INDEPENDENT -1

class MutexInfo
{
public:
	MutexInfo()
		: gid(MUTEX_GID_INDEPENDENT), prior(0), mutex(false)
	{
	}

	inline bool CheckDataValidity() const;
	MutexGID gid; // 互斥组ID由技能编辑器生成
	int32_t prior;
	bool mutex;
	NESTED_DEFINE(gid, prior, mutex);
};

inline bool MutexInfo::CheckDataValidity() const
{
	return gid >= MUTEX_GID_INDEPENDENT && prior >= 0;
}

} // namespace skill

#endif // SKILL_DATATEMPL_MUTEX_INFO_H_
