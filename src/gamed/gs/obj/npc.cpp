#include "npc.h"

#include "gs/scene/world.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/base_datatempl.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/dbgprt.h"
#include "gs/global/math_types.h"
#include "gs/global/randomgen.h"
#include "gs/global/gmatrix.h"
#include "gs/global/game_util.h"
#include "gs/scene/world_man.h"

#include "monster_imp.h"
#include "service_npc_imp.h"
#include "world_boss_imp.h"
#include "npc_sender.h"
#include "ainpc.h"


namespace gamed {

using namespace dataTempl;
using namespace mapDataSvr;
using namespace npcdef;

namespace {


///
/// state switch
///
#define SHIFT(state) (1 << state)

// 顺序必须与npcdef::NpcStates一致
static ShiftNode state_shift_table[] =
{
    /*cur-status*/     /*permit-status*/
    { STATE_IDLE,      SHIFT(STATE_NORMAL) | SHIFT(STATE_ZOMBIE) | SHIFT(STATE_REBORN) | SHIFT(STATE_RECYCLE) },
    { STATE_NORMAL,    SHIFT(STATE_IDLE)   | SHIFT(STATE_ZOMBIE) | SHIFT(STATE_REBORN) | SHIFT(STATE_RECYCLE) },
    { STATE_ZOMBIE,    SHIFT(STATE_NORMAL) | SHIFT(STATE_ZOMBIE) | SHIFT(STATE_REBORN) | SHIFT(STATE_RECYCLE) },
    { STATE_REBORN,    SHIFT(STATE_RECYCLE)                                                                   },
    { STATE_RECYCLE,   0                                                                                      },
};


///
/// runnable task
///
class RebornTask : public RunnablePool::Task
{
public:
	RebornTask(int64_t id, int32_t elem_id, int32_t templ_id)
		: npc_id_(id),
		  elem_id_(elem_id),
		  templ_id_(templ_id)
	{ }

	virtual void Run()
	{
		// 找到npc，并执行下线操作
		Npc* pNpc = Gmatrix::FindNpcFromMan(npc_id_);
		if (NULL == pNpc)
			return;

		WorldObjectLockGuard lock(pNpc);
		if (!pNpc->IsActived())
			return;

		// 必须是reborn状态
		ASSERT(pNpc->state() == STATE_REBORN);

		// 从地图中删除前获取npc的相关信息
		XID msg_target   = pNpc->world_plane()->world_xid();
		XID src_xid      = pNpc->object_xid();
		int32_t templ_id = pNpc->templ_id();
		int32_t elem_id  = pNpc->elem_id();
		ASSERT(templ_id == templ_id_ && elem_id == elem_id_);
		
		// 从地图中删除
		WorldManager* pManager = Gmatrix::FindWorldManager(pNpc->world_id(), pNpc->world_tag());
		if (pManager)
		{
			pManager->RemoveNpc(pNpc);
		}
		else
		{
			__PRINTF("Npc%ld不在world中，npc的world_id:%d", pNpc->object_id(), pNpc->world_id());
		}

		// 通知地图Reborn该Npc, 应该放在地图删除怪物之后RemoveNpc()
		MSG msg;
		plane_msg_npc_reborn param;
		param.elem_id  = elem_id;
		param.templ_id = templ_id;
		BuildMessage(msg, GS_PLANE_MSG_NPC_REBORN, msg_target, src_xid, 0, &param, sizeof(param));
		Gmatrix::SendWorldMsg(msg);

		// 回收Npc指针
		Gmatrix::RemoveNpcFromMan(pNpc);
		Gmatrix::FreeNpc(pNpc);
	}

private:
	int64_t npc_id_;
	int32_t elem_id_;
	int32_t templ_id_;
};


///
/// RecycleTask
///
class RecycleTask : public RunnablePool::Task
{
public:
	RecycleTask(int64_t id, int32_t elem_id, int32_t templ_id)
		: npc_id_(id),
		  elem_id_(elem_id),
		  templ_id_(templ_id)
	{ }

