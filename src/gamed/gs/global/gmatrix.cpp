#include "gmatrix.h"

#include "shared/lua/lua_engine_lite.h"
#include "shared/lua/lua_value.h"
#include "shared/logsys/logging.h"
#include "shared/base/strtoken.h"

#include "game_module/combat/include/combat_header.h"
#include "game_module/skill/include/skill_info.h"
#include "game_module/pathfinder/include/path_finder.h"
#include "game_module/task/include/task.h"
#include "game_module/achieve/include/achieve.h"

#include "gs/obj/npc.h"
#include "gs/obj/area.h"
#include "gs/obj/matter.h"
#include "gs/netio/netio_if.h"
#include "gs/netmsg/link_msg_proc.h"
#include "gs/netmsg/master_msg_proc.h"
#include "gs/netmsg/send_to_master.h"
#include "gs/player/player.h"
#include "gs/player/player_sender.h"
#include "gs/scene/world_cluster.h"
#include "gs/scene/world_man.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/player_class_templ.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/monster_templ.h"
#include "gs/template/extra_templ/extratempl_man.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/item/item_manager.h"
#include "gs/eventsys/evsys_if.h"
#include "gs/global/game_types.h"

#include "obj_query.h"
#include "runnable_pool.h"
#include "timed_task.h"
#include "timer.h"
#include "dbgprt.h"
#include "combat_tick.h"
#include "glogger.h"
#include "game_mall.h"
#include "map_elem_ctrl.h"
#include "global_data.h"
#include "auction_config.h"
#include "achieve_config.h"
#include "sys_mail_config.h"
#include "global_counter.h"


namespace gamed {

using namespace std;
using namespace shared;

///
/// static only in this file 
/// 
static std::string s_root_dir;
static std::string s_config_dir;
static std::string s_gamedata_dir;
static std::string s_evscript_dir;
static std::string s_npcpolicy_dir;
static std::string s_mapscript_dir;
static const char* s_path_separator = "/";
// config/
static std::string s_cfg_player_class_dir;

static const int kMaxPlayerClassCount = playerdef::CLS_MAXCLS_LABEL;
static gmatrixdef::PlayerClassDefaultTempl s_playercls_default_cfg[kMaxPlayerClassCount];
static gmatrixdef::PlayerLvlUpConfigTable  s_player_lvlup_exp_cfg;
static gmatrixdef::PlayerCatVisionConfig   s_playe_cat_vision_cfg;
static gmatrixdef::PetPowerLvlUpConfig     s_pet_power_lvlup_cfg;

///
/// Tick
///
namespace 
{
	// run in separate thread
	void timer_tick(TickIndex index, void* pdata, int remain)
	{
		class World_Tick_Task : public RunnablePool::Task
		{
		public:
			World_Tick_Task(TickIndex index) : tick_index_(index) { }
			virtual void Run()
			{
				s_pGmatrix->HeartbeatTick(tick_index_);
			}

		private:
			const TickIndex tick_index_;
		};

		// 保证Tick心跳能在一个线程里顺序执行（线程池里的0号线程）
		RunnablePool::AddTickTask(new World_Tick_Task(index));
	}
	
} // namespace Anonymous


///
/// Gmatrix static member
///
ObjectManager<Player, TICK_PER_SEC, ObjInsertor, kMaxPlayerCount> Gmatrix::s_player_manager_(ObjQuery::FindPlayer);
ObjectManager<Npc, TICK_PER_SEC, ObjInsertor, kMaxNpcCount> Gmatrix::s_npc_manager_(ObjQuery::FindNpc);
ObjectManager<AreaObj, TICK_PER_SEC, ObjInsertor, kMaxAreaObjCount> Gmatrix::s_areaobj_manager_(ObjQuery::FindAreaObj);
ObjectManager<Matter, TICK_PER_SEC, ObjInsertor, kMaxMatterCount> Gmatrix::s_matter_manager_(ObjQuery::FindMatter);

int Gmatrix::s_gameserver_id_ = -1;
shared::AtomicInt64 Gmatrix::s_cur_highest_obj_id_;
gmatrixdef::ServerParamInfo Gmatrix::s_svr_param_info_;
gmatrixdef::GameVersionInfo Gmatrix::s_game_ver_info_;


///
/// static member function
///

///
/// Player
///
Player* Gmatrix::FindPlayerFromMan(RoleID roleid)
{
	return s_player_manager_.Find(roleid);
}

Player* Gmatrix::AllocPlayer(RoleID roleid)
{
	Player* pplayer = s_player_manager_.Alloc();
	if (pplayer == NULL)
		return NULL;

	size_t index = s_player_manager_.GetIndex(pplayer);
	if (!ObjQuery::MapPlayer(roleid, index))
	{
		s_player_manager_.Free(pplayer);
		LOG_ERROR << "MapPlayer() error! player already online! roleid:" << roleid;
		return NULL;
	}

	return pplayer;
}

void Gmatrix::FreePlayer(Player* player_ptr)
{
	int n = ObjQuery::UnmapPlayer(player_ptr->role_id());
	ASSERT(n == 1);
	s_player_manager_.Free(player_ptr);
}

void Gmatrix::InsertPlayerToMan(Player* player_ptr)
{
	player_ptr->SetActive();
	s_player_manager_.Insert(player_ptr);
	s_pGmatrix->OnlinePlayersChange(player_ptr, true);	
}

void Gmatrix::RemovePlayerFromMan(Player* player_ptr)
{
	player_ptr->ClrActive();
	s_player_manager_.Remove(player_ptr);
	s_pGmatrix->OnlinePlayersChange(player_ptr, false);	
}

///
/// Npc
///
Npc* Gmatrix::FindNpcFromMan(XID::IDType id)
{
	return s_npc_manager_.Find(id);
}

Npc* Gmatrix::AllocNpc()
{
	return s_npc_manager_.Alloc();
}

void Gmatrix::FreeNpc(Npc* npc_ptr)
{
	s_npc_manager_.Free(npc_ptr);
}

void Gmatrix::InsertNpcToMan(Npc* npc_ptr)
{
	npc_ptr->SetActive();
	s_npc_manager_.Insert(npc_ptr);
}

void Gmatrix::RemoveNpcFromMan(Npc* npc_ptr)
{
	npc_ptr->ClrActive();
	s_npc_manager_.Remove(npc_ptr);
}

///
/// AreaObj
///
AreaObj* Gmatrix::FindAreaObjFromMan(XID::IDType id)
{
	return s_areaobj_manager_.Find(id);
}

AreaObj* Gmatrix::AllocAreaObj()
{
	return s_areaobj_manager_.Alloc();
}

void Gmatrix::FreeAreaObj(AreaObj* obj_ptr)
{
	s_areaobj_manager_.Free(obj_ptr);
}

void Gmatrix::InsertAreaObjToMan(AreaObj* area_ptr)
{
	area_ptr->SetActive();
	s_areaobj_manager_.Insert(area_ptr);
}

void Gmatrix::RemoveAreaObjFromMan(AreaObj* area_ptr)
{
	area_ptr->ClrActive();
	s_areaobj_manager_.Remove(area_ptr);
}

///
/// Matter
///
Matter* Gmatrix::FindMatterFromMan(XID::IDType id)
{
	return s_matter_manager_.Find(id);
}

Matter* Gmatrix::AllocMatter()
{
	return s_matter_manager_.Alloc();
}

void Gmatrix::FreeMatter(Matter* matter_ptr)
{
	s_matter_manager_.Free(matter_ptr);
}

void Gmatrix::InsertMatterToMan(Matter* matter_ptr)
{
	matter_ptr->SetActive();
	s_matter_manager_.Insert(matter_ptr);
}

void Gmatrix::RemoveMatterFromMan(Matter* matter_ptr)
{
	matter_ptr->ClrActive();
	s_matter_manager_.Remove(matter_ptr);
}

///
/// get object max count
///
size_t Gmatrix::GetMaxNpcCount()
{
	return kMaxNpcCount;
}

size_t Gmatrix::GetMaxPlayerCount()
{
	return kMaxPlayerCount;
}

size_t Gmatrix::GetMaxAreaObjCount()
{
	return kMaxAreaObjCount;
}

size_t Gmatrix::GetMaxMatterCount()
{
	return kMaxMatterCount;
}

///
/// get object const ptr
///
const Player* Gmatrix::GetPlayerByIndex(size_t index)
{
	return s_player_manager_.GetByIndex(index);
}

const Npc* Gmatrix::GetNpcByIndex(size_t index)
{
	return s_npc_manager_.GetByIndex(index);
}

const AreaObj* Gmatrix::GetAreaObjByIndex(size_t index)
{
	return s_areaobj_manager_.GetByIndex(index);
}

const Matter* Gmatrix::GetMatterByIndex(size_t index)
{
	return s_matter_manager_.GetByIndex(index);
}

///
/// WorldManager
///
int Gmatrix::InsertWorldManager(MapID id, WorldManager* manager, MapTag tag)
{
	return wcluster::InsertWorldManager(id, manager, tag);
}

WorldManager* Gmatrix::FindWorldManager(MapID world_id, MapTag world_tag)
{
	return wcluster::FindWorldManager(world_id, world_tag);
}

void Gmatrix::GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec)
{
	wcluster::GetAllWorldInCharge(id_vec);
}

