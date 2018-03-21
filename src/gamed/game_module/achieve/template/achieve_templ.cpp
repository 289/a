#include "achieve_templ.h"

namespace achieve
{

INIT_STATIC_SYS_TEMPL(AchieveTempl, TEMPL_TYPE_ACHIEVE);

void AchieveTempl::OnMarshal()
{
    MARSHAL_SYS_TEMPL_VALUE(flag, point, name, desc, icon);
    MARSHAL_SYS_TEMPL_VALUE(premise, award);
	MARSHAL_SYS_TEMPL_VALUE(parent, next_sibling, first_child);
}

void AchieveTempl::OnUnmarshal()
{
    UNMARSHAL_SYS_TEMPL_VALUE(flag, point, name, desc, icon);
    UNMARSHAL_SYS_TEMPL_VALUE(premise, award);
	UNMARSHAL_SYS_TEMPL_VALUE(parent, next_sibling, first_child);
}

// 检测数据有效性
bool AchieveTempl::OnCheckDataValidity() const
{
	CHECK_VALIDITY(premise)
	CHECK_VALIDITY(award)
	if (point < 0 || parent < 0 || next_sibling < 0 || first_child < 0)
	{
		return false;
	}
    return true;
}

} // namespace achieve