	virtual void Run()
	{
		// 找到npc，并执行下线操作
		Npc* pNpc = Gmatrix::FindNpcFromMan(npc_id_);
		if (NULL == pNpc)
		{
			LOG_WARN << "npc not found！";
			return;
		}

		WorldObjectLockGuard lock(pNpc);
		if (!pNpc->IsActived())
			return;

		// 必须是recycle状态
		ASSERT(pNpc->state() == STATE_RECYCLE);

		// 从地图中删除前获取npc的相关信息
		XID msg_target   = pNpc->world_plane()->world_xid();
		XID src_xid      = pNpc->object_xid();
		int32_t templ_id = pNpc->templ_id();
		int32_t elem_id  = pNpc->elem_id();
		ASSERT(templ_id == templ_id_ && elem_id == elem_id_);
		
		// 从地图中删除
		WorldManager* pManager = Gmatrix::FindWorldManager(pNpc->world_id(), pNpc->world_tag());
		if (pManager)
		{
			pManager->RemoveNpc(pNpc);
		}
		else
		{
			__PRINTF("Npc%ld不在world中，Npc的world_id:%d", pNpc->object_id(), pNpc->world_id());
		}

		// 回收Npc指针
		Gmatrix::RemoveNpcFromMan(pNpc);
		Gmatrix::FreeNpc(pNpc);
	}

private:
	int64_t npc_id_;
	int32_t elem_id_;
	int32_t templ_id_;
};


bool is_world_boss(const BaseMapData* pelem)
{
	if (pelem->GetType() == MAPDATA_TYPE_SPOT_MONSTER)
	{
		const SpotMonster* ptmp = dynamic_cast<const SpotMonster*>(pelem);
		if (ptmp && ptmp->combat_rule == mapDataSvr::MODEL_WORLD_BOSS)
		{
			return true;
		}
	}
	else if (pelem->GetType() == MAPDATA_TYPE_AREA_MONSTER)
	{
		const AreaMonster* ptmp = dynamic_cast<const AreaMonster*>(pelem);
		if (ptmp && ptmp->combat_rule == mapDataSvr::MODEL_WORLD_BOSS)
		{
			return true;
		}
	}

	return false;
}

} // Anonymous


static const int32_t kStrollRandLimit = 16 - 1; // 31

///
/// Npc
///
Npc::Npc()
	: elem_id_(0),
	  templ_id_(0),
	  pimp_(NULL),
	  ai_core_(NULL),
	  sender_(NULL),
	  stroll_timer_(kStrollRandLimit),
	  idle_timer_(31),
	  zombie_timer_(0),
	  reborn_timer_(0),
	  state_(STATE_IDLE)
{
}

Npc::~Npc()
{
}

bool Npc::Init(const npcdef::NpcInitData& init_data)
{
	elem_id_  = init_data.elem_id;
	templ_id_ = init_data.templ_id;
	// init base data
	set_dir(init_data.dir);
	set_pos(init_data.pos);

	const BaseDataTempl* pdata = s_pDataTempl->QueryBaseDataTempl(templ_id_);
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Npc的DataTempl，elem_id=" << init_data.elem_id 
			<< " templ_id=" << init_data.templ_id;
		return false;
	}

	const BaseMapData* pelem = s_pMapData->QueryBaseMapDataTempl(elem_id_);
	if (pelem == NULL)
	{
		LOG_ERROR << "没有找到Npc的MapDataTempl，elem_id=" << init_data.elem_id 
			<< " templ_id=" << init_data.templ_id;
		return false;
	}
	
	int type = pdata->GetType();
	switch (type)
	{
		case TEMPL_TYPE_MONSTER_TEMPL:
			{
				set_xid(init_data.id, XID::TYPE_MONSTER);
				if (is_world_boss(pelem))
				{
					pimp_ = new WorldBossImp(*this);
				}
				else // normal monster
				{
					pimp_ = new MonsterImp(*this);
				}
			}
			break;

		case TEMPL_TYPE_SERVICE_NPC_TEMPL:
			{
				set_xid(init_data.id, XID::TYPE_SERVICE_NPC);
				pimp_ = new ServiceNpcImp(*this);
			}
			break;

		default:
			return false;
	}
	
	///
	/// 开始初始化本体数据
	///
	if (!InitRuntimeData())
	{
		__PRINTF("Npc运行时数据初始化失败！");
		return false;
	}

