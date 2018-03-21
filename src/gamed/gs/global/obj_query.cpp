#include "obj_query.h"

#include "gs/player/player.h"
#include "gs/obj/area.h"
#include "gs/obj/npc.h"
#include "gs/obj/matter.h"

#include "gmatrix.h"


namespace gamed {

using namespace shared;

ObjQuery::RoleIdToIndexMap ObjQuery::s_roleid_mapto_index_;
RWLock ObjQuery::s_pmap_rwlock_;

Player* ObjQuery::FindPlayer(RoleID role_id)
{
	size_t index = std::numeric_limits<size_t>::max();
	{
		RWLockReadGuard rdlock(s_pmap_rwlock_);
		RoleIdToIndexMap::const_iterator it = s_roleid_mapto_index_.find(role_id);
		if (it == s_roleid_mapto_index_.end())
			return NULL;
		index = it->second;
	}
	return Gmatrix::s_player_manager_.GetByIndex(index);
}

Npc* ObjQuery::FindNpc(int64_t obj_id)
{
	size_t index = ID2IDX(obj_id);
	return Gmatrix::s_npc_manager_.GetByIndex(index);
}

AreaObj* ObjQuery::FindAreaObj(int64_t obj_id)
{
	size_t index = ID2IDX(obj_id);
	return Gmatrix::s_areaobj_manager_.GetByIndex(index);
}

Matter* ObjQuery::FindMatter(int64_t obj_id)
{
	size_t index = ID2IDX(obj_id);
	return Gmatrix::s_matter_manager_.GetByIndex(index);
}

bool ObjQuery::MapPlayer(RoleID role_id, size_t index)
{
	RWLockWriteGuard wrlock(s_pmap_rwlock_);
	return s_roleid_mapto_index_.insert(std::pair<RoleID, size_t>(role_id, index)).second;
}

int ObjQuery::UnmapPlayer(RoleID role_id)
{
	RWLockWriteGuard wrlock(s_pmap_rwlock_);
	return s_roleid_mapto_index_.erase(role_id);
}

} // namespace gamed
