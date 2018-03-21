#include "monster_package_templ.h"

namespace task
{

using namespace std;

INIT_STATIC_SYS_TEMPL(MonsterPackageTempl, TEMPL_TYPE_MONSTER_PACKAGE);

void MonsterPackageTempl::OnMarshal()
{
	MARSHAL_SYS_TEMPL_VALUE(name, monster_list);
}

void MonsterPackageTempl::OnUnmarshal()
{
	UNMARSHAL_SYS_TEMPL_VALUE(name, monster_list);
}

// 检测数据有效性
bool MonsterPackageTempl::OnCheckDataValidity() const
{
	set<int32_t>::const_iterator it = monster_list.begin();
	for (; it != monster_list.end(); ++it)
	{
		if (*it <= 0)
		{
			return false;
		}
	}
	return true;
}

} // namespace task
