#ifndef GAMED_GS_SCENE_WORLD_H_
#define GAMED_GS_SCENE_WORLD_H_

#include "shared/base/noncopyable.h"
#include "shared/base/conf.h"
#include "shared/base/rwlock.h"

#include "aoi.h"
#include "world_def.h"


namespace mapDataSvr
{
	class MapDefaultArea;
	class AreaWithRules;
}


namespace gamed {

namespace playerdef 
{
	struct AreaRulesInfo;
}

class Player;
class AreaObj;
class Npc;

/**
 * @brief World
 *    （1）World用来保存着地图的各种静态或者变化很少的数据，供WorldObject调用。
 *         不同与它的管理类WorldManager，World中很少有操作型的接口函数
 *    （2）每一个WorldObject上都会有一个World的指针，因此World大部分public函数都要是线程安全的
 */
class World : shared::noncopyable
{
	///
	/// TODO: World的friend class只能调用World的public及protected成员函数
	/// 人为保证这个限制
	///
	friend class WorldManager;

public:
	// ---- thread safe ----
	bool    QueryObject(const XID& id, world::worldobj_base_info& info) const;
	bool    QueryPlayer(RoleID roleid, world::player_base_info& info) const;
    bool    QueryPlayerExtra(RoleID roleid, world::player_extra_info& info) const;
	bool    QueryAreaRules(const std::vector<int32_t>& area_vec, playerdef::AreaRulesInfo& rules_info) const;

	// ---- thread safe ----
	bool    InsertObjToAOI(XID obj, const A2DVECTOR& init_pos, int64_t param);
	bool    InsertAreaToAOI(XID obj, const std::vector<A2DVECTOR>& vertexes);
	void    RemoveObjFromAOI(XID obj);
	void    RemoveAreaFromAOI(XID obj);
	void    UpdateAOI(XID obj, const A2DVECTOR& pos);

	// ---- thread safe ----
	bool    IsWalkablePos(const A2DVECTOR& pos) const;
	bool    PlaneSwitch(Player* pplayer, int32_t target_world_id, const A2DVECTOR& pos) const; // 调用时player必须已经上锁

	// ---- thread safe ----
	void    GetAllObjectInWorld(std::vector<XID>& obj_vec);
    void    GetAllPlayerInWorld(std::vector<XID>& player_vec);
    void    GetAllPlayerLinkInfo(std::vector<world::player_link_info>& player_vec);
    int32_t GetPlayerCount();

    // ---- thread safe ----
    void    SetPlayerExtraInfo(const Player* player_ptr);

	inline bool    PosInWorld(const A2DVECTOR& pos) const;
	inline bool    GetInsInfo(world::instance_info& info) const;
    inline bool    GetBGInfo(world::battleground_info& info) const;
    inline bool    HasMapTeam() const;    // 本地图是不是允许地图内组队
    inline bool    HasMapCounter() const; // 本地图是不是有地图内计数器
    inline bool    HasMapGather() const;  // 本地图是不是有地图采集事件

	inline MapID   world_id() const;
	inline MapTag  world_tag() const;
	inline const XID& world_xid() const;
	
	// grid
	inline bool    IsOutsideGrid(float x, float y) const;
	inline bool    IsInGridLocal(float x, float y) const;

	// **** thread safe ****
	inline time_t  get_create_time() const;


protected:
	World();
	~World();

	bool    Init(const world::WorldInitData& initData);

	/**
	 * @brief Heartbeat 
	 *     心跳频率由kWorldHeartbeatRate值决定, 
	 *
	 * **** thread unsafe ****
	 */
	void    Heartbeat();

	/**
	 * @brief InsertPlayer, RemovePlayer 
	 *     1.根据player的位置，插入一个对象到世界中，返回
	 *       插入的区域索引号
	 *     2.从世界中移出一个对象，不free
	 *     3.InsertXXXX返回值小于0时表示插入失败
	 *
	 * ---- thread safe ----
	 */
	int     InsertPlayer(Player* player_ptr);
	void    RemovePlayer(Player* player_ptr);
	int     InsertNpc(Npc* npc_ptr);
	void    RemoveNpc(Npc* npc_ptr);
	int		InsertAreaObj(AreaObj* area_ptr);
	void    RemoveAreaObj(AreaObj* area_ptr);
	int     InsertMatter(Matter* matter_ptr);
	void    RemoveMatter(Matter* matter_ptr);
	
	// ---- thread safe ----	
	void    InsertActiveRuleArea(int32_t elem_id);
	void    RemoveActiveRuleArea(int32_t elem_id);

	// ---- thread safe ----
	bool    ObjInsertToAOI(XID obj, const char* mode, const A2DVECTOR& init_pos, int64_t param);
	void    ObjUpdateAOI(XID obj, const char* mode, const A2DVECTOR& pos);

	// **** thread safe ****
	inline bool SetInsInfo(const world::instance_info& info);
	inline bool SetBGInfo(const world::battleground_info& info);
    inline void SetMapTeamFlag();
    inline void SetMapCounterFlag();
    inline void SetMapGatherFlag();

	// **** thread safe ****
	inline void    set_world_id(MapID id);
	inline void    set_world_tag(MapTag tag);

	
protected:
	/**
	 * @brief SendObjectMSG
	 *     向object发送消息
	 */
	void SendObjectMSG(int message, const XID& target, int64_t param = 0) const;
	void SendObjectMSG(int message, const XID& target, const void* buf, size_t len) const;

