#include "area.h"

#include "gamed/client_proto/G2C_error.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/instance_templ.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/eventsys/evsys_if.h"
#include "gs/global/dbgprt.h"
#include "gs/global/randomgen.h"
#include "gs/global/gmatrix.h"
#include "gs/global/runnable_pool.h"
#include "gs/scene/world_man.h"
#include "gs/scene/world.h"


namespace gamed {

using namespace dataTempl;
using namespace mapDataSvr;

namespace {

///
/// RecycleTask
///
class RecycleTask : public RunnablePool::Task
{
public:
	RecycleTask(int64_t id, int32_t elem_id)
		: area_id_(id),
		  elem_id_(elem_id)
	{ }

	virtual void Run()
	{
		// 找到area，并执行下线操作
		AreaObj* pArea = Gmatrix::FindAreaObjFromMan(area_id_);
		if (NULL == pArea)
		{
			LOG_WARN << "area not found！";
			return;
		}

		WorldObjectLockGuard lock(pArea);
		if (!pArea->IsActived())
			return;

		// 必须是zombie状态
		//ASSERT(pArea->state() == STATE_ZOMBIE);

		// 从地图中删除前获取area的相关信息
		XID msg_target   = pArea->world_plane()->world_xid();
		XID src_xid      = pArea->object_xid();
		int32_t elem_id  = pArea->elem_id();
		ASSERT(elem_id == elem_id_);
		
		// 从地图中删除
		WorldManager* pManager = Gmatrix::FindWorldManager(pArea->world_id(), pArea->world_tag());
		if (pManager)
		{
			pManager->RemoveAreaObj(pArea);
		}
		else
		{
			__PRINTF("Area%ld不在world中，Area的world_id:%d", pArea->object_id(), pArea->world_id());
		}

		// 回收Area指针
		Gmatrix::RemoveAreaObjFromMan(pArea);
		Gmatrix::FreeAreaObj(pArea);
	}

private:
	int64_t area_id_;
	int32_t elem_id_;
};

} // Anonymous


///
/// AreaObj
///
AreaObj::AreaObj()
	: pimp_(NULL)
{
}

AreaObj::~AreaObj()
{
	pimp_ = NULL;
}

bool AreaObj::Init(const AreaInitData& init_data)
{
	const BaseMapData* pmapdata = s_pMapData->QueryBaseMapDataTempl(init_data.elem_id);
	if (!pmapdata) 
	{
		LOG_ERROR << "没有找到AreaObj的MapDataTempl，elem_id=" << init_data.elem_id;
		return false;
	}

	int type = pmapdata->GetType();
	switch (type)
	{
		case mapDataSvr::MAPDATA_TYPE_AREA_WITH_RULES:
			{
				set_xid(init_data.id, XID::TYPE_MAP_AREA_OBJ);
				pimp_ = new AreaWithRulesImp(*this);
			}
			break;

		case mapDataSvr::MAPDATA_TYPE_TRANSFER_AREA:
			{
				set_xid(init_data.id, XID::TYPE_MAP_TRANSFER_AREA);
				pimp_ = new TransferAreaImp(*this);
			}
			break;

		case mapDataSvr::MAPDATA_TYPE_AREA_MONSTER:
			{
				set_xid(init_data.id, XID::TYPE_MAP_LANDMINE_AREA);
				pimp_ = new LandmineAreaImp(*this);
			}
			break;

		default:
			return false;
	}
	set_pos(init_data.pos);
	elem_id_ = init_data.elem_id;

	if (NULL == pimp_ || !pimp_->OnInit())
		return false;

	return true;
}

void AreaObj::Release()
{
	// always first
	MapElementDeactive();
	
	// delete imp 应放在最前面
	SAFE_DELETE(pimp_);

	//
	// release members
	//
	elem_id_ = 0;
	
	vertexes_.clear();
	players_in_view_.clear();
	
	WorldObject::Release();
}

void AreaObj::HasInsertToWorld()
{
	pimp_->OnInsertToWorld();

	// enable map element
	SendPlaneMsg(GS_PLANE_MSG_MAP_ELEM_ACTIVE, elem_id_);
}

void AreaObj::PushBackVertex(const A2DVECTOR& point)
{
	vertexes_.push_back(point);
}

int AreaObj::OnDispatchMessage(const MSG& msg)
{
	return MessageHandler(msg);
}

void AreaObj::OnHeartbeat()
{
	pimp_->OnHeartbeat();
}

int AreaObj::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_OBJ_ENTER_VIEW:
			{
				if (msg.source != world_plane()->world_xid())
				{
					return 0;
				}

				CHECK_CONTENT_PARAM(msg, msg_obj_enter_view);
				msg_obj_enter_view& param = *(msg_obj_enter_view*)msg.content;
				if (param.object.type == XID::TYPE_PLAYER)
				{
					PlayerEnterView(param.object.id);
				}
				else if (param.object.IsObject())
				{
					ASSERT(param.object != object_xid());
				}

				pimp_->ObjectEnterArea(param.object);
			}
			break;

