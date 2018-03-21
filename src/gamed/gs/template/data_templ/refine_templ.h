#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_REFINE_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_REFINE_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief: 装备精练表模板
 */
class RefineTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(RefineTempl, TEMPL_TYPE_REFINE_CONFIG);

public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	struct refine_entry
	{
		int32_t money;	//精练金币
		int32_t ratio;	//属性加成比例(万分数)
		NESTED_DEFINE(money, ratio);
	};

	std::vector<refine_entry> refine_table;

	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(refine_table);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(refine_table);
	}

	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < refine_table.size(); ++ i)
		{
			const refine_entry& entry = refine_table[i];
			if (entry.money < 0 || entry.ratio < 0)
				return false;
		}
		return true;
	}
};

}; // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_REFINE_TEMPL_H_