bool Gmatrix::GetWorldXID(MapID world_id, XID& xid)
{
	WorldManager* pmanager = FindWorldManager(world_id, 0);
	if (NULL == pmanager) 
		return false;

	xid = pmanager->GetWorldXID();
	return true;
}

bool Gmatrix::IsWorldInCharge(MapID world_id)
{
	WorldInChangeSet::const_iterator it = s_pGmatrix->world_inchange_set_.find(world_id);
	if (it != s_pGmatrix->world_inchange_set_.end())
		return true;

	return false;
}

int Gmatrix::GetGameServerID()
{
	return s_gameserver_id_;
}

bool Gmatrix::LoadMapDataGbd(const char* file)
{
	return s_pMapData->ReadFromFile(file);
}

bool Gmatrix::LoadMoveMapGbd(int mapid, const char* path)
{
	return pathfinder::LoadMoveMap(mapid, path);
}

XID::IDType Gmatrix::AssignNpcID(Npc* pnpc)
{
	size_t index = s_npc_manager_.GetIndex(pnpc);
	return MAKE_OBJID<Npc>(index);
}

XID::IDType Gmatrix::AssignAreaObjID(AreaObj* parea)
{
	size_t index = s_areaobj_manager_.GetIndex(parea);
	return MAKE_OBJID<AreaObj>(index);
}

XID::IDType Gmatrix::AssignMatterID(Matter* pmatter)
{
	size_t index = s_matter_manager_.GetIndex(pmatter);
	return MAKE_OBJID<Matter>(index);
}

void Gmatrix::SendObjectMsg(const MSG& msg, bool is_sequence)
{
    if (msg.target.IsObject())
    {
        if (msg.message > GS_MSG_NULL && msg.message < GS_MSG_MAX)
        {
            if (is_sequence)
            {
                s_pGmatrix->obj_msg_seq_queue_.AddMsg(msg);
            }
            else
            {
                s_pGmatrix->obj_msg_queue_.AddMsg(msg);
            }
        }
        else
        {
            LOG_ERROR << "SendObjectMsg recv error type message:" << msg.message;
        }
    }
    else
    {
        LOG_ERROR << "SendObjectMsg msg target is not a Object message:" << msg.message 
            << " target_id:" << msg.target.id << " target_type:" << msg.target.type;
    }
}
	
void Gmatrix::SendWorldMsg(const MSG& msg)
{
	if (msg.target.IsWorld())
    {
        if (msg.message > GS_PLANE_MSG_NULL && msg.message < GS_PLANE_MSG_MAX)
        {
            MapID world_id   = MAP_ID(msg.target);
            MapTag world_tag = MAP_TAG(msg.target);
            s_pGmatrix->wm_msg_seq_queue_.AddMsg(world_id, msg, world_tag);
        }
        else
        {
            LOG_ERROR << "SendWorldMsg recv error type message:" << msg.message;
        }
    }
    else
    {
        LOG_ERROR << "SendWorldMsg msg target is not a World message:" << msg.message 
            << " target_id:" << msg.target.id << " target_type:" << msg.target.type;
    }
}

WorldObject* Gmatrix::LocateObjectFromMsg(const MSG& msg)
{
	return LocateObjectFromXID(msg.target);
}

WorldObject* Gmatrix::LocateObjectFromXID(const XID& target)
{
	switch (target.type)
	{
		case XID::TYPE_PLAYER:
			{
				WorldObject* w_obj = static_cast<WorldObject*>(FindPlayerFromMan(target.id));
				return w_obj;
			}
			break;

		case XID::TYPE_MAP_TRANSFER_AREA:
		case XID::TYPE_MAP_AREA_OBJ:
		case XID::TYPE_MAP_LANDMINE_AREA:
		case XID::TYPE_AREA_OBJ:
			{
				WorldObject* w_obj = static_cast<WorldObject*>(FindAreaObjFromMan(target.id));
				return w_obj;
			}
			break;

		case XID::TYPE_SERVICE_NPC:
		case XID::TYPE_MONSTER:
		case XID::TYPE_NPC:
			{
				WorldObject* w_obj = static_cast<WorldObject*>(FindNpcFromMan(target.id));
				return w_obj;
			}
			break;

		case XID::TYPE_MATTER:
			{
				WorldObject* w_obj = static_cast<WorldObject*>(FindMatterFromMan(target.id));
				return w_obj;
			}
			break;

		default:
			return NULL;
	}

	return NULL;
}

/*
// needed lock outside
int Gmatrix::CallMessageHandler(WorldObject* obj, const MSG& msg)
{
	int rst = -1;

	if (obj) 
	{
		rst = obj->DispatchMessage(msg);
	}

	return rst;
}
*/

void Gmatrix::DispatchObjectMessage(const MSG& msg)
{
	WorldObject* obj = LocateObjectFromMsg(msg);
	if (NULL == obj)
		return;

	// lock and check actived
	WorldObjectLockGuard lock(obj);
	if (!obj->IsActived())
		return;

	if (obj->object_id() == msg.target.id)
	{
		int ret = obj->DispatchMessage(msg);
		if (0 != ret) 
		{
			LOG_ERROR << "MSG handle error, message type:" << msg.message << " return:" << ret;
		}
	}
}

