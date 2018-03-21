#include "op_util.h"
#include "task_op.h"
#include "task_interface.h"
#include "task_data.h"
#include "task_award.h"
#include "util.h"
#include "task_templ.h"
#include "notify_util.h"
#include "entry_util.h"
#include "task_manager.h"
#include "ratio_table_templ.h"

namespace task
{

using namespace std;
using namespace shared::net;

void OpUtil::ServerTransport(Player* player, const Trans& trans)
{
	int32_t pos_num = trans.pos_list.size();
	if (pos_num == 0)
	{
		return;
	}
	const Position& pos = trans.pos_list[0];
	__PRINTF("ServerTransport world=%d x=%f y=%f", pos.world_id, pos.x, pos.y);
	player->ServerTransport(pos.world_id, pos.x, pos.y);
}

static void DeliverRandItem(Player* player, const DeliverItem& deliver_item)
{
	// 归一化概率
	vector<int32_t> prob_vec;
	const ItemDeliveredVec& item_list = deliver_item.item_list;;
	ItemDeliveredVec::const_iterator it = item_list.begin();
	for (; it != item_list.end(); ++it)
	{
		prob_vec.push_back(it->prob);
	}

	int32_t rand_index = Util::Rand(prob_vec);
	if (rand_index < 0 || rand_index >= (int32_t)item_list.size())
	{
		return;
	}
	const ItemDeliveredInfo& info = item_list[rand_index];
	__PRINTF("DeliverRandItem id=%d, count=%d", info.id, info.count);
	player->DeliverItem(info.id, info.count, info.valid_time);
}

static void DeliverAllItem(Player* player, const DeliverItem& deliver_item)
{
	const ItemDeliveredVec& item_list = deliver_item.item_list;
	ItemDeliveredVec::const_iterator it = item_list.begin();
	for (; it != item_list.end(); ++it)
	{
		__PRINTF("DeliverAllItem id=%d, count=%d", it->id, it->count);
		player->DeliverItem(it->id, it->count, it->valid_time);
	}
}

void OpUtil::DeliverItemToPlayer(Player* player, const DeliverItem& deliver_item)
{
	if (deliver_item.rand_deliver)
	{
		DeliverRandItem(player, deliver_item);
	}
	else
	{
		DeliverAllItem(player, deliver_item);
	}
}

void OpUtil::RecycleItemFromPlayer(Player* player, const RecycleItem& recycle_item)
{
	const ItemInfoVec& item_list = recycle_item.item_list;
	ItemInfoVec::const_iterator it = item_list.begin();
	for (; it != item_list.end(); ++it)
	{
		__PRINTF("RecycleItem id=%d, count=%d", it->id, it->count);
		player->TakeAwayAllItem(it->id, it->count);
	}
}

void OpUtil::ModifyBWList(Player* player, const ModifyNPCBWList& modify_bw)
{
	const NPCBWList& bw_list = modify_bw.bw_list;
	NPCBWList::const_iterator it = bw_list.begin();
	for (; it != bw_list.end(); ++it)
	{
		__PRINTF("OpUtil::ModifyBWList id=%d black=%s add=%s", it->templ_id, (it->black ? "true" : "false"), (it->add ? "true" : "false"));
		player->ModifyNPCBWList(it->templ_id, it->black, it->add);
	}
}

void OpUtil::CtrlElement(Player* player, const MapElementCtrl& ctrl_element)
{
	const MapElementVec& element_list = ctrl_element.element_list;
	MapElementVec::const_iterator it = element_list.begin();
	for (; it != element_list.end(); ++it)
	{
		__PRINTF("CtrlElement id=%d open=%d", it->element_id, it->open);
		player->CtrlMapElement(it->element_id, it->open);
	}
}

void OpUtil::ModifyCounterInfo(Player* player, const ModifyCounter& counter)
{
	const CounterVec& counter_list = counter.counter_list;
	CounterVec::const_iterator it = counter_list.begin();
	for (; it != counter_list.end(); ++it)
	{
		__PRINTF("ModifyCounterInfo type=%d op=%d id=%d world_id=%d value=%d", it->type, it->op, it->id, it->world_id, it->value);
        switch (it->type)
        {
        case COUNTER_GLOBAL:
            player->ModifyGlobalCounter(it->id, it->op, it->value);
            break;
        case COUNTER_MAP:
            player->ModifyMapCounter(it->world_id, it->id, it->op, it->value);
            break;
        case COUNTER_PLAYER:
            player->ModifyPlayerCounter(it->id, it->op, it->value);
            break;
        default:
            break;
        }
	}
}

void OpUtil::ModifyNPCFriend(Player* player, const ModifyFriend& modify_friend)
{
    const FriendOpVec& friend_list = modify_friend.friend_list;
    FriendOpVec::const_iterator it = friend_list.begin();
    for (; it != friend_list.end(); ++it)
    {
		__PRINTF("ModifyNPCFriend id=%d op=%d", it->id, it->value);
        switch (it->value)
        {
        case FRIEND_OP_ADD:
            player->AddNPCFriend(it->id);
            break;
        case FRIEND_OP_DEL:
            player->DelNPCFriend(it->id);
            break;
        case FRIEND_OP_ONLINE:
            player->OnlineNPCFriend(it->id);
            break;
        case FRIEND_OP_OFFLINE:
            player->OfflineNPCFriend(it->id);
            break;
        default:
            break;
        }
    }
}

void OpUtil::SendMail(Player* player, const SendSysMail& mail)
{
	if (mail.sender == "" || mail.title == "")
	{
		return;
	}

	SendSysMail& tmail = const_cast<SendSysMail&>(mail);
	ByteBuffer buf;
	tmail.Pack(buf);
	player->SendSysMail(buf);
}

void OpUtil::SendChat(Player* player, const SendSysChat& chat)
{
	ChatVec::const_iterator it = chat.chat_list.begin();
	for (; it != chat.chat_list.end(); ++it)
	{
		player->SendSysChat(it->channel, it->sender_name, it->content);
	}
}

void OpUtil::DecFinishTime(Player* player, TaskID taskid, int32_t num)
{
    FinishTimeTask* finish_time = player->GetFinishTimeTask();
    TaskEntry* entry = finish_time->GetEntry(taskid);
    if (entry == NULL)
    {
        return;
    }
    int32_t cur_num = 0;
    if ((cur_num = EntryUtil::DecFinishTime(*entry, num)) == 0)
    {
        finish_time->Erase(taskid);
    }
    NotifyUtil::SendLimitRes(player, taskid, cur_num);
}

void OpUtil::ModifyPlayerReputation(Player* player, const ModifyReputation& rep)
{
	ReputationVec::const_iterator rit = rep.reputation_list.begin();
	for (; rit != rep.reputation_list.end(); ++rit)
	{
		__PRINTF("ModifyReputation op=%d id=%d count=%d", rit->op, rit->id, rit->value);
        if (rit->op < REPUTATION_OP_OPEN || rit->op > REPUTATION_OP_DEC)
        {
            continue;
        }
        if (rit->op == REPUTATION_OP_OPEN)
        {
            player->OpenReputation(rit->id);
        }
        else
        {
            int32_t delta = rit->op == REPUTATION_OP_ADD ? rit->value : -(rit->value);
            player->ModifyReputation(rit->id, delta);
        }
	}
}

void OpUtil::ModifyPlayerUI(Player* player, const ModifyUI& ui)
{
	UIOpVec::const_iterator uit = ui.ui_list.begin();
	for (; uit != ui.ui_list.end(); ++uit)
	{
		__PRINTF("ModifyUI op=%d id=%d", uit->value, uit->id);
        player->ModifyUI(uit->id, uit->value == UI_OP_SHOW);
	}
}

void OpUtil::ModifyPlayerBuff(Player* player, const ModifyBuff& buff, bool save)
{
    BuffOpVec::const_iterator bit = buff.buff_list.begin();
    for (; bit != buff.buff_list.end(); ++bit)
    {
		__PRINTF("ModifyBuff op=%d id=%d", bit->value, bit->id);
        player->ModifyWorldBuff(bit->value, bit->id, save);
    }
}

void OpUtil::RecyclePlayerBuff(Player* player, const ModifyBuff& buff)
{
    BuffOpVec::const_iterator bit = buff.buff_list.begin();
    for (; bit != buff.buff_list.end(); ++bit)
    {
        if (bit->value != BUFF_OP_ADD)
        {
            continue;
        }
		__PRINTF("RecycleBuff id=%d", bit->id);
        player->ModifyWorldBuff(BUFF_OP_DEL, bit->id, false);
    }
}

void OpUtil::ModifyPlayerTalent(Player* player, const ModifyTalent& talent)
{
    for (size_t i = 0; i < talent.talent_list.size(); ++i)
    {
		__PRINTF("OpenTalent id=%d", talent.talent_list[i]);
        player->OpenTalent(talent.talent_list[i]);
    }
}

void OpUtil::ModifyPlayerTitle(Player* player, const ModifyTitle& title)
{
    for (size_t i = 0; i < title.title_list.size(); ++i)
    {
		__PRINTF("OpenTitle id=%d", title.title_list[i]);
        player->OpenTitle(title.title_list[i]);
    }
}

void OpUtil::SubscribeCounter(Player* player, const TaskTempl* task, bool subscribe)
{
    const CounterCond* cond = NULL;
    for (size_t i = 0; i < 2; ++i)
    {
        cond = i == 0 ? &(task->succ_cond.global_counter) : &(task->fail_cond.global_counter);
        CounterVec::const_iterator cit = cond->counter_list.begin();
        for (; cit != cond->counter_list.end(); ++cit)
        {
            player->SubscribeGlobalCounter(cit->id, subscribe);
        }

        cond = i == 0 ? &(task->succ_cond.map_counter) : &(task->fail_cond.map_counter);
        cit = cond->counter_list.begin();
        for (; cit != cond->counter_list.end(); ++cit)
        {
            player->SubscribeMapCounter(cit->world_id, cit->id, subscribe);
        }
    }
}

static void Assign(gamed::GameObjInfo& linfo, const task::GameObjInfo& rinfo)
{
    linfo.id = rinfo.id;
    linfo.dir = rinfo.dir;
    linfo.pos.world_id = rinfo.pos.world_id;
    linfo.pos.x = rinfo.pos.x;
    linfo.pos.y = rinfo.pos.y;
    linfo.layout = rinfo.layout;
    linfo.appear_type = rinfo.appear;
    linfo.name = rinfo.name;
}

static gamed::NPCInfo Convert(const task::NPCInfo& info)
{
    gamed::NPCInfo tmp;
    Assign(tmp, info);
    tmp.action = info.action;
    return tmp;
}

static gamed::MonsterInfo Convert(const task::MonsterInfo& info)
{
    gamed::MonsterInfo tmp;
    Assign(tmp, info);
    tmp.monster_group = info.group;
    tmp.scene_id = info.scene;
    tmp.active = info.active;
    tmp.action = info.action;
    return tmp;
}

static gamed::MineInfo Convert(const task::MineInfo& info)
{
    gamed::MineInfo tmp;
    Assign(tmp, info);
    return tmp;
}

// 客户端操作
void OpUtil::Summon(TaskID taskid, Player* player, const SummonNPC& npc)
{
    int32_t world_id = 0;
	double x = 0, y = 0, cur_x = 0, cur_y = 0;
	int32_t cur_world_id = player->GetPos(cur_x, cur_y);

	const NPCVec& npc_list = npc.npc_list;
	NPCVec::const_iterator it = npc_list.begin();
	for (; it != npc_list.end(); ++it)
	{
		__PRINTF("SummonNPC taskid=%d, id=%d", taskid, it->id);
		if (it->absolute)
		{
            world_id = it->pos.world_id;
			x = it->pos.x;
			y = it->pos.y;
		}
		else
		{
            world_id = it->pos.world_id == 0 ? cur_world_id : it->pos.world_id;
			x = cur_x + it->pos.x;
			y = cur_y + it->pos.y;
		}
        NPCInfo tmp = *it;
        tmp.pos.world_id = world_id;
        tmp.pos.x = x;
        tmp.pos.y = y;
		player->SummonNPC(taskid, Convert(tmp));
	}
}

void OpUtil::Summon(TaskID taskid, Player* player, const SummonMonster& monster, bool revive)
{
	double x = 0, y = 0, cur_x = 0, cur_y = 0;
	player->GetPos(cur_x, cur_y);

	const MonsterVec& monster_list = monster.monster_list;
	MonsterVec::const_iterator it = monster_list.begin();
	for (; it != monster_list.end(); ++it)
	{
		__PRINTF("SummonMonster taskid=%d, id=%d", taskid, it->id);
		if (it->absolute)
		{
			x = it->pos.x;
			y = it->pos.y;
		}
		else
		{
			x = cur_x + it->pos.x;
			y = cur_y + it->pos.y;
		}
        MonsterInfo tmp = *it;
        tmp.pos.x = x;
        tmp.pos.y = y;
        player->SummonMonster(taskid, Convert(tmp), revive);
	}
}

void OpUtil::Summon(TaskID taskid, Player* player, const SummonMine& mine)
{
	double x = 0, y = 0, cur_x = 0, cur_y = 0;
	player->GetPos(cur_x, cur_y);

	const MineVec& mine_list = mine.mine_list;
	MineVec::const_iterator it = mine_list.begin();
	for (; it != mine_list.end(); ++it)
	{
		__PRINTF("SummonMine taskid=%d, id=%d", taskid, it->id);
		if (it->absolute)
		{
			x = it->pos.x;
			y = it->pos.y;
		}
		else
		{
			x = cur_x + it->pos.x;
			y = cur_y + it->pos.y;
		}
        MineInfo tmp = *it;
        tmp.pos.x = x;
        tmp.pos.y = y;
        player->SummonMine(taskid, Convert(tmp));
	}
}

void OpUtil::Recycle(TaskID taskid, Player* player, const SummonNPC& npc)
{
	const NPCVec& npc_list = npc.npc_list;
	NPCVec::const_iterator it = npc_list.begin();
	for (; it != npc_list.end(); ++it)
	{
		__PRINTF("RecycleNPC taskid=%d, id=%d", taskid, it->id);
		player->RecycleSummonObj(taskid, it->id);
	}
}

void OpUtil::Recycle(TaskID taskid, Player* player, const SummonMonster& monster)
{
	const MonsterVec& monster_list = monster.monster_list;
	MonsterVec::const_iterator it = monster_list.begin();
	for (; it != monster_list.end(); ++it)
	{
		__PRINTF("OpUtil::RecycleMonster taskid=%d, id=%d", taskid, it->id);
		player->RecycleSummonObj(taskid, it->id);
	}
}

void OpUtil::Recycle(TaskID taskid, Player* player, const SummonMine& mine)
{
	const MineVec& mine_list = mine.mine_list;
	MineVec::const_iterator it = mine_list.begin();
	for (; it != mine_list.end(); ++it)
	{
		__PRINTF("OpUtil::RecycleMine taskid=%d, id=%d", taskid, it->id);
		player->RecycleSummonObj(taskid, it->id);
	}
}

void OpUtil::Transport(TaskID taskid, Player* player, const Trans& trans)
{
	int32_t pos_num = trans.pos_list.size();
	if (pos_num == 0)
	{
		return;
	}

	int32_t pos_index = trans.trans_type == TRANS_NORMAL ? 0 : Util::Rand(0, pos_num);
	const Position& pos = trans.pos_list[pos_index];
	double x = 0, y = 0;
	int32_t world_id = player->GetPos(x, y);
	//if (pos.IsEqual(world_id, (float)x, (float)y))
    if (Util::IsEqual(pos, world_id, (float)x, (float)y))
	{
		return;
	}

	__PRINTF("Transport task=%d world=%d x=%f y=%f", taskid, pos.world_id, pos.x, pos.y);
	player->TransportTo(taskid, pos.world_id, pos.x, pos.y);
}

void OpUtil::ShowItemUseHint(TaskID taskid, Player* player, const ItemUseHint& hint)
{
    for (size_t i = 0; i < hint.zone_list.size(); ++i)
    {
        const ItemUseZone& info = hint.zone_list[i];
        if (player->GetItemCount(info.item_id) == 0)
        {
            continue;
        }
        int32_t world_id = 0;
        double x = 0, y = 0;
        world_id = player->GetPos(x, y);
        //if (info.Match(world_id, x, y))
        if (Util::IsInZone(info.zone, world_id, x, y))
        {
            return player->ShowItemUseHint(taskid, info.item_id, info.tip);
        }
    }
}

void OpUtil::HideItemUseHint(TaskID taskid, Player* player, const ItemUseHint& hint)
{
    for (size_t i = 0; i < hint.zone_list.size(); ++i)
    {
        player->HideItemUseHint(taskid, hint.zone_list[i].item_id);
    }
}

void OpUtil::DoCameraMask(TaskID taskid, Player* player, const CameraMask& mask)
{
    if (mask.op == CAMERA_MASK_NONE)
    {
        return;
    }
    player->DoCameraMaskGfx(mask.gfx_path, mask.op == CAMERA_MASK_OPEN);
}

void OpUtil::UndoCameraMask(TaskID taskid, Player* player, const CameraMask& mask)
{
    if (mask.op == CAMERA_MASK_NONE)
    {
        return;
    }
    player->DoCameraMaskGfx(mask.gfx_path, !(mask.op == CAMERA_MASK_OPEN));
}

void OpUtil::TransportToScriptArea(Player* player, const TaskTempl* task)
{
    if (task->op.exclusive_op != EXOP_SCRIPT || player->IsInMapArea(task->track.id))
    {
        return;
    }
    const PathInfo* info = task->path_finder.GetPathInfo(PATH_COMPLETE);
    player->TransportTo(task->id, info->pos.world_id, info->pos.x, info->pos.y);
}

void OpUtil::ExecuteOp(Player* player, const TaskTempl* task)
{
	ExecuteRepeatOp(player, task, false);
	ExecuteOnceOp(player, task);
}

void OpUtil::ExecuteRepeatOp(Player* player, const TaskTempl* task, bool finish, bool revive, bool script_end)
{
    if ((task->op.scale & SCALE_EXE))
    {
        player->ChangeScale(task->op.scale & 0x0F);
    }

    if (!revive)
    {
        Summon(task->id, player, task->op.npc);
        Summon(task->id, player, task->op.mine);
        ModifyPlayerBuff(player, task->op.buff, false);
        DoCameraMask(task->id, player, task->op.camera_mask);
        SubscribeCounter(player, task, !finish);
    }

	if (finish)
	{
		return;
	}

    if (!revive && task->op.exclusive_op == EXOP_SCRIPT && player->IsInMapArea(task->track.id))
    {
		Summon(task->id, player, task->op.monster, revive);
        if (!script_end)
        {
		    player->ExeScript(task->id);
        }
    }
    else if (!revive && task->op.exclusive_op == EXOP_GUIDE)
    {
		Summon(task->id, player, task->op.monster, revive);
		player->ExeGuide(task->id);
    }
    else if (!revive && task->op.exclusive_op == EXOP_TRANS)
    {
		Transport(task->id, player, task->op.trans);
    }
    else if (!revive && task->op.exclusive_op == EXOP_INS)
    {
		player->TransferIns(task->id, task->op.ins_id);
    }
    else if (!revive && task->op.exclusive_op == EXOP_BG)
    {
		player->TransferBG(task->id, task->op.bg_id);
    }
    else if (task->op.exclusive_op == EXOP_NONE)
    {
		Summon(task->id, player, task->op.monster, revive);
        // 对于特殊类型的任务需要显示对话框
        int32_t dlg_type = task->succ_cond.dlg_type;
        if (task->succ_cond.goal == GOAL_UI && dlg_type > DLG_NONE)
        {
            player->PopupDlg(task->id, dlg_type);
        }
        else if (task->succ_cond.goal == GOAL_MINIGAME && dlg_type != 0)
        {
            player->MiniGameStart(task->id, dlg_type);
        }
    }
}

void OpUtil::ExecuteOnceOp(Player* player, const TaskTempl* task)
{
	CtrlElement(player, task->op.ctrl_element);
	DeliverItemToPlayer(player, task->op.deliver_item);
	RecycleItemFromPlayer(player, task->op.recycle_item);
	ModifyCounterInfo(player, task->op.counter);
	SendChat(player, task->op.chat);
	ModifyNPCFriend(player, task->op.modify_friend);
}

void OpUtil::ExecuteAward(Player* player, const TaskTempl* task, bool succ)
{
	int64_t roleid = player->GetId();
	__PRINTF("OpUtil::ExecuteAward taskid=%d roleid=%ld", task->id, roleid);
	const TaskAward& award = succ ? task->succ_award : task->fail_award;
	int32_t level = player->GetLevel();
	int32_t value = s_pTask->GetRatioValue(RATIO_GOLD, level, award.gold.value, award.gold.ratio);
	if (value > 0)
	{
		__PRINTF("DeliverAwardGold roleid=%ld gold=%d", roleid, value);
		player->DeliverGold(value);
	}

	value = s_pTask->GetRatioValue(RATIO_EXP, level, award.exp.value, award.exp.ratio);
	if (value > 0)
	{
		__PRINTF("DeliverAward roleid=%ld exp=%d", roleid, value);
		player->DeliverExp(value);
	}

	value = s_pTask->GetRatioValue(RATIO_SCORE, level, award.score.value, award.score.ratio);
	if (value > 0)
	{
		__PRINTF("DeliverAward roleid=%ld score=%d", roleid, value);
		player->DeliverScore(value);
	}

	if (award.skill_point != 0)
	{
		__PRINTF("DeliverAward roleid=%ld skill=%d", roleid, award.skill_point);
		player->DeliverSkillPoint(award.skill_point);
	}
	if (award.transfer_cls != CLS_NONE)
	{
		__PRINTF("OpUtil::DeliverAward roleid=%ld cls=%d", roleid, award.transfer_cls);
		player->TransferClass(award.transfer_cls);
	}

	DeliverItemToPlayer(player, award.deliver_item);
	RecycleItemFromPlayer(player, award.recycle_item);
	ModifyBWList(player, award.modify_bw);
	ModifyCounterInfo(player, award.counter);
	SendMail(player, award.mail);
	SendChat(player, award.chat);
	CtrlElement(player, award.ctrl_element);
    ModifyPlayerReputation(player, award.modify_reputation);
    ModifyPlayerUI(player, award.modify_ui);
    ModifyPlayerBuff(player, award.modify_buff, true);
    ModifyPlayerTalent(player, award.modify_talent);
    ModifyPlayerTitle(player, award.modify_title);
	ModifyNPCFriend(player, award.modify_friend);

	if (award.deliver_task != 0)
	{
		__PRINTF("OpUtil::DeliverAward roleid=%ld dtask=%d", roleid, award.deliver_task);
		player->DeliverTask(award.deliver_task);
	}
	if (award.succ_task != 0)
	{
		__PRINTF("OpUtil::DeliverAward roleid=%ld stask=%d", roleid, award.succ_task);
		player->FinishPlayerTask(award.succ_task, true);
	}
	if (award.fail_task != 0)
	{
		__PRINTF("OpUtil::DeliverAward roleid=%ld ftask=%d", roleid, award.fail_task);
		player->FinishPlayerTask(award.fail_task, false);
	}
    TaskInfoVec::const_iterator it = award.dec_finish_time.begin();
    for (; it != award.dec_finish_time.end(); ++it)
    {
        DecFinishTime(player, it->id, it->value);
    }
	if (award.succ_ins != 0)
	{
		__PRINTF("OpUtil::DeliverAward roleid=%ld succ_ins=%d", roleid, award.succ_ins);
		player->FinishPlayerIns(award.succ_ins, true);
	}
	if (award.fail_ins != 0)
	{
		__PRINTF("OpUtil::DeliverAward roleid=%ld fail_ins=%d", roleid, award.fail_ins);
		player->FinishPlayerIns(award.fail_ins, false);
	}
    if (award.open_star != 0)
    {
		__PRINTF("OpUtil::DeliverAward roleid=%ld open_star=%d", roleid, award.open_star);
		player->OpenStar(award.open_star);
    }
    if (award.gevent_gid != 0)
    {
        __PRINTF("OpUtil::DeliverAward roleid=%ld gevent_gid=%d", roleid, award.gevent_gid);
        player->CompleteGevent(award.gevent_gid);
    }
    if (award.mount_equip_index != -1)
    {
        __PRINTF("OpUtil::DeliverAward roleid=%ld mount_equip_index=%d", roleid, award.mount_equip_index);
        player->OpenMountEquip(award.mount_equip_index);
    }
    if (award.battle_ground_id != 0)
    {
        __PRINTF("OpUtil::DeliverAward roleid=%ld battle_group_id=%d", roleid, award.battle_ground_id);
        player->ExitBattleGround(award.battle_ground_id);
    }
}

} // namespace task