		case GS_MSG_OBJ_LEAVE_VIEW:
			{
				if (msg.source != world_plane()->world_xid())
				{
					return 0;
				}

				CHECK_CONTENT_PARAM(msg, msg_obj_leave_view);
				msg_obj_leave_view& param = *(msg_obj_leave_view*)msg.content;
				if (param.object.type == XID::TYPE_PLAYER)
				{
					PlayerLeaveView(param.object.id);
				}
				else if (param.object.IsObject())
				{
					ASSERT(param.object != object_xid());
				}

				pimp_->ObjectLeaveArea(param.object);
			}
			break;

		case GS_MSG_WORLD_CLOSING:
			{
				if (msg.source != world_xid())
				{
					ASSERT(false);
					return 0;
				}

				HandleWorldClose();
			}
			break;

		case GS_MSG_RELATE_MAP_ELEM_CLOSED:
			{
				if (msg.source != world_xid())
				{
					ASSERT(false);
					return 0;
				}

				LifeExhaust();
			}
			break;

	    default:
			if (WorldObject::MessageHandler(msg) != 0 && pimp_->OnMessageHandler(msg) != 0)
			{
				ASSERT(false && "无法处理未知类型的inter-message");
				return -1;
			}
			return 0;
	}

	return 0;
}

void AreaObj::PlayerEnterView(RoleID playerid)
{
	player_in_view info;
	info.role_id = playerid;
	if (!players_in_view_.insert(std::make_pair(playerid, info)).second)
	{
		//ASSERT(false);
		LOG_WARN << playerid << " enter view error in area:" << object_id();
		return;
	}
}

void AreaObj::PlayerLeaveView(RoleID playerid)
{
	size_t n = players_in_view_.erase(playerid);
	//(void)n; ASSERT(n == 1);
	if (n != 1)
	{
		LOG_WARN << playerid << " leave view error in area:" << object_id();
		return;
	}
}

void AreaObj::HandleWorldClose()
{
	LifeExhaust();
}

void AreaObj::LifeExhaust()
{
	world_plane()->RemoveAreaFromAOI(object_xid());
	RunnablePool::AddTask(new RecycleTask(object_id(), elem_id_));
}

void AreaObj::MapElementDeactive()
{
	// disable map element
	SendPlaneMsg(GS_PLANE_MSG_MAP_ELEM_DEACTIVE, elem_id_);
}


///
/// AreaObjImp
///
AreaObjImp::AreaObjImp(AreaObj& area)
	: area_obj_(area)
{
}

AreaObjImp::~AreaObjImp()
{
}


///
/// AreaWithRulesImp
///
AreaWithRulesImp::AreaWithRulesImp(AreaObj& area)
	: AreaObjImp(area),
	  pelemdata_(NULL),
	  event_if_(NULL)
{
}

AreaWithRulesImp::~AreaWithRulesImp()
{
	area_obj_.SendPlaneMsg(GS_PLANE_MSG_RULE_AREA_DISABLE, area_obj_.elem_id());

	SAFE_DELETE(event_if_);
}

