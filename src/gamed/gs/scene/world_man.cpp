#include "world_man.h"

#include "shared/logsys/logging.h"

#include "gs/scene/instance/ins_worldman_imp.h"
#include "gs/scene/battle/bg_worldman_imp.h"
#include "gs/scene/instance/ins_cluster.h"
#include "gs/scene/battle/bg_cluster.h"
#include "gs/global/gmatrix.h"
#include "gs/global/game_def.h"
#include "gs/global/dbgprt.h"
#include "gs/global/message.h"
#include "gs/obj/npc_gen.h"
#include "gs/obj/area_gen.h"
#include "gs/obj/matter_gen.h"
#include "gs/obj/npc.h"
#include "gs/obj/area.h"
#include "gs/obj/matter.h"
#include "gs/player/player.h"

#include "gamed/client_proto/G2C_proto.h"
#include "gs/netmsg/send_to_link.h"

#include "base_wm_imp.h"
#include "world_cluster.h"


namespace gamed {

using namespace shared;

namespace
{
	static int kCountdownPerSecond  = TICK_PER_SEC / kWorldHeartbeatInterval;

	void str2rect(rect& rt, const char* str)
	{
		sscanf(str,"{%f,%f} , {%f,%f}", &rt.left, &rt.top, &rt.right, &rt.bottom);
	}

	void SendToPlayer(RoleID roleid, int32_t linkid, int32_t client_sid, shared::net::ProtoPacket& packet)
	{
		NetToLink::SendS2CGameData(linkid, roleid, client_sid, packet);
	}

} // Anonymous


///
/// runnable task
///
class SwitchWorldTask : public RunnablePool::Task
{
public:
	SwitchWorldTask(MapID src_map, MapID des_map, const A2DVECTOR& des_pos, RoleID roleid)
		: src_world_(src_map),
		  des_world_(des_map),
		  player_roleid_(roleid),
		  des_pos_(des_pos)
	{ }

