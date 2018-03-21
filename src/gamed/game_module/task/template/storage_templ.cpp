#include "storage_templ.h"

namespace task
{

INIT_STATIC_SYS_TEMPL(StorageTempl, TEMPL_TYPE_STORAGE);

void StorageTempl::OnMarshal()
{
	MARSHAL_SYS_TEMPL_VALUE(interval, max_num, refresh, quality, cash);
	MARSHAL_SYS_TEMPL_VALUE(task_list);
}

void StorageTempl::OnUnmarshal()
{
	UNMARSHAL_SYS_TEMPL_VALUE(interval, max_num, refresh, quality, cash);
	UNMARSHAL_SYS_TEMPL_VALUE(task_list);
}

// 检测数据有效性
bool StorageTempl::OnCheckDataValidity() const
{
	if (interval < 0 || max_num < 0 || cash < 0)
	{
		return false;
	}
    CHECK_INRANGE(quality, QUALITY_WHITE, QUALITY_GOLD);
	CHECK_VEC_VALIDITY(task_list)
	return true;
}

} // namespace task