	// init ai module
	ai_core_ = new NpcAI(*this);
	AggroParam default_aggro; // 构造函数里已填入默认值
	if (ai_core_ == NULL || !ai_core_->Init(default_aggro))
	{
		__PRINTF("怪物elem_id:%d templ_id:%d type:%d ai_core初始化失败！", 
				 elem_id_, templ_id_, type);
		return false;
	}

	// sender module
	sender_ = new NpcSender(*this);
	if (sender_ == NULL)
	{
		__PRINTF("怪物elem_id:%d templ_id:%d type:%d sender初始化失败！", 
				 elem_id_, templ_id_, type);
		return false;
	}

	// always the last!
	if (!pimp_->OnInit()) 
	{
		LOG_ERROR << "NpcImp::OnInit() error elem_id:" << elem_id_
			<< " templ_id:" << templ_id_ << " type:" << type;
		__PRINTF("怪物elem_id:%d templ_id:%d type:%d imp初始化失败！", 
				 elem_id_, templ_id_, type);
		return false;
	}

	return true;
}

bool Npc::InitRuntimeData()
{
	stroll_timer_ = mrand::Rand(0, kStrollRandLimit);
	idle_timer_   = mrand::Rand(0, 60);
	zombie_timer_ = 0;
	reborn_timer_ = 0;
	birth_place_  = pos();
	return true;
}

void Npc::Release()
{
	// always first
	MapElementDeactive();

	// release members
	elem_id_      = 0;
	templ_id_     = 0;
	state_        = STATE_IDLE;
	stroll_timer_ = 60;
	idle_timer_   = 60;
	zombie_timer_ = 0;
	reborn_timer_ = 0;

	in_view_players_set_.clear();

	SAFE_DELETE(ai_core_);
	SAFE_DELETE(sender_);
	SAFE_DELETE(pimp_);
	
	Unit::Release();
}

void Npc::HasInsertToWorld()
{
	// enable map element
	SendPlaneMsg(GS_PLANE_MSG_MAP_ELEM_ACTIVE, elem_id_);

    // imp
    pimp_->OnHasInsertToWorld();
}

void Npc::OnHeartbeat()
{
	if (ai_core_ && !pimp_->OnPauseAICore())
	{
		ai_core_->Heartbeat();
	}

	pimp_->OnHeartbeat();

	// state switch func
	DriveMachine();
}

void Npc::NPCSessionStart(int task_id, int session_id)
{
	if (ai_core_)
	{
		ai_core_->SessionStart(task_id, session_id);
	}
}
	
void Npc::NPCSessionEnd(int task_id, int session_id, int retcode)
{
	if (ai_core_)
	{
		ai_core_->SessionEnd(task_id, session_id, retcode);
	}
}

int Npc::OnDispatchMessage(const MSG& msg)
{
	return MessageHandler(msg);
}

bool Npc::CanCombat() const
{
    return pimp_->OnCanCombat();
}

void Npc::PlayerEnterView(RoleID playerid, int32_t linkid, int32_t sid)
{
	if (!in_view_players_set_.insert(playerid).second)
	{
		//ASSERT(false);
		LOG_WARN << playerid << " enter view error in npc:" << object_id();
		return;
	}

	// to client
	sender_->PlayerEnterView(playerid, linkid, sid);

	// 有玩家进入视野，激活心跳
	idle_timer_ = 0;
}

void Npc::PlayerLeaveView(RoleID playerid)
{
	size_t n = in_view_players_set_.erase(playerid);
	//(void)n; assert(n == 1);
	if (n != 1)
	{
		LOG_WARN << playerid << " leave view error in npc:" << object_id();
		return;
	}

	sender_->PlayerLeaveView(playerid);
}

bool Npc::StepMove(const A2DVECTOR& offset)
{
	A2DVECTOR newpos(pos());
	newpos += offset;
	
	World* pworld = world_plane();
	if (pworld->IsOutsideGrid(newpos.x, newpos.y))
	{
		return false;
	}

	if (!pworld->IsInGridLocal(newpos.x, newpos.y))
	{
		// 超出边界
		return false;
	}

	// 计算朝向
	set_dir(maths::calc_direction(offset));
	// 设为新位置坐标
	set_pos(newpos);
	// 更新AOI自己的位置
	pworld->UpdateAOI(object_xid(), newpos);

	return true;
}

