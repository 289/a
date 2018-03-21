#include "ins_cluster.h"

#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/scene/world_man.h"

#include "ins_controller.h"


namespace gamed {

using namespace shared;

InstanceCluster::InstanceCluster()
{
}

InstanceCluster::~InstanceCluster()
{
	RWLockWriteGuard wrlock(ins_ctrl_map_rwlock_);
	InsControllerMap::iterator it = ins_ctrl_map_.begin();
	for (; it != ins_ctrl_map_.end(); ++it)
	{
		DELETE_SET_NULL(it->second);
	}
	ins_ctrl_map_.clear();
}

bool InstanceCluster::CreateController(shared::Conf* pconf, int32_t world_id, const char* world_name)
{
	RWLockWriteGuard wrlock(ins_ctrl_map_rwlock_);
	InsControllerMap::iterator it = ins_ctrl_map_.find(world_id);
	if (it != ins_ctrl_map_.end())
	{
		__PRINTF("副本地图：%d 重复创建Controller！", world_id);
		return false;
	}

	InsController* ins_ctrl = new InsController();
	if (!ins_ctrl->Init(world_id, pconf, world_name))
	{
		__PRINTF("副本地图：%d 控制器初始化失败！", world_id);
		DELETE_SET_NULL(ins_ctrl);
		return false;
	}

	ASSERT(ins_ctrl_map_.insert(std::make_pair(world_id, ins_ctrl)).second);
	__PRINTF("副本控制器创建成功%s , 副本地图id:%d", world_name, world_id);
	return true;
}

bool InstanceCluster::CreateWorldMan(int32_t world_id, int64_t ins_serial_num, int32_t ins_templ_id, world::instance_info& ret_info)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(world_id);
	if (it == ins_ctrl_map_.end())
	{
		return false;
	}

	WorldManager* new_worldman = it->second->CreateWorldMan(ins_serial_num, ins_templ_id);
	if (new_worldman == NULL)
	{
		return false;
	}

	ASSERT(new_worldman->GetInsInfo(ret_info));
	return true;
}

bool InstanceCluster::FindInsWorldMan(const world::instance_info& info, world::instance_info& ret_info)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(info.world_id);
	if (it == ins_ctrl_map_.end())
	{
		return false;
	}

	return it->second->FindWorldMan(info, ret_info);
}

void InstanceCluster::RemoveInsWorldMan(int32_t world_id, int32_t world_tag)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(world_id);
	if (it == ins_ctrl_map_.end())
	{
		return;
	}

	it->second->RemoveInsWorldMan(world_id, world_tag);
}

void InstanceCluster::CloseInsWorldMan(int32_t world_id, int32_t world_tag)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(world_id);
	if (it == ins_ctrl_map_.end())
	{
		return;
	}

	it->second->CloseInsWorldMan(world_id, world_tag);
}

WorldManager* InstanceCluster::FindWorldManager(MapID world_id, MapTag world_tag)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(world_id);
	if (it == ins_ctrl_map_.end())
	{
		return NULL;
	}
	if (!it->second->IsActiveIns(world_id, world_tag))
	{
		return NULL;
	}
	return WorldCluster::FindWorldManager(world_id, world_tag);
}

void InstanceCluster::GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.begin();
	for (; it != ins_ctrl_map_.end(); ++it)	
	{
		gmatrixdef::WorldIDInfo info;
		info.world_id  = Gmatrix::GetRealWorldID(it->first);
		info.world_tag = 0;
		id_vec.push_back(info);
	}
}

void InstanceCluster::EnableMapElem(int32_t world_id, int32_t elem_id)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(world_id);
	if (it == ins_ctrl_map_.end())
		return; // not found

	it->second->EnableMapElem(elem_id);	
}

void InstanceCluster::DisableMapElem(int32_t world_id, int32_t elem_id)
{
	RWLockReadGuard rdlock(ins_ctrl_map_rwlock_);
	InsControllerMap::const_iterator it = ins_ctrl_map_.find(world_id);
	if (it == ins_ctrl_map_.end())
		return; // not found

	it->second->DisableMapElem(elem_id);
}

/*
int32_t InstanceCluster::GetNextInsTeamId()
{
	return cur_team_id_.increment_and_get();
}
*/

} // namespace gamed