	virtual void Run()
	{
		// 找到player
		Player* pPlayer = Gmatrix::FindPlayerFromMan(player_roleid_);
		if (NULL == pPlayer)
			return; 

		PlayerLockGuard lock(pPlayer); 
		if (!pPlayer->IsActived()) 
			return;

		// 状态不对
		if (!pPlayer->CanSwitch())
		{
			__PRINTF("Player:%ld 状态不正常，无法进行地图切换！", player_roleid_);
			return;
		}

		if (pPlayer->world_id() != src_world_)
		{
			__PRINTF("Player:%ld 地图切换错误，原地图不一致！", player_roleid_);
			return;
		}

		if (pPlayer->world_id() == des_world_)
		{
			__PRINTF("Player:%ld 地图切换错误，玩家已经完成切换！", player_roleid_);
			return;
		}

		// 查找目标世界
		WorldManager* targetWorld = Gmatrix::FindWorldManager(des_world_, 0);
		if (NULL == targetWorld)
		{
			__PRINTF("玩家%ld地图切换找不到目标地图World:%d", player_roleid_, des_world_);
			return;
		}

		// 查找玩家现在所在世界
		WorldManager* sourceWorld = Gmatrix::FindWorldManager(src_world_, 0);
		if (NULL == sourceWorld)
		{
			__PRINTF("玩家%ld地图切换找不到原地图World:%d", player_roleid_, src_world_);
			return;
		}

		ASSERT(targetWorld != sourceWorld);
		ASSERT(targetWorld->PosInWorld(des_pos_));
		
		// 先从原地图中移出
		pPlayer->LeaveMap();
		sourceWorld->RemovePlayer(pPlayer);
		
		// 位置在前面的MSG request处理时已经验证过
		A2DVECTOR oldpos = pPlayer->pos();
		pPlayer->set_pos(des_pos_);

		// 插入新地图
		if (targetWorld->InsertPlayer(pPlayer) < 0)
		{
			// error
			pPlayer->set_pos(oldpos);
			sourceWorld->InsertPlayer(pPlayer);
			pPlayer->EnterMap();
			ASSERT(false);
		}

		// success
		pPlayer->EnterMap();
		pPlayer->NotifySelfPos();
	}

private:
	MapID     src_world_;
	MapID     des_world_;
	RoleID    player_roleid_;
	A2DVECTOR des_pos_;
};


///
/// WorldManager
///
WorldManager::WorldManager()
	: is_actived_(false),
	  plane_(NULL),
	  obj_gen_owner_(0),
	  npc_gen_(NULL),
	  area_gen_(NULL),
	  matter_gen_(NULL),
	  wm_type_(INVALID_WORLD),
	  pimp_(NULL),
	  is_forbid_obj_insert_(false),
	  sec_hb_counter_(0)
{
}

WorldManager::~WorldManager()
{
}

void WorldManager::Release()
{
	if (IsObjGenOwner())
	{
		DELETE_SET_NULL(npc_gen_);
		DELETE_SET_NULL(area_gen_);
		DELETE_SET_NULL(matter_gen_);
	}

	is_actived_           = false;
	min_visible_range_    = 0.0f;
	max_visible_range_    = 0.0f;
	wm_type_              = INVALID_WORLD;
	obj_gen_owner_        = 0;
	is_forbid_obj_insert_ = false;
	sec_hb_counter_       = 0;

	DELETE_SET_NULL(pimp_);
	SAFE_DELETE(plane_);
}

bool WorldManager::IsActived() const
{
	return is_actived_;
}

void WorldManager::SetActive()
{
	is_actived_ = true;
}

void WorldManager::ClrActive()
{
	is_actived_ = false;
}

bool WorldManager::CreateNewWorld()
{
	ASSERT(plane_ == NULL);
	plane_ = new World();
	if (plane_ == NULL)
	{
		LOG_ERROR << "new World() error, plane_ is NULL!";
		return false;
	}
	return true;
}

bool WorldManager::CreateWorldPlane()
{
	return CreateNewWorld();
}

bool WorldManager::Init(shared::Conf* pconf, const char* world_name, WorldManType wmtype, int32_t worldtag)
{
	Conf::section_type section = "World_";
	section += world_name;

	///
	/// new world
	///
	if (plane_ == NULL && !CreateNewWorld())
	{
		LOG_ERROR << "CreateNewWorld() error!";
		return false;
	}

	///
	/// base data
	///
	if (0 != InitBase(pconf, section.c_str(), worldtag))
	{
		LOG_ERROR << "WorldManager::InitBase() error!";
		return false;
	}

	///
	/// runtime data: 
	///      must before InitGenWorldObjects(), after InitBase()
	///
	if (!InitRuntimeModule(wmtype))
	{
		LOG_ERROR << "InitRuntimeModule() error!";
		return false;
	}
	
	///
	/// object gen
	///
	if (!InitGenWorldObjects())
	{
		LOG_ERROR << "InitGenWorldObjects() error!";
		return false;
	}

	///
	/// OnInit(): always the last one
	///
	if (!OnInit())
	{
		LOG_ERROR << "WorldManager derive class OnInit() error!";
		return false;
	}
	return true;
}

int WorldManager::InitBase(shared::Conf* pconf, const char* section, int32_t worldtag)
{
	ASSERT(plane_ != NULL);
	// 读取world的索引和标签
	plane_->set_world_id(pconf->get_int_value(section, "world_id"));
	// 副本等特殊地图会填写world_tag
	plane_->set_world_tag(worldtag);
	
	///
	/// 读取玩家最大视野
	///
	std::string sight_range = pconf->find(section, "sight_range");
	float min_vision = 0.f;
	float max_vision = 0.f;
	sscanf(sight_range.c_str(), "{%f,%f}", &min_vision, &max_vision);
	if (min_vision < 4.f)
	{
		min_visible_range_ = kDefaultSightRange;
		max_visible_range_ = min_visible_range_ * 1.5f;
	}
	else
	{
		min_visible_range_ = min_vision;
		max_visible_range_ = max_vision;
	}
	if (min_visible_range_ > max_visible_range_) 
	{
		return -1;
	}

	__PRINTF("地图%d可视范围:%lf ~ %lf", plane_->world_id(), min_visible_range_, max_visible_range_);


	///
	/// Init World Plane
	///
	world::WorldInitData init_data;
	init_data.world_id   = GetWorldID();
	init_data.world_tag  = GetWorldTag();

	std::string grid_str = pconf->find(section, "grid");
	int row      = 800;
	int column   = 800;
	float xstart = 0.f;
	float ystart = 0.f;
	float step   = 12.5f;
	sscanf(grid_str.c_str(), "{%d,%d,%f,%f,%f}", &column, &row, &step, &xstart, &ystart);
	init_data.grid.row    = row;
	init_data.grid.column = column;
	init_data.grid.step   = step;
	init_data.grid.xstart = xstart;
	init_data.grid.ystart = ystart;

	rect local_rt;
	str2rect(local_rt, pconf->find(section, "local_region").c_str());
	init_data.grid.local_rt = local_rt;
	
	// 广播视野
	init_data.visible.min_visible_range = min_visible_range_;
	init_data.visible.max_visible_range = max_visible_range_;

	if (!plane_->Init(init_data))
	{
		LOG_ERROR << "World::Init() error!";
		return -1;
	}

	return 0;
}

bool WorldManager::InitRuntimeModule(WorldManType wmtype)
{
	wm_type_ = wmtype;

	switch (wmtype)
	{
		case NORMAL_WORLD:
			{
				NpcGenerator* npcgen       = new NpcGenerator(GetWorldID());
				AreaGenerator* areagen     = new AreaGenerator(GetWorldID());
				MatterGenerator* mattergen = new MatterGenerator(GetWorldID());
				SetObjGenerator(npcgen, areagen, mattergen, (int64_t)this);
				pimp_ = new WorldManagerImp(*this);
			}
			break;

		case INSTANCE_WORLD:
			{
				pimp_ = new InsWorldManImp(*this);
			}
			break;

        case BATTLEGROUND_WORLD:
            {
                pimp_ = new BGWorldManImp(*this);
            }
            break;

		default:
			return false;
	}

	return true;
}

bool WorldManager::InitGenWorldObjects()
{
	if (!InitGenNpcs())
	{
		LOG_ERROR << "InitGenNpcs() error!";
		return false;
	}

	if (!InitGenAreas())
	{
		LOG_ERROR << "InitGenAreas() error!";
		return false;
	}

	if (!InitGenMatters())
	{
		LOG_ERROR << "InitGenMatters() error!";
		return false;
	}

	return true;
}

bool WorldManager::InitGenNpcs()
{
	std::vector<Npc*> tmp_npc_list;

	if (npc_gen_->Init()) 
	{
		if (!npc_gen_->GenerateAllNpc(tmp_npc_list))
		{
			LOG_ERROR << "npc_gen_.GenerateAllNpc() error!";
			return false;
		}
	}
	else
	{
		LOG_ERROR << "npc_gen_.Init() error";
		return false;
	}

	std::vector<Npc*>::iterator it = tmp_npc_list.begin();
	for (; it != tmp_npc_list.end(); ++it)
	{
		AcceptingNpc(*it);	
	}
	
	return true;
}

bool WorldManager::InitGenAreas()
{
	std::vector<AreaObj*> tmp_area_list;

	if (area_gen_->Init()) 
	{
		if (!area_gen_->GenerateAllAreas(tmp_area_list))
		{
			LOG_ERROR << "area_gen_.GenerateAllArea() error!";
			return false;
		}
	}
	else
	{
		LOG_ERROR << "area_gen_.Init() error";
		return false;
	}

	std::vector<AreaObj*>::iterator it = tmp_area_list.begin();
	for (; it != tmp_area_list.end(); ++it)
	{
		AcceptingAreaObj(*it);	
	}

	return true;
}

bool WorldManager::InitGenMatters()
{
	std::vector<Matter*> tmp_matter_list;

	if (matter_gen_->Init()) 
	{
		if (!matter_gen_->GenerateAllMatter(tmp_matter_list))
		{
			LOG_ERROR << "matter_gen_.GenerateAllMatter() error!";
			return false;
		}
	}
	else
	{
		LOG_ERROR << "matter_gen_.Init() error";
		return false;
	}

	std::vector<Matter*>::iterator it = tmp_matter_list.begin();
	for (; it != tmp_matter_list.end(); ++it)
	{
		AcceptingMatter(*it);	
	}
	
	return true;
}

bool WorldManager::OnInit()
{
	return pimp_->OnInit();
}

void WorldManager::OnHeartbeat()
{
	if ((++sec_hb_counter_) % kCountdownPerSecond == 0)
	{
		// 按秒做心跳
		MutexLockGuard lock(wm_imp_mutex_);
		pimp_->OnHeartbeat();
	}
}

void WorldManager::SetObjGenerator(NpcGenerator* npcgen, AreaGenerator* areagen, MatterGenerator* mattergen, int64_t ownerptr)
{
	ASSERT(npcgen != NULL && areagen != NULL && mattergen != NULL);

	obj_gen_owner_ = ownerptr;
	npc_gen_       = npcgen;
	area_gen_      = areagen;
	matter_gen_    = mattergen;
}

void WorldManager::SetWorldTag(int32_t world_tag)
{
	plane_->set_world_tag(world_tag);
}

void WorldManager::Heartbeat()
{
	plane_->Heartbeat();

	OnHeartbeat();
}

int WorldManager::InsertPlayer(Player* player_ptr)
{
	if (NULL == player_ptr) 
		return -1;

	if (is_forbid_obj_insert_)
		return -1;

	int ret = -1;
	// imp needed lock
	{
        // 下面几行的顺序要小心，imp里可能会用到world_id因此要放在plane_插入之后
        MutexLockGuard lock(wm_imp_mutex_);
        ret = plane_->InsertPlayer(player_ptr);
        if (ret >= 0)
        {
            if (pimp_->OnInsertPlayer(player_ptr))
            {
                pimp_->OnPlayerEnterMap(player_ptr->role_id());
            }
            else
            {
                ret = -2;
            }
        }
    }

	return ret;
}
	
void WorldManager::RemovePlayer(Player* player_ptr)
{
	ASSERT(player_ptr != NULL);
	// imp needed lock
	{
		MutexLockGuard lock(wm_imp_mutex_);
		pimp_->OnPlayerLeaveMap(player_ptr->role_id());
	}
	plane_->RemovePlayer(player_ptr);
}

int WorldManager::InsertNpc(Npc* npc_ptr)
{
	if (NULL == npc_ptr)
		return -1;

	if (is_forbid_obj_insert_)
		return -1;

	return plane_->InsertNpc(npc_ptr);
}

void WorldManager::RemoveNpc(Npc* npc_ptr)
{
	ASSERT(npc_ptr != NULL);
	plane_->RemoveNpc(npc_ptr);
}

int WorldManager::InsertAreaObj(AreaObj* area_ptr)
{
	if (NULL == area_ptr)
		return -1;
	
	if (is_forbid_obj_insert_)
		return -1;

	return plane_->InsertAreaObj(area_ptr);
}

void WorldManager::RemoveAreaObj(AreaObj* area_ptr)
{
	ASSERT(area_ptr != NULL);
	plane_->RemoveAreaObj(area_ptr);
}

int WorldManager::InsertMatter(Matter* matter_ptr)
{
	if (NULL == matter_ptr)
		return -1;
	
	if (is_forbid_obj_insert_)
		return -1;

	return plane_->InsertMatter(matter_ptr);
}
void WorldManager::RemoveMatter(Matter* matter_ptr)
{
	ASSERT(matter_ptr != NULL);
	plane_->RemoveMatter(matter_ptr);
}

void WorldManager::RemoveSelfFromCluster()
{
	wcluster::RemoveWorldManager(this);
}

void WorldManager::SendObjectMSG(int message, const XID& target, int64_t param) const
{
	plane_->SendObjectMSG(message, target, param);
}

void WorldManager::SendObjectMSG(int message, const XID& target, const void* buf, size_t len) const
{
	plane_->SendObjectMSG(message, target, buf, len);
}

void WorldManager::SendWorldMSG(int message, const XID& target, int64_t param) const
{
	plane_->SendWorldMSG(message, target, param);
}

void WorldManager::SendWorldMSG(int message, const XID& target, const void* buf, size_t len) const
{
	plane_->SendWorldMSG(message, target, buf, len);
}

int WorldManager::DispatchMessage(const MSG& msg)
{
	return OnDispatchMessage(msg);
}

int WorldManager::OnDispatchMessage(const MSG& msg)
{
	return MessageHandler(msg);
}

int WorldManager::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_PLANE_MSG_RULE_AREA_ENABLE:
			{
				ASSERT(msg.source.IsArea());
				plane_->InsertActiveRuleArea(static_cast<int32_t>(msg.param));
			}
			break;