bool AreaWithRulesImp::OnInit()
{
	pelemdata_ = s_pMapData->QueryMapDataTempl<mapDataSvr::AreaWithRules>(area_obj_.elem_id());
	if (NULL == pelemdata_) 
		return false;

	for (size_t i = 0; i < pelemdata_->vertexes.size(); ++i)
	{
		A2DVECTOR point;
		point.x = pelemdata_->vertexes[i].x;
		point.y = pelemdata_->vertexes[i].y;
		area_obj_.PushBackVertex(point);
	}

	if (pelemdata_->has_evscript())
	{
		// ??????????
		//event_if_ = new AreaEventIf(area_obj_);
		//event_if_->Init(pelemdata_->event_id); // FIXME: Init失败要返回false
	}

	return true;
}

void AreaWithRulesImp::OnInsertToWorld()
{
	area_obj_.SendPlaneMsg(GS_PLANE_MSG_RULE_AREA_ENABLE, area_obj_.elem_id());
}

void AreaWithRulesImp::ObjectEnterArea(const XID& xid)
{
	area_obj_.SendMsg(GS_MSG_ENTER_RULES_AREA, xid, area_obj_.elem_id());

	// 玩家进入区域的事件触发
	if (event_if_ && xid.IsPlayer())
	{
		event_if_->PlayerEnterArea(xid.id);
	}
}

void AreaWithRulesImp::ObjectLeaveArea(const XID& xid)
{
	area_obj_.SendMsg(GS_MSG_LEAVE_RULES_AREA, xid, area_obj_.elem_id());
}

void AreaWithRulesImp::OnHeartbeat()
{
}

///
/// TransferAreaImp
///
TransferAreaImp::TransferAreaImp(AreaObj& area)
	: AreaObjImp(area),
	  pelemdata_(NULL),
	  pins_templ_(NULL)
{
}

TransferAreaImp::~TransferAreaImp()
{
	pelemdata_  = NULL;
	pins_templ_ = NULL;
}

bool TransferAreaImp::OnInit()
{
	pelemdata_ = s_pMapData->QueryMapDataTempl<mapDataSvr::TransferArea>(area_obj_.elem_id());
	if (NULL == pelemdata_) 
		return false;

	for (size_t i = 0; i < pelemdata_->vertexes.size(); ++i)
	{
		A2DVECTOR point;
		point.x = pelemdata_->vertexes[i].x;
		point.y = pelemdata_->vertexes[i].y;
		area_obj_.PushBackVertex(point);
	}

	// 副本传送
	if (pelemdata_->ins_templ_id > 0)
	{
		pins_templ_ = s_pDataTempl->QueryDataTempl<InstanceTempl>(pelemdata_->ins_templ_id);
		if (pins_templ_ == NULL)
		{
			return false;
		}
	}
	return true;
}

void TransferAreaImp::OnHeartbeat()
{
	InViewPlayersMap::iterator it = players_map_.begin();
	for (; it != players_map_.end(); ++it)
	{
		if (!CheckMapCondition(it->second))
			continue;

		if ((--it->second.counter) < 0)
		{
			if (CheckPlayerCond(it->first))
			{
				SendTransportMsg(it->second);
			}
			it->second.counter = kTimeCounter;
		}
	}
}

void TransferAreaImp::ObjectEnterArea(const XID& xid)
{
	if (xid.IsPlayer())
	{
		entry_t ent;
		ent.counter    = kTimeCounter; 
		ent.player_xid = xid;
		ent.ins_tid    = pelemdata_->ins_templ_id;

		// 传送到副本地图
		if (pelemdata_->ins_templ_id > 0)
		{
			ent.mapid  = pins_templ_->ins_map_id;
			ent.pos.x  = 0;
			ent.pos.y  = 0;
		}
		else // 传送到普通地图
		{
			size_t coord_size = pelemdata_->transfer_coords.size();
			uint32_t index = (coord_size == 1) ? 0 : mrand::Rand(0, coord_size - 1);
			ASSERT(index >= 0 && index < coord_size);

			ent.mapid  = pelemdata_->transfer_coords[index].mapid;
			ent.pos.x  = pelemdata_->transfer_coords[index].pos.x;
			ent.pos.y  = pelemdata_->transfer_coords[index].pos.y;
		}

		// 小心，上面的数据不能有return，一定要进players_map_，不然LeaveArea会assert
		ASSERT(players_map_.insert(std::make_pair(xid.id, ent)).second);
		if (CheckPlayerCond(xid.id))
		{
			SendTransportMsg(ent);
		}
	}
}

