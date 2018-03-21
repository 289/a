#include "ins_controller.h"

#include "shared/base/conf.h"

#include "gs/global/dbgprt.h"
#include "gs/global/timer.h"
#include "gs/global/message.h"
#include "gs/global/gmatrix.h"
#include "gs/scene/world_man.h"
#include "gs/scene/world_cluster.h"
#include "gs/obj/npc_gen.h"
#include "gs/obj/area_gen.h"
#include "gs/obj/matter_gen.h"


namespace gamed {

///
/// static func
///
void InsController::SendMapElemEnable(const world::instance_info& info, int64_t elemid)
{
	MSG msg;
	XID target = MakeWorldXID(info.world_id, info.world_tag);
	BuildMessage(msg, GS_PLANE_MSG_ENABLE_MAP_ELEM, target, XID(), elemid);
	Gmatrix::SendWorldMsg(msg);
}
	
void InsController::SendMapElemDisable(const world::instance_info& info, int64_t elemid)
{
	MSG msg;
	XID target = MakeWorldXID(info.world_id, info.world_tag);
	BuildMessage(msg, GS_PLANE_MSG_DISABLE_MAP_ELEM, target, XID(), elemid);
	Gmatrix::SendWorldMsg(msg);
}

///
/// InsController
///
InsController::InsController()
	: ins_world_id_(-1),
	  pconf_(NULL)
{
}

InsController::~InsController()
{
	DELETE_SET_NULL(npc_gen_);
	DELETE_SET_NULL(area_gen_);
	DELETE_SET_NULL(matter_gen_);
}

bool InsController::Init(MapID world_id, shared::Conf* conf, const char* world_name)
{
	ins_world_id_ = world_id;
	pconf_        = conf;
	world_name_   = world_name;

	npc_gen_      = new NpcGenerator(world_id);
	area_gen_     = new AreaGenerator(world_id);
	matter_gen_   = new MatterGenerator(world_id);
	return true;
}

WorldManager* InsController::CreateWorldMan(int64_t ins_serial_num, int32_t ins_templ_id)
{
	WorldManager* gwm = wcluster::AllocWorldManager();
	if (gwm != NULL)
	{
		// create world
		if (!gwm->CreateWorldPlane())
		{
			__PRINTF("副本世界%s - CreateWorldPlane失败！", world_name_.c_str());
			wcluster::FreeWorldManager(gwm);
			return NULL;
		}

		// set gen
		gwm->SetObjGenerator(npc_gen_, area_gen_, matter_gen_, (int64_t)this);

		// set instance info
		world::instance_info ins_info;
		ins_info.ins_serial_num  = ins_serial_num;
		ins_info.ins_templ_id    = ins_templ_id;
		ins_info.world_id        = ins_world_id_;
		ins_info.world_tag       = get_new_tag_id();
		ins_info.ins_create_time = g_timer->GetSysTimeMsecs();
		ASSERT(gwm->SetInsInfo(ins_info));

		// init
		bool ret = gwm->Init(pconf_, world_name_.c_str(), WorldManager::INSTANCE_WORLD, ins_info.world_tag);
		if (!ret)
		{
			__PRINTF("副本世界%s初始化失败, world_id:%d", world_name_.c_str(), ins_world_id_);
			wcluster::FreeWorldManager(gwm);
			return NULL;
		}

		// insert to manager
		int res = wcluster::InsertWorldManager(gwm->GetWorldID(), gwm, gwm->GetWorldTag());
		if (res)
		{
			__PRINTF("世界%s Insert失败%d", world_name_.c_str(), res);
			wcluster::FreeWorldManager(gwm);
			return NULL;
		}

		ASSERT(ins_world_id_ == gwm->GetWorldID());
		ASSERT(ins_info.world_tag == gwm->GetWorldTag());
		
		// lock
		{
			RWLockWriteGuard wrlock(change_rwlock_);
			ASSERT(active_ins_map_.insert(std::make_pair(gwm->GetWorldXID().id, ins_info)).second);
		}

		// locked inside
		TraversalActiveMapElem(ins_info);
	}

	return gwm;
}

bool InsController::IsActiveIns(int32_t worldid, int32_t worldtag)
{
	if (worldid != ins_world_id_)
		return false;

	RWLockReadGuard rdlock(change_rwlock_);
	return is_active_ins(worldid, worldtag);
}

bool InsController::FindWorldMan(const world::instance_info& info, world::instance_info& ret_info)
{
	if (info.world_id != ins_world_id_)
		return false;

	RWLockReadGuard rdlock(change_rwlock_);
	int64_t world64_id = MakeWorldID(info.world_id, info.world_tag);
	ActiveInsMap::const_iterator it = active_ins_map_.find(world64_id);
	if (it != active_ins_map_.end())
	{
		if (it->second == info)
		{
			ret_info = it->second;
			return true;
		}
	}

	// not found
	if (info.world_tag == 0 && info.ins_create_time == 0)
	{
		ActiveInsMap::const_iterator it = active_ins_map_.begin();
		for (; it != active_ins_map_.end(); ++it)
		{
			if (it->second.ins_serial_num == info.ins_serial_num &&
				it->second.ins_templ_id == info.ins_templ_id)
			{
				ret_info = it->second;
				return true;
			}
		}
	}

	return false;
}

void InsController::RemoveInsWorldMan(int32_t world_id, int32_t world_tag)
{
	if (world_id != ins_world_id_)
		return;

	RWLockWriteGuard wrlock(change_rwlock_);
	int64_t world64_id = MakeWorldID(world_id, world_tag);
	size_t n = active_ins_map_.erase(world64_id);
	ASSERT(n == 1); (void)n;
}

void InsController::CloseInsWorldMan(int32_t world_id, int32_t world_tag)
{
    // 地图关闭，但还没有remove，这里还不能从active里移除，因为还需要Find这个WorldMan处理MSG
	if (world_id != ins_world_id_)
		return;

    // 暂时不需要处理
}

void InsController::EnableMapElem(int32_t elem_id)
{
	MutexLockGuard lock(mapelem_set_mutex_);
    deactive_mapelem_set_.erase(elem_id);

	ActiveMapElemSet::iterator it = active_mapelem_set_.find(elem_id);
	if (it != active_mapelem_set_.end())
		return; // found
	ASSERT(active_mapelem_set_.insert(elem_id).second);

	// foreach ins world, locked inside
	ForEachInsWorld(SendMapElemEnable, elem_id);
}

void InsController::DisableMapElem(int32_t elem_id)
{
	MutexLockGuard lock(mapelem_set_mutex_);
	active_mapelem_set_.erase(elem_id);

	ActiveMapElemSet::iterator it = deactive_mapelem_set_.find(elem_id);
	if (it != deactive_mapelem_set_.end())
		return; // not found
    ASSERT(deactive_mapelem_set_.insert(elem_id).second);

	// foreach ins world, locked inside
	ForEachInsWorld(SendMapElemDisable, elem_id);
}

void InsController::TraversalActiveMapElem(const world::instance_info& info)
{
	MutexLockGuard lock(mapelem_set_mutex_);
	ActiveMapElemSet::iterator it = active_mapelem_set_.begin();
	for (; it != active_mapelem_set_.end(); ++it)
	{
		SendMapElemEnable(info, *it);
	}

    it = deactive_mapelem_set_.begin();
    for (; it != deactive_mapelem_set_.end(); ++it)
    {
        SendMapElemDisable(info, *it);
    }
}

void InsController::ForEachInsWorld(EachInsWorldCB callback, int64_t param)
{
	RWLockReadGuard rdlock(change_rwlock_);
	ActiveInsMap::const_iterator it = active_ins_map_.begin();
	for (; it != active_ins_map_.end(); ++it)
	{
		callback(it->second, param);
	}
}

} // namespace gamed
