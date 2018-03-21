#ifndef GAMED_GS_GLOBAL_GAME_TYPES_H_
#define GAMED_GS_GLOBAL_GAME_TYPES_H_

#include <stdint.h>
#include <assert.h>

#include "shared/base/assertx.h"
#include "shared/base/base_define.h"

#include "math_types.h"


namespace gamed {

class Npc;
class AreaObj;
class Matter;
struct XID;


///
/// types define
///
typedef int32_t MapID;
typedef int32_t MapTag;
typedef int64_t TickIndex;


// 需要与data_templ/base_datatempl.h里的定义一致
// 没写成typedef是因为不希望在本文件include太多.h
#define ClassMask shared::net::BitSet<128>


/**
 * @brief XID
 *    （1）服务器内部的id类型，每个对象都有一个xid，包括world
 *    （2）不能再加类型2014-08-01
 */
struct XID
{
	enum
	{
	  /* 服务器内部使用 */
	  // 0
		TYPE_INVALID = 0,
		TYPE_SERVICE_NPC,       // id 是service NPC
		TYPE_MONSTER,           // id 是怪物 
		TYPE_PLAYER,            // id 是用户的roleid
		TYPE_MATTER,            // id 是matter

	  // 5
		TYPE_WORLD_PLANE,       // id 是world
		TYPE_TIMED_TASK,        // session timed管理器（timed_task）
		TYPE_MAP_AREA_OBJ,      // 地图上的区域对象area
		TYPE_MAP_TRANSFER_AREA, // 传送区域
		TYPE_MAP_LANDMINE_AREA, // 暗雷区域

	  // 10
		TYPE_COMBAT,            // 战斗系统


	  /* 与client交互时使用 */
		TYPE_NPC,               // 这是一个总的类型，用于客户端发过来时，自动识别obj_id的类型，
		                        // 找到对应的obj pool
		TYPE_AREA_OBJ,          // 区域对象，这是一个总类型


		TYPE_MAX_ELEM           // 标识最大的type，用于判断type是否有效
	};
	
	typedef uint16_t Type;
	typedef int64_t  IDType;
	Type    type;
	IDType  id;         

	XID() 
		: type(TYPE_INVALID),
		  id(-1)
	{ }

	XID(IDType obj_id, Type a_type)
		: type(a_type),
		  id(obj_id)
	{ }

	inline bool operator==(const XID& rhs) const;
	inline bool operator!=(const XID& rhs) const;

