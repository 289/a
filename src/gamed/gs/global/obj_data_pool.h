#ifndef GAMED_GS_GLOBAL_OBJ_DATA_POOL_H_
#define GAMED_GS_GLOBAL_OBJ_DATA_POOL_H_

#include "shared/base/base_define.h"
#include "common/obj_data/player_data.h"

#include "gs/global/game_def.h"


namespace gamed {

using namespace common;

/**
 * @brief 该文件暂时没有使用
 */
class ObjDataPool
{
	friend class PlayerReadOnlyGuard;
public:
	static inline PlayerData* AttachPlayer(RoleID roleid);
	static inline int    DetachPlayer(RoleID roleid);

  // 获取ReadOnly指针请使用下面的Guard类
private:
	static inline const PlayerData* AttachPlayerReadOnly(RoleID roleid);
	static inline int    DetachPlayerReadOnly(RoleID roleid);
	static inline int    DetachPlayerReadOnly(PlayerData* p);

	ObjDataPool();
	~ObjDataPool();


private:
	static ObjectDataManager<RoleID, PlayerData, kMaxPlayerCount> s_player_data_manager_;
};

class PlayerReadOnlyGuard
{
public:
	PlayerReadOnlyGuard()
	{ }

	const PlayerData* AttachReadOnly(RoleID roleid)
	{
		roleid_ = roleid;
		const PlayerData* playerdata = ObjDataPool::AttachPlayerReadOnly(roleid_);
		return playerdata;	
	}

	~PlayerReadOnlyGuard()
	{
		ObjDataPool::DetachPlayerReadOnly(roleid_);
	}

private:
	RoleID roleid_;
};

///
/// inline function
///
inline PlayerData* ObjDataPool::AttachPlayer(RoleID roleid)
{
	return s_player_data_manager_.Attach(roleid);
}
	
inline int ObjDataPool::DetachPlayer(RoleID roleid)
{
	return s_player_data_manager_.Detach(roleid);
}

inline const PlayerData* ObjDataPool::AttachPlayerReadOnly(RoleID roleid)
{
	return s_player_data_manager_.AttachReadOnly(roleid);
}
	
inline int ObjDataPool::DetachPlayerReadOnly(RoleID roleid)
{
	return s_player_data_manager_.DetachReadOnly(roleid);
}
	
inline int ObjDataPool::DetachPlayerReadOnly(PlayerData* p)
{
	return s_player_data_manager_.DetachReadOnly(p);
}

} // namespace gamed

#endif // GAMED_GS_GLOBAL_OBJ_DATA_POOL_H_
