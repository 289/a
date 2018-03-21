#include "task_if.h"

#include "game_module/task/include/task_data.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/global/global_counter.h"
#include "gs/scene/world_cluster.h"
#include "gs/scene/world_man.h"

#include "player.h"
#include "player_sender.h"
#include "game_module/task/include/task.h"


namespace gamed {

using namespace mapDataSvr;

PlayerTaskIf::PlayerTaskIf(Player* player)
	: pplayer_(player)
{
}

PlayerTaskIf::~PlayerTaskIf()
{
}

int64_t PlayerTaskIf::GetId() const
{
	return pplayer_->role_id();
}

int8_t PlayerTaskIf::GetLevel() const
{
	return pplayer_->level();
}

int8_t PlayerTaskIf::GetGender() const
{
	return pplayer_->gender();
}

int32_t PlayerTaskIf::GetRoleClass() const
{
	return pplayer_->role_class();
}

int32_t PlayerTaskIf::GetItemCount(int32_t itemid) const
{
	return pplayer_->CountItem(itemid) + pplayer_->CountEquipItem(itemid);
}

int64_t PlayerTaskIf::GetGoldNum() const
{
	return pplayer_->GetMoney();
}

int32_t PlayerTaskIf::GetCash() const
{
	return pplayer_->GetCash();
}

int32_t PlayerTaskIf::GetScore() const
{
	return pplayer_->GetScore();
}

int32_t PlayerTaskIf::GetPos(double& x, double& y) const
{
	x = pplayer_->pos().x;
	y = pplayer_->pos().y;
	return pplayer_->world_id();
}

int32_t PlayerTaskIf::GetCurTime() const
{
	return g_timer->GetSysTime();
}

bool PlayerTaskIf::CanDeliverItem(int8_t where, int32_t empty_slot_count) const
{
	return pplayer_->HasSlot(where, empty_slot_count);
}

bool PlayerTaskIf::IsInMapArea(int32_t areaid)
{
    const AreaWithRules* parea = s_pMapData->QueryMapDataTempl<AreaWithRules>(areaid);
    if (parea == NULL)
        return false;
        
    if (parea->area_type != AreaWithRules::AT_MAP_AREA)
        return false;

    if (parea->map_id != pplayer_->world_id())
        return false;

    CGLib::Polygon polygon;
    for (size_t i = 0; i < parea->vertexes.size(); ++i)
	{
        CGLib::Point2d point(parea->vertexes[i].x, parea->vertexes[i].y);
        polygon.push_back(point);
	}
    if (!polygon.point_in_polygon(pplayer_->pos().x, pplayer_->pos().y))
        return false;

    return true;
}

void PlayerTaskIf::DeliverGold(int32_t num)
{
	if (num > 0)
	{
		pplayer_->GainMoney(num);
	}
}

void PlayerTaskIf::DeliverExp(int32_t exp)
{
	if (exp > 0)
	{
		pplayer_->IncExp(exp);
	}
}

void PlayerTaskIf::DeliverScore(int32_t score)
{
	pplayer_->GainScore(score);
}

void PlayerTaskIf::DeliverSkillPoint(int32_t point)
{
}

void PlayerTaskIf::DeliverTask(int32_t task_id)
{
	task::DeliverTask(this, task_id, false);
}

void PlayerTaskIf::FinishPlayerTask(int32_t task_id, bool succ)
{
	task::FinishPlayerTask(this, task_id, succ);
}

void PlayerTaskIf::DeliverItem(int32_t itemid, int32_t num, int32_t valid_time)
{
    if (itemid != 0 && num != 0)
    {
	    pplayer_->GainItem(itemid, num, valid_time, GIM_TASK);
    }
}

void PlayerTaskIf::TakeAwayGold(int32_t num)
{
	pplayer_->SpendMoney(num);
}

void PlayerTaskIf::TakeAwayCash(int32_t num)
{
    pplayer_->UseCash(num);
}

void PlayerTaskIf::TakeAwayScore(int32_t num)
{
	pplayer_->SpendScore(num);
}

void PlayerTaskIf::TakeAwayItem(int32_t itemid, int32_t num)
{
    if (itemid != 0 && num != 0)
    {
	    pplayer_->TakeOutItem(itemid, num);
    }
}

void PlayerTaskIf::TakeAwayAllItem(int32_t itemid, int32_t num)
{
    if (itemid != 0 && num != 0)
    {
	    pplayer_->TakeOutAllItem(itemid, num);
    }
}

void PlayerTaskIf::TransferClass(int8_t role_class)
{
	pplayer_->TransferCls(role_class);
}

task::ActiveTask* PlayerTaskIf::GetActiveTask()
{
	return pplayer_->GetActiveTask();
}

task::FinishTask* PlayerTaskIf::GetFinishTask()
{
	return pplayer_->GetFinishTask();
}

task::FinishTimeTask* PlayerTaskIf::GetFinishTimeTask()
{
	return pplayer_->GetFinishTimeTask();
}

task::TaskDiary* PlayerTaskIf::GetTaskDiary()
{
	return pplayer_->GetTaskDiary();
}

task::TaskStorage* PlayerTaskIf::GetTaskStorage()
{
	return pplayer_->GetTaskStorage();
}

void PlayerTaskIf::SendTaskNotify(uint16_t type, const void* buf, size_t size)
{
	pplayer_->sender()->TaskNotifyClient(type, buf, size);
}

void PlayerTaskIf::ShowActiveTask()
{
	std::string tmpstr;
	task::ActiveTask* tasks = GetActiveTask();
	task::EntryMap::const_iterator it = tasks->task_list.begin();
	for (; it != tasks->task_list.end(); ++it)
	{
		tmpstr += itos(it->first);
		tmpstr += ", ";
	}
	__PRINTF("........player:%ld Active task list: %s", pplayer_->role_id(), tmpstr.c_str());
}

void PlayerTaskIf::ShowFinishTask()
{
	std::string tmpstr;
	task::FinishTask* tasks = GetFinishTask();
	task::EntryMap::const_iterator it = tasks->task_list.begin();
	for (; it != tasks->task_list.end(); ++it)
	{
		tmpstr += itos(it->first);
		tmpstr += ", ";
	}
	__PRINTF("........player:%ld Finish task list: %s", pplayer_->role_id(), tmpstr.c_str());
}

void PlayerTaskIf::ShowFinishTimeTask()
{
	std::string tmpstr;
	task::FinishTimeTask* tasks = GetFinishTimeTask();
	task::EntryMap::const_iterator it = tasks->task_list.begin();
	for (; it != tasks->task_list.end(); ++it)
	{
		tmpstr += itos(it->first);
		tmpstr += ", ";
	}
	__PRINTF("........player:%ld FinishTime task list: %s", pplayer_->role_id(), tmpstr.c_str());
}

void PlayerTaskIf::ModifyNPCBWList(int32_t templ_id, bool black, bool add)
{
	pplayer_->ModifyNPCBWList(templ_id, black, add);
}

void PlayerTaskIf::ExeScript(int32_t task_id)  
{ 
	pplayer_->SetRuningDramaScript(true);
}

void PlayerTaskIf::ScriptEnd(int32_t task_id, bool skip)
{
	pplayer_->SetRuningDramaScript(false);
}

void PlayerTaskIf::ExeGuide(int32_t task_id)  
{ 
	//pplayer_->SetRuningDramaScript(true);
}

void PlayerTaskIf::GuideEnd(int32_t task_id)
{
	//pplayer_->SetRuningDramaScript(false);
}

void PlayerTaskIf::ModifyUI(int32_t ui_id, bool show)
{
    pplayer_->ModifyUIConf(ui_id, show);
}

void PlayerTaskIf::CtrlMapElement(int32_t element_id, bool open)
{
	pplayer_->CtrlMapElement(element_id, open);
}

int32_t PlayerTaskIf::GetInsFinishCount(int32_t ins_id) const
{
    return pplayer_->GetInsFinishCount(ins_id);
}

bool PlayerTaskIf::GetGlobalCounter(int32_t counter_id, int32_t& value) const
{
    return pplayer_->GetGlobalCounter(counter_id, value);
}

bool PlayerTaskIf::GetMapCounter(int32_t id, int32_t world_id, int32_t& value) const
{
    if (pplayer_->world_id() != world_id)
    {
        LOG_ERROR << "PlayerTaskIf::GetMapCounter() 玩家不在对应的地图！world_id:" << world_id;
        return false;
    }

    WorldManager* wman = wcluster::FindWorldManager(pplayer_->world_id(), pplayer_->world_tag());
    if (wman != NULL && wman->IsActived())
    {
        value = wman->GetMapCounter(id);
        return true;
    }

    return false;
}

bool PlayerTaskIf::GetPlayerCounter(int32_t counter_id, int32_t& value) const
{
    return pplayer_->GetPCounter(counter_id, value);
}

void PlayerTaskIf::ModifyGlobalCounter(int32_t counter_id, int8_t op, int32_t delta)
{
    pplayer_->ModifyGlobalCounter(counter_id, op, delta);
}

void PlayerTaskIf::ModifyPlayerCounter(int32_t id, int8_t op, int32_t value)
{
    pplayer_->ModifyPCounter(id, op, value);
}

void PlayerTaskIf::ModifyMapCounter(int32_t world_id, int32_t id, int8_t op, int32_t value)
{
}

void PlayerTaskIf::SendSysMail(shared::net::ByteBuffer& data)
{
	pplayer_->SendSysMail(data);
}

void PlayerTaskIf::EnterInstance(int32_t ins_id)
{
	pplayer_->TaskTransferSoloIns(ins_id);
}

void PlayerTaskIf::FinishPlayerIns(int32_t ins_id, bool succ)
{
	// ins_id 副本模板id
	pplayer_->FinishInstance(ins_id, succ);
}

bool PlayerTaskIf::MiniGameEnd(int32_t game_type)
{
	// game_type 小游戏模板id
	return pplayer_->TaskMiniGameEnd(game_type);
}

void PlayerTaskIf::SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content)
{
	pplayer_->SendSysChat(channel, sender_name, content);
}

