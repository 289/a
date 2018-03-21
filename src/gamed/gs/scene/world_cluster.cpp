#include "world_cluster.h"

#include "shared/logsys/logging.h"

#include "gs/global/gmatrix.h"
#include "gs/global/game_def.h"
#include "gs/global/obj_manager.h"
#include "gs/global/dbgprt.h"
#include "gs/scene/instance/ins_cluster.h"
#include "gs/scene/instance/ins_script_if.h"
#include "gs/scene/battle/bg_cluster.h"
#include "gs/scene/battle/bg_script_if.h"

#include "world_man.h"


namespace gamed {

static const int kWaitingRemoveDelay = 60 * 3;  // 单位s

static ObjectAllocator<WorldManager> s_worldman_pool(kMaxWorldManagerCount);
static int64_t   s_tick_counter = 0;

///
/// wcluster
///
namespace wcluster {

int CreateWorldManager(shared::Conf* pconf, const char* world_name)
{
	Conf::section_type section = "World_";
	section += world_name;
	MapID world_id = pconf->get_int_value(section, "world_id");

	///
	/// init start
	///
	__PRINTF("-------------------- 开始初始化世界 %s --------------------", world_name);

	///
	/// 得到当前地图的基础根目录
	///
	std::string mapdatafile = Gmatrix::GetMapDataDir();
	mapdatafile += "mapdata_" + itos(world_id) + ".gbd";
	__PRINTF("%s资源文件全路径: %s", section.c_str(), mapdatafile.c_str());
	if (!Gmatrix::LoadMapDataGbd(mapdatafile.c_str()))
	{
		__PRINTF("map:%s - LoadMapData() failure!", section.c_str());
		return -1;
	}

	///
	/// Create WorldManager
	///
	if (IS_INS_MAP(world_id))
	{
		if (!s_pInstanceWC->CreateController(pconf, world_id, world_name))
		{
			__PRINTF("副本世界%d初始化失败", world_id);
			return -1;
		}
	}
    else if (IS_BG_MAP(world_id))
    {
        if (!s_pBGCluster->CreateController(pconf, world_id, world_name))
        {
            __PRINTF("战场世界%d初始化失败", world_id);
            return -1;
        }
    }
	else // normal world
	{
		WorldManager* gwm = AllocWorldManager();
		bool ret = gwm->Init(pconf, world_name, WorldManager::NORMAL_WORLD, 0);
		if (!ret)
		{
			__PRINTF("世界%s初始化失败%d", world_name, ret);
			FreeWorldManager(gwm);
			return -1;
		}

		int res = InsertWorldManager(gwm->GetWorldID(), gwm, gwm->GetWorldTag());
		if (res)
		{
			__PRINTF("世界%s Insert失败%d", world_name, res);
			FreeWorldManager(gwm);
			return -1;
		}
	}

	return 0;
}

int InsertWorldManager(MapID id, WorldManager* manager, MapTag tag)
{
	if (IS_INS_MAP(id))
	{
		return s_pInstanceWC->InsertWorldManager(id, manager, tag);
	}
    else if (IS_BG_MAP(id))
    {
        return s_pBGCluster->InsertWorldManager(id, manager, tag);
    }
	else
	{
		return s_pNormalWC->InsertWorldManager(id, manager, tag);
	}

	return -1;
}

void RemoveWorldManager(WorldManager* manager)
{
	if (IS_INS_MAP(manager->GetWorldID()))
	{
		s_pInstanceWC->RemoveWorldManager(manager);
	}
    else if (IS_BG_MAP(manager->GetWorldID()))
    {
        s_pBGCluster->RemoveWorldManager(manager);
    }
	else
	{
		s_pNormalWC->RemoveWorldManager(manager);
	}
}

WorldManager* FindWorldManager(MapID world_id, MapTag world_tag)
{
	if (IS_INS_MAP(world_id))
	{
		return s_pInstanceWC->FindWorldManager(world_id, world_tag);
	}
    else if (IS_BG_MAP(world_id))
    {
        return s_pBGCluster->FindWorldManager(world_id, world_tag);
    }
	else
	{
		return s_pNormalWC->FindWorldManager(world_id, world_tag);
	}

	return NULL;
}

WorldManager* AllocWorldManager()
{
	return s_worldman_pool.Alloc();
}

void FreeWorldManager(WorldManager* manager)
{
	s_worldman_pool.Free(manager);
}

bool CreateInsWorld(int32_t world_id, int64_t ins_serial_num, int32_t ins_templ_id, world::instance_info& ret_info)
{
	return s_pInstanceWC->CreateWorldMan(world_id, ins_serial_num, ins_templ_id, ret_info);
}

bool FindInsWorld(const world::instance_info& info, world::instance_info& ret_info)
{
	return s_pInstanceWC->FindInsWorldMan(info, ret_info);
}

bool CreateBGWorld(int32_t world_id, int64_t bg_serial_num, int32_t bg_templ_id, world::battleground_info& ret_info)
{
    return s_pBGCluster->CreateWorldMan(world_id, bg_serial_num, bg_templ_id, ret_info);
}

bool FindBGWorld(const world::battleground_info& info, world::battleground_info& ret_info)
{
    return s_pBGCluster->FindBGWorldMan(info, ret_info);
}

void EnableMapElem(int32_t world_id, int32_t elem_id)
{
	if (IS_NORMAL_MAP(world_id))
	{
		MSG msg;
		XID target = MakeWorldXID(world_id, 0);
		BuildMessage(msg, GS_PLANE_MSG_ENABLE_MAP_ELEM, target, XID(), elem_id);
		Gmatrix::SendWorldMsg(msg);
	}
	else if (IS_INS_MAP(world_id))
	{
		s_pInstanceWC->EnableMapElem(world_id, elem_id);
	}
    else if (IS_BG_MAP(world_id))
    {
        s_pBGCluster->EnableMapElem(world_id, elem_id);
    }
}

void DisableMapElem(int32_t world_id, int32_t elem_id)
{
	if (IS_NORMAL_MAP(world_id))
	{
		MSG msg;
		XID target = MakeWorldXID(world_id, 0);
		BuildMessage(msg, GS_PLANE_MSG_DISABLE_MAP_ELEM, target, XID(), elem_id);
		Gmatrix::SendWorldMsg(msg);
	}
	else if (IS_INS_MAP(world_id))
	{
		s_pInstanceWC->DisableMapElem(world_id, elem_id);
	}
    else if (IS_BG_MAP(world_id))
    {
        s_pBGCluster->DisableMapElem(world_id, elem_id);
    }
}

void GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec)
{
	s_pNormalWC->GetAllWorldInCharge(id_vec);
	s_pInstanceWC->GetAllWorldInCharge(id_vec);
    s_pBGCluster->GetAllWorldInCharge(id_vec);
}

void ReleaseAll()
{
	s_pNormalWC->ReleaseAll();
	s_pInstanceWC->ReleaseAll();
    s_pBGCluster->ReleaseAll();
}

void HeartbeatTick()
{
	s_pNormalWC->HeartbeatTick();
	s_pInstanceWC->HeartbeatTick();
    s_pBGCluster->HeartbeatTick();
}

void InitInsScriptSys(const char* script_dir)
{
    InsScriptIf::InitScriptSys(script_dir);
}

void InitBGScriptSys(const char* script_dir)
{
    BGScriptIf::InitScriptSys(script_dir);
}

} // namespace wcluster