void Gmatrix::DispatchWorldMessage(const WORLDMSG& world_msg)
{
	WorldManager* manager = FindWorldManager(world_msg.world_id, world_msg.world_tag);
	if (NULL == manager) return;
	manager->DispatchMessage(world_msg.msg);
}

void Gmatrix::KickoutPlayerByLinkId(Player* player, void* pdata)
{
	int32_t linkid = *(int32_t*)(pdata);
	if (player->link_id() == linkid)
	{
		if (!player->state().IsOffline())
		{
			player->PlayerLogout(playerdef::LT_LINK_DISCONNECT);
		}
	}
}

void Gmatrix::KickoutPlayerByMasterId(Player* player, void* pdata)
{
	int32_t masterid = *(int32_t*)(pdata);
	if (player->master_id() == masterid)
	{
		if (!player->state().IsOffline())
		{
			player->PlayerLogout(playerdef::LT_MASTER_DISCONNECT);
		}
	}
}

void Gmatrix::KickoutPlayerByGSvrStop(Player* player, void* pdata)
{
	player->PlayerLogout(playerdef::LT_GS_STOP);
}

gmatrixdef::PlayerClassDefaultTempl Gmatrix::GetPlayerClsTemplCfg(int cls)
{
	if (cls < playerdef::CLS_NEWBIE || cls >= playerdef::CLS_MAXCLS_LABEL)
	{
		ASSERT(false && "获取的职业id不在有效范围内");
		return gmatrixdef::PlayerClassDefaultTempl();
	}

	return s_playercls_default_cfg[cls];
}

int16_t Gmatrix::GetPlayerMaxLevel(int16_t cls)
{
	if (cls < playerdef::CLS_NEWBIE || cls >= playerdef::CLS_MAXCLS_LABEL)
	{
		ASSERT(false && "获取的职业id不在有效范围内");
		return -1;
	}
	return s_playercls_default_cfg[cls].templ_ptr->the_highest_level;
}

int32_t Gmatrix::GetPlayerNormalAttackID(int16_t cls)
{
	if (cls < playerdef::CLS_NEWBIE || cls >= playerdef::CLS_MAXCLS_LABEL)
	{
		ASSERT(false && "获取的职业id不在有效范围内");
		return -1;
	}
	return s_playercls_default_cfg[cls].templ_ptr->normal_attack_id;
}

std::string Gmatrix::GetPlayerModelPath(int16_t cls)
{
	if (cls < playerdef::CLS_NEWBIE || cls >= playerdef::CLS_MAXCLS_LABEL)
	{
		ASSERT(false && "获取的职业id不在有效范围内");
		return std::string();
	}
	return s_playercls_default_cfg[cls].templ_ptr->cls_model_src.to_str();
}

int32_t Gmatrix::GetPlayerLevelUpExp(int16_t cur_lvl)
{
	if (cur_lvl <= 0 || cur_lvl >= playerdef::MAX_PLAYER_LEVEL)
	{
		ASSERT(false && "玩家等级超出正常值范围");
		return -1;
	}
	return s_player_lvlup_exp_cfg.lvlup_exp_tbl[cur_lvl];
}

int32_t Gmatrix::GetCatVisionMaxLevel()
{
	/*获取瞄类视觉的最大有效等级*/
	return s_playe_cat_vision_cfg.max_cat_vision_level;
}

int32_t Gmatrix::GetCatVisionLevelUpExp(int16_t cur_lvl)
{
	if (cur_lvl <= 0 || cur_lvl >= playerdef::MAX_CAT_VISION_LEVEL)
	{
		ASSERT(false && "瞄类视觉等级超出正常值范围");
		return -1;
	}
	return s_playe_cat_vision_cfg.lvlup_exp_tbl[cur_lvl];
}

int32_t Gmatrix::GetCatVisionEPSpeedUse()
{
	return s_playe_cat_vision_cfg.ep_speed_use;
}

int32_t Gmatrix::GetCatVisionEPGenSpeed()
{
	return s_playe_cat_vision_cfg.ep_speed_gen;
}

int32_t Gmatrix::GetCatVisionEPGenInterval()
{
	return s_playe_cat_vision_cfg.interval_gen;
}

const gmatrixdef::PetPowerLvlUpConfig& Gmatrix::GetPetPowerLvlUpConfig()
{
	return s_pet_power_lvlup_cfg;
}

void Gmatrix::CombatSendCMD(int64_t roleid, int32_t combat_id, const shared::net::ProtoPacket& packet)
{
	Player* player = FindPlayerFromMan(roleid);
	if (player && player->IsActived())
	{
		int32_t linkid     = player->link_id();
		RoleID roleid      = player->role_id();
		int32_t client_sid = player->sid_in_link();
		NetToLink::SendS2CGameData(linkid, roleid, client_sid, const_cast<shared::net::ProtoPacket&>(packet));
	}
}

void Gmatrix::CombatPVEResult(int64_t roleid, int32_t combat_id, int32_t unit_id, const combat::CombatPVEResult& result, int32_t master_server_id)
{
	Player* player = FindPlayerFromMan(roleid);
	if (!player)
	{
		__PRINTF("CombatPVEResult: 玩家 %ld 不在线!!! 战斗结果写入DB", roleid);
		NetToMaster::SaveCombatPVEResult(master_server_id, combat_id, roleid, unit_id, result);
		return;
	}

	shared::net::ByteBuffer buffer;
    MsgContentMarshal(const_cast<combat::CombatPVEResult&>(result), buffer);

	MSG msg;
	BuildMessage(msg, GS_MSG_COMBAT_PVE_RESULT, XID(roleid, XID::TYPE_PLAYER), XID(combat_id, XID::TYPE_COMBAT), 0, buffer.contents(), buffer.size());
	SendObjectMsg(msg, true);
}

void Gmatrix::CombatPVPResult(int64_t roleid, int32_t combat_id, int32_t unit_id, const combat::CombatPVPResult& result, int32_t master_server_id)
{
	Player* player = FindPlayerFromMan(roleid);
	if (!player)
	{
		__PRINTF("CombatPVPResult: 玩家 %ld 不在线!!! 战斗结果写入DB", roleid);
		NetToMaster::SaveCombatPVPResult(master_server_id, combat_id, roleid, unit_id, result);
		return;
	}

	shared::net::ByteBuffer buffer;
    MsgContentMarshal(const_cast<combat::CombatPVPResult&>(result), buffer);

	MSG msg;
	BuildMessage(msg, GS_MSG_COMBAT_PVP_RESULT, XID(roleid, XID::TYPE_PLAYER), XID(combat_id, XID::TYPE_COMBAT), 0, buffer.contents(), buffer.size());
	SendObjectMsg(msg, true);
}

void Gmatrix::CombatStart(int64_t roleid, int32_t combat_id)
{
	Player* player = FindPlayerFromMan(roleid);
	if (player)
	{
		MSG msg;
		BuildMessage(msg, GS_MSG_COMBAT_START, XID(roleid, XID::TYPE_PLAYER), XID(combat_id, XID::TYPE_COMBAT), 0, 0, 0);
		SendObjectMsg(msg, true);
	}
}

