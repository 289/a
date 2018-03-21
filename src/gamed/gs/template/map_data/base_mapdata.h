#ifndef GAMED_GS_TEMPLATE_MAPDATA_BASE_MAPDATA_H_
#define GAMED_GS_TEMPLATE_MAPDATA_BASE_MAPDATA_H_

#include "shared/base/singleton.h"

#include "map_util.h"
#include "map_types.h"


namespace mapDataSvr {

using namespace shared::net;

enum MAP_DATA_TYPE
{
// 0
	MAPDATA_TYPE_INVALID = 0,
	MAPDATA_TYPE_SPOT_MONSTER,       // 定点怪
	MAPDATA_TYPE_AREA_WITH_RULES,    // 规则区域
	MAPDATA_TYPE_AREA_MONSTER,       // 区域怪（明雷、暗雷）
	MAPDATA_TYPE_AREA_NPC,           // 区域Npc

// 5
	MAPDATA_TYPE_MAP_DEFAULT_AREA,   // 地图的默认区域
	MAPDATA_TYPE_SCENE_EFFECT,       // 地图光效
	MAPDATA_TYPE_TRANSFER_AREA,      // 传送区域
	MAPDATA_TYPE_SPOT_MINE,          // 定点矿
	MAPDATA_TYPE_AREA_MINE,          // 区域矿

	MAPDATA_TYPE_MAX
};

///
/// 怪物相关的enum
///
enum APMode 
{ 
	PASSIVE  = 0,   // 被动怪
	ACTIVE   = 1,   // 主动怪
};

enum CombatRule 
{ 
	NO_MODEL         = 0,  // 进入战斗则模型消失
	MODEL_NO_COMBAT  = 1,  // 进入战斗后模型不消失，但是别的玩家不能通过这个怪物出发新的战斗事件
	MODEL_WORLD_BOSS = 2,  // 进入战斗后模型不消失，执行世界boss战斗逻辑，boss在多场战斗里共享血量
    NEVER_COMBAT     = 3,  // 不能战斗，树桩怪
};

///
/// struct 
///
struct Coordinate
{
	float x;
	float y;
	NESTED_DEFINE(x, y);
};

const int32_t kNormalizedProbValue = 10000; // 万分数的1
const int32_t kMaxVertexCount      = 255;   // 区域的多边形最多顶点数

/**
 * @brief check_prob 
 *    用于检查概率是否有效，单位都有万分数
 */
inline bool check_prob_valid(int probability)
{
	if (probability < 0 || probability > kNormalizedProbValue)
		return false;
	return true;
}


/**
 * @brief 新加的mapdata template都要在base_data.cpp里做INIT
 *    1.定义成员变量类型是注意参考map_types.h中的typedef
 */
class BaseMapData : public BasePacket
{
public:
	BaseMapData(BasePacket::Type id)
		: BasePacket(id),
		  map_id(0),
		  elem_id(0),
		  is_default_activate(false)
	{ }

	virtual ~BaseMapData() { }

	MapID     map_id;  
	ElemID    elem_id;             // 地图元素模板id，全局唯一
	bool      is_default_activate; // 是否默认开启，默认为true

	virtual void Marshal();
	virtual void Unmarshal();
	bool CheckDataValidity();

	inline void set_mapdata_info(MapID mapid, ElemID elemid);


protected:
	// 子类使用以下函数
	virtual void OnMarshal() = 0;
	virtual void OnUnmarshal() = 0;
	virtual bool OnCheckDataValidity() const = 0;  // 检查模板里数据的有效性，每个子类自行定义
};

///
/// inline func
///
inline void BaseMapData::set_mapdata_info(MapID mapid, ElemID elemid)
{
	map_id  = mapid;
	elem_id = elemid;
}

///
/// BaseMapDataManager
///
class BaseMapDataManager : public shared::Singleton<BaseMapDataManager>
{
	friend class shared::Singleton<BaseMapDataManager>;
public:
	static inline BaseMapDataManager* GetInstance() {
		return &(get_mutable_instance());
	}

	static BaseMapData* CreatePacket(BaseMapData::Type id);
	static bool InsertPacket(uint16_t type, BaseMapData* packet);
	static bool IsValidType(int32_t type);


protected:
	BaseMapDataManager() { }
	~BaseMapDataManager() { }

	bool OnInsertPacket(uint16_t type, BaseMapData* packet);
	BaseMapData* OnCreatePacket(BaseMapData::Type id);


private:
	typedef std::map<BaseMapData::Type, BaseMapData*> BaseMapDataMap;
	BaseMapDataMap packet_map_;
};

} // namespace mapDataSvr

#endif // GAMED_GS_TEMPLATE_MAPDATA_BASE_MAPDATA_H_
