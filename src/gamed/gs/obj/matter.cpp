#include "matter.h"

#include "shared/logsys/logging.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/dbgprt.h"
#include "gs/global/randomgen.h"
#include "gs/global/gmatrix.h"
#include "gs/global/game_util.h"
#include "gs/scene/world.h"
#include "gs/scene/world_man.h"

#include "mine_imp.h"
#include "matter_sender.h"


namespace gamed {

using namespace mapDataSvr;
using namespace matterdef;

namespace {


///
/// state switch
///
#define SHIFT(state) (1 << state)

// 顺序必须与npcdef::MatterStates一致
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
		: matter_id_(id),
		  elem_id_(elem_id),
		  templ_id_(templ_id)
	{ }

	virtual void Run()
	{
		// 找到matter，并执行下线操作
		Matter* pMatter = Gmatrix::FindMatterFromMan(matter_id_);
		if (NULL == pMatter)
			return;

		WorldObjectLockGuard lock(pMatter);
		if (!pMatter->IsActived())
			return;

		// 必须是reborn状态
		ASSERT(pMatter->state() == STATE_REBORN);

		// 从地图中删除前获取matter的相关信息
		XID msg_target   = pMatter->world_plane()->world_xid();
		XID src_xid      = pMatter->object_xid();
		int32_t templ_id = pMatter->templ_id();
		int32_t elem_id  = pMatter->elem_id();
		ASSERT(templ_id == templ_id_ && elem_id == elem_id_);
		
		// 从地图中删除
		WorldManager* pManager = Gmatrix::FindWorldManager(pMatter->world_id(), pMatter->world_tag());
		if (pManager)
		{
			pManager->RemoveMatter(pMatter);
		}
		else
		{
			__PRINTF("Matter%ld不在world中，matter的world_id:%d", pMatter->object_id(), pMatter->world_id());
		}

		// 通知地图Reborn该Matter, 应该放在地图删除怪物之后RemoveMatter()
		MSG msg;
		plane_msg_matter_reborn param;
		param.elem_id  = elem_id;
		param.templ_id = templ_id;
		BuildMessage(msg, GS_PLANE_MSG_MATTER_REBORN, msg_target, src_xid, 0, &param, sizeof(param));
		Gmatrix::SendWorldMsg(msg);

		// 回收Matter指针
		Gmatrix::RemoveMatterFromMan(pMatter);
		Gmatrix::FreeMatter(pMatter);
	}

private:
	int64_t matter_id_;
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
		: matter_id_(id),
		  elem_id_(elem_id),
		  templ_id_(templ_id)
	{ }

	virtual void Run()
	{
		// 找到matter，并执行下线操作
		Matter* pMatter = Gmatrix::FindMatterFromMan(matter_id_);
		if (NULL == pMatter)
		{
			LOG_WARN << "matter not found!";
			return;
		}

		WorldObjectLockGuard lock(pMatter);
		if (!pMatter->IsActived())
			return;

		// 必须是recycle状态
		ASSERT(pMatter->state() == STATE_RECYCLE);

		// 从地图中删除前获取matter的相关信息
		XID msg_target   = pMatter->world_plane()->world_xid();
		XID src_xid      = pMatter->object_xid();
		int32_t templ_id = pMatter->templ_id();
		int32_t elem_id  = pMatter->elem_id();
		ASSERT(templ_id == templ_id_ && elem_id == elem_id_);
		
		// 从地图中删除
		WorldManager* pManager = Gmatrix::FindWorldManager(pMatter->world_id(), pMatter->world_tag());
		if (pManager)
		{
			pManager->RemoveMatter(pMatter);
		}
		else
		{
			__PRINTF("Matter%ld不在world中，matter的world_id:%d", pMatter->object_id(), pMatter->world_id());
		}

		// 回收Matter指针
		Gmatrix::RemoveMatterFromMan(pMatter);
		Gmatrix::FreeMatter(pMatter);
	}

private:
	int64_t matter_id_;
	int32_t elem_id_;
	int32_t templ_id_;
};

} // Anonymous