		case GS_PLANE_MSG_RULE_AREA_DISABLE:
			{
				ASSERT(msg.source.IsArea());
				plane_->RemoveActiveRuleArea(static_cast<int32_t>(msg.param));
			}
			break;

		case GS_PLANE_MSG_NPC_REBORN:
			{
				ASSERT(msg.source.IsNpc());
				CHECK_CONTENT_PARAM(msg, plane_msg_npc_reborn);
				plane_msg_npc_reborn* param = (plane_msg_npc_reborn*)msg.content;
				RecreateNpc(param->elem_id, param->templ_id);
			}
			break;

		case GS_PLANE_MSG_MATTER_REBORN:
			{
				ASSERT(msg.source.IsMatter());
				CHECK_CONTENT_PARAM(msg, plane_msg_matter_reborn);
				plane_msg_matter_reborn* param = (plane_msg_matter_reborn*)msg.content;
				RecreateMatter(param->elem_id, param->templ_id);
			}
			break;

		case GS_PLANE_MSG_AREA_OBJ_RELEASE:
			{
				ASSERT(msg.source.IsArea());
				plane_->RemoveAreaFromAOI(msg.source);
			}
			break;

		case GS_PLANE_MSG_MAP_ELEM_ACTIVE:
			{
				if (msg.source.IsObject() && !msg.source.IsPlayer())
				{
					MapElementActive(msg.param, msg.source);
				}
				else
				{
					ASSERT(false);
				}
			}
			break;

