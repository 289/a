#ifndef TASK_TASK_FAIL_H_
#define TASK_TASK_FAIL_H_

#include "basic_cond.h"

namespace task
{

enum TaskFailFlag
{
	TASK_FAIL_LOGOUT = 1,
	TASK_FAIL_DEAD = 1 << 1,
};

// 任务失败条件
class TaskFail
{
public:
    TaskFail()
        : npc_dead(0), fail_flag(0)
    {
    }

	inline bool CheckDataValidity() const;
	inline bool fail_dead() const;
	inline bool fail_logout() const;

	ZoneCond    zone;
	TimeCond    time;
	CounterCond global_counter;
	int32_t     npc_dead;
	int8_t      fail_flag;
	CounterCond map_counter;
	CounterCond player_counter;
    ItemCond    item;

	NESTED_DEFINE(zone, time, global_counter, npc_dead, fail_flag, map_counter, player_counter, item);
};

inline bool TaskFail::CheckDataValidity() const
{
	CHECK_VALIDITY(zone)
	CHECK_VALIDITY(time)
	CHECK_VALIDITY(global_counter)
	CHECK_VALIDITY(map_counter)
	CHECK_VALIDITY(player_counter)
    CHECK_VALIDITY(item)
	return npc_dead >= 0;
}

inline bool TaskFail::fail_dead() const
{
	return (fail_flag & TASK_FAIL_DEAD) != 0;
}

inline bool TaskFail::fail_logout() const
{
	return (fail_flag & TASK_FAIL_LOGOUT) != 0;
}

} // namespace task

#endif // TASK_TASK_FAIL_H_
