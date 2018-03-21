#ifndef __GAMED_MODULE_COMBAT_STATE_H__
#define __GAMED_MODULE_COMBAT_STATE_H__

#include "combat_types.h"
#include "state.h"
#include "state_machine.h"

namespace combat
{

enum XOPT
{
    XOPT_INVALID,
    XOPT_REGISTER_ATB,
    XOPT_UNREGISTER_ATB,
    XOPT_PLAYER_OPERATE,
    XOPT_SCRIPT_OPERATE,
    XOPT_STOP,
    XOPT_SUSPEND,
    XOPT_WAIT_SELECT_SKILL,
    XOPT_WAIT_SELECT_PET,
    XOPT_MAX,
};

enum XSTATUS
{
	XSTATUS_CLOSED,             // 战场处于关闭状态，对象池里面的空闲战场均处于本状态。
    //XSTATUS_OPEN_WAIT,        // 战场等待开放，PVP战斗等待对方玩家加入战斗时的状态。
	XSTATUS_OPEN_READY,         // 战场已经就绪，等待战斗正式开始，这个时间留给客户端播放角色出场准备。
	XSTATUS_RUNNING,            // 战场已经开放，战斗进行中。
	XSTATUS_STOPPED,            // 战斗被ATB暂停，状态超时后继续战斗。
	XSTATUS_SUSPEND,            // 战斗被脚本暂停，状态超时后继续执行脚本，脚本执行完后再继续战斗。
	XSTATUS_CLOSE_WAIT,         // 战场关闭阶段1，场上有人在行动。但是从现在开始不会有新的攻击发起了。
	XSTATUS_LAST_WAIT,          // 战场关闭阶段2，场上的人全部回位了，但是死亡的可能正在播死亡动作。
    XSTATUS_WAIT_SELECT_SKILL,  // 战斗被脚本暂停，状态超时或者玩家选择技能后继续执行脚本，脚本执行完后再继续战斗
    XSTATUS_WAIT_SELECT_PET,    // 战斗被脚本暂停，状态超时或者玩家选择宠物后继续执行脚本，脚本执行完后再继续战斗
	XSTATUS_MAX,
};

enum XEVENT
{
	XEVENT_INIT,                // 初始化战场
	XEVENT_STOP,                // 战斗对象暂停战斗
	XEVENT_SUSPEND,             // 场景脚本暂停战斗
    XEVENT_RESUME,              // 脚本暂停结束，继续战斗
	XEVENT_TIMEOUT,             // 状态超时事件
    XEVENT_CLOSING,             // 战斗结束了，不会有新的攻击发起了
    XEVENT_CLOSE,               // 关闭战场，场上有一方全部处于死亡状态了或者战场异常提前结束
    XEVENT_WAIT_SELECT_SKILL,   // 场景脚本暂停战斗，等待选择技能
    XEVENT_WAIT_SELECT_PET,     // 场景脚本暂停战斗，等待选择宠物
	XEVENT_MAX,
};


class Combat;

/**
 * @class CombatState
 * @brief 战场状态
 */
class CombatState
{
private:
	XState<Combat>* state_;
	Combat* combat_;

public:
	explicit CombatState(Combat* combat): state_(NULL), combat_(combat)
    {
    }
	virtual ~CombatState()
    {
        if (state_)
        {
            delete state_;
            state_ = NULL;
        }
    }

    void Init();
    void Reset();
	void Update(XEVENT event, int timeout/*tick*/=-1);
    bool OptionPolicy(XOPT opt) const;
	void HeartBeat();
    void Clear();

    inline bool IsStateClosed() const;
    inline bool IsStateOpenReady() const;
    inline bool IsStateRunning() const;
    inline bool IsStateStopped() const;
    inline bool IsStateSuspend() const;
    inline bool IsStateCloseWait() const;
    inline bool IsStateLastWait() const;
    inline bool IsStateWaitSelectSkill() const;
    inline bool IsStateWaitSelectPet() const;

public:
	static FSM<Combat> fsm_;

	static void Initialize();
	static void Release();
};


///
/// StateClosed
///
class StateClosed : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_CLOSED, StateClosed, Combat);
};

///
/// StateOpenReady
///
class StateOpenReady : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_OPEN_READY, StateOpenReady, Combat);

	virtual void OnEnter(Combat*);
	virtual void OnLeave(Combat*);
};

///
/// StateRunning
///
class StateRunning : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_RUNNING, StateRunning, Combat);

	virtual void OnEnter(Combat*);
};

///
/// StateStoped
///
class StateStoped : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_STOPPED, StateStoped, Combat);
};

///
/// StateSuspend
///
class StateSuspend : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_SUSPEND, StateSuspend, Combat);

	virtual void OnLeave(Combat*);
};

///
/// StateCloseWait
///
class StateCloseWait : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_CLOSE_WAIT, StateCloseWait, Combat);

	virtual void OnEnter(Combat*);
	virtual void OnHeartBeat(Combat*);
};

///
/// StateLastWait
///
class StateLastWait : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_LAST_WAIT, StateLastWait, Combat);

	virtual void OnLeave(Combat*);
};

///
/// StateWaitSelectSkill
///
class StateWaitSelectSkill : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_WAIT_SELECT_SKILL, StateWaitSelectSkill, Combat);

	virtual void OnLeave(Combat*);
};

///
/// StateWaitSelectPet
///
class StateWaitSelectPet : public XState<Combat>
{
public:
	DECLARE_STATE(XSTATUS_WAIT_SELECT_PET, StateWaitSelectPet, Combat);

	virtual void OnLeave(Combat*);
};

///
/// inline func
///
inline bool CombatState::IsStateClosed() const
{
    return state_->GetStatus() == XSTATUS_CLOSED;
}

inline bool CombatState::IsStateOpenReady() const
{
    return state_->GetStatus() == XSTATUS_OPEN_READY;
}

inline bool CombatState::IsStateRunning() const
{
    return state_->GetStatus() == XSTATUS_RUNNING;
}

inline bool CombatState::IsStateStopped() const
{
    return state_->GetStatus() == XSTATUS_STOPPED;
}

inline bool CombatState::IsStateSuspend() const
{
    return state_->GetStatus() == XSTATUS_SUSPEND;
}

inline bool CombatState::IsStateCloseWait() const
{
    return state_->GetStatus() == XSTATUS_CLOSE_WAIT;
}

inline bool CombatState::IsStateLastWait() const
{
    return state_->GetStatus() == XSTATUS_LAST_WAIT;
}

inline bool CombatState::IsStateWaitSelectSkill() const
{
    return state_->GetStatus() == XSTATUS_WAIT_SELECT_SKILL;
}

inline bool CombatState::IsStateWaitSelectPet() const
{
    return state_->GetStatus() == XSTATUS_WAIT_SELECT_PET;
}

}; // namespace combat

#endif // __GAMED_MODULE_COMBAT_STATE_H__
