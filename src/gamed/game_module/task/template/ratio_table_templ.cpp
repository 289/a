#include "ratio_table_templ.h"

namespace task
{

INIT_STATIC_SYS_TEMPL(RatioTableTempl, TEMPL_TYPE_RATIO_TABLE);

void RatioTableTempl::OnMarshal()
{
	MARSHAL_SYS_TEMPL_VALUE(ratio_table);
}

void RatioTableTempl::OnUnmarshal()
{
	UNMARSHAL_SYS_TEMPL_VALUE(ratio_table);
}

// 检测数据有效性
bool RatioTableTempl::OnCheckDataValidity() const
{
	RatioMap::const_iterator it = ratio_table.begin();
	for (; it != ratio_table.end(); ++it)
	{
		if (it->first <= 0 || it->second <= 0)
		{
			return false;
		}
	}
	return true;
}

int32_t RatioTableTempl::GetValue(int32_t level) const
{
	RatioMap::const_iterator it = ratio_table.find(level);
	return it == ratio_table.end() ? 0 : it->second;
}

} // namespace task