///
/// Matter 动态物品
///
Matter::Matter()
	: elem_id_(-1),
	  templ_id_(-1),
	  pimp_(NULL),
	  sender_(NULL),
	  idle_timer_(31),
	  zombie_timer_(0),
	  reborn_timer_(0),
	  state_(STATE_IDLE)
{
}

Matter::~Matter()
{
}

void Matter::Release()
{
	// always first
	MapElementDeactive();

	// release members
	elem_id_      = -1;
	templ_id_     = -1;
	idle_timer_   = 60;
	zombie_timer_ = 0;
	reborn_timer_ = 0;
	state_        = STATE_IDLE;

	in_view_players_set_.clear();

	DELETE_SET_NULL(pimp_);
	DELETE_SET_NULL(sender_);

	WorldObject::Release();
}

void Matter::HasInsertToWorld()
{
	// enable map element
	SendPlaneMsg(GS_PLANE_MSG_MAP_ELEM_ACTIVE, elem_id_);
}

bool Matter::Init(const MatterInitData& init_data)
{
	elem_id_  = init_data.elem_id;
	templ_id_ = init_data.templ_id;
	// init base data
	set_dir(init_data.dir);
	set_pos(init_data.pos);
	set_xid(init_data.id, XID::TYPE_MATTER);

	const BaseMapData* pmapdata = s_pMapData->QueryBaseMapDataTempl(init_data.elem_id);
	if (!pmapdata) 
	{
		LOG_ERROR << "没有找到Matter的MapDataTempl，elem_id=" << init_data.elem_id;
		return false;
	}

	int type = pmapdata->GetType();
	switch (type)
	{
		case MAPDATA_TYPE_AREA_NPC:
		case MAPDATA_TYPE_SCENE_EFFECT:
			pimp_ = new MatterImp(*this);
			break;

		case MAPDATA_TYPE_SPOT_MINE:
		case MAPDATA_TYPE_AREA_MINE:
			pimp_ = new MineImp(*this);
			break;

		default:
			return false;
	}

	///
	/// 开始初始化本体数据
	///
	if (!InitRuntimeData())
	{
		__PRINTF("Matter运行时数据初始化失败！");
		return false;
	}

	// sender module
	sender_ = new MatterSender(*this);
	if (sender_ == NULL)
	{
		__PRINTF("Matter elem_id:%d templ_id:%d type:%d sender初始化失败！", 
				 elem_id_, templ_id_, type);
		return false;
	}

	// always the last!
	if (!pimp_->OnInit()) 
	{
		__PRINTF("动态物品elem_id:%d templ_id:%d type:%d imp初始化失败！", 
				 elem_id_, templ_id_, type);
		return false;
	}

	return true;
}

bool Matter::InitRuntimeData()
{
	idle_timer_   = mrand::Rand(0, 60);
	zombie_timer_ = 0;
	reborn_timer_ = 0;
	birth_place_  = pos();
	return true;
}

void Matter::OnHeartbeat()
{
	pimp_->OnHeartbeat();

	// state switch func
	DriveMachine();
}

int Matter::OnDispatchMessage(const MSG& msg)
{
	return MessageHandler(msg);
}

bool Matter::StepMove(const A2DVECTOR& offset)
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

void Matter::PlayerEnterView(RoleID playerid, int32_t linkid, int32_t sid)
{
	if (!in_view_players_set_.insert(playerid).second)
	{
		//ASSERT(false);
		LOG_WARN << playerid << " enter view error in matter:" << object_id();
		return;
	}

	// to client
	sender_->PlayerEnterView(playerid, linkid, sid);

	// 有玩家进入视野，激活心跳
	idle_timer_ = 0;

	// imp
	pimp_->OnPlayerEnterView(playerid);
}
	
void Matter::PlayerLeaveView(RoleID playerid)
{
	size_t n = in_view_players_set_.erase(playerid);
	//(void)n; assert(n == 1);
	if (n != 1)
	{
		LOG_WARN << playerid << " leave view error in matter:" << object_id();
		return;
	}

	sender_->PlayerLeaveView(playerid);

	// imp
	pimp_->OnPlayerLeaveView(playerid);
}

