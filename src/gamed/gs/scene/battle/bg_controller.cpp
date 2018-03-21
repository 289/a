#include "bg_controller.h"

#include "gs/global/dbgprt.h"
#include "gs/global/message.h"
#include "gs/global/timer.h"
#include "gs/global/gmatrix.h"
#include "gs/obj/npc_gen.h"
#include "gs/obj/area_gen.h"
#include "gs/obj/matter_gen.h"
#include "gs/scene/world_man.h"
#include "gs/scene/world_cluster.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/battleground_templ.h"


namespace gamed {

using namespace dataTempl;

namespace {

    bool IsUniqueBGMap(int32_t bg_tid)
    {
        const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(bg_tid);
        if (ptempl == NULL)
        {
		    LOG_ERROR << "没有找到对应的战场模板，templ_id:" << bg_tid;
            return false;
        }

        if (ptempl->bg_type == BattleGroundTempl::BGT_PVE_WORLD_BOSS)
        {
            return true;
        }

        return false;
    }

} // Anonymous


///
/// static func
///
void BGController::SendMapElemEnable(const world::battleground_info& info, int64_t elemid)
{
	MSG msg;
	XID target = MakeWorldXID(info.world_id, info.world_tag);
	BuildMessage(msg, GS_PLANE_MSG_ENABLE_MAP_ELEM, target, XID(), elemid);
	Gmatrix::SendWorldMsg(msg);
}
	
void BGController::SendMapElemDisable(const world::battleground_info& info, int64_t elemid)
{
	MSG msg;
	XID target = MakeWorldXID(info.world_id, info.world_tag);
	BuildMessage(msg, GS_PLANE_MSG_DISABLE_MAP_ELEM, target, XID(), elemid);
	Gmatrix::SendWorldMsg(msg);
}


///
/// BGController
///
BGController::BGController()
	: bg_world_id_(-1),
	  pconf_(NULL)
{
}

BGController::~BGController()
{
	DELETE_SET_NULL(npc_gen_);
	DELETE_SET_NULL(area_gen_);
	DELETE_SET_NULL(matter_gen_);
}

bool BGController::Init(MapID world_id, shared::Conf* conf, const char* world_name)
{
	bg_world_id_  = world_id;
	pconf_        = conf;
	world_name_   = world_name;

	npc_gen_      = new NpcGenerator(world_id);
	area_gen_     = new AreaGenerator(world_id);
	matter_gen_   = new MatterGenerator(world_id);
	return true;
}

WorldManager* BGController::CreateWorldMan(int64_t bg_serial_num, int32_t bg_templ_id)
{
	WorldManager* gwm = wcluster::AllocWorldManager();
	if (gwm != NULL)
	{
		// create world
		if (!gwm->CreateWorldPlane())
		{
			__PRINTF("战场世界%s - CreateWorldPlane失败！", world_name_.c_str());
			wcluster::FreeWorldManager(gwm);
			return NULL;
		}

		// set gen
		gwm->SetObjGenerator(npc_gen_, area_gen_, matter_gen_, (int64_t)this);

		// set battleground info
		world::battleground_info bg_info;
		bg_info.bg_serial_num  = bg_serial_num;
		bg_info.bg_templ_id    = bg_templ_id;
		bg_info.world_id       = bg_world_id_;
		bg_info.world_tag      = get_new_tag_id();
		bg_info.bg_create_time = g_timer->GetSysTimeMsecs();
		ASSERT(gwm->SetBGInfo(bg_info));

		// init
		bool ret = gwm->Init(pconf_, world_name_.c_str(), WorldManager::BATTLEGROUND_WORLD, bg_info.world_tag);
		if (!ret)
		{
			__PRINTF("战场世界%s初始化失败, world_id:%d", world_name_.c_str(), bg_world_id_);
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

		ASSERT(bg_world_id_ == gwm->GetWorldID());
		ASSERT(bg_info.world_tag == gwm->GetWorldTag());
		
		// lock
		{
			RWLockWriteGuard wrlock(change_rwlock_);
			ASSERT(active_bg_map_.insert(std::make_pair(gwm->GetWorldXID().id, bg_info)).second);
            if (IsUniqueBGMap(bg_templ_id))
            {
                if (unique_bg_map_.insert(std::make_pair(bg_templ_id, gwm->GetWorldXID().id)).second != true)
                {
                    LOG_ERROR << "唯一战场怎么会同时有两个world_man？bg_tid:" << bg_templ_id;
                }
            }
		}

		// locked inside
		TraversalActiveMapElem(bg_info);
	}

	return gwm;
}

bool BGController::IsActiveBG(int32_t worldid, int32_t worldtag)
{
	if (worldid != bg_world_id_)
		return false;

	RWLockReadGuard rdlock(change_rwlock_);
	return is_active_bg(worldid, worldtag);
}

bool BGController::FindWorldMan(const world::battleground_info& info, world::battleground_info& ret_info)
{
	if (info.world_id != bg_world_id_)
		return false;

	RWLockReadGuard rdlock(change_rwlock_);
	int64_t world64_id = MakeWorldID(info.world_id, info.world_tag);
	ActiveBGMap::const_iterator it = active_bg_map_.find(world64_id);
	if (it != active_bg_map_.end())
	{
		if (it->second == info)
        {
            ret_info = it->second;
            return true;
        }
    }

    //
	// not found
    //

    // find from unique map
    if (IsUniqueBGMap(info.bg_templ_id))
    {
        UniqueBGMap::const_iterator un_it = unique_bg_map_.find(info.bg_templ_id);
        if (un_it != unique_bg_map_.end())
        {
		    ActiveBGMap::const_iterator it = active_bg_map_.find(un_it->second);
            if (it != active_bg_map_.end())
            {
                ret_info = it->second;
                return true;
            }
        }
    }

    // for the first time to enter
	if (info.world_tag == 0 && info.bg_create_time == 0)
	{
		ActiveBGMap::const_iterator it = active_bg_map_.begin();
		for (; it != active_bg_map_.end(); ++it)
		{
			if (it->second.bg_serial_num == info.bg_serial_num &&
				it->second.bg_templ_id == info.bg_templ_id)
            {
                ret_info = it->second;
                return true;
            }
        }
	}

	return false;
}

void BGController::RemoveBGWorldMan(int32_t world_id, int32_t world_tag)
{
	if (world_id != bg_world_id_)
		return;

	RWLockWriteGuard wrlock(change_rwlock_);
	int64_t world64_id = MakeWorldID(world_id, world_tag);
    // reomve from unique
    ActiveBGMap::const_iterator it = active_bg_map_.find(world64_id);
    if (it != active_bg_map_.end()) {
        unique_bg_map_.erase(it->second.bg_templ_id);
    }
    // remove from active
	size_t n = active_bg_map_.erase(world64_id);
	ASSERT(n == 1); (void)n;
}

void BGController::CloseBGWorldMan(int32_t world_id, int32_t world_tag)
{
    // 地图关闭，但还没有remove，这里还不能从active里移除，因为还需要Find这个WorldMan处理MSG
    if (world_id != bg_world_id_)
		return;

	RWLockWriteGuard wrlock(change_rwlock_);
	int64_t world64_id = MakeWorldID(world_id, world_tag);
    // reomve from unique
    ActiveBGMap::const_iterator it = active_bg_map_.find(world64_id);
    if (it != active_bg_map_.end()) {
        unique_bg_map_.erase(it->second.bg_templ_id);
    }
}

void BGController::EnableMapElem(int32_t elem_id)
{
    MutexLockGuard lock(mapelem_set_mutex_);
    deactive_mapelem_set_.erase(elem_id);

	ActiveMapElemSet::iterator it = active_mapelem_set_.find(elem_id);
	if (it != active_mapelem_set_.end())
		return; // found
	ASSERT(active_mapelem_set_.insert(elem_id).second);

	// foreach bg world, locked inside
	ForEachBGWorld(SendMapElemEnable, elem_id);
}

void BGController::DisableMapElem(int32_t elem_id)
{
    MutexLockGuard lock(mapelem_set_mutex_);
	active_mapelem_set_.erase(elem_id);

	ActiveMapElemSet::iterator it = deactive_mapelem_set_.find(elem_id);
	if (it != deactive_mapelem_set_.end())
		return; // not found
    ASSERT(deactive_mapelem_set_.insert(elem_id).second);

	// foreach bg world, locked inside
	ForEachBGWorld(SendMapElemDisable, elem_id);
}

void BGController::TraversalActiveMapElem(const world::battleground_info& info)
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

void BGController::ForEachBGWorld(EachBGWorldCB callback, int64_t param)
{
	RWLockReadGuard rdlock(change_rwlock_);
	ActiveBGMap::const_iterator it = active_bg_map_.begin();
	for (; it != active_bg_map_.end(); ++it)
	{
		callback(it->second, param);
	}
}

} // namespace gamed
