#include "aipolicy.h"

#include "shared/logsys/logging.h"
#include "gs/global/randomgen.h"
#include "gs/movement/npc_mvsession.h"

#include "npc.h"


namespace gamed {

namespace {

	Npc* get_npc_ptr(AIObject* self)
	{
		Npc* pnpc = dynamic_cast<Npc*>(self->GetAIOwnerPtr());
		ASSERT(pnpc);
		return pnpc;
	}

} // Anonymous namespace


///
/// AITask
///
AITask::AITask()
	: self_(NULL),
	  aipolicy_(NULL),
	  session_id_(-1)
{
}

AITask::~AITask()
{
	self_       = NULL;
	aipolicy_   = NULL;
	session_id_ = -1;
}

bool AITask::Init(AIObject* self, AIPolicy* policy)
{
	self_     = self;
	aipolicy_ = policy;
	return true;
}

bool AITask::EndTask()
{
	aipolicy_->TaskEnd();
	return true;
}

void AITask::SessionStart(int session_id)
{
	session_id_ = session_id;
	OnSessionStart(session_id);
}

void AITask::SessionEnd(int session_id, int reason)
{
	// session_id 可能会不一致，这是由于会有多个session
	if (session_id < session_id_) 
		return;

	// 重置，Execute()会检查
	session_id_ = -1;

	// for derived class
	OnSessionEnd(session_id, reason);
}


///
/// AIReturnHomeTask
///
AIReturnHomeTask::AIReturnHomeTask(const A2DVECTOR& pos)
	: home_pos_(pos),
	  retry_counter_(kReturnRetryCount),
	  heartbeat_timeout_(kReturnRetryCount * kReturnTimeoutSecs)
{
}

AIReturnHomeTask::~AIReturnHomeTask()
{
}

bool AIReturnHomeTask::StartTask()
{
	// 设置回家状态
	self_->SetGoingHome(AIObject::GHT_START_GOING);

	// 超过有效距离，回出生点
	NpcPatrolSession *psession = new NpcPatrolSession(get_npc_ptr(self_));
	psession->SetTarget(home_pos_, kReturnTimeoutSecs, kMinReturnRange);
	psession->SetAITask(aipolicy_->GetTaskID());
	self_->AddSession(psession);
	return true;
}

bool AIReturnHomeTask::EndTask()
{
    // 检查距离
    A2DVECTOR cur_pos;
	self_->GetSelfPos(cur_pos);
	if (maths::squared_distance(cur_pos, home_pos_) <= kMinReturnRange * kMinReturnRange * 1.5)
    {
        // 到达home pos
	    self_->SetGoingHome(AIObject::GHT_REACH_GOAL);
        self_->ReachDestination(home_pos_);
    }
    else
    {
        // AItask被结束
        self_->SetGoingHome(AIObject::GHT_TASK_END);
    }

	// 调用父类函数
	AITask::EndTask();
	return true;
}

void AIReturnHomeTask::OnSessionEnd(int session_id, int reason)
{
	A2DVECTOR cur_pos;
	self_->GetSelfPos(cur_pos);
	if (maths::squared_distance(cur_pos, home_pos_) > kMinReturnRange * kMinReturnRange)
	{
		if (--retry_counter_ > 0)
		{
			NpcPatrolSession *psession = new NpcPatrolSession(get_npc_ptr(self_));
			psession->SetTarget(home_pos_, kReturnTimeoutSecs, kMinReturnRange);
			psession->SetAITask(aipolicy_->GetTaskID());
			self_->AddSession(psession);
		}
		else
		{
			// 瞬移回出生点
			self_->ReturnHome(home_pos_);
			EndTask();
			return;
		}
	}
	else
	{
		EndTask();
		return;
	}
}

void AIReturnHomeTask::OnHeartbeat()
{
	if (--heartbeat_timeout_ < 0)
	{
		A2DVECTOR cur_pos;
		self_->GetSelfPos(cur_pos);
		if (maths::squared_distance(cur_pos, home_pos_) > kMinReturnRange * kMinReturnRange)
		{
			// 瞬移回出生点
			self_->ReturnHome(home_pos_);
		}
		else
		{
			EndTask();
			return;
		}
	}
}

void AIReturnHomeTask::OnAggro()
{
	ASSERT(false);
}


///
/// AIRestTask
///
const int AIRestTask::kTimeoutSecs  = 2;
const float AIRestTask::kMoveRadius = 2.5f;

AIRestTask::AIRestTask()
	: timeout_(kTimeoutSecs)
{
}

AIRestTask::~AIRestTask()
{
}

bool AIRestTask::StartTask()
{
	Execute();
	return true;
}

void AIRestTask::OnHeartbeat()
{
	if (aipolicy_->IsInCombat())
	{
		EndTask();
		aipolicy_->DeterminePolicy(XID(-1, -1));
		return;
	}

	if (--timeout_ < 0)
	{
		EndTask();
		return;
	}
}

void AIRestTask::OnAggro()
{
	// 中断当前的session结束自己，重新搜寻任务
	if (session_id_ != -1)
		self_->ClearSessions();
	else
		EndTask();

	aipolicy_->DeterminePolicy(XID(-1, -1));
}

void AIRestTask::OnSessionEnd(int session_id, int reason)
{
	if (aipolicy_->HasNextTask())
	{
		EndTask();
	}
	else
	{
		if (mrand::RandF(0.f, 1.f) < 0.1f)
			Execute();
		else
			EndTask();
	}
}

void AIRestTask::Execute()
{
	if (session_id_ != -1) return;

	A2DVECTOR pos;
	self_->GetPatrolPos(pos);

	NpcStrollSession* psession = new NpcStrollSession(get_npc_ptr(self_));
	psession->SetTarget(pos, kTimeoutSecs, kMoveRadius);
	psession->SetAITask(aipolicy_->GetTaskID());
	self_->AddSession(psession);
}

///
/// AITargetTask
///
AITargetTask::AITargetTask(const XID& target)
	: target_(target)
{
}

AITargetTask::~AITargetTask()
{
}

bool AITargetTask::StartTask()
{
	ASSERT(target_.IsValid());
	Execute();
	return true;
}

void AITargetTask::OnAggro()
{
	if (self_->GetAggroCount())
	{
		XID id;
		self_->GetFirstAggro(id);
		ChangeTarget(id);
	}
}

void AITargetTask::OnSessionEnd(int session_id, int reason)
{
	switch (reason)
	{
		case NSRC_ERR_PATHFINDING:
			{
				if (target_.IsPlayer())
				{
					EndTask();
					aipolicy_->GoHome();
					return;
				}
			}
			break;

		case NSRC_OUT_OF_RANGE:
			{
				if (target_.IsPlayer())
				{
					EndTask();
					aipolicy_->GoHome();
					return;
				}
			}
			break;
	}

	if (aipolicy_->HasNextTask())
	{
		EndTask();
	}
	else
	{
		Execute();
	}
}

bool AITargetTask::ChangeTarget(const XID& target)
{
	ASSERT(target.IsValid());
	if (target_ == target) 
		return true;

	target_ = target;
	self_->AddSession(new NpcEmptySession(get_npc_ptr(self_)));
	Execute();
	return true;
}

void AITargetTask::OnHeartbeat()
{
	if (aipolicy_->HasNextTask())
	{
		if (!self_->HasSession())
		{
			EndTask();
		}
		else
		{
			self_->AddSession(new NpcEmptySession(get_npc_ptr(self_)));
		}
		return;
	}

	XID tg(-1, -1);
	if (self_->GetFirstAggro(tg))
	{
		if (tg != target_ || !self_->HasSession())
		{
			target_     = XID(-1, -1);
			session_id_ = -1;
			ChangeTarget(tg);
		}
	}
	else
	{
		// 不论如何都加入一个空session
		self_->AddSession(new NpcEmptySession(get_npc_ptr(self_)));
		if (session_id_ == -1)
		{
			EndTask();
		}
		else
		{
			target_ = XID(-1, -1);
		}
	}
}

///
/// AIAttackTask
///
const int AIAttackTask::kTimeoutSecs = 20;   // 单位:秒

AIAttackTask::AIAttackTask(const XID& target)
	: AITargetTask(target)
{
}

AIAttackTask::~AIAttackTask()
{
}

void AIAttackTask::Execute()
{
	if (session_id_ != -1) 
		return ;

	if (!target_.IsValid())
	{
		EndTask();
		return;
	}

	AIObject::target_info info;
	if (!self_->QueryTarget(target_, info))
	{
		// 目标已经无法再被攻击，任务结束
		self_->RemoveAggroEntry(target_);
		EndTask();
		return;
	}

	A2DVECTOR self_pos = get_npc_ptr(self_)->pos();
	if (self_pos.squared_distance(info.pos) < kAggroNpcMaxCombatDisSquare)
	{
		self_->TriggerCombat(target_);
	}
	else
	{
		// 超过战斗的有效距离，则进入追击
		float min_range, max_range;
		self_->GetChaseRange(min_range, max_range);
		NpcFollowTargetSession *psession = new NpcFollowTargetSession(get_npc_ptr(self_));
		psession->SetTarget(target_, min_range, max_range, kTimeoutSecs);
		psession->SetAITask(aipolicy_->GetTaskID());
		self_->AddSession(psession);
		return;
	}
}


///
/// AIPolicy
///
AIPolicy::AIPolicy(AIObject* obj)
	: self_(obj),
	  cur_task_(NULL),
	  task_id_(-1),
	  in_combat_mode_(false),
	  ev_policy_(NULL),
	  path_agent_(NULL),
	  primary_strategy_(STRATEGY_DEFENSIVE)
{
}

AIPolicy::~AIPolicy()
{
	self_           = NULL;
	task_id_        = -1;
	in_combat_mode_ = false;

	SAFE_DELETE(cur_task_);
	SAFE_DELETE(ev_policy_);
	SAFE_DELETE(path_agent_)

	DeleteEndedTasks();
	RemoveNextTasks();
}

bool AIPolicy::Init()
{
	ASSERT(self_ != NULL);
	ASSERT(cur_task_ == NULL);
	return true;
}

void AIPolicy::StartTask()
{
	ASSERT(cur_task_ == NULL);
	while (tasks_list_.size())
	{
		cur_task_ = tasks_list_[0];
		tasks_list_.erase(tasks_list_.begin());
		++task_id_;
		if (cur_task_->StartTask())
		{
			break;
		}
		RemoveCurTask();
	}
}

void AIPolicy::TaskEnd()
{
	ASSERT(cur_task_);
	RemoveCurTask();
	StartTask();
}

void AIPolicy::RemoveCurTask()
{
	if (cur_task_)
	{
		waiting_delete_tasks_.push_back(cur_task_);
		cur_task_ = NULL;
	}
}

void AIPolicy::ClearAllTasks()
{
	RemoveCurTask();
	RemoveNextTasks();
}

void AIPolicy::RemoveNextTasks()
{
	for (size_t i = 0; i < tasks_list_.size(); ++i)
	{
		DELETE_SET_NULL(tasks_list_[i]);
	}
	tasks_list_.clear();
}

void AIPolicy::DeleteEndedTasks()
{
	for (size_t i = 0; i < waiting_delete_tasks_.size(); ++i)
	{
		DELETE_SET_NULL(waiting_delete_tasks_[i]);
	}
	waiting_delete_tasks_.clear();
}

void AIPolicy::AddTask(AITask* ptask)
{
	if (tasks_list_.size() >= 2)
	{
		// 不要过多的堆积任务
		DELETE_SET_NULL(ptask);
		return;
	}

	bool rst = tasks_list_.empty();
	tasks_list_.push_back(ptask);
	// 如果添加的任务为npc当前的唯一任务，则执行该任务
	if (rst && cur_task_ == NULL)
	{
		StartTask();
	}
}

void AIPolicy::Heartbeat()
{
	// delete ended tasks 
	DeleteEndedTasks();

	if (cur_task_)
	{
		// 优先做当前的任务
		cur_task_->OnHeartbeat();
		if (IsInCombat() && self_->GetAggroCount() == 0)
		{
			RollBack();
		}
	}
	else
	{
		if (!IsInCombat())
		{
            if (self_->IsGoingHome())
            {
                GoHome();
                return;
            }

			HaveRest();
			return;
		}

		if (self_->GetAggroCount())
		{
			DeterminePolicy(XID(-1, -1));
		}
		else
		{
			RollBack();
			return;
		}
	}
}

void AIPolicy::HaveRest()
{
	// 是否能闲逛
	if (!cur_task_ && self_->CanRest())
	{
		// 加入休息策略
		AddTask<AIRestTask>();
	}
}

void AIPolicy::RollBack()
{
	// 仇恨列表清空，做恢复操作
    RelieveAttack();
	EnableCombat(true);
	ClearAllTasks();

	if (path_agent_)
	{
		// ???????????
	}
	else
	{
		// ??????? 设置无敌
		// ???????? 判断是否回到出生点
		// ??????
		A2DVECTOR target;
		self_->GetPatrolPos(target);
		AddPosTask<AIReturnHomeTask>(target);
	}
}

bool AIPolicy::DetermineTarget(XID& target)
{
	return self_->GetFirstAggro(target);
}

void AIPolicy::EnableCombat(bool can_combat)
{
	if (can_combat)
	{
		if (IsInCombat() && ev_policy_)
		{
			//ev_policy_->Reset(); //?????????
		}
		self_->SetAggroWatch(true);
	}
	else
	{
		if (!IsInCombat() && ev_policy_)
		{
			//ev_policy_->StartCombat(); // ????????
		}
		self_->SetAggroWatch(false);
	}
	in_combat_mode_ = can_combat ? false : true;
}

void AIPolicy::OnAggro()
{
	if (IsInCombat())
		return;

	// 如果是第一次出现aggro，那么就立刻进行反应
	XID target;
	if (!DetermineTarget(target))
	{
		// 无法确定目标 
		return;
	}

	//_self->ActiveCombatState(true);
	EnableCombat(false);

	if (cur_task_)
	{
		cur_task_->OnAggro();
		return;
	}

	// 如果仇恨列表中只有一个，立刻发送一次HATE
	if (!target.IsNpc())
		self_->HateTarget(target);

	// 决定下一步策略
	DeterminePolicy(target);
}

void AIPolicy::DeterminePolicy(const XID& target)
{
	if (target.IsValid())
	{
		AddPrimaryTask(target, primary_strategy_);
		if (cur_task_) return;
	}

	// 无目标 或者处理原目标失败
	int count = 3;
	XID old_target(-1, -1);
	while (cur_task_ == NULL && self_->GetAggroCount() && count > 0)
	{
		XID new_target;
		if (!DetermineTarget(new_target))
		{
			break;
		}
		AddPrimaryTask(new_target, primary_strategy_);
		if (old_target == new_target)
			break;
		old_target = new_target;
	}
}

void AIPolicy::AddPrimaryTask(const XID& target, int strategy)
{
	// target 必须是有效的
	ASSERT(target.IsValid());

	switch (strategy)
	{
		case STRATEGY_DEFENSIVE:
			break;

		case STRATEGY_AGGRESSIVE:
			AddTargetTask<AIAttackTask>(target);
			break;

		default:
			ASSERT(false);
	}
}

bool AIPolicy::UnderAttack(const XID& attacker)
{
	// 真正进入战斗
	NpcCombatSession* psession = new NpcCombatSession(get_npc_ptr(self_));
	self_->AddSession(psession);

	if (IsInCombat())
		return false;

	EnableCombat(false);
	ClearAllTasks(); // ?????????
	return true;
}

void AIPolicy::RelieveAttack()
{
	if (!IsInCombat())
	{
		//LOG_ERROR << "怪物没有收到攻击怎么会解除攻击状态？";
		return; 
	}

	// 主要是清掉NpcCombatSession
	self_->ClearSessions();
}

void AIPolicy::ResetPolicy()
{
    // 清除仇恨
	self_->ClearAggro();
	// 主要是清掉NpcCombatSession
    self_->ClearSessions();
    // 清所有的任务
    ClearAllTasks();
    // 设置可以战斗
    EnableCombat(true);
}

void AIPolicy::OnGreeting(const XID& xid)
{
	return;
}

void AIPolicy::GoHome()
{
	// 清除仇恨
	self_->ClearAggro();

	EnableCombat(true);
	RemoveNextTasks();

	A2DVECTOR target;
	self_->GetPatrolPos(target);
	AddPosTask<AIReturnHomeTask>(target);
}

} // namespace gamed