		case GS_PLANE_MSG_MAP_ELEM_DEACTIVE:
			{
				if (msg.source.IsObject() && !msg.source.IsPlayer())
				{
					MapElementDeactive(msg.param, msg.source);
				}
				else
				{
					ASSERT(false);
				}
			}
			break;

		case GS_PLANE_MSG_ENABLE_MAP_ELEM:
			{
				EnableMapElement(msg.param);
			}
			break;

		case GS_PLANE_MSG_DISABLE_MAP_ELEM:
			{
				DisableMapElement(msg.param);
			}
			break;

		case GS_PLANE_MSG_QUERY_NPC_ZONE_INFO:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_query_npc_zone_info);
				plane_msg_query_npc_zone_info& param = *(plane_msg_query_npc_zone_info*)msg.content;
				ASSERT(msg.source.IsPlayer());
                QueryNpcZoneInfo(msg.source, param.elem_id, param.link_id, param.sid_in_link);
			}
			break;
		
	    default:
		    if (OnMessageHandler(msg) != 0)
			{
				ASSERT(false && "无法处理未知类型的inter-message");
				return -1;
			}
			return 0;
	}

	return 0;
}

int WorldManager::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_PLANE_MSG_SWITCH_REQUEST:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_switch_request);
				plane_msg_switch_request& param = *(plane_msg_switch_request*)msg.content;

				// 只能处理别的地图发来的请求
				bool is_success = false;
				if (msg.source != plane_->world_xid())
				{
					if (plane_->PosInWorld(param.dest_pos) && plane_->IsWalkablePos(param.dest_pos))
					{
						is_success = true;
					}
				}
				else
				{
					ASSERT(false);
				}

				// send reply to world
				plane_msg_switch_reply reply;
				reply.is_success    = is_success;
				reply.dest_pos      = param.dest_pos;
				reply.player_roleid = param.player_roleid;
				SendWorldMSG(GS_PLANE_MSG_SWITCH_REPLY, msg.source, &reply, sizeof(reply));
			}
			break;

		case GS_PLANE_MSG_SWITCH_REPLY:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_switch_reply);
				plane_msg_switch_reply& param = *(plane_msg_switch_reply*)msg.content;

				// 本地图是不能进行switch的
				if (msg.source != plane_->world_xid())
				{
					if (param.is_success)
					{
						RunnablePool::AddTask(new SwitchWorldTask(plane_->world_id(), 
									                              MAP_ID(msg.source), 
									                              param.dest_pos, 
									                              param.player_roleid));
					}
					else 
					{
						SendObjectMSG(GS_MSG_PLAYER_SWITCH_ERROR, 
								      XID(param.player_roleid, XID::TYPE_PLAYER));
					}
				}
				else
				{
					ASSERT(false);
				}
			}
			break;

		default:
			{
				MutexLockGuard lock(wm_imp_mutex_);
				if (pimp_->OnMessageHandler(msg) != 0)
				{
					ASSERT(false && "无法处理未知类型的inter-message");
					return -1;
				}
			}
			return 0;
	}

	return 0;
}

