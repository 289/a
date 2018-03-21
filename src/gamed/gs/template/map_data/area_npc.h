#ifndef GAMED_GS_TEMPLATE_MAPDATA_AREA_NPC_H_
#define GAMED_GS_TEMPLATE_MAPDATA_AREA_NPC_H_

#include "base_mapdata.h"


namespace mapDataSvr {

class AreaNpc : public BaseMapData
{
	DECLARE_MAPDATA(AreaNpc, MAPDATA_TYPE_AREA_NPC);
public:
	static const int32_t kMaxNpcCount = 16;

	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}


public:
	BoundArray<Coordinate, kMaxVertexCount> vertexes; // 区域顶点数组
	BoundArray<TemplID, kMaxNpcCount>       associated_templ_vec; // 关联的npc模板id数组，如果没用关联的npc则小工具无需导出


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(vertexes, associated_templ_vec);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(vertexes, associated_templ_vec);
	}

	virtual bool OnCheckDataValidity() const
	{
		// 必须是方形的
		if (vertexes.size() != 4)
			return false;
		// 至少挂一个npc
		if (associated_templ_vec.size() == 0)
			return false;

		return true;
	}
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_AREA_NPC_H_
