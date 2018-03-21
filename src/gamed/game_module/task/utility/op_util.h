#ifndef TASK_OP_UTIL_H_
#define TASK_OP_UTIL_H_

#include "task_types.h"

namespace task
{

class Trans;
class DeliverItem;
class RecycleItem;
class ModifyNPCBWList;
class MapElementCtrl;
class ModifyReputation;
class ModifyUI;
class SummonNPC;
class SummonMonster;
class SummonMine;
class ModifyCounter;
class ModifyBuff;
class ModifyTalent;
class ModifyTitle;
class ModifyFriend;
class SendSysMail;
class SendSysChat;
class ItemUseHint;
class CameraMask;
class CounterCond;
class TaskTempl;

// 任务操作辅助函数
class OpUtil
{
public:
	static void ExecuteOp(Player* player, const TaskTempl* task);
	static void ExecuteRepeatOp(Player* player, const TaskTempl* task, bool finish, bool revive = false, bool scrit_end = false);
	static void ExecuteOnceOp(Player* player, const TaskTempl* task);

	static void ExecuteAward(Player* player, const TaskTempl* task, bool succ);

    // 服务器执行的操作
	static void DeliverItemToPlayer(Player* player, const DeliverItem& deliver_item);
	static void RecycleItemFromPlayer(Player* player, const RecycleItem& recycle_item);
	static void ModifyBWList(Player* player, const ModifyNPCBWList& modify_bw);
	static void CtrlElement(Player* player, const MapElementCtrl& ctrl_element);
	static void ModifyCounterInfo(Player* player, const ModifyCounter& counter);
	static void ModifyNPCFriend(Player* player, const ModifyFriend& modify_friend);
	static void SendMail(Player* player, const SendSysMail& mail);
	static void SendChat(Player* player, const SendSysChat& chat);
	static void ServerTransport(Player* player, const Trans& trans);
    static void DecFinishTime(Player* player, TaskID taskid, int32_t num);
    static void ModifyPlayerReputation(Player* player, const ModifyReputation& rep);
    static void ModifyPlayerUI(Player* player, const ModifyUI& ui);
    static void ModifyPlayerBuff(Player* player, const ModifyBuff& buff, bool save);
    static void RecyclePlayerBuff(Player* player, const ModifyBuff& buff);
    static void ModifyPlayerTalent(Player* player, const ModifyTalent& talent);
    static void ModifyPlayerTitle(Player* player, const ModifyTitle& title);
    static void SubscribeCounter(Player* player, const TaskTempl* task, bool subscribe);

    // 客户端执行的操作
	static void Summon(TaskID taskid, Player* player, const SummonNPC& npc);
	static void Summon(TaskID taskid, Player* player, const SummonMonster& monster, bool revive);
	static void Summon(TaskID taskid, Player* player, const SummonMine& mine);
	static void Recycle(TaskID taskid, Player* player, const SummonNPC& npc);
	static void Recycle(TaskID taskid, Player* player, const SummonMonster& monster);
	static void Recycle(TaskID taskid, Player* player, const SummonMine& mine);
	static void Transport(TaskID taskid, Player* player, const Trans& trans);
    static void ShowItemUseHint(TaskID taskid, Player* player, const ItemUseHint& hint);
    static void HideItemUseHint(TaskID taskid, Player* player, const ItemUseHint& hint);
    static void DoCameraMask(TaskID taskid, Player* player, const CameraMask& mask);
    static void UndoCameraMask(TaskID taskid, Player* player, const CameraMask& mask);
    static void TransportToScriptArea(Player* player, const TaskTempl* task);
};

} // namespace task

#endif // TASK_OP_UTIL_H_