void PlayerTaskIf::ServerTransport(int32_t world_id, double x, double y)
{
	if (pplayer_->world_id() != world_id)
	{
		__PRINTF("玩家通过跳过任务脚本进行传送，但不是本地图，无法进行传送！");
		return;
	}

	msg_player_region_transport param;
	param.source_world_id = pplayer_->world_id();
	param.target_world_id = world_id;
	param.target_pos.x    = x;
	param.target_pos.y    = y;
	pplayer_->SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, pplayer_->object_xid(), &param, sizeof(param));
}

int32_t PlayerTaskIf::GetReputation(int32_t reputation_id) const
{
    return pplayer_->GetReputation(reputation_id);
}

int32_t PlayerTaskIf::GetCombatValue() const
{
	return pplayer_->CalcCombatValue();
}

int32_t PlayerTaskIf::GetPetPower() const
{
    int32_t power = 0, power_cap = 0, speed = 0;
    pplayer_->QueryPetPowerInfo(power, power_cap, speed);
    return power_cap;
}

int32_t PlayerTaskIf::GetPetNum(int32_t level, int32_t blevel, int32_t rank) const
{
    return pplayer_->GetCombatPetNum(level, blevel, rank - 1);
}

int32_t PlayerTaskIf::GetCardNum(int32_t level, int32_t rank) const
{
    return pplayer_->GetCardNum(level, rank);
}