void WorldManager::RecreateNpc(int32_t elem_id, int32_t templ_id)
{
	Npc* new_npc = npc_gen_->GenerateNpcByElemID(elem_id, templ_id);
	if (new_npc != NULL)
	{
		AcceptingNpc(new_npc);
	}
}

void WorldManager::RecreateMatter(int32_t elem_id, int32_t templ_id)
{
	Matter* new_matter = matter_gen_->GenerateMatterByElemID(elem_id, templ_id);
	if (new_matter != NULL)
	{
		AcceptingMatter(new_matter);
	}
}

void WorldManager::RecreateAreaObj(int32_t elem_id)
{
	AreaObj* new_areaobj = area_gen_->GenerateAreaObjByElemID(elem_id);
	if (new_areaobj != NULL)
	{
		AcceptingAreaObj(new_areaobj);
	}
}

bool WorldManager::CreateNpcMapElem(int32_t elem_id)
{
	std::vector<Npc*> npc_vec;
	if (!npc_gen_->GenerateNpcByElemID(elem_id, npc_vec))
	{
		ASSERT(npc_vec.size() == 0);
		return false;
	}

	for (size_t i = 0; i < npc_vec.size(); ++i)
	{
		AcceptingNpc(npc_vec[i]);
	}
	return true;
}

