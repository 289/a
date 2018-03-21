#include "world.h"

#include <algorithm>

#include "shared/logsys/logging.h"

#include "game_module/pathfinder/include/path_finder.h"

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/game_util.h"
#include "gs/global/message.h"
#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/timer.h"
#include "gs/player/player.h"
#include "gs/player/player_def.h"
#include "gs/obj/npc.h"
#include "gs/obj/area.h"
#include "gs/obj/matter.h"


using namespace mapDataSvr;
using namespace gamed::world;


namespace gamed {

void World::DispatchAOIMessage(AOI::AOI_Type type, AOI::ObjectInfo watcher, AOI::ObjectInfo marker, void* pdata)
{
	XID source, target;
	source      = *(XID*)(pdata); // world_id_
	target.id   = watcher.id;
	target.type = watcher.type;

	if (AOI::OBJ_ENTER == type)
	{
		MSG msgenter;
		msg_obj_enter_view param;
		param.object.id   = marker.id;
		param.object.type = marker.type;
		param.param       = marker.param;
		BuildMessage(msgenter, GS_MSG_OBJ_ENTER_VIEW, target, source, 0, &param, sizeof(param));
		if (target.IsObject())
		{
			Gmatrix::SendObjectMsg(msgenter, true);
		}
	}
	else if (AOI::OBJ_LEAVE == type)
	{
		MSG msgleave;
		msg_obj_leave_view param;
		param.object.id   = marker.id;
		param.object.type = marker.type;
		BuildMessage(msgleave, GS_MSG_OBJ_LEAVE_VIEW, target, source, 0, &param, sizeof(param));
		if (target.IsObject())
		{
			Gmatrix::SendObjectMsg(msgleave, true);
		}
	}
	else
	{
		ASSERT(false);
	}
}

World::World()
	: world_id_(-1),
	  world_tag_(-1),
	  world_default_area_(NULL),
	  create_time_(0),
      has_map_team_(false),
      has_map_counter_(false),
      has_map_gather_(false)
{
}

World::~World()
{
    xid_                = XID();
    world_id_           = 0;
    world_tag_          = 0;
	world_default_area_ = NULL;
	create_time_        = 0;
    has_map_team_       = false;
    has_map_counter_    = false;
    has_map_gather_     = false;
}

bool World::Init(const WorldInitData& initData)
{
	world_id_    = initData.world_id;
	world_tag_   = initData.world_tag;

	int row      = initData.grid.row;
	int column   = initData.grid.column;
	float step   = initData.grid.step;
	float xstart = initData.grid.xstart;
	float ystart = initData.grid.ystart;
	if (!CreateGrid(row, column, step, xstart, ystart))
	{
		LOG_ERROR << "Can not CreateGrid in the world - index:" << world_id_ ;
		return false;
	}

	if (!grid_.SetRegion(initData.grid.local_rt, 1.f))
	{
		LOG_ERROR << "配置文件中的区域数据不正确(local_region) world_id:" << world_id_;
		return false;
	}

	grid_.set_inner_region(initData.grid.local_rt);

	// obj vision
	obj_far_vision_  = initData.visible.max_visible_range;
	obj_near_vision_ = initData.visible.min_visible_range;
	set_obj_visible_range(obj_near_vision_, obj_far_vision_);
	
	// 地形、通过图
	std::string movemap_path = Gmatrix::GetMapDataDir();
	if (!Gmatrix::LoadMoveMapGbd(world_id_, movemap_path.c_str()))	
	{
		LOG_ERROR << "地图：" << world_id_ << " MoveMap载入失败！路径："
			<< movemap_path;
	}

	// 地图默认规则区域，一张地图只能有一个默认区域
	std::vector<const MapDefaultArea*> default_area_vec;
	s_pMapData->QueryMapDataByType(world_id_, default_area_vec);
	ASSERT(default_area_vec.size() == 1);
	world_default_area_ = default_area_vec[0];

	// 记录创建时间
	create_time_ = g_timer->GetSysTime();
	return true;
}

bool World::CreateGrid(int row, int column, float step, float startX, float startY)
{
	return grid_.Create(row, column, step, startX, startY);
}

void World::Heartbeat()
{
	aoi_module_.AOI_Message(DispatchAOIMessage, static_cast<void*>(&xid_));
}

int World::InsertPlayer(Player* player_ptr)
{
	Slice* pPiece = grid_.Locate(player_ptr->pos().x, player_ptr->pos().y);
	if (NULL == pPiece) return -1;

	player_ptr->set_world_id(world_id_, world_tag_);
	player_ptr->set_world_plane(this);

	pPiece->InsertPlayer(player_ptr);

	// lock
	{
		RWLockWriteGuard wrlock(query_player_rwlock_);
		players_map_.insert(std::pair<XID::IDType, const Player*>(player_ptr->object_id(), player_ptr));
	}

    // lock
    {
        RWLockWriteGuard wrlock(query_player_extra_rwlock_);
        world::player_extra_info info;
        BuildPlayerExtraInfo(player_ptr, info);
        players_extra_map_.insert(std::make_pair(player_ptr->object_id(), info));
    }

	return grid_.GetSliceIndex(pPiece);
}

void World::RemovePlayer(Player* player_ptr)
{
	Slice* pPiece = grid_.Locate(player_ptr->pos().x, player_ptr->pos().y);
	ASSERT(pPiece);
	ASSERT(world_id_ == player_ptr->world_id() && world_tag_ == player_ptr->world_tag());

	pPiece->RemovePlayer(player_ptr);

	// lock
	{
		RWLockWriteGuard wrlock(query_player_rwlock_);
		players_map_.erase(player_ptr->object_id());
	}

    // lock
    {
        RWLockWriteGuard wrlock(query_player_extra_rwlock_);
        players_extra_map_.erase(player_ptr->object_id());
    }
}

int World::InsertNpc(Npc* npc_ptr)
{
	Slice* pPiece = grid_.Locate(npc_ptr->pos().x, npc_ptr->pos().y);
	if (NULL == pPiece) return -1;

	npc_ptr->set_world_id(world_id_, world_tag_);
	npc_ptr->set_world_plane(this);

	pPiece->InsertNPC(npc_ptr);
	// lock 
	{
		RWLockWriteGuard wrlock(query_npc_rwlock_);
		npcs_map_.insert(std::pair<XID::IDType, const Npc*>(npc_ptr->object_id(), npc_ptr));
	}

	return grid_.GetSliceIndex(pPiece);
}

void World::RemoveNpc(Npc* npc_ptr)
{
	Slice* pPiece = grid_.Locate(npc_ptr->pos().x, npc_ptr->pos().y);
	ASSERT(pPiece);
	ASSERT(world_id_ == npc_ptr->world_id() && world_tag_ == npc_ptr->world_tag());

	pPiece->RemoveNPC(npc_ptr);

	// lock
	{
		RWLockWriteGuard wrlock(query_npc_rwlock_);
		npcs_map_.erase(npc_ptr->object_id());
	}
}

int World::InsertAreaObj(AreaObj* area_ptr)
{
	std::vector<A2DVECTOR> vertexes;
	area_ptr->GetVertexes(vertexes);
	ASSERT(vertexes.size());
	for (size_t i = 0; i < vertexes.size(); ++i)
	{
		const A2DVECTOR& pos = vertexes[i];
		Slice* pPiece = grid_.Locate(pos.x, pos.y);
		if (NULL == pPiece) return -1;
	}

	Slice* pPiece = grid_.Locate(area_ptr->pos().x, area_ptr->pos().y);
	if (NULL == pPiece) return -1;

	area_ptr->set_world_id(world_id_, world_tag_);
	area_ptr->set_world_plane(this);

	pPiece->InsertAreaObj(area_ptr);

	// lock
	{
		RWLockWriteGuard wrlock(query_area_rwlock_);
		areas_map_.insert(std::pair<XID::IDType, const AreaObj*>(area_ptr->object_id(), area_ptr));
	}

	return 0;
}

void World::RemoveAreaObj(AreaObj* area_ptr)
{
	Slice* pPiece = grid_.Locate(area_ptr->pos().x, area_ptr->pos().y);
	ASSERT(pPiece);
	ASSERT(world_id_ == area_ptr->world_id() && world_tag_ == area_ptr->world_tag());

	pPiece->RemoveAreaObj(area_ptr);

	// lock
	{
		RWLockWriteGuard wrlock(query_area_rwlock_);
		areas_map_.erase(area_ptr->object_id());
	}
}

int World::InsertMatter(Matter* matter_ptr)
{
	Slice* pPiece = grid_.Locate(matter_ptr->pos().x, matter_ptr->pos().y);
	if (NULL == pPiece) return -1;

	matter_ptr->set_world_id(world_id_, world_tag_);
	matter_ptr->set_world_plane(this);

	pPiece->InsertMatter(matter_ptr);
	// lock 
	{
		RWLockWriteGuard wrlock(query_matter_rwlock_);
		matters_map_.insert(std::pair<XID::IDType, const Matter*>(matter_ptr->object_id(), matter_ptr));
	}

	return grid_.GetSliceIndex(pPiece);
}

void World::RemoveMatter(Matter* matter_ptr)
{
	Slice* pPiece = grid_.Locate(matter_ptr->pos().x, matter_ptr->pos().y);
	ASSERT(pPiece);
	ASSERT(world_id_ == matter_ptr->world_id() && world_tag_ == matter_ptr->world_tag());

	pPiece->RemoveMatter(matter_ptr);

	// lock
	{
		RWLockWriteGuard wrlock(query_matter_rwlock_);
		matters_map_.erase(matter_ptr->object_id());
	}
}

void World::InsertActiveRuleArea(int32_t elem_id)
{
	const AreaWithRules* ptmpl = s_pMapData->QueryMapDataTempl<AreaWithRules>(elem_id);
	ASSERT(ptmpl);

	// lock
	{
		RWLockWriteGuard wrlock(query_area_rules_rwlock_);
		//ASSERT(active_rule_areas_.insert(std::make_pair(elem_id, ptmpl)).second);
		if (!active_rule_areas_.insert(std::make_pair(elem_id, ptmpl)).second)
		{
			LOG_WARN << "InsertActiveRuleArea() insert error! elem_id:" << elem_id;
		}
	}
}

void World::RemoveActiveRuleArea(int32_t elem_id)
{
	// lock
	RWLockWriteGuard wrlock(query_area_rules_rwlock_);
	size_t n = active_rule_areas_.erase(elem_id);
	//(void)n; ASSERT(n == 1);
	if (n != 1) 
	{
		LOG_WARN << "RemoveActiveRuleArea() erase error! elem_id:" << elem_id;
	}
}

bool World::InsertAreaToAOI(XID obj, const std::vector<A2DVECTOR>& vertexes)
{
	bool rst = false;
	ASSERT(obj.IsArea());
	std::vector<CGLib::Point2d> tmp_points;
	for (size_t i = 0; i < vertexes.size(); ++i)
	{
		CGLib::Point2d point(vertexes[i].x, vertexes[i].y);
		tmp_points.push_back(point);
	}
	rst = aoi_module_.InsertPolygonArea(obj.id, obj.type, tmp_points);
	return rst;
}

void World::RemoveAreaFromAOI(XID obj)
{
	ASSERT(aoi_module_.DropPolygonArea(obj.id));
}

bool World::InsertObjToAOI(XID obj, const A2DVECTOR& init_pos, int64_t param)
{
	ASSERT(obj.IsObject());
	if (obj.IsPlayer())
	{
		return ObjInsertToAOI(obj, "wm", init_pos, param);
	}
	else
	{
		return ObjInsertToAOI(obj, "wm", init_pos, param);
	}

	return false;
}

void World::UpdateAOI(XID obj, const A2DVECTOR& pos)
{
	if (obj.IsObject())
	{
		ObjUpdateAOI(obj, "wm", pos);
	}
	else
	{
		ASSERT(false);
	}
}

void World::RemoveObjFromAOI(XID obj)
{
	ASSERT(obj.IsObject());
	ObjUpdateAOI(obj, "d", A2DVECTOR());
}

bool World::ObjInsertToAOI(XID obj, const char* mode, const A2DVECTOR& init_pos, int64_t param)
{
	bool rst         = false;
	bool area_detect = obj.IsPlayer() ? true : false;

	ASSERT(obj.IsObject());
	rst = aoi_module_.InsertAnnulusObj(obj.id, obj.type, param, mode, init_pos, area_detect);
	return rst;
}

void World::ObjUpdateAOI(XID obj, const char* mode, const A2DVECTOR& pos)
{
	aoi_module_.AOI_Update(obj.id, mode, pos);
}

bool World::IsWalkablePos(const A2DVECTOR& pos) const
{
	return pathfinder::IsWalkable(world_id_, A2D_TO_PF(pos));
}

void World::SendObjectMSG(int message, const XID& target, int64_t param) const
{
	MSG msg;
	BuildMessage(msg, message, target, world_xid(), param, NULL, 0);
	Gmatrix::SendObjectMsg(msg);
}

void World::SendObjectMSG(int message, const XID& target, const void* buf, size_t len) const
{
	MSG msg;
	BuildMessage(msg, message, target, world_xid(), 0, buf, len);
	Gmatrix::SendObjectMsg(msg);
}

void World::SendWorldMSG(int message, const XID& target, int64_t param) const
{
	MSG msg;
	BuildMessage(msg, message, target, world_xid(), param, NULL, 0);
	Gmatrix::SendWorldMsg(msg);
}

void World::SendWorldMSG(int message, const XID& target, const void* buf, size_t len) const
{
	MSG msg;
	BuildMessage(msg, message, target, world_xid(), 0, buf, len);
	Gmatrix::SendWorldMsg(msg);
}

bool World::PlaneSwitch(Player* pplayer, int32_t target_world_id, const A2DVECTOR& pos) const
{
	ASSERT(pplayer->IsLocked());
	
	if (target_world_id == world_id_)
	{
		// 调用此接口不能做本地图跳转
		return false;
	}

    if (!IS_NORMAL_MAP(target_world_id))
    {
	    // 副本、战场、竞技场等特殊地图的检查
        return false;
    }

	// find out the world
	XID target_world;
	if (!Gmatrix::GetWorldXID(target_world_id, target_world))
	{
		// 地图不在本gs
		return false;
	}
		
	// send request msg
	plane_msg_switch_request param;
	param.player_roleid = pplayer->role_id();
	param.dest_pos      = pos;
	SendWorldMSG(GS_PLANE_MSG_SWITCH_REQUEST, target_world, &param, sizeof(param));
	return true;
}

bool World::QueryObject(const XID& xid, worldobj_base_info& info) const
{
	if (xid.IsNpc())
	{
		size_t index = ID2IDX(xid.id);
		if (index >= Gmatrix::GetMaxNpcCount())
		{
			return false;
		}

		const Npc* pnpc = Gmatrix::GetNpcByIndex(index);
		if ( !pnpc->IsActived() 
		  || pnpc->world_id() != world_id_ 
		  || pnpc->world_tag() != world_tag_ )
		{
			return false;
		}

		info.tid = pnpc->templ_id();
		info.eid = pnpc->elem_id();
		info.pos = pnpc->pos();
		info.dir = pnpc->dir();
        info.can_combat = pnpc->CanCombat();
		return true;
	}
	else if (xid.IsMatter())
	{
		size_t index = ID2IDX(xid.id);
		if (index >= Gmatrix::GetMaxMatterCount())
		{
			return false;
		}

		const Matter* pmatter = Gmatrix::GetMatterByIndex(index);
		if ( !pmatter->IsActived() 
		  || pmatter->world_id() != world_id_ 
		  || pmatter->world_tag() != world_tag_ )
		{
			return false;
		}

		info.tid = pmatter->templ_id();
		info.eid = pmatter->elem_id();
		info.pos = pmatter->pos();
		info.dir = pmatter->dir();
        info.can_combat = false;
		return true;
	}

	return false;
}

// 只能查一些静态的信息，以动态方式存储的信息不能查询，比如名字用string保存的
bool World::QueryPlayer(RoleID roleid, player_base_info& info) const
{
	RWLockReadGuard rdlock(query_player_rwlock_);
	ConstPlayerMap::const_iterator it = players_map_.find(roleid);
	if (it == players_map_.end())
		return false;

    const Player* pplayer = it->second;
    info.xid          = pplayer->object_xid();
    info.master_id    = pplayer->master_id();
	info.link_id      = pplayer->link_id();
	info.sid_in_link  = pplayer->sid_in_link();
	info.pos          = pplayer->pos();
	info.gender       = pplayer->gender();
	info.cls          = pplayer->role_class();
	info.dir          = pplayer->dir();
	info.level        = pplayer->level();
	info.faction      = pplayer->faction();
	info.can_combat   = pplayer->CanCombat();
	info.immune_mask  = player_base_info::IMMUNE_MASK_INVALID;
    info.combat_value = pplayer->GetCombatValue();

	if (pplayer->IsImmuneActiveMonster())
	{
		info.immune_mask |= player_base_info::IMMUNE_MASK_ACTIVE_MONSTER;
	}

	return true;
}

bool World::QueryPlayerExtra(RoleID roleid, world::player_extra_info& info) const
{
    RWLockReadGuard rdlock(query_player_extra_rwlock_);
    PlayerExtraInfoMap::const_iterator it = players_extra_map_.find(roleid);
    if (it == players_extra_map_.end())
        return false;

    info = it->second;
    return true;
}

void World::SetPlayerExtraInfo(const Player* player_ptr)
{
    world::player_extra_info info;
    BuildPlayerExtraInfo(player_ptr, info);
    
    RWLockWriteGuard wrlock(query_player_extra_rwlock_);
    PlayerExtraInfoMap::iterator it = players_extra_map_.find(player_ptr->role_id());
    if (it == players_extra_map_.end())
    {
        LOG_ERROR << "玩家 " << player_ptr->role_id() << " 没有进入players_extra_map_列表？";
        return;
    }

    it->second = info;
}

void World::FillDefaultAreaRules(playerdef::AreaRulesInfo& rules_info) const
{
	///
	/// 使用默认区域填充
	///
	rules_info.is_allow_pk = world_default_area_->is_allow_pk;
	if (world_default_area_->has_resurrect_coord())
	{
		rules_info.resurrect_coord.map_id  = world_default_area_->resurrect_coord.map_id;
		rules_info.resurrect_coord.coord.x = world_default_area_->resurrect_coord.coord.x;
		rules_info.resurrect_coord.coord.y = world_default_area_->resurrect_coord.coord.y;
	}
	if (world_default_area_->has_battle_scene())
	{
		for (size_t i = 0; i < world_default_area_->battle_scene_list.size(); ++i)
		{
			rules_info.battle_scene_list.push_back(world_default_area_->battle_scene_list[i]);
		}
	}
}

bool World::QueryAreaRules(const std::vector<int32_t>& area_vec, playerdef::AreaRulesInfo& rules_info) const
{
	// 先用默认地图区域填充
	FillDefaultAreaRules(rules_info);

	typedef std::multimap<int, const mapDataSvr::AreaWithRules*, std::greater<int> > AreaRulesOrderByLevel;
	AreaRulesOrderByLevel rule_areas_by_level;

	// sort areas by level
	{
		RWLockReadGuard rdlock(query_area_rules_rwlock_);
		// 根据area level排列区域
		for (size_t i = 0; i < area_vec.size(); ++i)
		{
			AreaWithRulesTmplMap::const_iterator it = active_rule_areas_.find(area_vec[i]);
			if (it == active_rule_areas_.end())
			{
				continue;
			}

			int area_level = it->second->area_level;
			rule_areas_by_level.insert(std::make_pair(area_level, it->second));
		}
	}

	// 根据排序后的area取对应信息
	AreaRulesOrderByLevel::const_iterator it = rule_areas_by_level.begin();
	bool allow_pk_filled    = false;
	bool resurrect_filled   = false;
	bool battlescene_filled = false;
	for (; it != rule_areas_by_level.end(); ++it)
	{
		const mapDataSvr::AreaWithRules* ptmpl = it->second;
		if (!allow_pk_filled)
		{
			rules_info.is_allow_pk = ptmpl->is_allow_pk;
			allow_pk_filled = true;
		}

		if (!resurrect_filled)
		{
			if (ptmpl->has_resurrect_coord())
			{
				rules_info.resurrect_coord.map_id  = ptmpl->resurrect_coord.map_id;
				rules_info.resurrect_coord.coord.x = ptmpl->resurrect_coord.coord.x;
				rules_info.resurrect_coord.coord.y = ptmpl->resurrect_coord.coord.y;
				resurrect_filled = true;
			}
		}

		if (!battlescene_filled)
		{
			if (ptmpl->has_battle_scene())
			{
                rules_info.battle_scene_list.clear();
				for (size_t i = 0; i < ptmpl->battle_scene_list.size(); ++i)
				{
					rules_info.battle_scene_list.push_back(ptmpl->battle_scene_list[i]);
				}
				battlescene_filled = true;
			}
		}
	}

	return true;
}

void World::GetAllObjectInWorld(std::vector<XID>& obj_vec)
{
	// player
	{
		RWLockReadGuard rdlock(query_player_rwlock_);
		ConstPlayerMap::const_iterator it = players_map_.begin();
		for (; it != players_map_.end(); ++it)
		{
			obj_vec.push_back(it->second->object_xid());
		}
	}

	// npc
	{
		RWLockReadGuard rdlock(query_npc_rwlock_);
		ConstNpcMap::const_iterator it = npcs_map_.begin();
		for (; it != npcs_map_.end(); ++it)
		{
			obj_vec.push_back(it->second->object_xid());
		}
	}

	// area
	{
		RWLockReadGuard rdlock(query_area_rwlock_);
		ConstAreaObjMap::const_iterator it = areas_map_.begin();
		for (; it != areas_map_.end(); ++it)
		{
			obj_vec.push_back(it->second->object_xid());
		}
	}

	// matter
	{
		RWLockReadGuard rdlock(query_matter_rwlock_);
		ConstMatterMap::const_iterator it = matters_map_.begin();
		for (; it != matters_map_.end(); ++it)
		{
			obj_vec.push_back(it->second->object_xid());
		}
	}
}

void World::GetAllPlayerInWorld(std::vector<XID>& player_vec)
{
    // player
	{
		RWLockReadGuard rdlock(query_player_rwlock_);
		ConstPlayerMap::const_iterator it = players_map_.begin();
		for (; it != players_map_.end(); ++it)
		{
			player_vec.push_back(it->second->object_xid());
		}
	}
}

void World::GetAllPlayerLinkInfo(std::vector<player_link_info>& player_vec)
{
    // player
    {
        RWLockReadGuard rdlock(query_player_rwlock_);
        ConstPlayerMap::const_iterator it = players_map_.begin();
        for (; it != players_map_.end(); ++it)
        {
            const Player* pplayer = it->second;
            player_link_info info;
            info.role_id     = pplayer->role_id();
            info.link_id     = pplayer->link_id();
            info.sid_in_link = pplayer->sid_in_link();
            player_vec.push_back(info);
        }
    }
}

int32_t World::GetPlayerCount()
{
    RWLockReadGuard rdlock(query_player_rwlock_);
    return players_map_.size();
}

void World::BuildPlayerExtraInfo(const Player* player_ptr, world::player_extra_info& info)
{
    info.first_name  = player_ptr->first_name();
    info.middle_name = player_ptr->middle_name();
    info.last_name   = player_ptr->last_name();
}

} // namespace gamed
