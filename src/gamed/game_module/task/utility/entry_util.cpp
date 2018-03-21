#include <time.h>
#include "entry_util.h"
#include "task_templ.h"
#include "task_manager.h"
#include "task_data.h"
#include "task_interface.h"

namespace task
{

using namespace std;

// 离线：不需要记录任务数据，只需要通过当前时间与任务发放时间比较即可
// 在线：前32位为过去在线时长，后32位为本次上线的时间
static void InitTimeData(int32_t now, const TaskSucc& cond, string& data)
{
	if (!cond.time.record_offline)
	{
		int32_t online_time = 0;
		data.append((char*)(&online_time), sizeof(online_time));
		data.append((char*)(&now), sizeof(now));
	}
}

// 杀怪数据只记数目，其余通过index索引
static void InitMonsterData(const TaskSucc& cond, string& data)
{
	int32_t monster_killed = 0;
	size_t monster_type = cond.monster.monster_list.size();
	for (size_t i = 0; i < monster_type; ++i)
	{
		data.append((char*)(&monster_killed), sizeof(monster_killed));
	}
}

static void InitSuccData(string& data)
{
    char result = 0;
    data.append(&result, sizeof(char));
}

static void InitSuccData(int32_t now, const TaskSucc& cond, string& data)
{
	switch (cond.goal)
	{
	case GOAL_WAIT_TIME:
		return InitTimeData(now, cond, data);
	case GOAL_KILL_MONSTER:
		return InitMonsterData(cond, data);
	default:
		return InitSuccData(data);
	}
}

static void InitFailData(int32_t now, const TaskFail& cond, string& data)
{
	if (cond.time.time != 0 && !cond.time.record_offline)
	{
		int32_t online_time = 0;
		data.append((char*)(&online_time), sizeof(online_time));
		data.append((char*)(&now), sizeof(now));
	}
}

void EntryUtil::Init(int32_t now, const TaskTempl* task, TaskEntry& entry)
{
	entry.id = task->id;
	entry.state = 0;
	entry.deliver_time = now;
	entry.finish_time = 0;
	entry.SetSuccess();
	entry.templ = task;
	InitSuccData(now, task->succ_cond, entry.data);
	InitFailData(now, task->fail_cond, entry.data);
	entry.data.append((char*)(&task->succ_cond.goal), sizeof(int8_t));
}

int32_t EntryUtil::CalcFailOffset(const TaskTempl* task)
{
	int32_t offset = 0;
	switch (task->succ_cond.goal)
	{
	case GOAL_WAIT_TIME:
		if (!task->succ_cond.time.record_offline)
		{
			offset = 2 * sizeof(int32_t);
		}
		break;
	case GOAL_KILL_MONSTER:
		offset = task->succ_cond.monster.monster_list.size() * sizeof(int32_t);
		break;
	default:
        offset = sizeof(int8_t);
		break;
	}
	return offset;
}

static int32_t TaskDataLen(const TaskTempl* task)
{
	int32_t len = EntryUtil::CalcFailOffset(task);
	if (task->fail_cond.time.time != 0 && !task->fail_cond.time.record_offline)
	{
		len += 2 * sizeof(int32_t);
	}
	return len + 1;
}

void EntryUtil::Revise(int32_t now, const TaskTempl* task, TaskEntry& entry)
{
	int8_t goal = task->succ_cond.goal;
	int32_t len = entry.data.length();
    int8_t value = entry.data[len - 1];
	if (len != TaskDataLen(task) || value != goal)
	{
#ifdef CLIENT_SIDE
		__PRINTF("Task Data Version Not Match");
		assert(0);
#else
		entry.data.clear();
		InitSuccData(now, task->succ_cond, entry.data);
		InitFailData(now, task->fail_cond, entry.data);
		entry.data.append((char*)(&goal), sizeof(goal));
#endif
	}
}

int32_t EntryUtil::GetFinishTime(TaskEntry& entry)
{
    int32_t* data = (int32_t*)(&(entry.data[0]));
    return data[0];
}

int32_t EntryUtil::IncFinishTime(TaskEntry& entry)
{
    if (entry.data.empty())
    {
        int32_t finish_time = 0;
        entry.data.append((char*)(&finish_time), sizeof(finish_time));
    }

    int32_t* data = (int32_t*)(&(entry.data[0]));
    data[0] += 1;
    return data[0];
}

int32_t EntryUtil::DecFinishTime(TaskEntry& entry, int32_t num)
{
    int32_t* data = (int32_t*)(&(entry.data[0]));
    int32_t diff = (num == -1 || data[0] <= num) ? data[0] : num;
    data[0] -= diff;
    return data[0];
}

static void ConvertTm(time_t time, struct tm& date)
{
#ifdef PLATFORM_WINDOWS
	localtime_s(&date, &time);
#else // !PLATFORM_WINDOWS
	localtime_r(&time, &date);
#endif // PLATFORM_WINDOWS
}

static bool CrossDay(Player* player, const TaskEntry& entry)
{
    struct tm finish_tm;
    ConvertTm(entry.finish_time, finish_tm);
    finish_tm.tm_mday += 1;
    finish_tm.tm_hour = 6;
    finish_tm.tm_min = 0;
    finish_tm.tm_sec = 0;
    return player->GetCurTime() >= mktime(&finish_tm);
}

static bool CrossWeek(Player* player, const TaskEntry& entry)
{
    struct tm finish_tm;
    ConvertTm(entry.finish_time, finish_tm);
    finish_tm.tm_mday += (7 - finish_tm.tm_wday);
    finish_tm.tm_hour = 6;
    finish_tm.tm_min = 0;
    finish_tm.tm_sec = 0;
    return player->GetCurTime() >= mktime(&finish_tm);
}

static bool CrossMonth(Player* player, const TaskEntry& entry)
{
    int32_t now = player->GetCurTime();
    int32_t finish_time = entry.finish_time;
    if (now <= finish_time)
    {
        return false;
    }

    struct tm now_tm;
    ConvertTm(now, now_tm);
    struct tm finish_tm;
    ConvertTm(finish_time, finish_tm);
    return now_tm.tm_year != finish_tm.tm_year || now_tm.tm_mon != finish_tm.tm_mon;
}

static bool CrossTime(Player* player, const TaskEntry& entry)
{
    int32_t now = player->GetCurTime();
    int32_t finish_time = entry.finish_time - 1;
    struct tm now_tm;
    ConvertTm(now, now_tm);
    struct tm finish_tm;
    ConvertTm(finish_time, finish_tm);

    // 如果中间间隔超过一年，则肯定满足
    if (now_tm.tm_year - finish_tm.tm_year >= 2)
    {
        return true;
    }

    time_t start = 0, end = 0;
    entry.templ->limit.segment.segment.GetNextTime(finish_time, start, end);
    if (start != end) // 年内就存在清除点
    {
        return now >= start;
    }
    entry.templ->limit.segment.segment.GetNextTime(start + 1, start, end);
    return now >= start;
}

bool EntryUtil::NeedUpdate(Player* player, const TaskEntry& entry)
{
    switch (entry.templ->limit.clean)
    {
    case CLEANUP_DAY:
        return CrossDay(player, entry);
    case CLEANUP_WEEK:
        return CrossWeek(player, entry);
    case CLEANUP_MONTH:
        return CrossMonth(player, entry);
    case CLEANUP_TIME:
        return CrossTime(player, entry);
    default:
        return false;
    }
}

} // namespace task
