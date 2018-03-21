#include "obj_data_pool.h"


namespace gamed {

using namespace common;

///
/// static member
///
ObjectDataManager<RoleID, PlayerData, kMaxPlayerCount> ObjDataPool::s_player_data_manager_(kMaxPlayerCount/2);

ObjDataPool::ObjDataPool()
{
}

ObjDataPool::~ObjDataPool()
{
}

} // namespace gamed