bool WorldManager::CreateMatterMapElem(int32_t elem_id)
{
	std::vector<Matter*> matter_vec;
	if (!matter_gen_->GenerateMatterByElemID(elem_id, matter_vec))
	{
		ASSERT(matter_vec.size() == 0);
		return false;
	}

	for (size_t i = 0; i < matter_vec.size(); ++i)
	{
		AcceptingMatter(matter_vec[i]);
	}
	return true;
}

bool WorldManager::CreateAreaMapElem(int32_t elem_id)
{
	AreaObj* new_areaobj = area_gen_->GenerateAreaObjByElemID(elem_id);
	if (new_areaobj == NULL)
		return false;
		
	AcceptingAreaObj(new_areaobj);
	return true;
}

void WorldManager::AcceptingNpc(Npc* new_npc)
{
	if (InsertNpc(new_npc) >= 0)
	{
		// 开始接收心跳及MSG，应该先插入到Manager里再插入到AOI里
		Gmatrix::InsertNpcToMan(new_npc);
		plane_->InsertObjToAOI(new_npc->object_xid(), new_npc->pos(), 0);
		// 插入地图后npc要做一些处理
		new_npc->HasInsertToWorld();
	}
	else
	{
		Gmatrix::FreeNpc(new_npc);
	}
}

void WorldManager::AcceptingMatter(Matter* new_matter)
{
	if (InsertMatter(new_matter) >= 0)
	{
		// 开始接收心跳及MSG，应该先插入到Manager里再插入到AOI里
		Gmatrix::InsertMatterToMan(new_matter);
		plane_->InsertObjToAOI(new_matter->object_xid(), new_matter->pos(), 0);
		// 插入地图后matter要做一些处理
		new_matter->HasInsertToWorld();
	}
	else
	{
		Gmatrix::FreeMatter(new_matter);
	}
}

