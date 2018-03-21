#include "obj_query.h"


namespace combat {

using namespace shared;

ObjQuery::RoleIdToIndexMap ObjQuery::s_roleid_mapto_index_;
RWLock ObjQuery::s_playermap_rwlock_;

ObjQuery::ObjectIdToIndexMap ObjQuery::s_worldboss_mapto_index_;
RWLock ObjQuery::s_wbmap_rwlock_;

///
/// ObjQuery
///
bool ObjQuery::GetPlayerIndex(RoleID role_id, size_t& index)
{
    RWLockReadGuard rdlock(s_playermap_rwlock_);
    RoleIdToIndexMap::const_iterator it = s_roleid_mapto_index_.find(role_id);
    if (it == s_roleid_mapto_index_.end())
        return false;

    index = it->second;
    return true;
}

bool ObjQuery::MapPlayer(RoleID role_id, size_t index)
{
    RWLockWriteGuard wrlock(s_playermap_rwlock_);
    return s_roleid_mapto_index_.insert(std::pair<RoleID, size_t>(role_id, index)).second;
}

int ObjQuery::UnmapPlayer(RoleID role_id)
{
    RWLockWriteGuard wrlock(s_playermap_rwlock_);
    return s_roleid_mapto_index_.erase(role_id);
}

bool ObjQuery::GetWorldBossIndex(ObjectID obj_id, size_t& index)
{
    RWLockReadGuard rdlock(s_wbmap_rwlock_);
    ObjectIdToIndexMap::const_iterator it = s_worldboss_mapto_index_.find(obj_id);
    if (it == s_worldboss_mapto_index_.end())
        return false;

    index = it->second;
    return true;
}

bool ObjQuery::MapWorldBoss(ObjectID obj_id, size_t index)
{
    RWLockWriteGuard wrlock(s_wbmap_rwlock_);
    return s_worldboss_mapto_index_.insert(std::pair<ObjectID, size_t>(obj_id, index)).second;
}

int ObjQuery::UnmapWorldBoss(ObjectID obj_id)
{
    RWLockWriteGuard wrlock(s_wbmap_rwlock_);
    return s_worldboss_mapto_index_.erase(obj_id);
}

} // namespace combat