void TransferAreaImp::ObjectLeaveArea(const XID& xid)
{
	if (xid.IsPlayer())
	{
		size_t n = players_map_.erase(xid.id);
		(void)n; ASSERT(n == 1);
	}
}

void TransferAreaImp::SendTransportMsg(const entry_t& ent)
{
	msg_player_region_transport param;
	param.elem_id         = area_obj_.elem_id();
	param.ins_templ_id    = ent.ins_tid;
	param.source_world_id = area_obj_.world_id();
	param.target_world_id = ent.mapid;
	param.target_pos      = ent.pos;
	area_obj_.SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, ent.player_xid, &param, sizeof(param));
}

bool TransferAreaImp::CheckMapCondition(const entry_t& ent)
{
    // 不是本图才检查
    if (ent.mapid != area_obj_.world_id())
    {
        if (!IS_NORMAL_MAP(ent.mapid) && pelemdata_->ins_templ_id <= 0)
        {
            __PRINTF("传送区域目标地图是副本地图，但没有配副本模板！elem_id:%d", area_obj_.elem_id_);
            return false;
        }

        if (!IS_NORMAL_MAP(area_obj_.world_id()) && !IS_NORMAL_MAP(ent.mapid))
        {
            __PRINTF("传送区域在非普通地图，不能传送到别的非普通地图！elem_id:%d", area_obj_.elem_id_);
            return false;
        }
    }

	return true;
}

bool TransferAreaImp::CheckPlayerCond(RoleID roleid) const
{
	if (!CheckPlayerLevel(roleid))
		return false;

	return true;
}