	/**
	 * @brief SendWorldMSG 
	 *    向别的World发给地图消息
	 */
	void SendWorldMSG(int message, const XID& target, int64_t param = 0) const;
	void SendWorldMSG(int message, const XID& target, const void* buf, size_t len) const;

	
private:
	static void DispatchAOIMessage(AOI::AOI_Type type, 
			                       AOI::ObjectInfo watcher, 
								   AOI::ObjectInfo marker, 
								   void* pdata);
	bool CreateGrid(int row, int column, float step, float startX, float startY);
	void FillDefaultAreaRules(playerdef::AreaRulesInfo& rules_info) const;
    void BuildPlayerExtraInfo(const Player* player_ptr, world::player_extra_info& info);
	inline void set_obj_visible_range(float rad_short, float rad_long);
	inline void refresh_xid();


private:
	XID        xid_;       // world的xid是用world_id_，world_tag_拼出来的唯一标识
	                       // 高位32位是world_id_，低32位是world_tag_
	MapID      world_id_;
	MapTag     world_tag_;
	Grid       grid_;
	AOI        aoi_module_;

	// 地图内对象视野是一个环形:
	// (1)保证进入最近视野的object一定能看到
	// (2)保证超出最远视野的object才会看不见
	// 这么做是为了防止aoi查询结果颠簸
	float      obj_far_vision_;
	float      obj_near_vision_;

	mutable RWLock query_player_rwlock_;
	RWLock     query_npc_rwlock_;
	RWLock     query_area_rwlock_;
	RWLock     query_matter_rwlock_;

	typedef std::map<XID::IDType, const Player*> ConstPlayerMap;
	ConstPlayerMap      players_map_;  // 不能直接操作player指针
    
	typedef std::map<XID::IDType, const Npc*> ConstNpcMap;
	ConstNpcMap         npcs_map_;     // 不能直接操作npc指针，只能取npc里一些不会变的数据，比如说id

	typedef std::map<XID::IDType, const AreaObj*> ConstAreaObjMap;
	ConstAreaObjMap     areas_map_;    // 不能直接操作area指针，只能取obj里一些不会变的数据

	typedef std::map<XID::IDType, const Matter*> ConstMatterMap;
	ConstMatterMap      matters_map_;  // 不能直接操作matter指针

	// 激活的规则区域都要注册到以下列表，用于玩家查询区域规则
	mutable RWLock query_area_rules_rwlock_;
	typedef std::map<int32_t, const mapDataSvr::AreaWithRules*> AreaWithRulesTmplMap;
	AreaWithRulesTmplMap active_rule_areas_;

    // 玩家的额外信息，该信息是用MSG的形式同步的
    mutable RWLock query_player_extra_rwlock_;
    typedef std::map<XID::IDType, world::player_extra_info> PlayerExtraInfoMap;
    PlayerExtraInfoMap   players_extra_map_;

	// 本地图的默认区域
	const mapDataSvr::MapDefaultArea* world_default_area_;

	// 副本信息
	world::instance_info ins_info_;    // 设置完毕后，副本生命周期内不会变化

    // 战场信息
    world::battleground_info bg_info_; // 设置完毕后，战场生命周期内不会变化

	// 地图创建时间
	time_t    create_time_;  // 单位s

    // 是否支持地图内组队
    bool      has_map_team_; 

    // 是否支持地图内的计数器
    bool      has_map_counter_;

    // 是否支持地图采矿事件
    bool      has_map_gather_;
};

///
/// inline function
///
inline MapID World::world_id() const
{
	return world_id_;
}

inline MapTag World::world_tag() const
{
	return world_tag_;
}

inline const XID& World::world_xid() const
{
	return xid_;
}

inline void World::set_world_id(MapID id)
{
	world_id_  = id;
	refresh_xid();
}

inline void World::set_world_tag(MapTag tag)
{
	world_tag_ = tag;
	refresh_xid();
}

inline time_t World::get_create_time() const
{
	return create_time_;
}

inline void World::refresh_xid()
{
	xid_ = MakeWorldXID(world_id_, world_tag_);
}

inline bool World::IsOutsideGrid(float x, float y) const
{
	return grid_.IsOutsideGrid(x, y);
}

inline bool World::IsInGridLocal(float x, float y) const
{
	return grid_.IsLocal(x, y);
}

inline void World::set_obj_visible_range(float rad_short, float rad_long)
{
	aoi_module_.SetObjectFieldOfView(rad_short, rad_long);
}

inline bool World::PosInWorld(const A2DVECTOR& pos) const
{
	return IsInGridLocal(pos.x, pos.y);
}

inline bool World::SetInsInfo(const world::instance_info& info)
{
	if (!IS_INS_MAP(info.world_id))
		return false;

	ins_info_ = info;
	return true;
}

inline bool World::GetInsInfo(world::instance_info& info) const
{
	if (!IS_INS_MAP(world_id_))
		return false;

	info = ins_info_;
	return true;
}

inline bool World::SetBGInfo(const world::battleground_info& info)
{
	if (!IS_BG_MAP(info.world_id))
		return false;

	bg_info_ = info;
	return true;
}

inline bool World::GetBGInfo(world::battleground_info& info) const
{
    if (!IS_BG_MAP(world_id_))
        return false;

    info = bg_info_;
    return true;
}

inline void World::SetMapTeamFlag()
{
    has_map_team_ = true;
}

inline bool World::HasMapTeam() const
{
    return has_map_team_;
}

inline void World::SetMapCounterFlag()
{
    has_map_counter_ = true;
}

inline bool World::HasMapCounter() const
{
    return has_map_counter_;
}

inline void World::SetMapGatherFlag()
{
    has_map_gather_ = true;
}

inline bool World::HasMapGather() const
{
    return has_map_gather_;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_WORLD_H_
