#ifndef TASK_PATH_INFO_H_
#define TASK_PATH_INFO_H_

#include "basic_info.h"

namespace task
{

// 任务追踪对应的寻径信息
class PathInfo
{
public:
	PathInfo()
		: templ_id(0), action(ACTION_NONE)
	{
	}

	inline bool CheckDataValidity() const;

	Position pos;
	int32_t templ_id;
	int8_t action;

	NESTED_DEFINE(pos, templ_id, action);
};

class PathFinder
{
public:
	inline bool CheckDataValidity() const;
	inline const PathInfo* GetPathInfo(PathType type) const;

	PathInfo deliver;
	PathInfo complete;
	PathInfo award;

	NESTED_DEFINE(deliver, complete, award);
};

inline bool PathInfo::CheckDataValidity() const
{
	CHECK_INRANGE(action, ACTION_NONE, ACTION_UI)
	return templ_id >= 0 && pos.CheckDataValidity();
}

inline bool PathFinder::CheckDataValidity() const
{
	CHECK_VALIDITY(deliver)
	CHECK_VALIDITY(complete)
	CHECK_VALIDITY(award)
	return true;
}

inline const PathInfo* PathFinder::GetPathInfo(PathType type) const
{
	switch (type)
	{
	case PATH_DELIVER:
		return &deliver;
	case PATH_COMPLETE:
		return &complete;
	case PATH_AWARD:
		return &award;
	default:
		return NULL;
	}
}

} // namespace task

#endif // TASK_PATH_INFO_H_
