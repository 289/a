#ifndef TASK_ZONE_INFO_H_
#define TASK_ZONE_INFO_H_

#include "task_types.h"
//#include "util.h"

namespace task
{

class ZoneInfo
{
public:
	ZoneInfo()
		: world_id(0), min_x(0), min_y(0), max_x(0), max_y(0)
	{
	}

	inline bool CheckDataValidity() const;
	//inline bool IsInZone(int32_t world_id, float x, float y) const;

	int32_t world_id; // world_id为0表示所有普通地图
	float min_x;
	float min_y;
	float max_x;
	float max_y;

	NESTED_DEFINE(world_id, min_x, min_y, max_x, max_y);
};

enum ZoneOpType
{
	ZONE_ENTER,
	ZONE_LEAVE,
};

class ZoneOpInfo
{
public:
	ZoneOpInfo()
		: op(ZONE_ENTER)
	{
	}

	inline bool CheckDataValidity() const;
	//inline bool Match(int32_t world_id, float x, float y) const;

	int8_t op;
	ZoneInfo zone;

	NESTED_DEFINE(op, zone);
};
typedef std::vector<ZoneOpInfo> ZoneOpVec;

class ItemUseZone
{
public:
	ItemUseZone()
        : item_id(0)
	{
	}

	inline bool CheckDataValidity() const;
	//inline bool Match(int32_t world_id, float x, float y) const;

	ZoneInfo zone;
    int32_t item_id;
    std::string tip;

	NESTED_DEFINE(zone, item_id, tip);
};
typedef std::vector<ItemUseZone> ItemUseZoneVec;

inline bool ZoneInfo::CheckDataValidity() const
{
	return world_id >= 0;
}

//inline bool ZoneInfo::IsInZone(int32_t world_id, float x, float y) const
//{
    //if (world_id == 0)
    //{
        //return Util::IsNormalMap(world_id);
    //}

	//if (this->world_id != world_id)
	//{
		//return false;
	//}
	//return x >= min_x && x <= max_x && y >= min_y && y <= max_y;
//}

inline bool ZoneOpInfo::CheckDataValidity() const
{
	CHECK_INRANGE(op, ZONE_ENTER, ZONE_LEAVE)
	CHECK_VALIDITY(zone)
	return true;
}

inline bool ItemUseZone::CheckDataValidity() const
{
	CHECK_VALIDITY(zone)
    return item_id >= 0;
}

// ZONE_ENTER：在区域内，并不是真正的指进入区域那一下
// ZONE_LEAVE：在区域外，同上不是出区域那一下
//inline bool ZoneOpInfo::Match(int32_t world_id, float x, float y) const
//{
	//bool in_zone = zone.IsInZone(world_id, x, y);
	//return (op == ZONE_ENTER && in_zone) || (op == ZONE_LEAVE && !in_zone);
//}

//inline bool ItemUseZone::Match(int32_t world_id, float x, float y) const
//{
    //return zone.IsInZone(world_id, x, y);
//}

} // namespace task

#endif // TASK_ZONE_INFO_H_
