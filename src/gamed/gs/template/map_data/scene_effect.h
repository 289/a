#ifndef GAMED_GS_TEMPLATE_MAPDATA_SCENE_EFFECT_H_
#define GAMED_GS_TEMPLATE_MAPDATA_SCENE_EFFECT_H_

#include "base_mapdata.h"


namespace mapDataSvr {

/**
 * @brief 要在base_mapdata.cpp中添加INIT语句才能生效（Clone生效）
 */
class SceneEffect : public BaseMapData
{
	DECLARE_MAPDATA(SceneEffect, MAPDATA_TYPE_SCENE_EFFECT);
public:
	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}

public:
// 0
	Coordinate   coord;


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(coord);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(coord);
	}

	virtual bool OnCheckDataValidity() const
	{
		return true;
	}
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_SCENE_EFFECT_H_
