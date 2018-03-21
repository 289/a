#ifndef GAMED_GS_SCENE_BATTLE_BG_CONTROLLER_H_
#define GAMED_GS_SCENE_BATTLE_BG_CONTROLLER_H_

#include <map>

#include "shared/base/atomic.h"
#include "shared/base/rwlock.h"
#include "gs/scene/world_def.h"


namespace shared {
	class Conf;
} // namespace shared


namespace gamed {

class WorldManager;
class NpcGenerator;
class AreaGenerator;
class MatterGenerator;

/**
 * @brief BGController
 *    （1）该类是线程不安全的
 */
class BGController
{
public:
    BGController();
	virtual ~BGController();

	bool Init(MapID world_id, shared::Conf* conf, const char* world_name);

    // ---- thread safe ----
	WorldManager* CreateWorldMan(int64_t bg_serial_num, int32_t bg_templ_id);
    bool          FindWorldMan(const world::battleground_info& info, world::battleground_info& ret_info);
	void          RemoveBGWorldMan(int32_t world_id, int32_t world_tag);
    void          CloseBGWorldMan(int32_t world_id, int32_t world_tag); // 地图关闭，当还没有remove，这里还不能从active里移除
	bool          IsActiveBG(int32_t worldid, int32_t worldtag);

    // ---- thread safe ----
	void          EnableMapElem(int32_t elem_id);
	void          DisableMapElem(int32_t elem_id);


private:
    // ---- thread safe ----
	typedef void (*EachBGWorldCB)(const world::battleground_info&, int64_t);
	void ForEachBGWorld(EachBGWorldCB callback, int64_t param);
	void TraversalActiveMapElem(const world::battleground_info& info);

    // **** thread unsafe ****
	static void SendMapElemEnable(const world::battleground_info& info, int64_t elemid);
	static void SendMapElemDisable(const world::battleground_info& info, int64_t elemid);

    // **** thread unsafe ****
	inline bool is_active_bg(int32_t worldid, int32_t worldtag) const;
	inline int32_t get_new_tag_id();


private:
    MapID               bg_world_id_;
	std::string         world_name_;
	shared::Conf*       pconf_;
	shared::AtomicInt32 cur_tag_id_;

	NpcGenerator*       npc_gen_;
	AreaGenerator*      area_gen_;
	MatterGenerator*    matter_gen_;

	shared::RWLock      change_rwlock_;

    // worldid_64
	typedef std::map<int64_t/*worldid_64*/, world::battleground_info> ActiveBGMap;
	ActiveBGMap         active_bg_map_;
    typedef std::map<int32_t/*bg_tid*/, int64_t/*worldid_64*/> UniqueBGMap;
    UniqueBGMap         unique_bg_map_; // 只开一张图的特殊战场，模板id做key

    // map element
	shared::MutexLock   mapelem_set_mutex_;    // 加锁顺序：先加mapelem_set_mutex_，再加change_rwlock_ 

    // 一个地图元素id不能同时存在active和deactive的set里
	typedef std::set<int32_t> ActiveMapElemSet;
	ActiveMapElemSet    active_mapelem_set_;   // 通过外部方式激活的地图元素
	ActiveMapElemSet    deactive_mapelem_set_; // 通过外部方式取消激活的地图元素
};

///
/// inline func
///
inline int32_t BGController::get_new_tag_id()
{
	return cur_tag_id_.increment_and_get();
}

// needed lock outside
inline bool BGController::is_active_bg(int32_t worldid, int32_t worldtag) const
{
	ActiveBGMap::const_iterator it = active_bg_map_.find(MakeWorldID(worldid, worldtag));
	if (it != active_bg_map_.end())
	{
		return true;
	}

	return false;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_CONTROLLER_H_
