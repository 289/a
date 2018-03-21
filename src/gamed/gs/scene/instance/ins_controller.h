#ifndef GAMED_GS_SCENE_INSTANCE_INS_CONTROLLER_H_
#define GAMED_GS_SCENE_INSTANCE_INS_CONTROLLER_H_

#include <set>
#include <map>

#include "shared/base/atomic.h"
#include "shared/base/rwlock.h"

#include "gs/global/game_types.h"
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
 * @brief InsController
 *    （1）该类是线程不安全的
 */
class InsController
{
public:
	InsController();
	virtual ~InsController();

	bool Init(MapID world_id, shared::Conf* conf, const char* world_name);

	// ---- thread safe ----
	WorldManager* CreateWorldMan(int64_t ins_serial_num, int32_t ins_templ_id);
	bool          FindWorldMan(const world::instance_info& info, world::instance_info& ret_info);
	void          RemoveInsWorldMan(int32_t world_id, int32_t world_tag);
	void          CloseInsWorldMan(int32_t world_id, int32_t world_tag);
	bool          IsActiveIns(int32_t worldid, int32_t worldtag);

	// ---- thread safe ----
	void          EnableMapElem(int32_t elem_id);
	void          DisableMapElem(int32_t elem_id);


private:
	// ---- thread safe ----
	typedef void (*EachInsWorldCB)(const world::instance_info&, int64_t);
	void ForEachInsWorld(EachInsWorldCB callback, int64_t param);
	void TraversalActiveMapElem(const world::instance_info& info);

	// **** thread unsafe ****
	static void SendMapElemEnable(const world::instance_info& info, int64_t elemid);
	static void SendMapElemDisable(const world::instance_info& info, int64_t elemid);

	// **** thread unsafe ****
	inline int32_t get_new_tag_id();
	inline bool is_active_ins(int32_t worldid, int32_t worldtag) const;


private:
	MapID               ins_world_id_;
	std::string         world_name_;
	shared::Conf*       pconf_;
	shared::AtomicInt32 cur_tag_id_;

	NpcGenerator*       npc_gen_;
	AreaGenerator*      area_gen_;
	MatterGenerator*    matter_gen_;

	shared::RWLock      change_rwlock_;

	// worldid_64
	typedef std::map<int64_t, world::instance_info> ActiveInsMap;
	ActiveInsMap        active_ins_map_;

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
inline int32_t InsController::get_new_tag_id()
{
	return cur_tag_id_.increment_and_get();
}

// needed lock outside
inline bool InsController::is_active_ins(int32_t worldid, int32_t worldtag) const
{
	ActiveInsMap::const_iterator it = active_ins_map_.find(MakeWorldID(worldid, worldtag));
	if (it != active_ins_map_.end())
	{
		return true;
	}

	return false;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_INSTANCE_INS_CONTROLLER_H_