void Gmatrix::CombatPVEEnd(int64_t object_id, 
		                   int64_t world_id, 
						   int32_t combat_id, 
						   bool win, 
						   const std::vector<int64_t> rolelist, 
						   const std::vector<combat::MobKilled>& mob_killed_vec)
{
	if (object_id != 0)
	{
		//通知玩家、怪物或区域战斗结束
		XID target;
		MAKE_XID(object_id, target);
		WorldObject* pobj = LocateObjectFromXID(target);
		if (pobj)
		{
			msg_combat_end param;
			param.set_combat_win(win);
			for (size_t i = 0; i < rolelist.size(); ++i)
			{
				param.push_back(rolelist[i]);
			}
			MSG msg;
			BuildMessage(msg, GS_MSG_COMBAT_PVE_END, target, XID(combat_id, XID::TYPE_COMBAT), 0, &param, sizeof(param));
			SendObjectMsg(msg, true);
		}
	}

	if (world_id != 0)
	{
		//通知副本或战场战斗结束
		XID target = MakeWorldXID(world_id);
		if (IS_INS_MAP(target) || IS_BG_MAP(target))
		{
			msgpack_map_monster_killed param;
			for (size_t i = 0; i < mob_killed_vec.size(); ++ i)
			{
				msgpack_map_monster_killed::monster_entry entry;
				entry.monster_tid = mob_killed_vec[i].mob_tid;
				entry.monster_count = mob_killed_vec[i].mob_count;
				param.monster_list.push_back(entry);
			}

			shared::net::ByteBuffer buf;
			MsgContentMarshal(param, buf);

			MSG msg;
			BuildMessage(msg, GS_PLANE_MSG_MAP_MONSTER_KILLED, target, XID(combat_id, XID::TYPE_COMBAT), 0, buf.contents(), buf.size());
			SendWorldMsg(msg);
		}
	}
}

void Gmatrix::CombatPVPEnd(int64_t object_id, int64_t world_id, int32_t combat_id, bool win)
{
	if (object_id != 0)
	{
		//通知玩家战斗结束
		XID target;
		MAKE_XID(object_id, target);
		WorldObject* pobj = LocateObjectFromXID(target);
		if (pobj)
		{
			MSG msg;
			BuildMessage(msg, GS_MSG_COMBAT_PVP_END, target, XID(combat_id, XID::TYPE_COMBAT), win?1:0, 0, 0);
			SendObjectMsg(msg, true);
		}
	}
}

void Gmatrix::WorldBossStatus(int32_t boss_object_id, const combat::WorldBossCombatStatus& result)
{
	shared::net::ByteBuffer buffer;
    MsgContentMarshal(const_cast<combat::WorldBossCombatStatus&>(result), buffer);

	XID target;
	MAKE_XID(boss_object_id, target);

	MSG msg;
	BuildMessage(msg, GS_MSG_COMBAT_WORLD_BOSS_END, target, XID(0, XID::TYPE_COMBAT), 0, buffer.contents(), buffer.size());
	SendObjectMsg(msg, true);
}

const std::string& Gmatrix::GetPlayerClassCfgDir()
{
	return s_cfg_player_class_dir;
}

const std::string& Gmatrix::GetGameDataDir()
{
	return s_gamedata_dir;
}

const std::string& Gmatrix::GetMapDataDir()
{
    static std::string map_data_dir = s_gamedata_dir + "map/";
	return map_data_dir;
}

const std::string& Gmatrix::GetInsScriptDir()
{
	static std::string ins_script_dir = s_mapscript_dir + "instance/";
    return ins_script_dir;
}

const std::string& Gmatrix::GetBGScriptDir()
{
    static std::string bg_script_dir = s_mapscript_dir + "battleground/";
    return bg_script_dir;
}

const std::string& Gmatrix::GetGameResVersion()
{
    return s_game_ver_info_.res_ver;
}

const gmatrixdef::ServerParamInfo Gmatrix::GetServerParam()
{
	return s_pGmatrix->s_svr_param_info_;
}

MapID Gmatrix::GetRealWorldID(MapID world_id)
{
	// 跨服副本、战场地图需要调整id
	if (s_svr_param_info_.is_cross_realm_svr && MAP_CAN_CR(world_id))
	{
		return CONVERT_TO_CR_MAP(world_id, GetGameServerID());
	}
	return world_id;
}

MapID Gmatrix::ConvertToCRWorldID(MapID world_id)
{
    // 调用这个接口的时候并不知道跨服的gsid，因此用0xFFFF代替
    return CONVERT_TO_CR_MAP(world_id, 0xFFFF);
}

bool Gmatrix::IsCrossRealmServer()
{
	return s_svr_param_info_.is_cross_realm_svr;
}

bool Gmatrix::IsInSameServer(RoleID rhs, RoleID lhs)
{
    int32_t rhs_masterid = s_pGmatrix->GetMasterIdBySvrId(ROLEID_TO_SVRID(rhs));
    int32_t lhs_masterid = s_pGmatrix->GetMasterIdBySvrId(ROLEID_TO_SVRID(lhs));
    return (rhs_masterid == lhs_masterid);
}

void Gmatrix::SvrIdMapToMasterId(int32_t svr_id, int32_t master_id)
{
    s_pGmatrix->SetSvrIdMapToMasterId(svr_id, master_id);
}

void Gmatrix::GetAllMasterId(std::set<int32_t>& masters)
{
    s_pGmatrix->CollectAllMasterId(masters);
}



///
/// member function
///
Gmatrix::Gmatrix()
	: obj_msg_queue_(DispatchObjectMessage),
	  obj_msg_seq_queue_(DispatchObjectMessage),
	  wm_msg_seq_queue_(DispatchWorldMessage)
{
	s_cur_highest_obj_id_.get_and_set(0);
}

Gmatrix::~Gmatrix()
{
}

bool Gmatrix::InitNetmsgProcThread()
{
	if (!s_pLinkMsgProc->StartProcThread()) 
		return false;

	if (!s_pMasterMsgProc->StartProcThread())
		return false;

	return true;
}

void Gmatrix::StopNetmsgProcThread()
{
	s_pLinkMsgProc->StopProcThread();
	s_pMasterMsgProc->StopProcThread();
}

