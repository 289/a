#include "slice.h"

#include "gs/player/player.h"
#include "gs/obj/npc.h"
#include "gs/obj/area.h"
#include "gs/obj/matter.h"


namespace gamed {

Slice::Slice()
{
}

Slice::~Slice()
{
}

bool Slice::insertObject(const WorldObject* pobject, ObjectSet& object_set)
{
	std::pair<ObjectSet::iterator, bool> ret;
	ret = object_set.insert(pobject);
	if (ret.second == false) {
		return false;
	}
	return true;
}

int Slice::removeObject(const WorldObject* pobject, ObjectSet& object_set)
{
	int ret = object_set.erase(pobject);
	return ret;
}

void Slice::InsertPlayer(const Player* pplayer)
{
	MutexLockTimedGuard lock(player_mutex_);
	bool ret = insertObject(static_cast<const WorldObject*>(pplayer), player_set_);
	if (ret == false)
	{
		LOG_ERROR << "can't insert player-" << pplayer->role_id()
			<< "into slice, because it has already in this slice.";
	}
}

void Slice::RemovePlayer(const Player* pplayer)
{
	MutexLockTimedGuard lock(player_mutex_);
	int ret = removeObject(static_cast<const WorldObject*>(pplayer), player_set_);
	Assert(ret == 1);
}

void Slice::InsertNPC(const Npc* pnpc)
{
	MutexLockTimedGuard lock(npc_mutex_);
	bool ret = insertObject(static_cast<const WorldObject*>(pnpc), npc_set_);
	if (ret == false)
	{
		LOG_ERROR << "can't insert npc-" << pnpc->object_id() 
			<< "into slice, because it has already in this slice.";
		return;
	}
}

void Slice::RemoveNPC(const Npc* pnpc)
{
	MutexLockTimedGuard lock(npc_mutex_);
	int ret = removeObject(static_cast<const WorldObject*>(pnpc), npc_set_);
	Assert(ret == 1);
}

void Slice::InsertAreaObj(const AreaObj* parea)
{
	MutexLockTimedGuard lock(area_mutex_);
	bool ret = insertObject(static_cast<const WorldObject*>(parea), area_set_);
	if (ret == false)
	{
		LOG_ERROR << "can't insert area-" << parea->object_id() 
			<< "into slice, because it has already in this slice.";
		return;
	}
}

void Slice::RemoveAreaObj(const AreaObj* parea)
{
	MutexLockTimedGuard lock(area_mutex_);
	int ret = removeObject(static_cast<const WorldObject*>(parea), area_set_);
	Assert(ret == 1);
}

void Slice::InsertMatter(const Matter* pmatter)
{
	MutexLockTimedGuard lock(matter_mutex_);
	bool ret = insertObject(static_cast<const WorldObject*>(pmatter), matter_set_);
	if (ret == false)
	{
		LOG_ERROR << "can't insert matter-" << pmatter->object_id() 
			<< "into slice, because it has already in this slice.";
		return;
	}
}

void Slice::RemoveMatter(const Matter* pmatter)
{
	MutexLockTimedGuard lock(matter_mutex_);
	int ret = removeObject(static_cast<const WorldObject*>(pmatter), matter_set_);
	Assert(ret == 1);
}

int Slice::MovePlayer(const Player* pplayer, Slice* dest)
{
	MutexLockTimedGuard lock(player_mutex_);
	removeObject(pplayer, player_set_);
	dest->InsertPlayer(pplayer);
	return 0;
}
	
int Slice::MoveNPC(const Npc* pnpc, Slice* dest)
{
	MutexLockTimedGuard lock(npc_mutex_);
	removeObject(pnpc, npc_set_);
	dest->InsertNPC(pnpc);
	return 0;
}

} // namespace gamed