///
/// WorldManHeartbeatInfo
///
void WorldCluster::WorldManHeartbeatInfo::Reset(const WorldManSet& worldManMap) 
{
	heart_count  = 0;
	cur_worldMan = NULL;
	cur_cursor   = worldManMap.begin();
}


///
/// WorldCluster
///
WorldCluster::WorldCluster()
{
	wm_heartbeat_info_.Reset(worldMan_set_);
}

WorldCluster::~WorldCluster()
{
	wm_heartbeat_info_.Reset(worldMan_set_);
}

void WorldCluster::ReleaseAll()
{
	//
	// 删除所有地图
	//
	{
		MutexLockGuard lock(lock_wm_set_);
		WorldManSet::iterator it_worldman = worldMan_set_.begin();
		for (; it_worldman != worldMan_set_.end(); ++it_worldman)
		{
			s_worldman_pool.Free(*it_worldman);
		}
		worldMan_set_.clear();
	}

	{
		MutexLockGuard lock(lock_change_);
		ChangeMap::iterator it_change = change_list_.begin();
		for (; it_change != change_list_.end(); ++it_change)
		{
			s_worldman_pool.Free(it_change->first);
		}
		change_list_.clear();
	}
}

WorldManager* WorldCluster::FindWorldManager(MapID world_id, MapTag world_tag)
{
	int64_t map_id = MakeWorldID(world_id, world_tag);
	size_t index   = std::numeric_limits<size_t>::max();

	// lock
	{
		RWLockReadGuard rdlock(wid_idx_map_rwlock_);
		WorldIdToIndexMap::const_iterator it = wid_mapto_index_.find(map_id);
		if (it == wid_mapto_index_.end())
			return NULL;
		index = it->second;
	}

	WorldManager* wman = s_worldman_pool.GetByIndex(index);
	if (wman != NULL && (!wman->IsActived()))
	{
		return NULL;
	}

	return wman;
}

