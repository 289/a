#ifndef GAMED_GS_TEMPLATE_EXTRATEMPL_CLIENT_MAP_GLOBAL_H_
#define GAMED_GS_TEMPLATE_EXTRATEMPL_CLIENT_MAP_GLOBAL_H_

#include "base_extratempl.h"


namespace extraTempl {

class ClientMapGlobal : public BaseExtraTempl
{
	DECLARE_EXTRATEMPLATE(ClientMapGlobal, TEMPL_TYPE_CLIENT_MAP_GLOBAL);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }
	virtual std::string TemplateName() const { return "client_map_global"; }


public:
    //0
	std::map<int32_t, int32_t> minimap_map; // 地图区域元素ID -> 小地图ID 映射表


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(minimap_map);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(minimap_map);
	}

	virtual bool OnCheckDataValidity() const
	{
		return true;
	}
};

} // namespace extraTempl

#endif // GAMED_GS_TEMPLATE_EXTRATEMPL_CLIENT_MAP_GLOBAL_H_
