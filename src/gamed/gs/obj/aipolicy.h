#ifndef GAMED_GS_OBJ_AIPOLICY_H_
#define GAMED_GS_OBJ_AIPOLICY_H_

#include <vector>

#include "shared/base/noncopyable.h"

#include "gs/global/math_types.h"
#include "gs/global/game_types.h"


namespace gamed {

class Npc;
class WorldObject;
class AIPolicy;
class ActiveSession;

///
/// AIObject
/// 
class AIObject : shared::noncopyable
{
public:
	struct target_info
	{
		A2DVECTOR pos;
	};

    enum GoHameType
    {
        GHT_START_GOING,  // 开始go home
        GHT_REACH_GOAL,   // 到达home pos
        GHT_TASK_END,     // home task被结束
    };

public:
	AIObject()          { }
	virtual ~AIObject() { }

	virtual WorldObject* GetAIOwnerPtr() = 0;

	//
	// session op
	//
	virtual void AddSession(ActiveSession* psession) = 0;
	virtual void ClearSessions() = 0;
	virtual bool HasSession() const = 0;

	//
	// common op
	//
	virtual bool CanRest() = 0;
	virtual bool QueryTarget(const XID& xid, target_info& info) = 0;
	virtual void GetPatrolPos(A2DVECTOR& pos) const = 0;
	virtual void GetSelfPos(A2DVECTOR& pos) const = 0;
	virtual void ReturnHome(const A2DVECTOR& target) = 0;
	virtual void SetGoingHome(GoHameType go_type) = 0;
	virtual bool IsGoingHome() const = 0;
    virtual void ReachDestination(const A2DVECTOR& pos) = 0;

	//
	// aggro op
	//
	virtual size_t GetAggroCount() const = 0;	
	virtual bool   GetFirstAggro(XID& xid) const = 0;
	virtual bool   CanAggroWatch() const = 0;
	virtual void   SetAggroWatch(bool bRst) = 0;
	virtual void   HateTarget(const XID& target) = 0;
	virtual void   RemoveAggroEntry(const XID& xid) = 0;
	virtual void   ClearAggro() = 0;

	//
	// combat op
	//
	virtual void  TriggerCombat(const XID& target) = 0;

	//
	// aggressive monster op
	//
	virtual void  GetChaseRange(float& min_range, float& max_range) const = 0;


private:
};


///
/// AITask
///
class AITask : shared::noncopyable
{
public:
	AITask();
	virtual ~AITask();

	bool Init(AIObject* self, AIPolicy* policy);

	void SessionStart(int session_id);
	void SessionEnd(int session_id, int reason);

	virtual bool StartTask() { return true; }
	virtual bool EndTask();
	virtual void OnHeartbeat() { ASSERT(false); }
	virtual void OnAggro()   { ASSERT(false); }


protected:
	virtual void OnSessionStart(int session_id) { }
	virtual void OnSessionEnd(int session_id, int reason) { }
	

protected:
	AIObject*    self_;
	AIPolicy*    aipolicy_;
	int          session_id_;
};


///
/// AIRestTask
///
class AIRestTask : public AITask
{
	static const int kTimeoutSecs;   // 单位:秒
	static const float kMoveRadius;  // 单位:米
	
public:
	AIRestTask();
	virtual ~AIRestTask();

	virtual bool StartTask();

	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();

	virtual void OnAggro();


private:
	void Execute();

private:
	int timeout_;
};


///
/// AIReturnHomeTask
///
class AIReturnHomeTask : public AITask
{
	static const int   kReturnTimeoutSecs = 60;   // 单位: 秒
	static const float kMinReturnRange    = 0.6;  // 单位: 米
	static const int   kReturnRetryCount  = 3;
public:
	AIReturnHomeTask(const A2DVECTOR& pos);
	virtual ~AIReturnHomeTask();

	virtual bool StartTask();
	virtual bool EndTask();

	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();
	virtual void OnAggro();


private:
	void Execute();


private:
	A2DVECTOR home_pos_;
	int       retry_counter_;
	int       heartbeat_timeout_;
};


///
/// AITargetTask
///
class AITargetTask : public AITask
{
public:
	AITargetTask(const XID& target);
	virtual ~AITargetTask();

