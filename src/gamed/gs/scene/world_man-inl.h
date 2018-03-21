#ifndef GAMED_GS_SCENE_WORLD_MANAGER_INL_H_
#define GAMED_GS_SCENE_WORLD_MANAGER_INL_H_

///
/// inline function
///
inline MapID WorldManager::GetWorldID() const
{
	return plane_->world_id();
}
	
inline MapTag WorldManager::GetWorldTag() const
{
	return plane_->world_tag();
}

inline XID WorldManager::GetWorldXID() const
{
	return plane_->world_xid();
}

inline bool WorldManager::PosInWorld(const A2DVECTOR& pos) const
{
	return plane_->PosInWorld(pos);
}

inline bool WorldManager::IsWalkablePos(const A2DVECTOR& pos) const
{
	return plane_->IsWalkablePos(pos);
}

inline bool WorldManager::IsObjGenOwner() const
{
	return (int64_t)this == obj_gen_owner_;
}

inline bool WorldManager::SetInsInfo(const world::instance_info& info)
{
	return plane_->SetInsInfo(info);
}

inline bool WorldManager::GetInsInfo(world::instance_info& info) const
{
	return plane_->GetInsInfo(info);
}

inline bool WorldManager::SetBGInfo(const world::battleground_info& info)
{
    return plane_->SetBGInfo(info);
}

inline bool WorldManager::GetBGInfo(world::battleground_info& info) const
{
    return plane_->GetBGInfo(info);
}

inline void WorldManager::SetMapTeamFlag()
{
    plane_->SetMapTeamFlag();
}

inline void WorldManager::SetMapCounterFlag()
{
    plane_->SetMapCounterFlag();
}

inline void WorldManager::SetMapGatherFlag()
{
    plane_->SetMapGatherFlag();
}

inline bool WorldManager::IsClosed() const
{
    return is_forbid_obj_insert_;
}

inline const World* WorldManager::plane() const
{
	return plane_;
}

#endif // GAMED_GS_SCENE_WORLD_MANAGER_INL_H_
