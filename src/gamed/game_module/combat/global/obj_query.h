#ifndef __GAME_MODULE_COMBAT_OBJ_QUERY_H__
#define __GAME_MODULE_COMBAT_OBJ_QUERY_H__

#include <map>
#include "shared/base/rwlock.h"
#include "combat_types.h"


namespace combat {

class ObjQuery
{
public:
    static bool GetPlayerIndex(RoleID role_id, size_t& index);
    static bool MapPlayer(RoleID role_id, size_t index);
    static int  UnmapPlayer(RoleID role_id);

    static bool GetWorldBossIndex(ObjectID obj_id, size_t& index);
    static bool MapWorldBoss(ObjectID obj_id, size_t index);
    static int  UnmapWorldBoss(ObjectID obj_id);

private:
    typedef std::map<RoleID, size_t> RoleIdToIndexMap;
    static RoleIdToIndexMap s_roleid_mapto_index_;
    static shared::RWLock   s_playermap_rwlock_;

    typedef std::map<ObjectID, size_t> ObjectIdToIndexMap;
    static ObjectIdToIndexMap s_worldboss_mapto_index_;
    static shared::RWLock     s_wbmap_rwlock_;
};

} // namespace combat

#endif // __GAME_MODULE_COMBAT_OBJ_QUERY_H__
