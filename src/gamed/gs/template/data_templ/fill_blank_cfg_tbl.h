#ifndef GAMED_GS_TEMPLATE_DATATEMPL_FILL_BLANK_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_FILL_BLANK_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

class FillBlankCfgTblTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(FillBlankCfgTblTempl, TEMPL_TYPE_FILL_BLANK_CFG_TBL);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxResourceLen = 512; // 最大资源路径长度
	static const int kMaxPointCount  = 20;  // 最大点位个数

	struct Point
	{
		float x;
		float y;
		TemplID item_id;
		NESTED_DEFINE(x, y, item_id);
	};

// 0
	BoundArray<uint8_t, kMaxResourceLen> resource_path;
	BoundArray<Point, kMaxPointCount> point_list;
	bool consume_item;//是否扣除物品


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(resource_path, point_list, consume_item);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(resource_path, point_list, consume_item);
	}

	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < point_list.size(); ++ i)
		{
			if (point_list[i].item_id <= 0)
				return false;
		}
		return true;
	}
};

}; // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_FILL_BLANK_TEMPL_H_

