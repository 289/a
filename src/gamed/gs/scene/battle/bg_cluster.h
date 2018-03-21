#ifndef GAMED_GS_SCENE_BATTLE_BG_CLUSTER_H_
#define GAMED_GS_SCENE_BATTLE_BG_CLUSTER_H_

#include <map>

#include "gs/scene/world_cluster.h"


namespace gamed {

class BGController;

/**
 * @brief battle-ground cluster
 *   （1）战场的cluster
 */
class BGCluster : public shared::Singleton<BGCluster>, public WorldCluster
{
	friend class shared::Singleton<BGCluster>;
public:
    static inline BGCluster* GetInstance() {
		return &(get_mutable_instance());
	}

	// ---- thread safe ----
    bool    CreateController(shared::Conf* pconf, int32_t world_id, const char* world_name);
    bool    CreateWorldMan(int32_t world_id, int64_t bg_serial_num, int32_t bg_templ_id, world::battleground_info& ret_info);
    bool    FindBGWorldMan(const world::battleground_info& info, world::battleground_info& ret_info);
    void    RemoveBGWorldMan(int32_t world_id, int32_t world_tag);
    void    CloseBGWorldMan(int32_t world_id, int32_t world_tag);

    // ---- thread safe ----
    virtual WorldManager* FindWorldManager(MapID world_id, MapTag world_tag);
    virtual void          GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec);

	// ---- thread safe ----
    void    EnableMapElem(int32_t world_id, int32_t elem_id);
    void    DisableMapElem(int32_t world_id, int32_t elem_id);


protected:
	BGCluster();
	virtual ~BGCluster();

private:
    typedef std::map<MapID, BGController*> BGControllerMap;
	BGControllerMap  bg_ctrl_map_;
	shared::RWLock   bg_ctrl_map_rwlock_;
};

#define s_pBGCluster gamed::BGCluster::GetInstance()

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_CLUSTER_H_
