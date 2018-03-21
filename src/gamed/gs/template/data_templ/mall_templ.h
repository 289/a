#ifndef GAMED_GS_TEMPLATE_DATATEMPL_MALL_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_MALL_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

class MallTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(MallTempl, TEMPL_TYPE_MALL);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxSpecClassName = 64;  // 商城特殊分类的名称
	static const int kMaxMallClassCount = 20; // 商城分类的最大个数

	BoundArray<uint8_t, kMaxSpecClassName> spec_class_name;
	BoundArray<TemplID, kMaxMallClassCount> mall_class_list;

	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(spec_class_name, mall_class_list);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(spec_class_name, mall_class_list);
	}

	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < mall_class_list.size(); ++ i)
			if (mall_class_list[i] < 0)
				return false;
		return true;
	}
};

}; // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_MALL_TEMPL_H_