int32_t PlayerTaskIf::GetEnhanceNum(int32_t level, int32_t rank) const
{
    return pplayer_->GetEnhanceNum(level, rank);
}

int32_t PlayerTaskIf::GetEquipNum(int32_t refine_level, int32_t rank) const
{
    return pplayer_->GetEquipNum(refine_level, rank);
}

int32_t PlayerTaskIf::GetFullActivateStarNum() const
{
    return pplayer_->GetFullActivateStarNum();
}

int32_t PlayerTaskIf::GetCatVisionLevel() const
{
    return pplayer_->GetCatVisionLevel();
}

int32_t PlayerTaskIf::GetTalentLevel(int32_t talent_gid, int32_t talent_id) const
{
    return pplayer_->GetTalentLevel(talent_gid, talent_id);
}

bool PlayerTaskIf::IsWorldBuffExist(int32_t buff_id) const
{
    return buff_id <= 0 ? false : pplayer_->IsFilterExistByID(buff_id);
}

void PlayerTaskIf::OpenReputation(int32_t reputation_id)
{
    pplayer_->OpenReputation(reputation_id);
}

void PlayerTaskIf::ModifyReputation(int32_t reputation_id, int32_t delta)
{
    pplayer_->ModifyReputation(reputation_id, delta);
}

void PlayerTaskIf::ModifyWorldBuff(int8_t op, int32_t buff_id, bool save)
{
    bool is_add = (op == task::BUFF_OP_ADD) ? true : false;
    bool reject_save = save ? false : true;
    pplayer_->ModifyFilterByID(buff_id, is_add, reject_save);
}

void PlayerTaskIf::StorageTaskComplete(int32_t taskid, int8_t quality)
{
    pplayer_->StorageTaskComplete(taskid, quality);
}

void PlayerTaskIf::OpenTalent(int32_t talent_gid)
{
    pplayer_->OpenTalentGroup(talent_gid);
}

void PlayerTaskIf::OpenTitle(int32_t title_id)
{
    pplayer_->GainTitle(title_id);
}

void PlayerTaskIf::SubscribeGlobalCounter(int32_t counter_id, bool is_subscribe)
{
    pplayer_->SubscribeGlobalCounter(counter_id, is_subscribe);
}

void PlayerTaskIf::SubscribeMapCounter(int32_t world_id, int32_t id, bool is_subscribe)
{
    pplayer_->SubscribeMapCounter(world_id, id, is_subscribe);
}

void PlayerTaskIf::AddNPCFriend(int32_t id)
{
    pplayer_->AddNPCFriend(id);
}

void PlayerTaskIf::DelNPCFriend(int32_t id)
{
    pplayer_->DelNPCFriend(id);
}

void PlayerTaskIf::OnlineNPCFriend(int32_t id)
{
    pplayer_->OnlineNPCFriend(id);
}

void PlayerTaskIf::OfflineNPCFriend(int32_t id)
{
    pplayer_->OfflineNPCFriend(id);
}

void PlayerTaskIf::OpenStar(int32_t id)
{
    pplayer_->OpenStar(id);
}

void PlayerTaskIf::CompleteGevent(int32_t id)
{
    pplayer_->CompleteGevent(id);
}

void PlayerTaskIf::OpenMountEquip(int32_t index)
{
    pplayer_->OpenMountEquip(index);
}

void PlayerTaskIf::ExitBattleGround(int32_t bg_tid)
{
    pplayer_->QuitBattleGround(bg_tid);
}

} // namespace gamed