bool Matter::HasPlayerInView() const
{
	return !in_view_players_set_.empty();
}

bool Matter::CheckIdleState()
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

void Matter::DriveMachine()
{
	switch (state_)
	{
		case STATE_IDLE:
			{
				if (HasPlayerInView())
				{
					set_state(STATE_NORMAL);
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

void Matter::SetIdleState()
{
    matterdef::MatterStates newState = STATE_IDLE;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
	idle_timer_ = mrand::Rand(0, 60);
}

void Matter::SetZombieState(int time_counter)
{
    matterdef::MatterStates newState = STATE_ZOMBIE;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
	zombie_timer_ = time_counter;
}

void Matter::SetRebornState(int time_counter)
{
    matterdef::MatterStates newState = STATE_REBORN;
    ASSERT(CheckStateSwitch(newState));
	set_state(newState);
	reborn_timer_ = time_counter;
}

void Matter::SetRecycleState()
{
    matterdef::MatterStates newState = STATE_RECYCLE;
    ASSERT(CheckStateSwitch(newState));
    set_state(newState);
}

bool Matter::CheckStateSwitch(MatterStates new_state) const
{
    matterdef::MatterStates cur_state = state();
    ASSERT((int)cur_state >= 0 && (int)cur_state < (int)sizeof(state_shift_table));
    if (SHIFT(new_state) & state_shift_table[cur_state].permit)
    {
        return true;
    }
    return false;
}

void Matter::LifeExhaust()
{
    if (state() == STATE_RECYCLE)
        return;

	world_plane()->RemoveObjFromAOI(object_xid());
	if (pimp_->CanAutoReborn())
	{
		SetRebornState(pimp_->GetRebornInterval());
	}
	else
	{
	    SetRecycleState();
		RunnablePool::AddTask(new RecycleTask(object_id(), elem_id_, templ_id_));
	}
}

void Matter::HandleWorldClose()
{
	RecycleSelf();
}

void Matter::RecycleSelf()
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

void Matter::MapElementDeactive()
{
	// disable map element
	SendPlaneMsg(GS_PLANE_MSG_MAP_ELEM_DEACTIVE, elem_id_);
}

void Matter::Reborn()
{
	RunnablePool::AddTask(new RebornTask(object_id(), elem_id_, templ_id_));
}

void Matter::RebornAtOnce()
{
    LifeExhaust();
}

bool Matter::CanTeleport()
{
    if (state() == STATE_IDLE || state() == STATE_NORMAL)
        return true;

    return false;
}

void Matter::HandleObjectTeleport(const MSG& msg)
{
    // 检查状态
    if (!CanTeleport())
    {
        __PRINTF("Matter::HandleObjectTeleport state reject teleport");
        return;
    }

    // 取参数
    CHECK_CONTENT_PARAM(msg, msg_object_teleport);
    const msg_object_teleport& param = *(const msg_object_teleport*)msg.content;
    if (param.elem_id != elem_id_)
    {
        __PRINTF("Matter::HandleObjectTeleport elem_id is not equal!");
        return;
    }

    // 检查目标点是否合法
    A2DVECTOR target_pos(param.pos_x, param.pos_y);
    if (!world_plane()->IsInGridLocal(target_pos.x, target_pos.y) ||
        world_plane()->IsOutsideGrid(target_pos.x, target_pos.y))
    {
        __PRINTF("Matter::HandleObjectTeleport target_pos is not in world!");
        return;
    }
    if (!world_plane()->IsWalkablePos(target_pos))
    {
        __PRINTF("Matter::HandleObjectTeleport target_pos is not walkable!");
        return;
    }
    
    // 做瞬移前的准备
    if (!pimp_->OnMapTeleport())
    {
        __PRINTF("Matter::HandleObjectTeleport imp OnMapTeleport() return false!");
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

void Matter::SetBirthPlace(const A2DVECTOR& pos)
{
    birth_place_ = pos;
    pimp_->SetBirthPlace(pos);
}

} // namespace gamed