int Gmatrix::InitNetIO(shared::Conf* gmconf)
{
	if (!NetIO::Init(s_pLinkMsgProc->get_recv_queue(), s_pMasterMsgProc->get_recv_queue()))
	{
		LOG_ERROR << "network init error!";
		return -1;
	}

	// link address
	int linkcount = gmconf->get_int_value("LinkServers", "count");
	if (linkcount < 1) 
	{
		LOG_ERROR << "linkserver has no address!";
		return -1;
	}
	while (linkcount > 0)
	{
		char buf[64];
		sprintf(buf, "LinkServer%d", linkcount);
		string l_ip_addr   = gmconf->find(buf, "address");
		int l_port         = gmconf->get_int_value(buf, "port");
		bool l_tcp_nodelay = gmconf->get_int_value(buf, "tcp_nodelay");
		NetIO::set_link_addr(l_ip_addr, l_port, l_tcp_nodelay);

		--linkcount;
	}

	// master address
	int mastercount = gmconf->get_int_value("MasterServers", "count");
	if (!s_svr_param_info_.is_cross_realm_svr && mastercount != 1)
	{
		LOG_ERROR << "non-cross-realm-svr must has only one master!";
		return -1;
	}
	if (mastercount < 1)
	{
		LOG_ERROR << "masterserver has no address!";
		return -1;
	}
	while (mastercount > 0)
	{
		char buf[64];
		sprintf(buf, "MasterServer%d", mastercount);
		string m_ip_addr   = gmconf->find(buf, "address");
		int m_port         = gmconf->get_int_value(buf, "port");
		bool m_tcp_nodelay = gmconf->get_int_value(buf, "tcp_nodelay");
		NetIO::set_master_addr(m_ip_addr, m_port, m_tcp_nodelay);

		--mastercount;
	}

	// netio start
	if (!NetIO::Start())
	{
		LOG_ERROR << "network start thread error!";
		return -1;
	}

	return 0;
}

void Gmatrix::StopNetIO()
{
	NetIO::Stop();
}

int Gmatrix::Init(Conf* gmconf, Conf* gsconf, Conf* aliasconf, int gs_id)
{
	// 
	// 读取并保存全局配置参数
	//
	s_root_dir       = gsconf->find("Template", "Root") + s_path_separator;
	s_config_dir     = s_root_dir + gsconf->find("Template", "Config") + s_path_separator;
	s_gamedata_dir   = s_root_dir + gsconf->find("Template", "GameData") + s_path_separator;
	s_evscript_dir   = s_root_dir + gsconf->find("Template", "EvScript") + s_path_separator;
	s_npcpolicy_dir  = s_root_dir + gsconf->find("Template", "CombatScript") + s_path_separator;
	s_mapscript_dir  = s_root_dir + gsconf->find("Template", "MapScript") + s_path_separator;
	s_gameserver_id_ = (gs_id > 0) ? gs_id : aliasconf->get_int_value("Identify", "ServerID");

	///
	/// 初始化各种目录配置，必须在必须在InitVariousConfig()之前
	///
	if (!InitGlobalDirConfig(s_config_dir))
	{
		__PRINTF("InitGlobalDirConfig error!");
		return -1;
	}

	///
	/// 检查一些预定义的游戏设置的合法性
	///
	if (!CheckOptionsValidity())
	{
		__PRINTF("options error!");
		return -1;
	}

	//
	// load 各种gbd数据文件，必须在InitVariousConfig()之前
	//
	if (!LoadGameBinaryData(gsconf))
	{
		__PRINTF("LoadGameBinaryData返回失败！");
		return -1;
	}
    if (!CheckGameBinaryData())
    {
        __PRINTF("CheckGameBinaryData返回失败！");
        return -1;
    }

	//
	// 初始化各种配置，比如服务器配置、默认角色模板配置等
	//
	if (!InitVariousConfig(gsconf, aliasconf))
	{
		__PRINTF("InitVariousConfig初始化失败！");
		return -1;
	}

	//
	// 初始化事件系统
	//
	if (!InitEventSystem(s_evscript_dir, gsconf))
	{
		__PRINTF("InitEventSystem初始化事件系统失败！");
		return -1;
	}

    //
    // 初始化拍卖行数据
    //
    if (!InitAuctionConfig(s_config_dir, gsconf))
    {
        __PRINTF("InitAuctionConfig初始化失败！");
        return -1;
    }

    //
    // 初始化成就配置数据
    //
    if (!InitAchieveConfig(s_config_dir, gsconf))
    {
        __PRINTF("InitAchieveConfig初始化失败！");
        return -1;
    }

    //
    // 初始化系统邮件格式配置
    //
    if (!InitSysMailConfig(s_config_dir, gsconf))
    {
        __PRINTF("InitSysMailConfig初始化失败！");
        return -1;
    }
    	
	//
	// 初始化地图
	//
	shared::StrToken token;
	const char* delim = ";,\r\n";

	std::vector<string> world_list;
	string servers = aliasconf->find("General","world_servers").c_str();
	if (token.GetTokenData<string>(servers.c_str(), delim, world_list))
	{
		LOG_ERROR << "配置文件gsalias.conf -> [General] -> world_servers 解析错误";
		return -1;
	}

	for (size_t i = 0; i < world_list.size(); ++i)
	{
		int res = wcluster::CreateWorldManager(gsconf, world_list[i].c_str());
		if (res)
		{
			LOG_ERROR<< "wcluster::CreateWorldManager error!";
			return -1;
		}
	}

	//
	// 游戏相关的信息及模块的初始化，必须在LoadGameBinaryData()之后调用
	//
	if (!InitGameRelatedModule())
	{
		__PRINTF("InitGameRelatedModule初始化失败！");
		return -1;
	}

	//
	// 初始化运行时数据，需要在地图全部初始化完毕后调用
	//
	if (!InitRuntimeData())
	{
		__PRINTF("Gmatrix::InitRuntimeData 初始化失败！");
		return -1;
	}

	//
	// Init GLog
	//
	if (!GLogger::Init(gmconf->file_name().c_str(), GLog::GLOG_TYPE_GS, GetGameServerID()))
	{
		__PRINTF("GLog初始化失败！");
		return -1;
	}
	
	//
	// 初始化网络
	// 
	if (0 != InitNetIO(gmconf))
	{
		__PRINTF("网络初始化失败！");
		return -1;
	}

	//
	// 初始化网络消息处理线程
	//
	if (!InitNetmsgProcThread())
	{
		__PRINTF("Netmsg初始化失败！");
		return -1;
	}
	
	///
	/// 开始全局Tick
	///
	g_timer->SetTimer(1, 0, 0, timer_tick, NULL);

	///
	/// 开始战斗心跳线程
	///
	s_pCombatTick->StartThread();

	return 0;
}

void Gmatrix::StopProcess()
{
	///
	/// 资源的释放和删除都放在ReleaseAll()里
	///
	
	// 停止战斗的tick
	s_pCombatTick->StopThread();
	
	// 踢掉所有玩家，必须在StopNetIO()前调用
	ForEachPlayer(KickoutPlayerByGSvrStop, NULL);
	
	// 等待IO send完毕
	sleep(1);
	
	// 停止IO
	StopNetIO();

	// 停止协议处理线程
	StopNetmsgProcThread();
}

void Gmatrix::ReleaseAll()
{
	// Delete所有object
	s_player_manager_.DeleteAllObjects();
	s_npc_manager_.DeleteAllObjects();
	s_areaobj_manager_.DeleteAllObjects();
	s_matter_manager_.DeleteAllObjects();

    // 等待object都释放完毕
	usleep(150000);

    // 删除所有地图
	wcluster::ReleaseAll();

    // 删除战斗系统资源
	combat::Release();
	
	// 释放寻路模块资源
	pathfinder::Release();

	// 释放商城资源
	s_pGameMall->Release();
}

