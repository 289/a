#ifndef TASK_NOTIFY_SENDER_H_
#define TASK_NOTIFY_SENDER_H_

#include "task_types.h"

namespace task
{

// 任务通知消息发送辅助类
class NotifyUtil
{
public:
	static int32_t SendErr(Player* player, TaskID taskid, int32_t err);
	static int32_t SendNew(Player* player, TaskID taskid);
	static int32_t SendMonsterKilled(Player* player, TaskID taskid, int32_t monster_index, int32_t monster_count);
	static int32_t SendModify(Player* player, TaskID taskid);
	static int32_t SendFinish(Player* player, TaskID taskid, bool succ);
	static int32_t SendComplete(Player* player, TaskID taskid);
	static int32_t SendLimitRes(Player* player, TaskID taskid, int32_t num);
	static int32_t SendStorageRes(Player* player, StorageID sid, bool override, const TaskIDVec& task_list);
	static int32_t SendStorageRes(Player* player, StorageID sid, bool override, TaskID tid);

	static int32_t SendAutoDeliver(Player* player, TaskID taskid);
	static int32_t SendCheckFinish(Player* player, TaskID taskid);
	static int32_t SendCheckAward(Player* player, TaskID taskid);
	static int32_t SendGiveUpTask(Player* player, TaskID taskid);
	static int32_t SendScriptEnd(Player* player, TaskID taskid, bool skip);
	static int32_t SendGuideEnd(Player* player, TaskID taskid);
	static int32_t SendMiniGameEnd(Player* player, TaskID taskid, bool succ);
	static int32_t SendLimitReq(Player* player, TaskID taskid);
	static int32_t SendPlayerRevive(Player* player, TaskID taskid);
	static int32_t SendPlayerLogin(Player* player, TaskID taskid);
	static int32_t SendUITask(Player* player, TaskID taskid);
	static int32_t SendStorageReq(Player* player, StorageID id);
};

} // namespace task

#endif // TASK_NOTIFY_SENDER_H_
