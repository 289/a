#include "notify_util.h"
#include "task_msg.h"
#include "task_interface.h"

namespace task
{

// 服务器发送消息
int32_t NotifyUtil::SendErr(Player* player, TaskID taskid, int32_t err)
{
	TaskNotifyErr msg;
	msg.id = taskid;
	msg.err = err;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return err;
}

int32_t NotifyUtil::SendNew(Player* player, TaskID taskid)
{
	TaskNotifyNew msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendMonsterKilled(Player* player, TaskID taskid, int32_t monster_index, int32_t monster_count)
{
	TaskNotifyMonsterKilled msg;
	msg.id = taskid;
	msg.monster_index = monster_index;
	msg.monster_count = monster_count;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendModify(Player* player, TaskID taskid)
{
	TaskNotifyModify msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendFinish(Player* player, TaskID taskid, bool succ)
{
	TaskNotifyFinish msg;
	msg.id = taskid;
	msg.succ = succ;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendComplete(Player* player, TaskID taskid)
{
	TaskNotifyComplete msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendLimitRes(Player* player, TaskID taskid, int32_t num)
{
	TaskNotifyLimitRes msg;
	msg.id = taskid;
    msg.num = num;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendStorageRes(Player* player, StorageID sid, bool override, const TaskIDVec& task_list)
{
	TaskNotifyStorageRes msg;
	msg.id = sid;
	msg.override = override;
	msg.task_list = task_list;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendStorageRes(Player* player, StorageID sid, bool override, TaskID tid)
{
	TaskNotifyStorageRes msg;
	msg.id = sid;
	msg.override = override;
	msg.task_list.push_back(tid);
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

// 客户端发送消息
int32_t NotifyUtil::SendAutoDeliver(Player* player, TaskID taskid)
{
	TaskNotifyAutoDeliver msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendCheckFinish(Player* player, TaskID taskid)
{
	TaskNotifyCheckFinish msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendCheckAward(Player* player, TaskID taskid)
{
	TaskNotifyCheckAward msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendGiveUpTask(Player* player, TaskID taskid)
{
	TaskNotifyGiveUp msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendScriptEnd(Player* player, TaskID taskid, bool skip)
{
	TaskNotifyScriptEnd msg;
	msg.id = taskid;
	msg.skip = skip;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendGuideEnd(Player* player, TaskID taskid)
{
	TaskNotifyGuideEnd msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendMiniGameEnd(Player* player, TaskID taskid, bool succ)
{
	TaskNotifyMiniGameEnd msg;
	msg.id = taskid;
	msg.succ = succ;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendLimitReq(Player* player, TaskID taskid)
{
	TaskNotifyLimitReq msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendPlayerRevive(Player* player, TaskID taskid)
{
	TaskNotifyRevive msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendPlayerLogin(Player* player, TaskID taskid)
{
	TaskNotifyLogin msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendUITask(Player* player, TaskID taskid)
{
	TaskNotifyUITask msg;
	msg.id = taskid;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

int32_t NotifyUtil::SendStorageReq(Player* player, StorageID id)
{
	TaskNotifyStorageReq msg;
	msg.id = id;
	msg.Marshal();
	player->SendTaskNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
	return ERR_TASK_SUCC;
}

} // namespace task