bool Gmatrix::InitGlobalDirConfig(const std::string& config_root)
{
	ASSERT(config_root.size());
	luabind::LuaEngineLite engine;

	// player_class dir
	std::string path = config_root + "global_script.lua";
	luabind::LuaValueArray args;
	args.push_back(s_config_dir);
	if(!engine.Call(path.c_str(), "getPlayerClassDir", &args))
	{
		__PRINTF("LuaEngine::Call Error config/global_script.lua getPlayerClassDir()\n");
		return false;
	}
	engine.PopValue(s_cfg_player_class_dir);
	ASSERT(s_cfg_player_class_dir.size());

	return true;
}

bool Gmatrix::InitVariousConfig(shared::Conf* gsconf, shared::Conf* aliasconf)
{
	if (!InitServerParamInfo(gsconf, aliasconf))
	{
		__PRINTF("InitServerParamInfo 初始化失败！");
		return false;
	}

    if (!InitGameVersionInfo(gsconf))
    {
        __PRINTF("InitGameVersionInfo 初始化失败！");
        return false;
    }

	if (!InitPlayerClassDefaultConfig())
	{
		__PRINTF("PlayerClassDefaultConfig 初始化失败！");
		return false;
	}

	if (!InitPlayerLevelUpExpConfig())
	{
		__PRINTF("PlayerLevelUpExpConfig 初始化失败！");
		return false;
	}

	if (!InitCatVisionLevelUpExpConfig())
	{
		__PRINTF("CatVisionLevelUpExpConfig 初始化失败！");
		return false;
	}

	if (!InitPetPowerLevelUpConfig())
	{
		__PRINTF("InitPetPowerLevelUpConfig 初始化失败！");
		return false;
	}

	return true;
}

bool Gmatrix::InitGameRelatedModule()
{
	//初始化物品管理器
	s_pItemMan->Initialize();
	
	//初始化游戏商城
	s_pGameMall->Initialize();

	//初始化战斗系统
	if (!InitCombat())
		return false;
	return true;
}

bool Gmatrix::InitEventSystem(const std::string& root, shared::Conf* gsconf)
{
	return s_pEventSys->Init(root, gsconf);
}

bool Gmatrix::InitAuctionConfig(const std::string& path, shared::Conf* gsconf)
{
    std::string auctionCfg = gsconf->find("Template", "AuctionCfgFile");
    std::string cfg_path   = "";
    if (auctionCfg.size()) 
    {
        cfg_path = s_config_dir + auctionCfg;
    }
    if (!s_pAuctionCfg->Init(cfg_path.c_str()))
    {
        __PRINTF("拍卖行数据初始化失败！");
        return false;
    }
    return true;
}

bool Gmatrix::InitAchieveConfig(const std::string& path, shared::Conf* gsconf)
{
    std::string achieveCfg = gsconf->find("Template", "AchieveCfgFile");
    std::string cfg_path   = "";
    if (achieveCfg.size()) 
    {
        cfg_path = s_config_dir + achieveCfg;
    }
    if (!s_pAchieveCfg->Init(cfg_path.c_str()))
    {
        __PRINTF("成就配置数据初始化失败！");
        return false;
    }
    return true;
}

bool Gmatrix::InitSysMailConfig(const std::string& path, shared::Conf* gsconf)
{
    std::string sysmailCfg = gsconf->find("Template", "SysMailCfgFile");
    std::string cfg_path   = "";
    if (sysmailCfg.size())
    {
        cfg_path = s_config_dir + sysmailCfg;
    }
    if (!s_pSysMailCfg->Init(cfg_path.c_str()))
    {
        __PRINTF("系统邮件格式配置数据初始化失败！");
        return false;
    }
    return true;
}

bool Gmatrix::InitRuntimeData()
{
	std::vector<gmatrixdef::WorldIDInfo> id_vec;
	GetAllWorldInCharge(id_vec);
	for (size_t i = 0; i < id_vec.size(); ++i)
	{
		ASSERT(world_inchange_set_.insert(id_vec[i].world_id).second);
	}

	if (!s_pMapElemCtrller->Init())
	{
		__PRINTF("s_pMapElemCtrller Init() error!");
		return false;
	}

    // 初始化副本脚本系统
    wcluster::InitInsScriptSys(GetInsScriptDir().c_str());
    // 初始化战场脚本系统
    wcluster::InitBGScriptSys(GetBGScriptDir().c_str());
    // 初始化玩家脚本系统
    Player::InitPlayerScriptSys(GetPlayerClassCfgDir().c_str());

	return true;
}

bool Gmatrix::InitCombat()
{
	//获取GS管理的地图ID列表
	std::vector<int/*mapid*/> mapid_vec;
	std::vector<gmatrixdef::WorldIDInfo> id_vec;
	GetAllWorldInCharge(id_vec);
	for (size_t i = 0; i < id_vec.size(); ++i)
	{
		mapid_vec.push_back(id_vec[i].world_id);
	}
	
	//获取职业属性同步规则
	std::vector<combat::cls_prop_sync_rule> prop_rule_list;
	prop_rule_list.resize(kMaxPlayerClassCount);
	for (int cls = 0; cls < kMaxPlayerClassCount; ++ cls)
	{
		if (s_playercls_default_cfg[cls].templ_ptr)
		{
			combat::cls_prop_sync_rule& entry = prop_rule_list[cls];
			entry.role_cls = cls;
			entry.hp_sync_rule   = s_playercls_default_cfg[cls].templ_ptr->properties[0].sync_rule;
			entry.mp_sync_rule   = s_playercls_default_cfg[cls].templ_ptr->properties[1].sync_rule;
			entry.ep_sync_rule   = s_playercls_default_cfg[cls].templ_ptr->properties[2].sync_rule;
			entry.power_gen_rule = s_playercls_default_cfg[cls].templ_ptr->power_gen_type;
		}
	}

	//初始化战斗系统
	if (!combat::Initialize(mapid_vec, s_npcpolicy_dir, prop_rule_list))
		return false;

	//初始化战斗系统回调函数
	combat::SetCombatCMDSenderCallBack(BIND_FREE_CB(gamed::Gmatrix::CombatSendCMD));
	combat::SetCombatPVEResultCallBack(BIND_FREE_CB(gamed::Gmatrix::CombatPVEResult));
	combat::SetCombatPVPResultCallBack(BIND_FREE_CB(gamed::Gmatrix::CombatPVPResult));
	combat::SetCombatStartCallBack(BIND_FREE_CB(gamed::Gmatrix::CombatStart));
	combat::SetCombatPVEEndCallBack(BIND_FREE_CB(gamed::Gmatrix::CombatPVEEnd));
	combat::SetCombatPVPEndCallBack(BIND_FREE_CB(gamed::Gmatrix::CombatPVPEnd));
	combat::SetWorldBossStatusCallBack(BIND_FREE_CB(gamed::Gmatrix::WorldBossStatus));
	return true;
}