	inline bool IsValid() const;
	inline bool IsPlayer() const;
	inline bool IsObject() const;
	inline bool IsNpc() const;
	inline bool IsArea() const;
	inline bool IsMatter() const;
	inline bool IsWorld() const;
};

SHARED_STATIC_ASSERT(sizeof(XID::IDType) == (sizeof(MapID) + sizeof(MapTag)));

///
/// inline XID
///
inline bool XID::operator==(const XID& rhs) const
{
	return type == rhs.type && id == rhs.id;
}

inline bool XID::operator!=(const XID& rhs) const
{
	return id != rhs.id || type != rhs.type;
}

inline bool XID::IsPlayer() const
{
	return TYPE_PLAYER == type;
}

inline bool XID::IsValid() const
{
	return type > TYPE_INVALID && type < TYPE_MAX_ELEM;
}

inline bool XID::IsObject() const
{
	return TYPE_PLAYER == type || 
		   TYPE_SERVICE_NPC == type || 
		   TYPE_MONSTER == type || 
		   TYPE_NPC == type ||
		   TYPE_MAP_AREA_OBJ == type || 
		   TYPE_MAP_TRANSFER_AREA == type ||
		   TYPE_MAP_LANDMINE_AREA == type ||
		   TYPE_AREA_OBJ == type ||
		   TYPE_MATTER == type;
}

inline bool XID::IsNpc() const
{
	return TYPE_SERVICE_NPC == type || TYPE_MONSTER == type || TYPE_NPC == type;
}

inline bool XID::IsArea() const
{
	return TYPE_MAP_AREA_OBJ == type || 
		   TYPE_MAP_TRANSFER_AREA == type || 
		   TYPE_MAP_LANDMINE_AREA == type || 
		   TYPE_AREA_OBJ == type;
}

inline bool XID::IsMatter() const
{
	return TYPE_MATTER == type;
}

inline bool XID::IsWorld() const
{
	return TYPE_WORLD_PLANE == type;
}


/**
 * @brief ROLEID_TO_SVRID 
 *  （1）roleid的高16位保存的是创建服的masterid，但这个值不一定等于他所在的master的id，
 *      因为可能已经合服，svr_id需要在gmatrix里GetMasterIdBySvrId()函数才能取到真正的masterid
 */
inline int32_t ROLEID_TO_SVRID(int64_t roleid)
{
    return roleid >> 48;
}


/**
 * @brief ID Transform
 *    （1）以下是ID转换函数，使用index + type的形式做id，可以很好的配合对象池
 *    （2）固定大小的对象池使得index也是固定的，从而对应指针的Obj的id其实也是固定的
 */
inline int32_t ID2IDX(int64_t id) 
{ 
	return id & 0x0FFFFFFF; 
}

template<typename T> inline int32_t MAKE_OBJID(int32_t id);
template<> inline int32_t MAKE_OBJID<Npc>(int32_t id)     { id &= 0x0FFFFFFF; return id | 0x80000000; }
template<> inline int32_t MAKE_OBJID<AreaObj>(int32_t id) { id &= 0x0FFFFFFF; return id | 0x70000000; }
template<> inline int32_t MAKE_OBJID<Matter>(int32_t id ) { id &= 0x0FFFFFFF; return id | 0x60000000; }

// 以下需要和客户端Share/network/protocol/gs/global/game_types.h里的XID::SetIDFromSvr()一致
inline void MAKE_XID(int64_t id, XID& xid)
{
	xid.id = id;
	if ((uint32_t)(id >> 32) != 0 &&
		(uint32_t)(id >> 32) != 0xFFFFFFFF)
	{
		xid.type = XID::TYPE_PLAYER;
	}
	else if (((int32_t)id & 0xF0000000) == 0x80000000)
	{
		xid.type = XID::TYPE_NPC;
	}
	else if (((int32_t)id & 0xF0000000) == 0x70000000)
	{
		xid.type = XID::TYPE_AREA_OBJ;
	}
	else if (((int32_t)id & 0xF0000000) == 0x60000000)
	{
		xid.type = XID::TYPE_MATTER;
	}
	else // error
	{
		xid.type = XID::TYPE_INVALID;
	}
}

inline XID MAKE_XID(int64_t id)
{
    XID xid;
    MAKE_XID(id, xid);
    return xid;
}

inline MapID MAP_ID(const XID& xid)
{
	ASSERT(xid.IsWorld());
	return low32bits(xid.id);
}

inline MapTag MAP_TAG(const XID& xid)
{
	ASSERT(xid.IsWorld());
	return high32bits(xid.id);
}

inline XID::IDType MakeWorldID(MapID id, MapTag tag)
{
	return makeInt64(tag, id);
}

inline XID MakeWorldXID(MapID id, MapTag tag)
{
	XID xid;
	xid.id   = MakeWorldID(id, tag);
	xid.type = XID::TYPE_WORLD_PLANE;
	return xid;
}

inline XID MakeWorldXID(int64_t longid)
{
	XID xid;
	xid.id   = longid;
	xid.type = XID::TYPE_WORLD_PLANE;
	return xid;
}


/**
 * @brief 地图号分类
 */
static const int32_t kMapIdOffset    = 1000;


///
/// world_id to map_id(client use or use to find map)
///
inline MapID WID_TO_MAPID(MapID id)
{
	return id & 0xFFFF;
}


///
/// intansce map 本gs副本地图地图号在6000 ~ 6999之间
///
static const int32_t kInsMapIdPrefix = 6;

inline bool IS_INS_MAP(MapID id)
{
	id = WID_TO_MAPID(id);
	if ((id / kMapIdOffset) == kInsMapIdPrefix)
	{
		return true;
	}
	return false;
}

inline bool IS_INS_MAP(const XID& xid)
{
	MapID mapid = MAP_ID(xid);
	return IS_INS_MAP(mapid);
}

inline bool IS_CR_INS(MapID id)
{
	if (IS_INS_MAP(id))
	{
        int32_t value = id & HIGH_16_MASK;
		if (value && (value != (int32_t)HIGH_16_MASK))
            return true;
	}
	return false;
}


///
/// battleground map 本gs副本地图地图号在7000 ~ 7999之间
///
static const int32_t kBGMapIdPrefix = 7;

inline bool IS_BG_MAP(MapID id)
{
	id = WID_TO_MAPID(id);
	if ((id / kMapIdOffset) == kBGMapIdPrefix)
	{
		return true;
	}
	return false;
}

inline bool IS_BG_MAP(const XID& xid)
{
	MapID mapid = MAP_ID(xid);
	return IS_BG_MAP(mapid);
}

inline bool IS_CR_BG(MapID id)
{
	if (IS_BG_MAP(id))
	{
        int32_t value = id & HIGH_16_MASK;
		if (value && (value != (int32_t)HIGH_16_MASK))
            return true;
	}
	return false;
}


///
/// 跨服地图
///
inline bool MAP_CAN_CR(MapID id)
{
    if (IS_BG_MAP(id) || IS_INS_MAP(id))
        return true;
    return false;
}

inline MapID CONVERT_TO_CR_MAP(MapID id, int32_t gs_id)
{
	ASSERT(MAP_CAN_CR(id));
    int32_t mask = gs_id << 16;
	return id | mask;
}


///
/// drama map 剧情地图地图号在2000 ~ 2999之间
///
static const int32_t kDramaMapIdPrefix = 2;

inline bool IS_DRAMA_MAP(MapID id)
{
	id = WID_TO_MAPID(id);
    if ((id / kMapIdOffset) == kDramaMapIdPrefix)
        return true;
	return false;
}

inline bool IS_DRAMA_MAP(const XID& xid)
{
	MapID mapid = MAP_ID(xid);
	return IS_DRAMA_MAP(mapid);
}


///
/// normal map 普通地图
///
inline bool IS_NORMAL_MAP(MapID id)
{
	if (IS_INS_MAP(id))
		return false;
    if (IS_BG_MAP(id))
        return false;
	return true;
}

} // namespace gametypes

#endif // GAMED_GS_GLOBAL_GAME_TYPES_H_