void Npc::DriveMachine()
{
	switch (state_)
	{
		case STATE_IDLE:
			{
				if (HasPlayerInView())
				{
					SetNormalState();
				}
			}
			break;

		case STATE_NORMAL:
			{
				if (!HasPlayerInView())
				{
					SetIdleState();
				}
			}
			break;

		case STATE_ZOMBIE:
			{
				if (--zombie_timer_ < 0)
				{
					LifeExhaust();
					zombie_timer_ = GS_INT32_MAX;
				}
			}
			break;

		case STATE_REBORN:
			{
				if (--reborn_timer_ < 0)
				{
					Reborn();
					reborn_timer_ = GS_INT32_MAX;
				}
			}
			break;

        case STATE_RECYCLE:
            break;

		default:
			ASSERT(false);
	}
}

bool Npc::CheckIdleState()
{
	if (state_ == STATE_IDLE)
	{
		--idle_timer_;
		if (idle_timer_ > 0)
		{
			return true;
		}
		else
		{
			idle_timer_ = mrand::Rand(0, 60);
			return false;
		}
	}
	return false;
}

bool Npc::HasPlayerInView() const
{
	return !in_view_players_set_.empty();
}

bool Npc::CanRest()
{
	if (!CanStroll())
		return false;

	if (state_ != STATE_NORMAL)
		return false;

	stroll_timer_ = (stroll_timer_ - 1) & kStrollRandLimit;
	return stroll_timer_ == 0;
}

bool Npc::CanStroll()
{
	return pimp_->OnCanStroll();
}

bool Npc::GetBirthPlace(A2DVECTOR& pos)
{
	pos = birth_place_;
	return true;
}

void Npc::SetNormalState()
{
    npcdef::NpcStates newState = STATE_NORMAL;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
}

void Npc::SetIdleState()
{
    npcdef::NpcStates newState = STATE_IDLE;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
	idle_timer_   = mrand::Rand(0, 60);
}

void Npc::SetZombieState(int time_counter)
{
    npcdef::NpcStates newState = STATE_ZOMBIE;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
	zombie_timer_ = time_counter;
}

void Npc::SetRebornState(int time_counter)
{
    npcdef::NpcStates newState = STATE_REBORN;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
	reborn_timer_ = time_counter;
}

void Npc::SetRecycleState()
{
    npcdef::NpcStates newState = STATE_RECYCLE;
    ASSERT(CheckStateSwitch(newState));
    set_state(newState);
}

bool Npc::CheckStateSwitch(NpcStates new_state) const
{
    npcdef::NpcStates cur_state = state();
    ASSERT((int)cur_state >= 0 && (int)cur_state < (int)sizeof(state_shift_table));
    if (SHIFT(new_state) & state_shift_table[cur_state].permit)
    {
        return true;
    }
    return false;
}

void Npc::LifeExhaust()
{
    if (state() == STATE_RECYCLE)
        return;

	world_plane()->RemoveObjFromAOI(object_xid());
	if (CanAutoReborn())
	{
		SetRebornState(pimp_->GetRebornInterval());
	}
	else
    {
        SetRecycleState();
        RunnablePool::AddTask(new RecycleTask(object_id(), elem_id_, templ_id_));
    }
}

void Npc::RebornAtOnce()
{
    LifeExhaust();
}

void Npc::Reborn()
{
	RunnablePool::AddTask(new RebornTask(object_id(), elem_id_, templ_id_));
}

const Npc::InViewPlayersSet& Npc::GetPlayersInView()
{
	return in_view_players_set_;
}

void Npc::TriggerCombat(const XID& target)
{
	pimp_->TriggerCombat(target);
}

float Npc::GetChaseSpeed() const
{
	return pimp_->GetChaseSpeed();
}

void Npc::ReachDestination(const A2DVECTOR& pos)
{
    return pimp_->OnReachDestination(pos);
}

void Npc::HandleWorldClose()
{
	RecycleSelf();
}

void Npc::RecycleSelf()
{
    if (state() == STATE_RECYCLE)
        return;

	if (state_ != STATE_REBORN)
	{
		world_plane()->RemoveObjFromAOI(object_xid());
	}
	SetRecycleState();
	RunnablePool::AddTask(new RecycleTask(object_id(), elem_id_, templ_id_));
}