	virtual bool StartTask();
	virtual void OnAggro();

	virtual void OnSessionEnd(int session_id, int reason);
	virtual void OnHeartbeat();


protected:
	virtual bool ChangeTarget(const XID& target);
	virtual void Execute() = 0;
	AITargetTask() { }

protected:
	XID target_;
};


///
/// AIAttackTask
///
class AIAttackTask : public AITargetTask
{
	static const int kTimeoutSecs;   // 单位:秒
public:
	AIAttackTask(const XID& target);
	virtual ~AIAttackTask();

private:
	virtual void Execute();
};


///
/// AIPolicy
///
class AIPolicy : shared::noncopyable
{
public:
	enum StrategyType
	{
		STRATEGY_DEFENSIVE = 0,  // 被动
		STRATEGY_AGGRESSIVE,     // 主动
	};

public:
	AIPolicy(AIObject* obj);
	~AIPolicy();

	bool Init();

	void StartTask();
	void TaskEnd();
	void Heartbeat();
	void GoHome();

	template <typename TASKCLASS>
	void AddTask();
	template <typename TASKCLASS>
	void AddTargetTask(const XID& target);
	template <typename TASKCLASS>
	void AddPosTask(const A2DVECTOR& pos);

	void OnGreeting(const XID& xid);
	void OnAggro(); // 仇恨度变化事件
	bool UnderAttack(const XID& attacker);
	void RelieveAttack();
	void DeterminePolicy(const XID& target);
    void ResetPolicy();

	inline void SetPrimaryStrategy(StrategyType type);
	inline bool HasNextTask() const;
	inline bool IsInCombat();
	inline int  GetTaskID() const;
	inline void SessionStart(int task_id, int session_id);
	inline void SessionEnd(int task_id, int session_id, int retcode);


private:
	void AddTask(AITask* ptask);
	void ClearAllTasks();
	void RemoveCurTask();
	void RemoveNextTasks();
	void DeleteEndedTasks();
	bool DetermineTarget(XID& target);
	void EnableCombat(bool can_combat); // 是否允许战斗
	void AddPrimaryTask(const XID& target, int strategy);

	void RollBack();
	void HaveRest();


private:
	AIObject*    self_;
	AITask*      cur_task_;
	int          task_id_;
	bool         in_combat_mode_;
	char*        ev_policy_;            // event policy
	char*        path_agent_;
	StrategyType primary_strategy_;

	typedef std::vector<AITask*> AITaskPtrVec;
	AITaskPtrVec tasks_list_;
	AITaskPtrVec waiting_delete_tasks_;
};

///
/// template func
///
template <typename TASKCLASS>
void AIPolicy::AddTask()
{
	AITask* ptask = new TASKCLASS();
	ptask->Init(self_, this);
	AddTask(ptask);
}

template <typename TASKCLASS>
void AIPolicy::AddTargetTask(const XID& target)
{
	AITargetTask* ptask = new TASKCLASS(target);
	ptask->Init(self_, this);
	AddTask(ptask);
}

template <typename TASKCLASS>
void AIPolicy::AddPosTask(const A2DVECTOR& pos)
{
	AITask* ptask = new TASKCLASS(pos);
	ptask->Init(self_, this);
	AddTask(ptask);
}


///
/// inline func
///
inline void AIPolicy::SetPrimaryStrategy(StrategyType type)
{
	primary_strategy_ = type;
}

inline bool AIPolicy::HasNextTask() const
{
	return !tasks_list_.empty();
}

inline int AIPolicy::GetTaskID() const
{
	return task_id_;
}

inline bool AIPolicy::IsInCombat()
{
	return in_combat_mode_;
}

inline void AIPolicy::SessionStart(int task_id, int session_id)
{
	if (task_id == task_id_ && cur_task_)
	{
		cur_task_->SessionStart(session_id);
	}
}

inline void AIPolicy::SessionEnd(int task_id, int session_id, int retcode)
{
	if (task_id == task_id_ && cur_task_)
	{
		cur_task_->SessionEnd(session_id, retcode);
	}
}

} // namespace gamed

#endif // GAMED_GS_OBJ_AIPOLICY_H_