void WorldManager::AcceptingAreaObj(AreaObj* new_areaobj)
{
	if (InsertAreaObj(new_areaobj) >= 0)
	{
		// 开始接收心跳及MSG，应该先插入到Manager里再插入到AOI里
		Gmatrix::InsertAreaObjToMan(new_areaobj);
		std::vector<A2DVECTOR> vertexes;
		new_areaobj->GetVertexes(vertexes);
		plane_->InsertAreaToAOI(new_areaobj->object_xid(), vertexes);
		// 插入World后，area可能还要做些处理工作
		new_areaobj->HasInsertToWorld();
	}
	else
	{
		Gmatrix::FreeAreaObj(new_areaobj);
	}
}

void WorldManager::SetForbidObjectInsert()
{
	is_forbid_obj_insert_ = true;
}

void WorldManager::BroadcastWorldManClosing()
{
    // 广播给所有object
	std::vector<XID> obj_vec;
	plane_->GetAllObjectInWorld(obj_vec);
	for (size_t i = 0; i < obj_vec.size(); ++i)
	{
		SendObjectMSG(GS_MSG_WORLD_CLOSING, obj_vec[i]);
	}
}

void WorldManager::BroadcastToAllPlayer(int message, const void* buf, size_t len)
{
    // 广播给所有的player
    std::vector<XID> player_vec;
    plane_->GetAllPlayerInWorld(player_vec);
    for (size_t i = 0; i < player_vec.size(); ++i)
    {
        SendObjectMSG(message, player_vec[i], buf, len);
    }
}

void WorldManager::MapElementActive(int32_t elem_id, const XID& obj)
{
	ActiveMapElemMap::iterator it = map_elem_map_.find(elem_id);
	if (it != map_elem_map_.end())
	{
		it->second.insert(obj.id);
	}
	else
	{
        MapElemObjIDSet obj_set;
		obj_set.insert(obj.id);
		map_elem_map_[elem_id] = obj_set; 
	}
}

void WorldManager::MapElementDeactive(int32_t elem_id, const XID& obj)
{
	ActiveMapElemMap::iterator it = map_elem_map_.find(elem_id);
	if (it != map_elem_map_.end())
	{
		it->second.erase(obj.id);
		if (it->second.empty())
		{
			map_elem_map_.erase(it);
			return; // 迭代器已经失效
		}
	}
	else
	{
		__PRINTF("地图元素:%d没有Enable就收到Disable消息, obj_id:%ld obj_type:%d", elem_id, obj.id, obj.type);
	}
}

void WorldManager::EnableMapElement(int32_t elem_id)
{
	ActiveMapElemMap::iterator it = map_elem_map_.find(elem_id);
	if (it != map_elem_map_.end())
		return;

	// not found
    MapElemObjIDSet obj_set;
	map_elem_map_[elem_id] = obj_set;

	//
	// 以下是并列关系，中间不能有return
	// 因为比如npc区域，它既是npc又是matter（有一个matter锚点）
	//
	if (npc_gen_->IsNpcMapElem(elem_id))
	{
		if (!CreateNpcMapElem(elem_id))
		{
			__PRINTF("开启Npc地图元素失败！eid:%d", elem_id);
		}
	}

	if (matter_gen_->IsMatterMapElem(elem_id))
	{
		if (!CreateMatterMapElem(elem_id))
		{
			__PRINTF("开启Matter地图元素失败！eid:%d", elem_id);
		}
	}

	if (area_gen_->IsAreaMapElem(elem_id))
	{
		if (!CreateAreaMapElem(elem_id))
		{
			__PRINTF("开启AreaObj地图元素失败！eid:%d", elem_id);
		}
	}
}

void WorldManager::DisableMapElement(int32_t elem_id)
{
	ActiveMapElemMap::iterator it_map = map_elem_map_.find(elem_id);
	if (it_map == map_elem_map_.end())
		return;

	// found
	MapElemObjIDSet::iterator it_set = it_map->second.begin();
	for (; it_set != it_map->second.end(); ++it_set)
	{
		XID object;
		MAKE_XID(*it_set, object);
		ASSERT(object.IsObject());
		SendObjectMSG(GS_MSG_RELATE_MAP_ELEM_CLOSED, object);
	}
}