bool Gmatrix::InitServerParamInfo(shared::Conf* gsconf, shared::Conf* aliasconf)
{
	// 是否是跨服的gs
	{
		std::string str = aliasconf->find("ServerParamInfo", "cross_realm_svr");
		if (strcmp(str.c_str(), "true") == 0)
		{
			s_svr_param_info_.is_cross_realm_svr = true;
			LOG_INFO << "############################################";
			LOG_INFO << "######## 注意：本服务器为跨服服务器 ########";
			LOG_INFO << "############################################";
		}
		else
		{
			s_svr_param_info_.is_cross_realm_svr = false;
		}
	}

	// 是否需要debug printf
	{
		bool is_debug_print = false;
		std::string tmpstr = gsconf->find("ServerGlobalParam", "debug_print_mode");
		if (strcmp(tmpstr.c_str(), "true") == 0)
		{
			is_debug_print = true;
			LOG_INFO << "DEBUG_PRINTF_MODE is true!";
		}

		SetDebugPrintMode(is_debug_print);
		s_svr_param_info_.permit_debug_print = is_debug_print;
	}

    // 是否允许debug cmd
    {
        bool is_debug_cmd = false;
        std::string tmpstr = gsconf->find("ServerGlobalParam", "debug_cmd_permit");
		if (strcmp(tmpstr.c_str(), "true") == 0)
		{
			is_debug_cmd = true;
			LOG_INFO << "DEBUG_CMD_PERMIT is true!";
		}

        s_svr_param_info_.permit_debug_cmd = is_debug_cmd;
    }

    // 是否要禁止打印移动相关的cmd
    {
        bool forbid_move_print = false;
        std::string tmpstr = gsconf->find("ServerGlobalParam", "forbid_move_print");
        if (strcmp(tmpstr.c_str(), "true") == 0)
        {
            forbid_move_print = true;
            LOG_INFO << "FORBID_MOVE_PRINT is true!";
        }
        
        s_svr_param_info_.forbid_move_print = forbid_move_print;
    }

	return true;
}

bool Gmatrix::InitGameVersionInfo(shared::Conf* gsconf)
{
	std::string gameverfile = s_config_dir;
    gameverfile += gsconf->find("Template", "GameVersionFile");

    shared::Conf game_ver_conf; 
    if (game_ver_conf.Init(gameverfile.c_str()))
    {
        s_game_ver_info_.res_ver = game_ver_conf.find("GameVersion", "resource_version");
    }

    return true;
}

void Gmatrix::SetDebugPrintMode(bool is_debug_print)
{
	gamed::__SETPRTFLAG(is_debug_print);
	combat::__SETPRTFLAG(is_debug_print);
	task::__SETPRTFLAG(is_debug_print);
	skill::__SETPRTFLAG(is_debug_print);
}

bool Gmatrix::InitPlayerClassDefaultConfig()
{
	luabind::LuaEngineLite engine;

	ASSERT(s_cfg_player_class_dir.size());
	std::string path = s_cfg_player_class_dir + "player_class_template.lua";
	for (int i = 0; i < kMaxPlayerClassCount; ++i)
	{
		gmatrixdef::PlayerClassDefaultTempl& cls_default = s_playercls_default_cfg[i];
		luabind::LuaValueArray args;
		args.push_back(i);
		if(!engine.Call(path.c_str(), "getPlayerClassDefaultConfig", &args))
		{
			__PRINTF("LuaEngine::Call Error config/player_class_template.lua\n");
			return false;
		}
		engine.PopValue(cls_default.cls_templ_id);
		engine.PopValue(cls_default.level);
		engine.PopValue(cls_default.dir);
		cls_default.templ_ptr = s_pDataTempl->QueryDataTempl<dataTempl::PlayerClassTempl>(cls_default.cls_templ_id);
	}

	return true;
}

bool Gmatrix::InitPlayerLevelUpExpConfig()
{
	std::vector<const dataTempl::GlobalConfigTempl *> list;
	s_pDataTempl->QueryDataTemplByType(list);
	ASSERT(list.size() == 1);

	const PlayerLvlUpExpConfig& tpl = list[0]->player_lvlup_exp_config;
	for (size_t i = 0; i < tpl.lvlup_exp_tbl.size(); ++ i)
		s_player_lvlup_exp_cfg.lvlup_exp_tbl[i] = tpl.lvlup_exp_tbl[i];

	return true;
}

bool Gmatrix::InitCatVisionLevelUpExpConfig()
{
	const PlayerCatVisionConfig& tpl = s_pDataTempl->QueryGlobalConfigTempl()->player_cat_vision_config;
	s_playe_cat_vision_cfg.ep_speed_use = tpl.ep_speed_use;
	s_playe_cat_vision_cfg.ep_speed_gen = tpl.ep_speed_gen;
	s_playe_cat_vision_cfg.interval_gen = tpl.interval_gen;

	for (int i = 0; i < playerdef::MAX_CAT_VISION_LEVEL; ++ i)
    {
		s_playe_cat_vision_cfg.lvlup_exp_tbl[i] = 0;
    }

	size_t i = 0;
	for (; i < tpl.lvlup_exp_table.size(); ++ i)
    {
		s_playe_cat_vision_cfg.lvlup_exp_tbl[i] = tpl.lvlup_exp_table[i];
    }

	s_playe_cat_vision_cfg.max_cat_vision_level = i+1;
	return true;
}

bool Gmatrix::InitPetPowerLevelUpConfig()
{
	const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);
	const dataTempl::PetLvlUpPowerConfig& tpl = pTpl->pet_lvlup_power_config;

	s_pet_power_lvlup_cfg.power_lvlup_tbl.resize(1);
	s_pet_power_lvlup_cfg.power_lvlup_tbl[0].probability = tpl.power_lvlup_table[0].probability;
	s_pet_power_lvlup_cfg.power_lvlup_tbl[0].power_inc_on_lvlup = tpl.power_lvlup_table[0].power_inc_on_lvlup;

	for (size_t i = 1; i < tpl.power_lvlup_table.size(); ++ i)
	{
		gmatrixdef::PetPowerLvlUpConfig::PowerEntry entry;
		entry.probability = s_pet_power_lvlup_cfg.power_lvlup_tbl[i-1].probability + tpl.power_lvlup_table[i].probability;
		entry.power_inc_on_lvlup = tpl.power_lvlup_table[i].power_inc_on_lvlup;
		s_pet_power_lvlup_cfg.power_lvlup_tbl.push_back(entry);
	}
	return true;
}