bool TransferAreaImp::CheckPlayerLevel(RoleID roleid) const
{
	if (pelemdata_->level_lower_limit > 0)
	{
		world::player_base_info info;
		if (area_obj_.world_plane()->QueryPlayer(roleid, info))
		{
			if (info.level < pelemdata_->level_lower_limit)
			{
				area_obj_.SendMsg(GS_MSG_ERROR_MESSAGE, info.xid, G2C::ERR_TRANSFER_NOT_MEET_LEVEL);
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}


///
/// LandmineAreaImp
///
const float LandmineAreaImp::kMinCalcDistanceSquare = 0.25*0.25;

LandmineAreaImp::LandmineAreaImp(AreaObj& area)
	: AreaObjImp(area),
	  pelemdata_(NULL)
{
}

LandmineAreaImp::~LandmineAreaImp()
{
}

bool LandmineAreaImp::OnInit()
{
	pelemdata_ = s_pMapData->QueryMapDataTempl<mapDataSvr::AreaMonster>(area_obj_.elem_id());
	if (NULL == pelemdata_) 
		return false;

	for (size_t i = 0; i < pelemdata_->vertexes.size(); ++i)
	{
		A2DVECTOR point;
		point.x = pelemdata_->vertexes[i].x;
		point.y = pelemdata_->vertexes[i].y;
		area_obj_.PushBackVertex(point);
	}
	return true;
}

void LandmineAreaImp::ObjectEnterArea(const XID& xid)
{
	if (xid.IsPlayer())
	{
		entry_t ent;
		ent.player_xid      = xid;
		ent.encounter_timer = 0;
        ent.next_interval   = pelemdata_->encounter_interval;
		ent.prev_pos.x      = INT_MAX;
		ent.prev_pos.y      = INT_MAX;

		world::player_base_info info;
		if (area_obj_.world_plane()->QueryPlayer(xid.id, info))
		{
			ent.prev_pos = info.pos;
		}

		in_view_players_map_.insert(std::make_pair(xid.id, ent));

        // notify player
        NotifyLandmineInfo(ent, PLAYER_ENTER_AREA);
	}
}

void LandmineAreaImp::ObjectLeaveArea(const XID& xid)
{
	if (xid.IsPlayer())
	{
		in_view_players_map_.erase(xid.id);

        // notify player
        entry_t ent;
        ent.player_xid = xid;
        NotifyLandmineInfo(ent, PLAYER_LEAVE_AREA);
	}
}

void LandmineAreaImp::OnHeartbeat()
{
    // 移到玩家身上进行计算
    /*
	InViewPlayersMap::iterator it = in_view_players_map_.begin();
	for (; it != in_view_players_map_.end(); ++it)
	{
		RoleID playerid = it->first;
		entry_t& ent    = it->second; 

		world::player_base_info info;
		if (area_obj_.world_plane()->QueryPlayer(playerid, info))
		{
			if (info.pos.squared_distance(ent.prev_pos) > kMinCalcDistanceSquare)
			{
				// update pos
				ent.prev_pos = info.pos;

				if (--ent.encounter_timer > 0)
					continue;

				if (!info.can_combat)
					continue;

				TriggerCombat(ent);
			}
		}
	}
    */
}

void LandmineAreaImp::TriggerCombat(entry_t& ent)
{
    // 移到玩家身上进行随机
    /*
	if (mrand::RandSelect((int32_t)pelemdata_->encounter_enermy_prob))
	{
		msg_obj_trigger_combat param;
		param.monster_group_id  = pelemdata_->monster_group_id;
		param.battle_scene_id   = pelemdata_->battle_scene_id;
		param.require_combatend_notify = true;
		param.landmine_interval = pelemdata_->encounter_interval;
		area_obj_.SendMsg(GS_MSG_OBJ_TRIGGER_COMBAT, ent.player_xid, &param, sizeof(param));

		ent.encounter_timer = INT_MAX;
	}
    */
}

int LandmineAreaImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_OBJ_TRIGGER_COMBAT_RE:
			{
				ASSERT(msg.source.IsPlayer());

				CHECK_CONTENT_PARAM(msg, msg_obj_trigger_combat_re);
				msg_obj_trigger_combat_re& param = *(msg_obj_trigger_combat_re*)msg.content;
				HandleCombatStart(msg.source.id, param);
			}
			break;

		case GS_MSG_COMBAT_PVE_END:
			{
				CHECK_CONTENT_PARAM(msg, msg_combat_end);
				msg_combat_end* param = (msg_combat_end*)msg.content;
				HandleCombatEnd(*param);
			}
			break;

		default:
			return -1;
	}

	return 0;
}

void LandmineAreaImp::HandleCombatStart(RoleID playerid, const msg_obj_trigger_combat_re& msg_param)
{
	InViewPlayersMap::iterator it = in_view_players_map_.find(playerid);
	if (it == in_view_players_map_.end())
		return;

	entry_t& ent = it->second;
	if (msg_param.is_success)
	{
		ent.encounter_timer = INT_MAX;
        ent.next_interval   = msg_param.landmine_interval;
	}
	else
	{
		ent.encounter_timer = 0;
		ent.next_interval   = pelemdata_->encounter_interval;
    }
    // notify player
    NotifyLandmineInfo(ent, ENCOUNTER_CHANGE);
}

void LandmineAreaImp::HandleCombatEnd(const msg_combat_end& msg_param)
{
	for (size_t i = 0; i < msg_param.size(); ++i)
	{
		InViewPlayersMap::iterator it = in_view_players_map_.find(msg_param.at(i));
		if (it == in_view_players_map_.end())
			continue;

		entry_t& ent        = it->second;
		ent.encounter_timer = ent.next_interval;
        // notify player
        NotifyLandmineInfo(ent, ENCOUNTER_CHANGE);
	}
}

void LandmineAreaImp::NotifyLandmineInfo(const entry_t& ent, EventType ev_type)
{   
    bool is_leave_area = false;
    switch (ev_type)
    {
        case PLAYER_ENTER_AREA:
        case ENCOUNTER_CHANGE:
            is_leave_area = false;
            break;

        case PLAYER_LEAVE_AREA:
            is_leave_area = true;
            break;

        default:
            ASSERT(false);
            return;
    }

    msg_notify_landmine_info param;
    param.is_leave_area   = is_leave_area;
    param.elem_id         = area_obj_.elem_id();
    param.encounter_timer = ent.encounter_timer;
    area_obj_.SendMsg(GS_MSG_NOTIFY_LANDMINE_INFO, ent.player_xid, &param, sizeof(param));
}

} // namespace gamed