void WorldManager::QueryNpcZoneInfo(const XID& player, int32_t elem_id, int32_t link_id, int32_t sid_in_link)
{
	ActiveMapElemMap::iterator it_map = map_elem_map_.find(elem_id);
	if (it_map == map_elem_map_.end())
	{
		__PRINTF("Npc区域地图元素没有激活，不能查询信息！eid:%d", elem_id);
		return;
	}

	// found
	G2C::QueryNpcZoneInfo_Re packet;
	packet.elem_id = elem_id;
	MapElemObjIDSet::iterator it_set = it_map->second.begin();
	for (; it_set != it_map->second.end(); ++it_set)
	{
		XID obj_xid;
		MAKE_XID(*it_set, obj_xid);
		if (obj_xid.IsNpc())
		{
			world::worldobj_base_info info;
			if (plane_->QueryObject(obj_xid, info))
			{
				G2C::QueryNpcZoneInfo_Re::Detail ent;
				ent.obj_id = static_cast<int32_t>(obj_xid.id);
				ent.tid    = info.tid;
				packet.npc_list.push_back(ent);
			}
		}
	}

	if (packet.npc_list.size() > 0)
	{
		SendToPlayer(player.id, link_id, sid_in_link, packet);
	}
}

bool WorldManager::QueryMapElemInfo(int32_t elem_id, std::vector<XID>& obj_vec) const
{
    ActiveMapElemMap::const_iterator it_map = map_elem_map_.find(elem_id);
	if (it_map == map_elem_map_.end())
        return false;

    MapElemObjIDSet::const_iterator set_it = it_map->second.begin();
    for (; set_it != it_map->second.end(); ++set_it)
    {
        obj_vec.push_back(MAKE_XID(*set_it));
    }
    return true;
}

int32_t WorldManager::GetMapCounter(int32_t index) const
{
    // 这里不用加锁，脚本可能会调用，容易引起死锁
    const BaseWorldManImp* tmpimp = dynamic_cast<const BaseWorldManImp*>(pimp_);
    if (tmpimp == NULL)
        return -1;

    return tmpimp->GetCounter(index);
}

bool WorldManager::IsMapCounterLocked(int32_t index) const
{
   // 这里不用加锁，脚本可能会调用，容易引起死锁
    const BaseWorldManImp* tmpimp = dynamic_cast<const BaseWorldManImp*>(pimp_);
    if (tmpimp == NULL)
        return -1;
    
    return tmpimp->IsCounterLocked(index);
}

bool WorldManager::CheckPlayerCountLimit(world::BGEnterType enter_type) const
{
    return pimp_->OnCheckPlayerCountLimit(enter_type);
}

void WorldManager::GetAllPlayerInWorld(std::vector<XID>& player_vec)
{
    plane_->GetAllPlayerInWorld(player_vec);
}

void WorldManager::GetAllPlayerLinkInfo(std::vector<world::player_link_info>& player_vec)
{
    plane_->GetAllPlayerLinkInfo(player_vec);
}

int32_t WorldManager::GetPlayerCount()
{
    return plane_->GetPlayerCount();
}

void WorldManager::SetMapTeamInfo(void* buf, int len)
{
    pimp_->OnSetMapTeamInfo(buf, len);
}

void WorldManager::CloseWorldMan()
{
    // 先禁止object插入地图
	SetForbidObjectInsert();

    // 调用函数做对应的处理
    int32_t worldid  = GetWorldID();
	int32_t worldtag = GetWorldTag();

    if (IS_INS_MAP(worldid))
	{
		s_pInstanceWC->CloseInsWorldMan(worldid, worldtag);
	}
    else if (IS_BG_MAP(worldid))
    {
        s_pBGCluster->CloseBGWorldMan(worldid, worldtag);
    }

	// 广播地图关闭
    BroadcastWorldManClosing();
}

} // namespace gamed
