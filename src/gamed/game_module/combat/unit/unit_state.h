#ifndef __GAME_MODULE_COMBAT_UNIT_STATE_H__
#define __GAME_MODULE_COMBAT_UNIT_STATE_H__

#include <set>
#include <map>
#include <vector>
#include "shared/base/mutex.h"

namespace combat
{

/**
 * 战斗对象的动作种类
 */
enum Option
{
	OPT_ACTION,            // 战斗对象开始攻击
	OPT_ATTACKED,          // 战斗对象被攻击
	OPT_SELECT_SKILL,      // 玩家选择技能
	OPT_PET_ATTACK,        // 玩家施放宠物技能
	OPT_REVIVE,            // 玩家从濒死状态复活
	OPT_RESURRECT,         // 玩家从死亡状态复活
	OPT_SUMMONED,          // 魔偶被召唤
    OPT_TRANSFORM,         // 怪物变身
    OPT_ESCAPE,            // 怪物逃跑
};

/**
 * 战斗对象状态,彼此互斥
 */
enum Status
{
    // 0
	STATUS_NULL,
	STATUS_NORMAL,
	STATUS_ACTION,
	STATUS_ATTACKED,
	STATUS_ZOMBIE_DYING,
    // 5
	STATUS_DYING,
	STATUS_DEAD,
	STATUS_SLEEP,
	STATUS_ZOMBIE,
	STATUS_SNEAKED,
    // 10
    STATUS_TRANSFORM_WAIT,
    STATUS_TRANSFORMING,
    STATUS_PLAYER_ACTION,
    STATUS_WAIT_GOLEM,
    STATUS_WG_ATTACKED,
    // 15
    STATUS_GOLEM_ACTION,
    STATUS_ESCAPE_WAIT,
	STATUS_MAX,
};

/**
 * 触发状态变更的事件
 */
enum Event
{
	EVENT_INIT,
	EVENT_SNEAKED,
	EVENT_ACTION,
	EVENT_ATTACKED,
	EVENT_DYING,
	EVENT_ZOMBIE,
	EVENT_RELEASE,
	EVENT_ACTIVED,
	EVENT_DEACTIVED,
	EVENT_POWER_FULL,
	EVENT_POWER_EMPTY,
	EVENT_REVIVE,
	EVENT_TIMEOUT,
    EVENT_TRANSFORM,
    EVENT_TRANSFORMING,
    EVENT_PLAYER_ACTION,
    EVENT_WAIT_GOLEM,
    EVENT_GOLEM_ACTION,
    EVENT_ESCAPE,
};

class UnitState;
class CombatUnit;
class State
{
protected:
	Status status;
	int timeout;//Tick
	std::set<int/*opt*/> opts;

public:
	State():
		status(STATUS_NULL),
		timeout(-1)
	{
	}
	explicit State(Status s):
		status(s),
		timeout(-1)
	{
	}
	State(Status s, int* first, size_t n):
		status(s),
		timeout(-1),
		opts(first, first+n)
	{
	}
	virtual ~State()
	{
		status = STATUS_NULL;
		timeout = -1;
		opts.clear();
	}

	friend class UnitState;

public:
	bool IsNull() const           { return status == STATUS_NULL; }
	bool IsNormal() const         { return status == STATUS_NORMAL; }
    bool IsAction() const         { return status == STATUS_ACTION || status == STATUS_PLAYER_ACTION || status == STATUS_GOLEM_ACTION || status == STATUS_ESCAPE_WAIT; }
	bool IsAttacked() const       { return status == STATUS_ATTACKED || status == STATUS_WG_ATTACKED; }
	bool IsZombieDying() const    { return status == STATUS_ZOMBIE_DYING; }
    bool IsDying2() const         { return status == STATUS_DYING; }
	bool IsDead() const           { return status == STATUS_DEAD; }
	bool IsZombie() const         { return status == STATUS_ZOMBIE; }
	bool IsSleep() const          { return status == STATUS_SLEEP; }
	bool IsSneaked() const        { return status == STATUS_SNEAKED; }
    bool IsTransformWait() const  { return status == STATUS_TRANSFORM_WAIT; }
    bool IsTransforming() const   { return status == STATUS_TRANSFORMING; }
    bool IsEscapeWait() const     { return status == STATUS_ESCAPE_WAIT; }
    bool IsPlayerAction() const   { return status == STATUS_PLAYER_ACTION; }
    bool IsWaitGolem() const      { return status == STATUS_WAIT_GOLEM; }
    bool IsGolemAction() const    { return status == STATUS_GOLEM_ACTION; }
    bool IsGolemAttacked() const  { return status == STATUS_WG_ATTACKED; }

