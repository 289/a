#ifndef GAMED_GS_SCENE_INSTANCE_INS_CLUSTER_H_
#define GAMED_GS_SCENE_INSTANCE_INS_CLUSTER_H_

#include <map>

//#include "shared/base/atomic.h"
#include "gs/scene/world_cluster.h"


namespace gamed {

class InsController;

/**
 * @brief InstanceCluster
 *   （1）副本的cluster
 */
class InstanceCluster : public shared::Singleton<InstanceCluster>, public WorldCluster
{
	friend class shared::Singleton<InstanceCluster>;
public:
	static inline InstanceCluster* GetInstance() {
		return &(get_mutable_instance());
	}

	// ---- thread safe ----
	bool    CreateController(shared::Conf* pconf, int32_t world_id, const char* world_name);
	bool    CreateWorldMan(int32_t world_id, int64_t ins_serial_num, int32_t ins_templ_id, world::instance_info& ret_info);
	bool    FindInsWorldMan(const world::instance_info& info, world::instance_info& ret_info);
	void    RemoveInsWorldMan(int32_t world_id, int32_t world_tag);
	void    CloseInsWorldMan(int32_t world_id, int32_t world_tag);

	// ---- thread safe ----
	virtual WorldManager* FindWorldManager(MapID world_id, MapTag world_tag);
	virtual void          GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec);

	// ---- thread safe ----
	void    EnableMapElem(int32_t world_id, int32_t elem_id);
	void    DisableMapElem(int32_t world_id, int32_t elem_id);

	// ---- thread safe ----
	//int32_t GetNextInsTeamId();


protected:
	InstanceCluster();
	virtual ~InstanceCluster();


private:
	typedef std::map<MapID, InsController*> InsControllerMap;
	InsControllerMap  ins_ctrl_map_;
	shared::RWLock    ins_ctrl_map_rwlock_;

	// atomic ins team id
	//shared::AtomicInt32 cur_team_id_;
};

#define s_pInstanceWC gamed::InstanceCluster::GetInstance()

} // namespace gamed

#endif // GAMED_GS_SCENE_INSTANCE_INS_CLUSTER_H_
