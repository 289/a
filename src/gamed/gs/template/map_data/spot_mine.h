#ifndef GAMED_GS_TEMPLATE_MAPDATA_SPOT_MINE_H_
#define GAMED_GS_TEMPLATE_MAPDATA_SPOT_MINE_H_

#include "base_mapdata.h"


namespace mapDataSvr {

class SpotMine : public BaseMapData
{
	DECLARE_MAPDATA(SpotMine, MAPDATA_TYPE_SPOT_MINE);
public:

	// mapid和elemid是必填项
	inline void set_mapdata_info(MapID mapid, ElemID elemid) 
	{ 
		BaseMapData::set_mapdata_info(mapid, elemid); 
	}


public:
// 0
	Coordinate   coord;                // 位置坐标
	TemplID      associated_templ_id;  // 关联的数据模板id
	int16_t      gather_player_num;    // 同时采集人数，默认为1，表示同时只允许一个玩家操作，0表示不限制采集人数
	int8_t       is_gather_disappear;  // 采集后是否消失，默认为1，表示采集后消失，0表示不消失
	int8_t       is_auto_refresh;      // 消失后是否刷新，默认为1，表示消失后刷新，0表示不刷新

// 5
	int16_t      refresh_time;         // 刷新时间，单位为妙，默认值60s，0表示无刷新时间，即时刷新


protected:
	virtual void OnMarshal()
	{
		MARSHAL_MAPDATA(coord, associated_templ_id, gather_player_num, is_gather_disappear, is_auto_refresh);
		MARSHAL_MAPDATA(refresh_time);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_MAPDATA(coord, associated_templ_id, gather_player_num, is_gather_disappear, is_auto_refresh);
		UNMARSHAL_MAPDATA(refresh_time);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (associated_templ_id <= 0)
			return false;

		if (gather_player_num < 0 || gather_player_num > 65530)
			return false;

		if (refresh_time < 0 || refresh_time > 65530)
			return false;

		return true;
	} 
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_SPOT_MINE_H_