	bool operator == (Status s) const
	{
		return status == s;
	}
	bool operator == (const State& rhs) const
	{ 
		return status == rhs.status;
	}
	bool operator != (const State& rhs) const
	{
		return status != rhs.status;
	}
	int GetTimeOut() const
	{
		return timeout;
	}
	bool OptionPolicy(int opt) const
	{
		return opts.find(opt) != opts.end();
	}

	void SetTimeOut(int time);
	int  HeartBeat(CombatUnit* unit);

private:
	void OnTime(CombatUnit* unit);
};

/**
 * @class UnitState
 * @brief finite state machine
 */
class UnitState
{
public:
	typedef std::map<Event,Status/*next-status*/> Event2StatusMap;
	typedef std::map<Status/*cur-status*/, Event2StatusMap> StateShiftMap;
	typedef std::map<int/*unit-type*/, StateShiftMap> StateShiftTable;
	typedef std::vector<State> StateVec;

	static StateShiftTable state_table;
	static StateVec state_pool;

	static void InitStateTable();

public:
	//test code
	struct ShiftRecord
	{
		Event event;
		Status cur;
		Status next;
		int   times;
	};

	typedef std::map<int, ShiftRecord> ShiftMap;
	typedef std::map<int, ShiftMap> StateShiftRecordMap;

	static StateShiftRecordMap state_shift_record_map;
	static shared::MutexLock   record_map_lock;
	static void RecordStateShift(int unit_type, Status cur, Status next, Event event);
	static void Trace();

private:
	State state;
	int unit_type;

public:
	UnitState();
	~UnitState();

	bool IsDead() const;
	bool IsDying() const;
    bool IsZombieDying() const;
    bool IsDying2() const;
	bool IsZombie() const;
	bool IsNormal() const;
	bool IsAction() const;
	bool IsAttacked() const;
	bool IsWaitATB() const;
	bool IsSleep() const;
	bool IsSneaked() const;
    bool IsTransformWait() const;
    bool IsEscapeWait() const;
    bool IsTransforming() const;
    bool IsPlayerAction() const;
    bool IsWaitGolem() const;
    bool IsGolemAction() const;
    bool IsGolemAttacked() const;

	void Init(int unit_type);
	char GetStatus() const;
	void SetTimeOut(int timeout);
	int  GetTimeOut() const;
	bool Update(Event event, int timeout=-1, CombatUnit* unit=NULL);
	void HeartBeat(CombatUnit* unit);
	bool OptionPolicy(int opt) const;
	void Clear();
};


///
/// inline func
///
inline char UnitState::GetStatus() const
{
	return state.status;
}

inline void UnitState::SetTimeOut(int timeout)
{
	state.SetTimeOut(timeout);
}

inline int UnitState::GetTimeOut() const
{
	return state.timeout;
}

inline bool UnitState::IsDead() const
{
	return state.IsDead();
}

inline bool UnitState::IsZombieDying() const
{
	return state.IsZombieDying();
}

inline bool UnitState::IsDying2() const
{
	return state.IsDying2();
}

inline bool UnitState::IsDying() const
{
	return IsZombieDying() || IsDying2();
}

inline bool UnitState::IsZombie() const
{
	return state.IsZombie();
}

inline bool UnitState::IsNormal() const
{
	return state.IsNormal();
}

inline bool UnitState::IsAction() const
{
	return state.IsAction();
}

inline bool UnitState::IsAttacked() const
{
	return state.IsAttacked();
}

inline bool UnitState::IsSleep() const
{
	return state.IsSleep();
}

inline bool UnitState::IsSneaked() const
{
	return state.IsSneaked();
}

inline bool UnitState::IsTransformWait() const
{
	return state.IsTransformWait();
}

inline bool UnitState::IsTransforming() const
{
    return state.IsTransforming();
}

inline bool UnitState::IsEscapeWait() const
{
	return state.IsEscapeWait();
}

inline bool UnitState::IsPlayerAction() const
{
    return state.IsPlayerAction();
}

inline bool UnitState::IsWaitGolem() const
{
    return state.IsWaitGolem();
}

inline bool UnitState::IsGolemAction() const
{
    return state.IsGolemAction();
}

inline bool UnitState::IsGolemAttacked() const
{
    return state.IsGolemAttacked();
}

};

#endif // __GAME_MODULE_COMBAT_UNIT_STATE_H__
