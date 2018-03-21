#ifndef GAMED_GS_TEMPLATE_EXTRATEMPL_MAPELEM_CTRL_H_
#define GAMED_GS_TEMPLATE_EXTRATEMPL_MAPELEM_CTRL_H_

#include "base_extratempl.h"


namespace extraTempl {

class MapElemCtrl : public BaseExtraTempl
{
	DECLARE_EXTRATEMPLATE(MapElemCtrl, TEMPL_TYPE_MAPELEM_CTRL_TEMPL);
public:
	static const int kMaxMapElemCount = 16; // 最多支持开启16个地图元素

	inline void set_templ_id(TemplID id)     { templ_id = id; }
	virtual std::string TemplateName() const { return "mapelem_ctrl"; }


public:
// 0
    TimeSegment      time_seg; // 开启时间段
	BoundArray<int32_t, kMaxMapElemCount> map_elem_list; // 地图元素列表


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(time_seg, map_elem_list);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(time_seg, map_elem_list);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (time_seg.is_valid && !CheckTimeSegment(time_seg))
			return false;

		if (map_elem_list.size() <= 0)
			return false;

		return true;
	}
};

} // namespace extraTempl

#endif // GAMED_GS_TEMPLATE_EXTRATEMPL_MAPELEM_CTRL_H_