bool Gmatrix::LoadGameBinaryData(Conf* gsconf)
{
	std::string datatemplfile = gsconf->find("Template", "DataTemplFile");
	datatemplfile = s_gamedata_dir + datatemplfile;
	if (!LoadDataTemplGbd(datatemplfile.c_str()))
	{
		__PRINTF("LoadDataTemplGbd() failure!");
		return false;
	}
	
	shared::StrToken token;
	const char* delim = ";,\r\n";
	std::vector<std::string> file_list;
	std::string files = gsconf->find("Template", "ExtraDataFile");
	if (token.GetTokenData<string>(files.c_str(), delim, file_list))
	{
		__PRINTF("解析ExtraData文件列表错误.");
		return false;
	}

	for (size_t i = 0; i < file_list.size(); ++ i)
	{
		std::string extradatafile = s_gamedata_dir + file_list[i];
		if (!LoadExtraDataGbd(extradatafile.c_str()))
		{
			__PRINTF("LoadExtraDataGbd() failure!");
			return false;
		}
	}

	// path finder load transfer table
	std::string transtable = s_gamedata_dir;
	transtable += gsconf->find("Template", "TransferTable");
	if (!LoadTransferTableGbd(transtable.c_str()))
	{
		__PRINTF("LoadTransferTableGbd() failure!");
		return false;
	}

	// load skill-data
	std::string skilldatafile = s_gamedata_dir;
	skilldatafile += gsconf->find("Template", "SkillDataFile");
	string actiondatafile = s_gamedata_dir;
    actiondatafile += gsconf->find("Template", "ActionDataFile");
	if (!LoadSkillDataGbd(skilldatafile.c_str(), actiondatafile.c_str()))
	{
		__PRINTF("LoadSkillDataGbd() failure!");
		return false;
	}

	// load task-data
	std::string taskdatafile = s_gamedata_dir;
	taskdatafile += gsconf->find("Template", "TaskDataFile");
	if (!LoadTaskDataGbd(taskdatafile.c_str()))
	{
		__PRINTF("LoadTaskDataGbd() failure!");
		return false;
	}

	// load achieve-data
	std::string achievedatafile = s_gamedata_dir;
	achievedatafile += gsconf->find("Template", "AchieveDataFile");
	if (!LoadAchieveDataGbd(achievedatafile.c_str()))
	{
		__PRINTF("LoadAchieveDataGbd() failure!");
		return false;
	}

	return true;
}

bool Gmatrix::CheckGameBinaryData() const
{
    std::vector<const extraTempl::MonsterGroupTempl*> group_vec;
	s_pExtraTempl->QueryExtraTemplByType<extraTempl::MonsterGroupTempl>(group_vec);
    for (size_t i = 0; i < group_vec.size(); ++i)
    {
        const extraTempl::MonsterGroupTempl* ptpl = group_vec[i];
        for (size_t j = 0; j < ptpl->monster_list.size(); ++j)
        {
            int32_t tid = ptpl->monster_list[j].monster_tid;
            if (s_pDataTempl->QueryDataTempl<dataTempl::MonsterTempl>(tid) == NULL)
            {
                __PRINTF("怪物组:%d 中的怪物:%d 在数据模板里没有找到！！", ptpl->templ_id, tid);
                return false;
            }
        }
    }
    return true;
}

bool Gmatrix::LoadDataTemplGbd(const char* file)
{
	if (!s_pDataTempl->ReadFromFile(file))
		return false;

	if (!s_pDataTempl->CheckAllTemplate())
		return false;

	return true;
}

bool Gmatrix::LoadExtraDataGbd(const char* file)
{
	return s_pExtraTempl->ReadFromFile(file);
}

bool Gmatrix::LoadTransferTableGbd(const char* file)
{
	return pathfinder::InitGlobalTransTable(file);
}

bool Gmatrix::LoadSkillDataGbd(const char* skill_file, const char* action_file)
{
    return skill::InitSkillData(skill_file) && skill::InitActionData(action_file);
}

bool Gmatrix::LoadTaskDataGbd(const char* file)
{
	return task::InitTaskSys(file);
}

bool Gmatrix::LoadAchieveDataGbd(const char* file)
{
	return achieve::InitAchieveSys(file);
}

void Gmatrix::HeartbeatTick(TickIndex tick_index)
{
	//MutexLockTimedGuard lock(heartbeat_mutex_);

	// obj heartbeat
	s_player_manager_.Heartbeat();
	s_npc_manager_.Heartbeat();
	s_areaobj_manager_.Heartbeat();
	s_matter_manager_.Heartbeat();

	// msg queue dispatch
	obj_msg_queue_.Dispatch();
	obj_msg_seq_queue_.DispatchSpec();
	wm_msg_seq_queue_.DispatchSpec();

	// worldmanagers heartbeat
	wcluster::HeartbeatTick();

	// game-mall heartbeat
	s_pGameMall->HeartbeatTick();

	// map element controller
	s_pMapElemCtrller->HeartbeatTick();

    // global counter 
    s_pGCounter->HeartbeatTick();
}

void Gmatrix::OnlinePlayersChange(Player* player_ptr, bool is_added_player)
{
	if (is_added_player)
	{
		MutexLockTimedGuard lock(online_players_mutex_);
		if (!online_players_.insert(std::pair<RoleID, int32_t>(player_ptr->role_id(), player_ptr->link_id())).second)
		{
			ASSERT(false);
		}
	}
	else
	{
		MutexLockTimedGuard lock(online_players_mutex_);
		size_t n = online_players_.erase(player_ptr->role_id());
		(void)n; ASSERT(n == 1);
	}
}

void Gmatrix::ForEachPlayer(EachPlayerCallback a_callback, void* pdata)
{
	MutexLockTimedGuard lock(online_players_mutex_);
	OnlinePlayersMap::iterator it = online_players_.begin();
	for (; it != online_players_.end(); ++it)
	{
		Player* player_ptr = FindPlayerFromMan(it->first);
		WorldObjectLockGuard lock(player_ptr);
		if(player_ptr->IsActived()) 
		{
			a_callback(player_ptr, pdata);
		}
	}
}

void Gmatrix::LinkDisconnect(int32_t linkid)
{
	// kick out player
	ForEachPlayer(KickoutPlayerByLinkId, &linkid);
}

void Gmatrix::MasterDisconnect(int32_t masterid)
{
	// global data
	s_pGlobalData->ClearByMasterId(masterid);

    // global counter
    s_pGCounter->ClearByMasterId(masterid);

	// kick out player
	ForEachPlayer(KickoutPlayerByMasterId, &masterid);

    // svrid to masterid remove
    ClrSvrIdMapToMasterId(masterid);
}

void Gmatrix::SetSvrIdMapToMasterId(int32_t svr_id, int32_t master_id)
{
    MutexLockGuard lock(svr_mid_map_mutex_);
    SvrIdToMasterIdMap::const_iterator it = svrid_to_masterid_map_.find(svr_id);
    ASSERT(it == svrid_to_masterid_map_.end());
    svrid_to_masterid_map_[svr_id] = master_id;
}

void Gmatrix::ClrSvrIdMapToMasterId(int32_t master_id)
{
    MutexLockGuard lock(svr_mid_map_mutex_);
    SvrIdToMasterIdMap::iterator it = svrid_to_masterid_map_.begin();
    while (it != svrid_to_masterid_map_.end())
    {
        if (it->second == master_id)
        {
            svrid_to_masterid_map_.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

int Gmatrix::GetMasterIdBySvrId(int32_t svr_id) const
{
    MutexLockGuard lock(svr_mid_map_mutex_);
    SvrIdToMasterIdMap::const_iterator it = svrid_to_masterid_map_.find(svr_id);
    if (it != svrid_to_masterid_map_.end())
    {
        return it->second;
    }
    return -1;
}

void Gmatrix::CollectAllMasterId(std::set<int32_t>& masters) const
{
    MutexLockGuard lock(svr_mid_map_mutex_);
    SvrIdToMasterIdMap::const_iterator it = svrid_to_masterid_map_.begin();
    for (; it != svrid_to_masterid_map_.end(); ++it)
    {
        masters.insert(it->second);
    }
}

} // namespace gamed
