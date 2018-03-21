#include "npc_mvsession.h"

#include <limits> // std::numeric_limits

#include "gamed/client_proto/types.h"
#include "gs/global/game_util.h"
#include "gs/global/game_def.h"
#include "gs/scene/world.h"
#include "gs/obj/npc.h"
#include "gs/obj/npc_sender.h"


namespace gamed {

#define INFINITE_TIMES (std::numeric_limits<int32_t>::max())

namespace 
{
	// 转为百分数
	uint16_t speed_to_percentage(float speed)
	{
		return static_cast<uint16_t>(speed*100.f + 0.5f);
	}

} // Anonymous


///
/// NpcStrollSession
///
const int NpcStrollSession::kMaxStrollTimes   = 100;

NpcStrollSession::NpcStrollSession(Npc* npc)
	: NpcSession(npc),
	  timeout_(0),
	  range_(0.f),
	  stop_flag_(false),
	  agent_(NULL)
{
}

NpcStrollSession::~NpcStrollSession()
{
	timeout_   = 0;
	range_     = 0.f;
	stop_flag_ = false;

	SAFE_DELETE(agent_);
}

void NpcStrollSession::SetTarget(const A2DVECTOR& center, int timeout, float range)
{
	center_  = center;
	timeout_ = timeout;
	range_   = range;
}

bool NpcStrollSession::OnStartSession(const ActiveSession* next_ses)
{
	ASSERT(timeout_ > 0);
	speed_to_client_ = speed_to_percentage(GetSpeed());

	// run 
	Run();

	int tick = millisecond_to_tick(kNpcStrollUseMsecTime);
	AutoSetTimer(tick, kMaxStrollTimes);
	return true;
}

int NpcStrollSession::OnEndSession()
{
	ASSERT(session_id() != -1);

	// 这里可能无需发送停止移动的消息，因为可能已经发送
	if (!stop_flag_)
	{
		pnpc_->sender()->MoveStop(pnpc_->pos(),
				                  speed_to_client_,
								  pnpc_->dir(),
								  movement::MOVE_MODE_WALK);
	}

	return NSRC_SUCCESS;
}

bool NpcStrollSession::RepeatSession(int times)
{
	if (--timeout_ < 0) 
		return false;
	return Run();
}

float NpcStrollSession::GetSpeed()
{
	return kNpcStrollSpeed;
}

bool NpcStrollSession::Run()
{
	float speed = GetSpeed();
	if (!agent_)
	{
		agent_ = new pathfinder::NpcRamble();
		agent_->Start(pnpc_->world_id(), 
				      A2D_TO_PF(pnpc_->pos()), 
					  A2D_TO_PF(center_), 
					  speed, 
					  range_);
	}
	if (agent_->GetToGoal())
	{
		return false;
	}

	if (!agent_->MoveOneStep())
    {
        return false;
    }

	// 移动后得到的新位置，传给客户端
	A2DVECTOR targetpos;
	targetpos  = PF_TO_A2D(agent_->GetCurPos());
	stop_flag_ = false;
	
	A2DVECTOR offset = targetpos;
	offset -= pnpc_->pos();

    //ASSERT(pnpc_->world_plane()->IsWalkablePos(targetpos));
	if (offset.squared_magnitude() < 1e-3)
	{
		if (!stop_flag_)
		{
			pnpc_->sender()->MoveStop(pnpc_->pos(),
				                      speed_to_client_,
								      pnpc_->dir(),
								      movement::MOVE_MODE_WALK);
		}
		stop_flag_ = true;
		// 只发送一次stop即可
		return !agent_->GetToGoal();
	}

	if (pnpc_->StepMove(offset))
	{
		if (agent_->GetToGoal())
		{
			stop_flag_ = true;
			pnpc_->sender()->MoveStop(pnpc_->pos(),
				                      speed_to_client_,
								      pnpc_->dir(),
								      movement::MOVE_MODE_WALK);
			// 只发送一次stop即可
			return false;
		}
		else
		{
			pnpc_->sender()->Move(targetpos, 
					              kNpcStrollUseMsecTime,
								  speed_to_client_,
								  movement::MOVE_MODE_WALK);
		}
	}

	return true;
}


///
/// NpcFollowTargetSession
///
const int NpcFollowTargetSession::kMaxFollowTimes = 100;   // 单位：次

NpcFollowTargetSession::NpcFollowTargetSession(Npc* npc)
	: NpcSession(npc),
	  target_(-1, -1),
	  min_range_squared_(1000.f),
	  max_range_squared_(0.f),
	  timeout_(0),
	  stop_flag_(false),
	  reachable_count_(0),
	  retcode_(0),
	  agent_(NULL)
{
}

NpcFollowTargetSession::~NpcFollowTargetSession()
{
	SAFE_DELETE(agent_);
}

float NpcFollowTargetSession::GetSpeed()
{
	return pnpc_->GetChaseSpeed();
}

bool NpcFollowTargetSession::OnStartSession(const ActiveSession* next_ses)
{
	// 后面有操作就不继续了，因为这样就无法进行正确的处理了
	if (next_ses) 
		return false;

	ASSERT(target_.IsValid());
	speed_to_client_ = speed_to_percentage(GetSpeed());
	self_start_pos_  = pnpc_->pos();

	Run();

	int tick = millisecond_to_tick(kNpcFollowUseMsecTime);
	AutoSetTimer(tick, kMaxFollowTimes);
	return true;
}

int NpcFollowTargetSession::OnEndSession()
{
	ASSERT(session_id() != -1);

	// 这里可能无需发送停止移动的消息，因为可能已经发送
	if (!stop_flag_)
	{
		pnpc_->sender()->MoveStop(pnpc_->pos(),
				                  speed_to_client_,
								  pnpc_->dir(),
								  movement::MOVE_MODE_RUN);
	}

	return retcode_;
}

bool NpcFollowTargetSession::RepeatSession(int times)
{
	if (--timeout_ <= 0 || stop_flag_)
		return false;
	return Run();
}

void NpcFollowTargetSession::TrySendStop()
{
	if (!stop_flag_)
	{
		stop_flag_ = true;
		pnpc_->sender()->MoveStop(pnpc_->pos(),
				                  speed_to_client_,
								  pnpc_->dir(),
								  movement::MOVE_MODE_RUN);
	}
}

void NpcFollowTargetSession::SetTarget(const XID& target, float min_range, float max_range, int timeout)
{
	target_            = target;
	min_range_squared_ = min_range*min_range;
	max_range_squared_ = max_range*max_range;
	timeout_           = timeout * (int)(((float)1000 / kNpcFollowUseMsecTime) + 0.5);
	range_target_      = min_range*0.7f; // 2015-05-21从0.85改成0.7
}

bool NpcFollowTargetSession::Run()
{
#define TEST_GETTOGOAL() \
	if (agent_->GetToGoal()) \
	{ \
		if (++reachable_count_ >= 3) \
		{ \
			retcode_ = NSRC_ERR_PATHFINDING; \
			return 0; \
		} \
		else \
		{ \
			TrySendStop(); \
			return 1; \
		} \
	}

	float     range;
	A2DVECTOR object_pos;
	if (target_.IsPlayer())
	{
		world::player_base_info info;
		if ( !pnpc_->world_plane()->QueryPlayer(target_.id, info) 
		  || (range = info.pos.squared_distance(self_start_pos_)) >= max_range_squared_ )
		{
			// 超出范围
			retcode_ = NSRC_OUT_OF_RANGE;
			return false;
		}

		object_pos = info.pos;
	}
	else if (target_.IsNpc())
	{
		world::worldobj_base_info info;
		if ( !pnpc_->world_plane()->QueryObject(target_, info)
		  || (range = info.pos.squared_distance(self_start_pos_)) >= max_range_squared_ )
		{
			// 超出范围
			retcode_ = NSRC_OUT_OF_RANGE;
			return false;
		}

		object_pos = info.pos;
	}
	else
	{
		// 追的是什么东西？
		ASSERT(false);
		return false;
	}

	if (range < min_range_squared_)
	{
		// 已追上，停止追击
		retcode_ = NSRC_SUCCESS;
		return false;
	}

	// 更新速度
	speed_to_client_ = speed_to_percentage(GetSpeed());

	//
	// 计算追击
	//
	const A2DVECTOR old_pos = pnpc_->pos();
	float step = GetSpeed() * ((float)kNpcFollowUseMsecTime/1000);
	bool first_call = false;
	if (!agent_)
	{
		first_call = true;
		agent_ = new pathfinder::NpcChase();
		agent_->Start(pnpc_->world_id(), A2D_TO_PF(old_pos), A2D_TO_PF(object_pos), range_target_);
		TEST_GETTOGOAL();
	}
	else
	{
		// 这里需要判断一下是否目标移动过多，以至于需要重新计算位置
		const A2DVECTOR oldtarget = PF_TO_A2D(agent_->GetTarget());
		float dis = maths::squared_distance(oldtarget, object_pos);
		if (dis > 0.25f)
		{
			// 重新进行Start
			agent_->Start(pnpc_->world_id(), A2D_TO_PF(old_pos), A2D_TO_PF(object_pos), range_target_);
			TEST_GETTOGOAL();
		}
	}

	// 不断的移动
	if (!agent_->MoveOneStep(step))
	{
		if (first_call) 
			return true;

		retcode_ = NSRC_ERR_PATHFINDING;
		return false;
	}
	
	// 移动后得到的新位置，传给客户端
	A2DVECTOR newpos;
	newpos = PF_TO_A2D(agent_->GetCurPos());

	A2DVECTOR offset = newpos;
	offset -= old_pos;
	if (offset.squared_magnitude() < 1e-3)
	{
		// 用距离判断是否真正发生了移动
        // 第一次移动有可能移动距离很小
        if (first_call)
            return true;

		TrySendStop();
		return true;
	}
	stop_flag_ = false;

	if (pnpc_->StepMove(offset))
	{
		pnpc_->sender()->Move(newpos, 
				              kNpcFollowUseMsecTime,
				              speed_to_client_,
							  movement::MOVE_MODE_RUN);
	}

	return true;
#undef TEST_GETTOGOAL
}


///
/// NpcPatrolSession
///
NpcPatrolSession::NpcPatrolSession(Npc* npc)
	: NpcSession(npc),
	  range_target_(0.f),
	  range_squared_(0.f),
	  timeout_(0),
	  speed_to_client_(0),
	  stop_flag_(false),
	  reachable_count_(0),
	  retcode_(0),
	  agent_(NULL)
{
}

NpcPatrolSession::~NpcPatrolSession()
{
	SAFE_DELETE(agent_);
}

void NpcPatrolSession::SetTarget(const A2DVECTOR& target, int timeout, float range)
{
	target_pos_    = target;
	timeout_       = timeout * (int)(((float)1000 / kNpcPatrolUseMsecTime) + 0.5);
	range_target_  = range * 0.95;
	range_squared_ = range*range;
}

float NpcPatrolSession::GetSpeed()
{
	return pnpc_->GetChaseSpeed();
}

bool NpcPatrolSession::OnStartSession(const ActiveSession* next_ses)
{
	speed_to_client_ = speed_to_percentage(GetSpeed());

	Run();

	int tick = millisecond_to_tick(kNpcPatrolUseMsecTime);
	AutoSetTimer(tick, kMaxPatrolTimes);
	return true;
}

int NpcPatrolSession::OnEndSession()
{
	ASSERT(session_id() != -1);

	// 这里可能无需发送停止移动的消息，因为可能已经发送
	if (!stop_flag_)
	{
		pnpc_->sender()->MoveStop(pnpc_->pos(),
				                  speed_to_client_,
								  pnpc_->dir(),
								  movement::MOVE_MODE_RUN);
	}

	return retcode_;
}

bool NpcPatrolSession::RepeatSession(int times)
{
	if (--timeout_ <= 0 || stop_flag_)
		return false;
	return Run();
}

void NpcPatrolSession::TrySendStop()
{
	if (!stop_flag_)
	{
		stop_flag_ = true;
		pnpc_->sender()->MoveStop(pnpc_->pos(),
				                  speed_to_client_,
								  pnpc_->dir(),
								  movement::MOVE_MODE_RUN);
	}
}

bool NpcPatrolSession::Run()
{
#define TEST_GETTOGOAL() \
	if (agent_->GetToGoal()) \
	{ \
		if (++reachable_count_ >= 3) \
		{ \
			retcode_ = NSRC_ERR_PATHFINDING; \
			return 0; \
		} \
		else \
		{ \
			TrySendStop(); \
			return 1; \
		} \
	}

	A2DVECTOR cur_pos = pnpc_->pos();
	if (maths::squared_distance(target_pos_, cur_pos) < range_squared_)
	{
		// 已追上，停止追击
		retcode_ = NSRC_SUCCESS;
		return false;
	}

	// 更新速度
	speed_to_client_ = speed_to_percentage(GetSpeed());

	//
	// 计算移动
	//
	const A2DVECTOR old_pos   = pnpc_->pos();
	float step = GetSpeed() * ((float)kNpcPatrolUseMsecTime/1000);
	bool first_call = false;
	if (!agent_)
	{
		first_call = true;
		agent_ = new pathfinder::NpcChase();
		agent_->Start(pnpc_->world_id(), A2D_TO_PF(old_pos), A2D_TO_PF(target_pos_), range_target_);
		TEST_GETTOGOAL();
	}
	else
	{
		// 这里需要判断一下是否目标移动过多，以至于需要重新计算位置
		const A2DVECTOR oldtarget = PF_TO_A2D(agent_->GetTarget());
		float dis = maths::squared_distance(oldtarget, target_pos_);
		if (dis > 1.f)
		{
			// 重新进行Start
			agent_->Start(pnpc_->world_id(), A2D_TO_PF(old_pos), A2D_TO_PF(target_pos_), range_target_);
			TEST_GETTOGOAL();
		}
	}


	// 不断的移动
	if (!agent_->MoveOneStep(step))
	{
		if (first_call) 
			return true;

		retcode_ = NSRC_ERR_PATHFINDING;
		return false;
	}
	
	// 移动后得到的新位置，传给客户端
	A2DVECTOR newpos;
	newpos = PF_TO_A2D(agent_->GetCurPos());

	A2DVECTOR offset = newpos;
	offset -= old_pos;
	if (offset.squared_magnitude() < 1e-3)
	{
		// 用距离判断是否真正发生了移动
        // 第一次移动有可能移动距离很小
        if (first_call)
            return true;

        TrySendStop();
        return true;
	}
	stop_flag_ = false;

	if (pnpc_->StepMove(offset))
	{
		pnpc_->sender()->Move(newpos, 
				              kNpcPatrolUseMsecTime,
				              speed_to_client_,
							  movement::MOVE_MODE_RUN);
	}

	return true;
#undef TEST_GETTOGOAL
}

} // namespace gamed
