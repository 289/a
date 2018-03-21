#ifndef TASK_ENTRY_UTIL_H_
#define TASK_ENTRY_UTIL_H_

#include "task_types.h"

namespace task
{

class TaskTempl;
class TaskEntry;

class EntryUtil
{
public:
	// 根据任务类型不同初始化任务数据
	static void Init(int32_t now, const TaskTempl* task, TaskEntry& entry);
	// 修正任务数据，存盘后如果任务模板内容出现了变化，根据新模板的内容修改存盘数据
	static void Revise(int32_t now, const TaskTempl* task, TaskEntry& entry);
	static int32_t CalcFailOffset(const TaskTempl* task);
    // 对有完成次数的任务进行处理
    static int32_t GetFinishTime(TaskEntry& entry);
    static int32_t IncFinishTime(TaskEntry& entry);
    static int32_t DecFinishTime(TaskEntry& entry, int32_t num);
    static bool NeedUpdate(Player* player, const TaskEntry& entry);
};

} // namespace task

#endif // TASK_ENTRY_UTIL_H_