int WorldCluster::InsertWorldManager(MapID id, WorldManager* manager, MapTag tag)
{
	int64_t map_id = MakeWorldID(id, tag);
	ASSERT(id == manager->GetWorldID() && tag == manager->GetWorldTag() && map_id == manager->GetWorldXID().id);

	// change lock
	{
		MutexLockGuard lock(lock_change_);
		change_list_[manager] ++;
	}

	// query lock
	{
		size_t wm_index = s_worldman_pool.GetIndex(manager);
		RWLockWriteGuard wrlock(wid_idx_map_rwlock_);
		if (!wid_mapto_index_.insert(std::pair<int64_t, size_t>(map_id, wm_index)).second)
		{
			LOG_ERROR << "插入相同的world_id至wid_mapto_index_查找表 WorldManager id:" << id << " tag:" << tag;
			ASSERT(false);
			return -1;
		}
	}

	manager->SetActive();
	return 0;
}

void WorldCluster::RemoveWorldManager(WorldManager* manager)
{
    int32_t worldid    = manager->GetWorldID();
    int32_t worldtag   = manager->GetWorldTag();
	int64_t world64_id = manager->GetWorldXID().id;

	manager->ClrActive();

	// change lock
	{
		MutexLockGuard lock(lock_change_);
		waiting_remove_map_[manager] = kWaitingRemoveDelay;
	}

	// query lock
	{
		RWLockWriteGuard wrlock(wid_idx_map_rwlock_);
		size_t n = wid_mapto_index_.erase(world64_id);
		ASSERT(n == 1); (void)n;
	}

	// instance
	if (IS_INS_MAP(worldid))
	{
		s_pInstanceWC->RemoveInsWorldMan(worldid, worldtag);
	}
    else if (IS_BG_MAP(worldid))
    {
        s_pBGCluster->RemoveBGWorldMan(worldid, worldtag);
    }
}

void WorldCluster::GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec)
{
	// find from set
	{
		MutexLockGuard lock(lock_wm_set_);
		WorldManSet::const_iterator it = worldMan_set_.begin();
		for (; it != worldMan_set_.end(); ++it)
		{
			gmatrixdef::WorldIDInfo info;
			info.world_id  = (*it)->GetWorldID();
			info.world_tag = (*it)->GetWorldTag();
			id_vec.push_back(info);
		}
	}

	// find from change list
	{
		MutexLockGuard lock(lock_change_);
		ChangeMap::const_iterator it = change_list_.begin();
		for (; it != change_list_.end(); ++it)
		{
			if (it->second >= 0)
			{
				gmatrixdef::WorldIDInfo info;
				info.world_id  = it->first->GetWorldID();
				info.world_tag = it->first->GetWorldTag();
				id_vec.push_back(info);
			}
		}
	}
}

