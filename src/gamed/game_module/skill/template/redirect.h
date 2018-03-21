#ifndef SKILL_DATATEMPL_REDIRECT_H_
#define SKILL_DATATEMPL_REDIRECT_H_

#include "skill_types.h"

namespace skill
{

class Redirect
{
public:
	Redirect()
		: type(REDIR_NO), num(0), max_selected(0)
	{
	}

	inline bool CheckDataValidity() const;
	int8_t type;
	int32_t num;
	int32_t max_selected; // type为随机时，单个目标可以被选中的次数上限
	NESTED_DEFINE(type, num, max_selected);
};

inline bool Redirect::CheckDataValidity() const
{
	if (type < REDIR_NO || type > REDIR_TRIGGER)
	{
		return false;
	}
	if (num < 0 || max_selected < 0 || max_selected > num)
	{
		return false;
	}
	return true;
}

} // namespace skill

#endif // SKILL_DATATEMPL_REDIRECT_H_