void Npc::MapElementDeactive()
{
	// disable map element
	SendPlaneMsg(GS_PLANE_MSG_MAP_ELEM_DEACTIVE, elem_id_);
}

bool Npc::CanAutoReborn()
{
	if (pimp_->GetRebornInterval() > 0)
		return true;

	return false;
}

bool Npc::CanTeleport()
{
    if (state() == STATE_IDLE || state() == STATE_NORMAL)
        return true;

    return false;
}

void Npc::HandleObjectTeleport(const MSG& msg)
{
    // 检查状态
    if (!CanTeleport())
    {
        __PRINTF("Npc::HandleObjectTeleport state reject teleport");
        return;
    }

    // 取参数
    CHECK_CONTENT_PARAM(msg, msg_object_teleport);
    const msg_object_teleport& param = *(const msg_object_teleport*)msg.content;
    if (param.elem_id != elem_id_)
    {
        __PRINTF("Npc::HandleObjectTeleport elem_id is not equal!");
        return;
    }

    // 检查目标点是否合法
    A2DVECTOR target_pos(param.pos_x, param.pos_y);
    if (!world_plane()->IsInGridLocal(target_pos.x, target_pos.y) ||
        world_plane()->IsOutsideGrid(target_pos.x, target_pos.y))
    {
        __PRINTF("Npc::HandleObjectTeleport target_pos is not in world!");
        return;
    }
    if (!world_plane()->IsWalkablePos(target_pos))
    {
        __PRINTF("Npc::HandleObjectTeleport target_pos is not walkable!");
        return;
    }
    
    // 做瞬移前的准备
    if (!pimp_->OnMapTeleport())
    {
        __PRINTF("Npc::HandleObjectTeleport imp OnMapTeleport() return false!");
        return;
    }

    //
    // success 
    //
    A2DVECTOR offset = target_pos;
	offset -= pos();
    if (StepMove(offset))
    {
        // directory
        set_dir(param.dir);
        // birth place
        SetBirthPlace(target_pos);
        // notify client
        sender_->NotifyObjectPos(target_pos, dir());
    }
}

void Npc::HandleMonsterMove(const MSG& msg)
{
    // 检查状态
    if (!CanTeleport())
    {
        __PRINTF("Npc::HandleMonsterMove state reject teleport");
        return;
    }

    // 取参数
    CHECK_CONTENT_PARAM(msg, msg_monster_move);
    const msg_monster_move& param = *(const msg_monster_move*)msg.content;
    if (param.elem_id != elem_id_)
    {
        __PRINTF("Npc::HandleMonsterMove elem_id is not equal!");
        return;
    }

    // 检查目标点是否合法
    A2DVECTOR target_pos(param.pos_x, param.pos_y);
    if (!world_plane()->IsInGridLocal(target_pos.x, target_pos.y) ||
        world_plane()->IsOutsideGrid(target_pos.x, target_pos.y))
    {
        __PRINTF("Npc::HandleMonsterMove target_pos is not in world!");
        return;
    }
    if (!world_plane()->IsWalkablePos(target_pos))
    {
        __PRINTF("Npc::HandleMonsterMove target_pos is not walkable!");
        return;
    }
    
    // 做移动准备
    SetBirthPlace(target_pos);
    if (!pimp_->OnMonsterMove(target_pos, param.speed))
    {
        __PRINTF("Npc::HandleMonsterMove imp OnMonsterMove() return false!");
        return;
    }
}

void Npc::HandleMonsterSpeed(const MSG& msg)
{
    // 取参数
    CHECK_CONTENT_PARAM(msg, msg_monster_speed);
    const msg_monster_speed& param = *(const msg_monster_speed*)msg.content;
    if (param.elem_id != elem_id_)
    {
        //__PRINTF("Npc::HandleMonsterSpeed elem_id is not equal!");
        return;
    }

    pimp_->OnSetMonsterSpeed(param.speed);
}

void Npc::SetBirthPlace(const A2DVECTOR& pos)
{
    birth_place_ = pos;
    pimp_->SetBirthPlace(pos);
}

} // namespace gamed
