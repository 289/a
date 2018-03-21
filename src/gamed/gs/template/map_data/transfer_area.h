#ifndef GAMED_GS_TEMPLATE_MAPDATA_TRANSFER_AREA_H_
#define GAMED_GS_TEMPLATE_MAPDATA_TRANSFER_AREA_H_

#include "base_mapdata.h"


namespace mapDataSvr {

class TransferArea : public BaseMapData
{
	DECLARE_MAPDATA(TransferArea, MAPDATA_TYPE_TRANSFER_AREA);
public:
	static const int kMaxTransferCoord = 10;
	static const int kMaxTaskCount     = 16;

	struct TransferCoord
	{
		TransferCoord()
			: mapid(0)
		{ }

		MapID       mapid;  // 默认值0表示无效
		Coordinate  pos;

		NESTED_DEFINE(mapid, pos);
	};

	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}


public:
// 0
	BoundArray<Coordinate, kMaxVertexCount>      vertexes; // 区域顶点数组
	BoundArray<TransferCoord, kMaxTransferCoord> transfer_coords; // 传送的坐标组，全局坐标
	int32_t  ins_templ_id; // 副本模板id，如果该项有值，则上面的transfer_coords数据不生效。
	                       // 默认值为0，表示不是传送到副本
	int32_t  level_lower_limit;  // 默认值为0，玩家的等级下限，玩家等级大于等于该值才能传送
	BoundArray<int32_t, kMaxTaskCount> require_tasks; // 检查玩家完成或携带有这些任务才能进行传送，默认空，表示传送没有要求任务


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(vertexes, transfer_coords, ins_templ_id, level_lower_limit, require_tasks);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(vertexes, transfer_coords, ins_templ_id, level_lower_limit, require_tasks);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (vertexes.size() < 3)
			return false;

		// 必须有一个传送坐标
		if (transfer_coords.size() < 1 && ins_templ_id <= 0)
			return false;

		if (ins_templ_id != 0 && transfer_coords.size() != 0)
			return false;

		for (size_t i = 0; i < transfer_coords.size(); ++i)
		{
			int32_t tmp_mapid = transfer_coords[i].mapid;
			if (tmp_mapid <= 0)
				return false;
		}

		if (ins_templ_id < 0)
			return false;

		if (level_lower_limit < 0)
			return false;

		for (size_t i = 0; i < require_tasks.size(); ++i)
		{
			if (require_tasks[i] <= 0)
				return false;
		}

		return true;
	}
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_TRANSFER_AREA_H_
