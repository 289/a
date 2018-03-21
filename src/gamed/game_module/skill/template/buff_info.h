#ifndef SKILL_DATATEMPL_BUFF_INFO_H_
#define SKILL_DATATEMPL_BUFF_INFO_H_

#include "skill_types.h"

namespace skill
{

class BuffInfo
{
public:
	BuffInfo()
		: type(BUFF_NORMAL), update(UPDATE_ROUNDSTART), time(0), num(0), overlap(0)
	{
	}

	inline bool CheckDataValidity() const;
	int8_t type;
	int8_t update;
	int32_t time;
	int32_t num;
	int32_t overlap;
	NESTED_DEFINE(type, update, time, num, overlap);
};

inline bool BuffInfo::CheckDataValidity() const
{
	CHECK_INRANGE(type, BUFF_NORMAL, BUFF_POINT)
	CHECK_INRANGE(update, UPDATE_ROUNDSTART, UPDATE_ROUNDEND)
	// 常态类型的Buff只在回合开始时结算
	if (type == BUFF_NORMAL && update != UPDATE_ROUNDSTART)
	{
		return false;
	}
	return overlap >= 0;
}

} // namespace skill

#endif // SKILL_DATATEMPL_BUFF_INFO_H_
