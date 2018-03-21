#include "bg_cluster.h"

#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/scene/world_man.h"

#include "bg_controller.h"


namespace gamed {

BGCluster::BGCluster()
{
}

BGCluster::~BGCluster()
{
    RWLockWriteGuard wrlock(bg_ctrl_map_rwlock_);
	BGControllerMap::iterator it = bg_ctrl_map_.begin();
	for (; it != bg_ctrl_map_.end(); ++it)
	{
		DELETE_SET_NULL(it->second);
	}
	bg_ctrl_map_.clear();
}

bool BGCluster::CreateController(shared::Conf* pconf, int32_t world_id, const char* world_name)
{
	RWLockWriteGuard wrlock(bg_ctrl_map_rwlock_);
	BGControllerMap::iterator it = bg_ctrl_map_.find(world_id);
	if (it != bg_ctrl_map_.end())
	{
		__PRINTF("战场地图：%d 重复创建Controller！", world_id);
		return false;
	}

	BGController* bg_ctrl = new BGController();
	if (!bg_ctrl->Init(world_id, pconf, world_name))
	{
		__PRINTF("战场地图：%d 控制器初始化失败！", world_id);
		DELETE_SET_NULL(bg_ctrl);
		return false;
	}

	ASSERT(bg_ctrl_map_.insert(std::make_pair(world_id, bg_ctrl)).second);
	__PRINTF("战场控制器创建成功%s , 战场地图id:%d", world_name, world_id);
	return true;
}

bool BGCluster::CreateWorldMan(int32_t world_id, int64_t bg_serial_num, int32_t bg_templ_id, world::battleground_info& ret_info)
{
    RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(world_id);
	if (it == bg_ctrl_map_.end())
	{
		return false;
	}

	WorldManager* new_worldman = it->second->CreateWorldMan(bg_serial_num, bg_templ_id);
	if (new_worldman == NULL)
	{
		return false;
	}

	ASSERT(new_worldman->GetBGInfo(ret_info));
	return true;
}

bool BGCluster::FindBGWorldMan(const world::battleground_info& info, world::battleground_info& ret_info)
{
	RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(info.world_id);
	if (it == bg_ctrl_map_.end())
	{
		return false;
	}

	return it->second->FindWorldMan(info, ret_info);
}

void BGCluster::RemoveBGWorldMan(int32_t world_id, int32_t world_tag)
{
    RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(world_id);
	if (it == bg_ctrl_map_.end())
	{
		return;
	}

	it->second->RemoveBGWorldMan(world_id, world_tag);
}

void BGCluster::CloseBGWorldMan(int32_t world_id, int32_t world_tag)
{
    RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(world_id);
	if (it == bg_ctrl_map_.end())
	{
		return;
	}

	it->second->CloseBGWorldMan(world_id, world_tag);
}

WorldManager* BGCluster::FindWorldManager(MapID world_id, MapTag world_tag)
{
    RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(world_id);
	if (it == bg_ctrl_map_.end())
	{
		return NULL;
	}
	if (!it->second->IsActiveBG(world_id, world_tag))
	{
		return NULL;
	}
	return WorldCluster::FindWorldManager(world_id, world_tag);
}

void BGCluster::GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec)
{
    RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.begin();
	for (; it != bg_ctrl_map_.end(); ++it)	
	{
		gmatrixdef::WorldIDInfo info;
		info.world_id  = Gmatrix::GetRealWorldID(it->first);
		info.world_tag = 0;
		id_vec.push_back(info);
	}
}

void BGCluster::EnableMapElem(int32_t world_id, int32_t elem_id)
{
	RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(world_id);
	if (it == bg_ctrl_map_.end())
		return; // not found

	it->second->EnableMapElem(elem_id);	
}

void BGCluster::DisableMapElem(int32_t world_id, int32_t elem_id)
{
	RWLockReadGuard rdlock(bg_ctrl_map_rwlock_);
	BGControllerMap::const_iterator it = bg_ctrl_map_.find(world_id);
	if (it == bg_ctrl_map_.end())
		return; // not found

	it->second->DisableMapElem(elem_id);
}

} // namespace gamed
