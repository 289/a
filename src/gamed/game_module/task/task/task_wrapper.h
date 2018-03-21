#ifndef TASK_TASK_WRAPPER_H_
#define TASK_TASK_WRAPPER_H_

#include "task_types.h"

namespace task
{

class TaskTempl;

class TaskWrapper
{
public:
	TaskWrapper(Player* player, const TaskTempl* task);

	// 通用函数
	bool CanShow() const;
	int32_t CanDeliver() const;
	int32_t CheckTaskFinish(bool succ) const;
	int32_t CanDeliverAward() const;
	void AddTask();
	void TaskDeliverOp();
	void TaskFinish(bool succ);
	bool TaskComplete();

	// 仅服务器使用
	void DeliverTask(TaskID sub_task = 0);
	void DeliverAward();
	void KillMonster(int32_t mob_id, int32_t mob_count, ItemMap& items);
private:
	void RecursiveDeliverTask(TaskID sub_task);
	void DeliverManual(TaskID sub_task);
	void DeliverRand();
	void DeliverOrder();
	void DeliverAll();

	void RecursiveDeliverAward(bool succ);
private:
	Player* player_;
	const TaskTempl* task_;
};

} // namespace task

#endif // TASK_TASK_WRAPPER_H_
