#ifndef TASK_SERVER_TASK_H_
#define TASK_SERVER_TASK_H_

#include "task_types.h"

namespace task
{

// 初始化任务系统
bool InitTaskSys(const char* task_file);

// 下面的接口只负责查询任务相关信息，并不检查玩家身上是否有该任务
bool IsTaskNPC(TaskID taskid, int32_t npc_id);
bool IsTaskMonster(TaskID taskid, int32_t mob_id, int32_t mob_group, int32_t& scene_id);
bool IsTaskMine(TaskID taskid, int32_t mine_id);
bool IsTaskTransPos(Player* player, TaskID taskid, int32_t world_id, double x, double y);
bool IsChangeNameTask(TaskID taskid, int8_t name_type);
bool IsTransferGenderClsTask(TaskID taskid);
bool IsStorageTask(StorageID sid, TaskID tid);
bool IsEnterInsTask(TaskID taskid, int32_t ins_id);
bool IsEnterBGTask(TaskID taskid, int32_t bg_id);
bool IsUITask(TaskID taskid);
bool IsTaskFinish(Player* player, TaskID taskid);
TaskID GetTopTaskID(TaskID taskid);

// 强制刷新库任务
// 普通刷新，重新随机库任务
int32_t UpdateTaskStorage(Player* player, int32_t sid);
// 高级刷新，必定刷出指定品级之上的一个库任务
int32_t UpdateTaskStorageByCash(Player* player, int32_t sid);

// 以下情况需要对玩家身上的任务进行特定操作
void PlayerEnterMap(Player* player);
//void PlayerRevive(Player* player);

// 发放任务
int32_t CanDeliverTask(Player* player, TaskID taskid);
int32_t DeliverTask(Player* player, TaskID taskid, bool npc = true);

// 接收客户端发放的任务消息
void RecvClientNotify(Player* player, uint16_t type, const char* buff, size_t size);

// 服务器触发任务完成的情况
void ChangeNameSucc(Player* player, TaskID taskid, int8_t name_type);
void TransferGenderClsSucc(Player* player, TaskID taskid);
void KillMonster(Player* player, int32_t mob_id, int32_t mob_count, ItemMap& items);
bool CombatFail(Player* player, TaskID taskid);
//void NPCDead(Player* player, int32_t npc_id);
//void PlayerLogout(Player* player);

// NPC发放任务奖励
void DeliverAward(Player* player, TaskID taskid, int32_t choice = 0);

// 直接将某任务完成
// 仅任务系统使用，GS除了CompleteTask接口以外，其他地方不应该使用
void FinishPlayerTask(Player* player, TaskID taskid, bool succ);

} // namespace task

#endif // TASK_SERVER_TASK_H_
