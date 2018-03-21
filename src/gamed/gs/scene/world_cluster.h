#ifndef GAMED_GS_SCENE_WORLD_CLUSTER_H_
#define GAMED_GS_SCENE_WORLD_CLUSTER_H_

#include <map>
#include <set>
#include <vector>

#include "shared/base/singleton.h"
#include "shared/base/rwlock.h"
#include "shared/base/mutex.h"

#include "gs/global/game_types.h"
#include "gs/global/gmatrix_def.h"
#include "gs/scene/world_def.h"


namespace shared {
	class Conf;
} // namespace shared


namespace gamed {

class WorldManager;


/**
 * @brief wcluster
 */
namespace wcluster {

	// ---- thread safe ----
	int           CreateWorldManager(shared::Conf* pconf, const char* world_name);
	int           InsertWorldManager(MapID id, WorldManager* manager, MapTag tag);
	void          RemoveWorldManager(WorldManager* manager);
	WorldManager* FindWorldManager(MapID world_id, MapTag world_tag);
	WorldManager* AllocWorldManager();
	void          FreeWorldManager(WorldManager* manager);

	// ---- thread safe ----
	bool          CreateInsWorld(int32_t world_id, int64_t ins_serial_num, int32_t ins_templ_id, world::instance_info& ret_info);
	bool          FindInsWorld(const world::instance_info& info, world::instance_info& ret_info);

    // ---- thread safe ----
	bool          CreateBGWorld(int32_t world_id, int64_t bg_serial_num, int32_t bg_templ_id, world::battleground_info& ret_info);
	bool          FindBGWorld(const world::battleground_info& info, world::battleground_info& ret_info);

	// ---- thread safe ----
	void          EnableMapElem(int32_t world_id, int32_t elem_id);
	void          DisableMapElem(int32_t world_id, int32_t elem_id);

	// **** thread unsafe ****
	void          GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec);
	void          ReleaseAll();
	void          HeartbeatTick();

    // **** thread unsafe ****
    void          InitInsScriptSys(const char* script_dir);
    void          InitBGScriptSys(const char* script_dir);

} // namespace wcluster


/**
 * @brief WorldCluster
 */
class WorldCluster
{
public:
	/**
	 * @brief InsertWorldManager 
	 *     1.InsertWorldManager需要在单线程中执行
	 *     2.Insert在启动的时候执行，因此FindWorldManager是安全的 - 
	 *       s_worldMan_map_启动后不会变（暂时没有考虑副本）
	 *     3.FindWorldManager()取到的指针是不能上锁的，因此取到的WorldManager指针 -
	 *       调用的所有函数都应该是线程安全的！
	 *     4.Player类里不能直接调用FindWorldManager()获取WorldManager的指针
	 */
	// ---- thread safe ----
	virtual WorldManager* FindWorldManager(MapID world_id, MapTag world_tag);
	virtual int           InsertWorldManager(MapID id, WorldManager* manager, MapTag tag);
	virtual void          RemoveWorldManager(WorldManager* manager);
	virtual void          GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec);
	
	// **** thread unsafe ****
	void    ReleaseAll();
	void    HeartbeatTick();


protected:
	WorldCluster();
	virtual ~WorldCluster();


private:
	void    HandleWaitingRemove(); // called in tick
	void    DoChange();
	void    DoInsert(WorldManager* manager);
	void    DoDelete(WorldManager* manager);
	void    CollectHeartbeat(std::vector<WorldManager*>& list);


private:
	typedef std::set<WorldManager*> WorldManSet;
	WorldManSet       worldMan_set_;
	shared::MutexLock lock_wm_set_;

	typedef std::map<int64_t, size_t> WorldIdToIndexMap;
	WorldIdToIndexMap wid_mapto_index_;
	shared::RWLock    wid_idx_map_rwlock_;

	typedef std::map<WorldManager*, int> ChangeMap;
	ChangeMap         change_list_;
	shared::MutexLock lock_change_;

	typedef std::map<WorldManager*, int> WaitingRemoveMap;
	WaitingRemoveMap  waiting_remove_map_;

	struct WorldManHeartbeatInfo
	{
		int              heart_count;
		WorldManager*    cur_worldMan;
		WorldManSet::const_iterator cur_cursor;

		void Reset(const WorldManSet& worldManMap);
	};
	WorldManHeartbeatInfo        wm_heartbeat_info_;
	std::vector<WorldManager*>   wm_hb_vec_;
};


/**
 * @brief NormalWorldCluster
 */
class NormalWorldCluster : public shared::Singleton<NormalWorldCluster>, public WorldCluster
{
	friend class shared::Singleton<NormalWorldCluster>;
public:
	static inline NormalWorldCluster* GetInstance() {
		return &(get_mutable_instance());
	}

protected:
	NormalWorldCluster() { }
	virtual ~NormalWorldCluster() { }
};

#define s_pNormalWC gamed::NormalWorldCluster::GetInstance()

} // namespace gamed

#endif // GAMED_GS_SCENE_WORLD_CLUSTER_H_