void WorldCluster::DoChange()
{
	MutexLockGuard lock(lock_change_);
	ChangeMap::const_iterator it = change_list_.begin();
	for (; it != change_list_.end(); ++it)
	{
		if (!it->second) continue;
		ASSERT(it->second == -1 || it->second == 1);
		if (it->second > 0)
		{
			DoInsert(it->first);
		}
		else
		{
			DoDelete(it->first);
		}
	}
	change_list_.clear();
}

void WorldCluster::DoInsert(WorldManager* manager)
{
	if (!worldMan_set_.insert(manager).second)
	{
		LOG_ERROR << "插入相同world_id的WorldManager id:" << manager->GetWorldID() 
			<< " tag:" << manager->GetWorldTag();
		ASSERT(false);
	}
}

void WorldCluster::DoDelete(WorldManager* manager)
{
	if (wm_heartbeat_info_.cur_worldMan == manager)
	{
		wm_heartbeat_info_.cur_cursor = worldMan_set_.find(manager);
		++(wm_heartbeat_info_.cur_cursor);

		if (wm_heartbeat_info_.cur_cursor == worldMan_set_.end())
		{
			wm_heartbeat_info_.cur_worldMan = NULL;
		}
		else
		{
			wm_heartbeat_info_.cur_worldMan = *(wm_heartbeat_info_.cur_cursor);
		}
	}

	size_t n = worldMan_set_.erase(manager);	
	(void)n; ASSERT(n == 1);

    // free and give back to pool
    wcluster::FreeWorldManager(manager);
}

// needed lock outside
void WorldCluster::CollectHeartbeat(std::vector<WorldManager*>& list)
{
	DoChange();

	int size = worldMan_set_.size();

	int& heart_count                        = wm_heartbeat_info_.heart_count;
	WorldManager*& cur_worldMan             = wm_heartbeat_info_.cur_worldMan;
	WorldManSet::const_iterator& cur_cursor = wm_heartbeat_info_.cur_cursor;

	heart_count += size;
	if (size == 0)
	{
		heart_count  = 0;
		cur_worldMan = NULL;
		return;
	}

	WorldManSet::const_iterator end = worldMan_set_.end();
	if (NULL == cur_worldMan)
	{
		cur_cursor = end;
	}
	else
	{
		cur_cursor = worldMan_set_.find(cur_worldMan);
	}

	for (; heart_count >= kWorldHeartbeatInterval; heart_count -= kWorldHeartbeatInterval)
	{
		if (cur_cursor == end)
		{
			cur_cursor = worldMan_set_.begin();
		}

		cur_worldMan = *cur_cursor;

		if (cur_worldMan->IsActived())
		{
			// 将这个WorldManager加入到收集列表中
			list.push_back(cur_worldMan);
		}

		++cur_cursor;
	}

	if (cur_cursor != end)
	{
		cur_worldMan = *cur_cursor;
	}
	else 
	{
		cur_worldMan = NULL;
	}
}

void WorldCluster::HeartbeatTick()
{
	HandleWaitingRemove();

	wm_hb_vec_.clear();

	// mutex lock worldManager map
	{
		MutexLockGuard lock(lock_wm_set_);
		CollectHeartbeat(wm_hb_vec_);
	}

	for (size_t i = 0; i < wm_hb_vec_.size(); ++i)
	{
		wm_hb_vec_[i]->Heartbeat();
	}
}

void WorldCluster::HandleWaitingRemove()
{
	if (((++s_tick_counter) % TICK_PER_SEC) == 0)
	{
		MutexLockGuard lock(lock_change_);
		WaitingRemoveMap::iterator it = waiting_remove_map_.begin();
		while (it != waiting_remove_map_.end())
		{
			int countdown = --(it->second);
			if (countdown <= 0)
			{
				change_list_[it->first] --;
				waiting_remove_map_.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}
}

} // namespace gamed
