#include "player_instance.h"

#include "gamed/client_proto/G2C_error.h"
#include "gamed/game_module/task/include/task.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/instance_group.h"
#include "gs/global/dbgprt.h"
#include "gs/global/timer.h"
#include "gs/global/time_slot.h"
#include "gs/global/msg_pack_def.h"
#include "gs/global/gmatrix.h"
#include "gs/global/drop_table.h"
#include "gs/global/global_data.h"
#include "gs/scene/world.h"
#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"
#include "gs/player/task_if.h"

// proto
#include "common/protocol/gen/global/player_change_map_data.pb.h"
#include "common/protocol/gen/M2G/instance_msg.pb.h"
#include "common/protocol/gen/G2M/instance_msg.pb.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;


namespace {

#define INS_TYPE_TEAM dataTempl::InstanceTempl::IT_TEAM
#define INS_TYPE_SOLO dataTempl::InstanceTempl::IT_SOLO
#define INS_TYPE_UNION dataTempl::InstanceTempl::IT_UNION

	static const int64_t kHistoryMaxRemainMsecs = 7*24*3600*1000; // 单位是毫秒

	bool FormatCopyFromDB(PlayerInstance::InsDataInfo& ins_info, const common::PlayerInstanceData::Detail& data)
	{
		// find ins template
		const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(data.ins_templ_id);
		if (ptempl == NULL)
		{
			ins_info.clear();
			return false;
		}

		ins_info.info.ins_serial_num  = data.ins_serial_num;
		ins_info.info.ins_templ_id    = data.ins_templ_id;
		ins_info.info.world_id        = data.world_id;
		ins_info.info.world_tag       = data.world_tag;
		ins_info.info.ins_create_time = data.ins_create_time;
		ins_info.is_consumed          = data.is_consumed_cond;
		ins_info.self_pos.x           = data.ins_pos_x;
		ins_info.self_pos.y           = data.ins_pos_y;
		ins_info.ins_type             = ptempl->ins_type;
		return true;
	}

	bool FormatCopyToDB(const PlayerInstance::InsDataInfo& ins_info, common::PlayerInstanceData::Detail* pdata)
	{
		pdata->ins_serial_num   = ins_info.info.ins_serial_num;
		pdata->ins_templ_id     = ins_info.info.ins_templ_id;
		pdata->world_id         = ins_info.info.world_id;
		pdata->world_tag        = ins_info.info.world_tag;
		pdata->ins_create_time  = ins_info.info.ins_create_time;
		pdata->is_consumed_cond = ins_info.is_consumed;
		pdata->ins_pos_x        = ins_info.self_pos.x;
		pdata->ins_pos_y        = ins_info.self_pos.y;
		return true;
	}

	inline bool CheckInsCreateTime(int64_t now, int64_t ins_create_time)
	{
		int64_t timespan = now - ins_create_time;
		if (timespan >= 0 && timespan < kHistoryMaxRemainMsecs)
			return true;

		return false;
	}

	void BuildPCrossInsInfo(const Player& player, global::PCrossInsInfo* info)
	{
		info->set_gender(player.gender());
		info->set_level(player.level());
		info->set_weapon_id(player.GetWeaponID());
		info->set_combat_value(player.CalcCombatValue());
		info->set_role_class(player.role_class());
		info->set_first_name(player.first_name());
		info->set_mid_name(player.middle_name());
		info->set_last_name(player.last_name());
	}

	void BuildG2CItemEntry(const DropTable::ItemEntry& data, G2C::ItemEntry& ent)
	{
		ent.item_id    = data.item_id;
		ent.item_count = data.item_count;
	}

    void BuildPCrossInsMembers(const std::vector<PlayerInstance::CIRTeammateData>& members, G2M::EnterInsRequest& request)
    {
        for (size_t i = 0; i < members.size(); ++i)
        {
            G2M::CIRTeammateInfo* info = request.add_members();
            info->set_roleid(members[i].roleid);
            info->set_src_world_id(members[i].src_world_id);
            info->set_src_pos_x(members[i].src_pos_x);
            info->set_src_pos_y(members[i].src_pos_y);
            info->mutable_detail()->CopyFrom(members[i].detail);
        }
    }

} // Anonymous 


const int PlayerInstance::kInsRequestTimeout    = 10; // 单位秒
const int PlayerInstance::kTeamCIRMaxRecordTime = 31; // 单位秒, TeamCrossInsRequest
const int PlayerInstance::kTeamCIRMinRecordTime = 2;  // 单位秒
const int PlayerInstance::kQueryInsInfoCooldown = 10; // 单位秒


///
/// PlayerInstance
///
PlayerInstance::PlayerInstance(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_INSTANCE, player),
	  runtime_insinfo_(NULL)
{
	ResetCurInsInfo();
	runtime_insinfo_ = new global::InstanceInfo();
	ASSERT(runtime_insinfo_);

	SAVE_LOAD_REGISTER(common::PlayerInstanceData, PlayerInstance::SaveToDB, PlayerInstance::LoadFromDB);
}

PlayerInstance::~PlayerInstance()
{
}

bool PlayerInstance::SaveToDB(common::PlayerInstanceData* pdata)
{
	ASSERT(CheckInsTypeCount());

    // 如果当前地图就是所在的副本地图才保存cur项
	if (cur_ins_.info.has_info() && 
        cur_ins_.info.world_id == player_.world_id())
	{
		InsDataInfo tmp_ins  = cur_ins_;
		int32_t tmp_world_id = cur_ins_.info.world_id;
		if (tmp_world_id == player_.world_id() &&
			IS_INS_MAP(tmp_world_id))
		{
			// 如果在跨服服务器，存盘时对world_id进行调整
			tmp_ins.info.world_id = Gmatrix::GetRealWorldID(tmp_world_id);
		}
		FormatCopyToDB(tmp_ins, &pdata->cur_ins);

        // clean history by cur_ins
        CleanUpHistoryList(cur_ins_);
	}
    else if (cur_ins_.info.has_info())
    {
        CopyToHistoryList(cur_ins_);
    }

	InsDataInfoMap::iterator it = history_ins_.begin();
	for (; it != history_ins_.end(); ++it)
	{
		common::PlayerInstanceData::Detail ent;
		if (FormatCopyToDB(it->second, &ent))
		{
			pdata->history_ins.push_back(ent);
		}
	}

	return true;
}

bool PlayerInstance::LoadFromDB(const common::PlayerInstanceData& data)
{
	int64_t now = g_timer->GetSysTimeMsecs();

	if (CheckInsCreateTime(now, data.cur_ins.ins_create_time))
	{
		if (FormatCopyFromDB(cur_ins_, data.cur_ins))
		{
			if (IS_CR_INS(cur_ins_.info.world_id))
			{
				cur_ins_.info.world_id = WID_TO_MAPID(cur_ins_.info.world_id);
			}
		}
		else
		{
			cur_ins_.clear();
		}
	}

	for (size_t i = 0; i < data.history_ins.size(); ++i)
	{
		if (CheckInsCreateTime(now, data.history_ins[i].ins_create_time))
		{
			InsDataInfo info;
			if (FormatCopyFromDB(info, data.history_ins[i]))
			{
				if (IS_CR_INS(info.info.world_id))
				{
					info.info.world_id = WID_TO_MAPID(info.info.world_id);
				}
				history_ins_[info.info.ins_templ_id] = info;
			}
		}
	}

    // clean history by cur_ins
    if (cur_ins_.info.has_info())
    {
        CleanUpHistoryList(cur_ins_);
    }

    // must be right
	ASSERT(CheckInsTypeCount());
	return true;
}

void PlayerInstance::OnRelease()
{
	ResetCurInsInfo();
	history_ins_.clear();
	cur_request_ins_.clear();
	team_localins_info_.clear();
	team_crossins_info_.clear();

	DELETE_SET_NULL(runtime_insinfo_);
}

void PlayerInstance::OnHeartbeat(time_t cur_time)
{
	if (cur_request_ins_.is_countdown())
	{
		if (--cur_request_ins_.time_out <= 0)
		{
			cur_request_ins_.clear();
		}
	}

    if (team_crossins_info_.is_countdown())
    {
        if (--team_crossins_info_.time_out <= 0)
        {
            HandleTeamCrossInsRequest();
        }
    }

    if (!query_cooldown_map_.empty())
    {
        QueryCoolDownMap::iterator it = query_cooldown_map_.begin();
        while (it != query_cooldown_map_.end())
        {
            if (--it->second <= 0) {
                query_cooldown_map_.erase(it++);
            }
            else {
                ++it;
            }
        }
    }
}

void PlayerInstance::ResetCurInsInfo()
{
	cur_ins_.clear();
}

bool PlayerInstance::CheckInsTypeCount() const
{
	int team_info_count = 0;
	int solo_info_count = 0;
	if (cur_ins_.info.has_info())
	{
		if (cur_ins_.ins_type == INS_TYPE_TEAM)
		{
			++team_info_count;
		}
		if (cur_ins_.ins_type == INS_TYPE_SOLO)
		{
			++solo_info_count;
		}
	}

	InsDataInfoMap::const_iterator it = history_ins_.begin();
	for (; it != history_ins_.end(); ++it)
	{
		if (it->second.ins_type == INS_TYPE_TEAM)
		{
			++team_info_count;
		}
		if (it->second.ins_type == INS_TYPE_SOLO)
		{
			++solo_info_count;
		}
	}

	if (team_info_count != 0 && team_info_count != 1)
		return false;
	if (solo_info_count != 0 && solo_info_count != 1)
		return false;

	return true;
}

bool PlayerInstance::CheckCondIsConsumed(int32_t ins_templ_id) const
{
	// 是否已经检查
	if (cur_ins_.info.has_info() &&
		cur_ins_.info.ins_templ_id == ins_templ_id &&
		cur_ins_.is_consumed)
	{
		return true;
	}

	InsDataInfoMap::const_iterator it = history_ins_.find(ins_templ_id);	
	if (it != history_ins_.end())
	{
		if (it->second.info.has_info() &&
			it->second.info.ins_templ_id == ins_templ_id &&
			it->second.is_consumed)
		{
			return true;
		}
	}

	return false;
}

bool PlayerInstance::CheckEnterCondition(const dataTempl::InstanceTempl* pdata, bool check_team) const
{
	// 检查身份
	if (pdata->ins_type == INS_TYPE_TEAM)
	{
		if (check_team && !player_.IsInTeam())
			return false;
	}
	if (pdata->ins_type == INS_TYPE_UNION)
	{
		// ??????????????????
		return false;
	}

	// 是否已经检查
	if (CheckCondIsConsumed(pdata->templ_id))
	{
		return true;
	}

	// 检查物品
	if (pdata->entrance_require_item > 0)
	{
		if (!player_.CheckItem(pdata->entrance_require_item, pdata->entrance_item_count))
		{
			return false;
		}
	}

	// 检查金钱
	if (pdata->entrance_require_money > 0)
	{
		if (!player_.CheckMoney(pdata->entrance_require_money))
		{
			return false;
		}
	}

	// 检查元宝
	if (pdata->entrance_require_cash > 0)
	{
		if (!player_.CheckCash(pdata->entrance_require_cash))
		{
			return false;
		}
	}

	// 检查声望
	if (pdata->entrance_reputation_id > 0)
	{
		// ?????????????????
		return false;
	}

	// 检查等级
	if (pdata->level_lower_limit > 0)
	{
		if (player_.level() < pdata->level_lower_limit)
		{
			return false;
		}
	}
	if (pdata->level_upper_limit > 0)
	{
		if (player_.level() > pdata->level_upper_limit)
		{
			return false;
		}
	}

	// 检查时间段
	if (pdata->time_seg.is_valid)
	{
		TimeSlot time_slot;
		BuildTimeSlot(time_slot, pdata->time_seg);
		if (!time_slot.IsMatch(g_timer->GetSysTime()))
		{
			return false;
		}
	}

	// 检查进入所需携带的任务
	if (pdata->entrance_require_task > 0)
	{
		if (!player_.HasActiveTask(pdata->entrance_require_task))
			return false;
	}

	// 检查进入所需的已完成任务
	if (pdata->entrance_req_finish_task > 0)
	{
		if (!player_.HasFinishTask(pdata->entrance_req_finish_task))
			return false;
	}

	return true;
}

// needed check condition outside
void PlayerInstance::ConsumeEnterCond(const dataTempl::InstanceTempl* pdata)
{
    // 检查物品
    if (pdata->entrance_item_proc == dataTempl::InstanceTempl::IPT_TAKE_OUT)
    {
        if (pdata->entrance_require_item > 0)
        {
            ASSERT(player_.TakeOutItem(pdata->entrance_require_item, pdata->entrance_item_count));
        }
    }

    // 检查声望
    if (pdata->entrance_reputation_id > 0)
    {
        // ?????????????????
        ASSERT(false);
    }

    // 检查金钱
    if (pdata->entrance_require_money > 0)
    {
        player_.SpendMoney(pdata->entrance_require_money);
    }

    // 检查元宝
    if (pdata->entrance_require_cash > 0)
    {
        player_.UseCash(pdata->entrance_require_cash);
    }
}

bool PlayerInstance::CheckCreateCondition(const dataTempl::InstanceTempl* pdata) const
{
	// 检查身份
	if (pdata->ins_type == INS_TYPE_TEAM)
	{
		if (!player_.IsTeamLeader())
			return false;
	}
	if (pdata->ins_type == INS_TYPE_UNION)
	{
		// ??????????????????
		return false;
	}

	// 是否已经检查
	if (CheckCondIsConsumed(pdata->templ_id))
	{
		return true;
	}

	// 检查物品
	if (pdata->create_require_item > 0)
	{
		int create_item_count = pdata->create_item_count;
		if (pdata->create_require_item == pdata->entrance_require_item)
		{
			create_item_count += pdata->entrance_item_count;
		}
		if (!player_.CheckItem(pdata->create_require_item, create_item_count))
		{
			return false;
		}
	}

	// 检查金钱
	if (pdata->require_create_money > 0)
	{
		int require_create_money = pdata->require_create_money;
		if (pdata->entrance_require_money > 0)
		{
			require_create_money += pdata->entrance_require_money;
		}
		if (!player_.CheckMoney(require_create_money))
		{
			return false;
		}
	}

	// 检查元宝
	if (pdata->require_create_cash > 0)
	{
		int require_create_cash = pdata->require_create_cash;
		if (pdata->entrance_require_cash > 0)
		{
			require_create_cash += pdata->entrance_require_cash;
		}
		if (!player_.CheckCash(require_create_cash))
		{
			return false;
		}
	}

	// 检查声望
	if (pdata->create_reputation_id > 0)
	{
		int create_reputation_id = pdata->create_reputation_id;
		if (pdata->create_reputation_id == pdata->entrance_reputation_id)
		{
			create_reputation_id += pdata->entrance_reputation_id;
		}
		// ?????????????????????
		return false;
	}

	return true;
}

// needed check condition outside
void PlayerInstance::ConsumeCreateCond(const dataTempl::InstanceTempl* pdata)
{
	if (pdata->create_item_proc == dataTempl::InstanceTempl::IPT_TAKE_OUT)
	{
		// 检查物品
		if (pdata->create_require_item > 0)
		{
			ASSERT(player_.TakeOutItem(pdata->create_require_item, pdata->create_item_count));
		}

		// 检查声望
		if (pdata->create_reputation_id > 0)
		{
			// ?????????????????????
			ASSERT(false);
		}

		// 检查金钱
		if (pdata->require_create_money > 0)
		{
			player_.SpendMoney(pdata->require_create_money);
		}

		// 检查元宝
		if (pdata->require_create_cash > 0)
		{
			player_.UseCash(pdata->require_create_cash);
		}
	}
}

bool PlayerInstance::GetRuntimeInsInfo(MapID target_world_id, common::protocol::global::InstanceInfo& insinfo) const
{
	if (target_world_id == runtime_insinfo_->world_id())
	{
		insinfo.CopyFrom(*runtime_insinfo_);
		return true;
	}
    LOG_ERROR << "获取副本runtime信息失败！target_wid: " << target_world_id 
        << " runtime_insinfo wid: " << runtime_insinfo_->world_id();
	return false;
}

void PlayerInstance::SetInstanceInfo(const world::instance_info& info)
{
	ASSERT(IS_INS_MAP(info.world_id));
	if (cur_ins_.info != info)
	{
		// save old cur_ins
		if (cur_ins_.info.has_info())
		{
			CopyToHistoryList(cur_ins_);
		}

		// find ins template
		const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(info.ins_templ_id);
		ASSERT(ptempl != NULL);
		cur_ins_.info        = info;
		cur_ins_.is_consumed = false;
		cur_ins_.ins_type    = ptempl->ins_type;
        cur_ins_.self_pos.x  = ptempl->entrance_pos.x;
        cur_ins_.self_pos.y  = ptempl->entrance_pos.y;

		// clean history by new cur_ins
		CleanUpHistoryList(cur_ins_);
	}
}

void PlayerInstance::CopyToHistoryList(const InsDataInfo& oldins)
{
	if (oldins.ins_type == INS_TYPE_TEAM)
	{
		if (ReplaceTeamInsInfo(oldins))
			return;
	}

	if (oldins.ins_type == INS_TYPE_SOLO)
	{
		if (ReplaceSoloInsInfo(oldins))
			return;
	}

	history_ins_[oldins.info.ins_templ_id] = oldins;
}

void PlayerInstance::CleanUpHistoryList(const InsDataInfo& new_curins)
{
    if (!new_curins.info.has_info())
    {
        LOG_ERROR << "为什么会CleanUp没有信息的副本？";
        return;
    }

	int count = 0;
	if (new_curins.ins_type == INS_TYPE_SOLO ||
		new_curins.ins_type == INS_TYPE_TEAM)
	{
		InsDataInfoMap::iterator it = history_ins_.begin();
		while (it != history_ins_.end())
		{
			if (it->second.ins_type == new_curins.ins_type)
			{
				++count;
				history_ins_.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}
	ASSERT(count == 0 || count == 1);
}

void PlayerInstance::CleanUpSpecInsInfo(int32_t ins_tid)
{
	if (cur_ins_.info.ins_templ_id == ins_tid)
	{
		ResetCurInsInfo();
	}

	history_ins_.erase(ins_tid);
}

bool PlayerInstance::ReplaceTeamInsInfo(const InsDataInfo& insinfo)
{
	if (insinfo.ins_type != INS_TYPE_TEAM)
		return false;

	int count = 0;
	InsDataInfoMap::iterator it = history_ins_.begin();
	for (; it != history_ins_.end(); ++it)
	{
		if (it->second.ins_type == INS_TYPE_TEAM)
		{
			++count;
			it->second = insinfo;
		}
	}
	ASSERT(count == 0 || count == 1);

	if (count == 0)
		return false;

	return true;
}
	
bool PlayerInstance::ReplaceSoloInsInfo(const InsDataInfo& insinfo)
{
	if (insinfo.ins_type != INS_TYPE_SOLO)
		return false;

	int count = 0;
	InsDataInfoMap::iterator it = history_ins_.begin();
	for (; it != history_ins_.end(); ++it)
	{
		if (it->second.ins_type == INS_TYPE_SOLO)
		{
			++count;
			it->second = insinfo;
		}
	}
	ASSERT(count == 0 || count == 1);

	if (count == 0)
		return false;

	return true;
}

void PlayerInstance::ResetInstanceInfo()
{
	ResetCurInsInfo();
}

bool PlayerInstance::GetInstancePos(A2DVECTOR& pos) const
{
	if (cur_ins_.info.has_info())
	{
		pos = cur_ins_.self_pos;
		return true;
	}
	return false;
}

bool PlayerInstance::QueryInsEntrancePos(A2DVECTOR& pos) const
{
	if (cur_ins_.info.has_info())
	{
        pos = cur_ins_.self_pos;
		return true;
	}
	return false;
}

void PlayerInstance::TaskTransferSoloIns(int32_t ins_tid) const
{
    SysEnterSoloIns(ins_tid, G2M::IRT_TASK_SOLO);
}

void PlayerInstance::GeventTransferSoloIns(int32_t ins_tid) const
{
    SysEnterSoloIns(ins_tid, G2M::IRT_UI_SOLO);
}

void PlayerInstance::SysEnterSoloIns(int32_t ins_tid, int32_t type) const
{
    if (!CanEnterInstance())
		return;

	if (cur_request_ins_.has_request())
	{
		__PRINTF("收到SysEnterSoloIns，但是玩家已经发出副本请求! type:%d", type);
		return;
	}

	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_tid);
	if (ptempl == NULL || ptempl->ins_type != INS_TYPE_SOLO)
	{
		LOG_WARN << "玩家" << player_.role_id() << "副本居然没找到对应模板？tid：" << ins_tid
			<< " 或者不是单人副本";
		return;
	}

	msg_ins_transfer_prepare ins_param;
	ins_param.ins_templ_id = ins_tid;
	ins_param.request_type = type;
	player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));
}

void PlayerInstance::CalcInstanceRecord(int32_t ins_tid, int32_t clear_time, bool is_svr_record, int32_t last_record)
{
	if (clear_time <= 0)
	{
		LOG_ERROR << "CalcInstanceRecord()调用错误！";
		return;
	}

	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_tid);
	if (ptempl == NULL)
	{
		LOG_ERROR << "CalcInstanceRecord()的ins_templ_id=" << ins_tid << "居然没找到对应模板？";
		return;
	}

	if (player_.world_id() != ptempl->ins_map_id)
	{
		LOG_ERROR << "副本胜利结算，模板配的地图和player所处地图不一致？";
		return;
	}

	G2C::InstanceEnd packet;
	packet.ins_templ_id    = ins_tid;
	packet.clear_time      = clear_time;
	packet.result          = G2C::InstanceEnd::IR_VICTORY;
    packet.last_svr_record = 0; // 0表示无效

	// 服务器记录
	if (is_svr_record)
	{
		ASSERT(ptempl->gold_record_time > 0 && clear_time < ptempl->gold_record_time);
		packet.award_class     = G2C::InstanceEnd::AC_SVR_RECORD;
        packet.last_svr_record = last_record;
		DeliverInsAward(ptempl->srv_record_award, packet);
		player_.sender()->SendCmd(packet);
		return;
	}

	// 金牌奖励
	if (ptempl->gold_record_time > 0 && clear_time <= ptempl->gold_record_time)
	{
		packet.award_class = G2C::InstanceEnd::AC_GOLD;
		DeliverInsAward(ptempl->gold_award, packet);
		player_.sender()->SendCmd(packet);
		return;
	}

	// 银牌奖励
	if (ptempl->silver_record_time > 0 && clear_time <= ptempl->silver_record_time)
	{
		packet.award_class = G2C::InstanceEnd::AC_SILVER;
		DeliverInsAward(ptempl->silver_award, packet);
		player_.sender()->SendCmd(packet);
		return;
	}

	// 铜牌奖励
	if (ptempl->bronze_record_time > 0 && clear_time <= ptempl->bronze_record_time)
	{
		packet.award_class = G2C::InstanceEnd::AC_BRONZE;
		DeliverInsAward(ptempl->bronze_award, packet);
		player_.sender()->SendCmd(packet);
		return;
	}

	// 普通奖励
	packet.award_class = G2C::InstanceEnd::AC_NORMAL;
    DeliverInsAward(ptempl->normal_award, packet);
	player_.sender()->SendCmd(packet);
}

void PlayerInstance::FinishInstance(int32_t ins_id, bool succ) const
{
	world::instance_info ins_info;
	if (player_.world_plane()->GetInsInfo(ins_info))
	{
		if (ins_info.ins_templ_id != ins_id)
		{
			__PRINTF("结束副本但副本tid不一致，ins_id:%d ins_templ_id:%d", 
					ins_id, ins_info.ins_templ_id);
			return;
		}
	}
	else
	{
		LOG_WARN << "结束副本错误，玩家不在副本里！";
		return;
	}

	plane_msg_sys_close_ins param;
	param.sys_type = plane_msg_sys_close_ins::ST_TASK;
	if (succ)
	{
		param.ins_result = plane_msg_sys_close_ins::PLAYER_VICTORY;
	}
	else
	{
		param.ins_result = plane_msg_sys_close_ins::PLAYER_FAILURE;
	}
	player_.SendPlaneMsg(GS_PLANE_MSG_SYS_CLOSE_INS, &param, sizeof(param));
}

void PlayerInstance::DeliverInsAward(const InstanceTempl::InsCompleteAward& award, G2C::InstanceEnd& packet)
{
	// 发经验
	if (award.exp > 0)
	{
		player_.IncExp(award.exp);
		packet.exp_gain = award.exp;
	}

	// 发游戏币
	if (award.money > 0)
	{
		player_.GainMoney(award.money);
		packet.money_gain = award.money;
	}

	std::vector<DropTable::ItemEntry> item_vec;
	// 普通掉落包
	if (award.normal_droptable > 0)
	{
		if (DropTable::DropItem(award.normal_droptable, item_vec))
		{
			for (size_t i = 0; i < item_vec.size(); ++i)
			{
				DropTable::ItemEntry& data = item_vec[i];
				if (player_.GainItem(data.item_id, data.item_count, 0, GIM_INS_AWARD))
				{
					G2C::ItemEntry ent;
					BuildG2CItemEntry(data, ent);
					packet.items_drop.push_back(ent);
				}
			}
		}
	}

	item_vec.clear();
	// 特殊掉落，抽奖掉落
	if (award.special_droptable > 0)
	{
		if (DropTable::DropItem(award.special_droptable, item_vec))
		{
			for (size_t i = 0; i < item_vec.size(); ++i)
			{
				DropTable::ItemEntry& data = item_vec[i];
				if (player_.GainItem(data.item_id, data.item_count, 0, GIM_INS_AWARD))
				{
					G2C::ItemEntry ent;
					BuildG2CItemEntry(data, ent);
					packet.items_lottery.push_back(ent);
				}
			}
		}
	}
}

void PlayerInstance::OnEnterWorld()
{
	if (!IS_INS_MAP(player_.world_id()) && cur_ins_.info.has_info())
	{
		CopyToHistoryList(cur_ins_);
		ResetCurInsInfo();
	}
}
	
void PlayerInstance::OnLeaveWorld()
{
	if (IS_INS_MAP(player_.world_id()))
	{
        // 如果不是玩家主动退出副本，则记录玩家在副本里的位置
        // 玩家主动退出副本会清空cur_ins_ , 见ins_player_imp.cpp
		if (cur_ins_.info.has_info())
        {
		    cur_ins_.self_pos = player_.pos();
        }
	}
}

void PlayerInstance::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetInstanceData, PlayerInstance::CMDHandler_GetInstanceData);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TeamLocalInsInvite, PlayerInstance::CMDHandler_TeamLocalInsInvite);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TeamCrossInsRequest, PlayerInstance::CMDHandler_TeamCrossInsRequest);
	REGISTER_NORMAL_CMD_HANDLER(C2G::LaunchTeamLocalIns, PlayerInstance::CMDHandler_LaunchTeamLocalIns);
	REGISTER_NORMAL_CMD_HANDLER(C2G::QuitTeamLocalInsRoom, PlayerInstance::CMDHandler_QuitTeamLocalInsRoom);
	REGISTER_NORMAL_CMD_HANDLER(C2G::QuitTeamCrossInsRoom, PlayerInstance::CMDHandler_QuitTeamCrossInsRoom);
	REGISTER_NORMAL_CMD_HANDLER(C2G::LaunchSoloLocalIns, PlayerInstance::CMDHandler_LaunchSoloLocalIns);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TeamLocalInsInvite_Re, PlayerInstance::CMDHandler_TeamLocalInsInviteRe);
	REGISTER_NORMAL_CMD_HANDLER(C2G::TeamCrossInsReady, PlayerInstance::CMDHandler_TeamCrossInsReady);
	REGISTER_NORMAL_CMD_HANDLER(C2G::PlayerQuitInstance, PlayerInstance::CMDHandler_PlayerQuitInstance);
    REGISTER_NORMAL_CMD_HANDLER(C2G::TeamCrossInsInvite_Re, PlayerInstance::CMDHandler_TeamCrossInsInviteRe);
    REGISTER_NORMAL_CMD_HANDLER(C2G::QueryInstanceRecord, PlayerInstance::CMDHandler_QueryInstanceRecord);
}

void PlayerInstance::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_INS_TRANSFER_PREPARE, PlayerInstance::MSGHandler_InsTransferPrepare);
	REGISTER_MSG_HANDLER(GS_MSG_ENTER_INS_REPLY, PlayerInstance::MSGHandler_EnterInsReply);
	REGISTER_MSG_HANDLER(GS_MSG_TEAMMEMBER_LOCALINS_REPLY, PlayerInstance::MSGHandler_TeammemberLocalInsReply);
	REGISTER_MSG_HANDLER(GS_MSG_QUIT_TEAM_LOCAL_INS, PlayerInstance::MSGHandler_QuitTeamLocalIns);
	REGISTER_MSG_HANDLER(GS_MSG_TEAM_LOCAL_INS_INVITE, PlayerInstance::MSGHandler_TeamLocalInsInvite);
    REGISTER_MSG_HANDLER(GS_MSG_TEAM_CROSS_INS_INVITE, PlayerInstance::MSGHandler_TeamCrossInsInvite);
    REGISTER_MSG_HANDLER(GS_MSG_TEAM_CROSS_INS_INVITE_RE, PlayerInstance::MSGHandler_TeamCrossInsInviteRe);
}

void PlayerInstance::CMDHandler_GetInstanceData(const C2G::GetInstanceData& packet)
{
	if (!IS_INS_MAP(player_.world_id()))
	{
		LOG_WARN << "玩家" << player_.role_id() << "不在副本，但获取副本信息！C2G::GetInstanceData";
		return;
	}

	if (!cur_ins_.info.has_info() || cur_ins_.info.world_id != player_.world_id())
	{
		LOG_ERROR << "GetInstanceData cur_ins_.info.world_id != player_.world_id()";
		return;
	}

	//
	// 发副本信息
	//
	player_.sender()->GetInstanceDataRe(cur_ins_.info.ins_templ_id, Gmatrix::IsCrossRealmServer());

	//
	// 发任务
	//
	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(cur_ins_.info.ins_templ_id);
	if (ptempl == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "副本居然没找到对应模板？tid：" << cur_ins_.info.ins_templ_id;
		return;
	}
	if (ptempl->ins_auto_tasks.size())
	{
		for (size_t i = 0; i < ptempl->ins_auto_tasks.size(); ++i)
		{
			int32_t taskid = ptempl->ins_auto_tasks[i];
			if (player_.DeliverTask(taskid) == 0)
			{
				__PRINTF("玩家%ld接到副本自动发放任务.......task_id:%d", player_.role_id(), taskid);
			}
		}
	}
}

int PlayerInstance::MSGHandler_InsTransferPrepare(const MSG& msg)
{
	if (!CanEnterInstance())
		return 0;

	if (cur_request_ins_.has_request())
		return 0;

	CHECK_CONTENT_PARAM(msg, msg_ins_transfer_prepare);
	const msg_ins_transfer_prepare& ins_param = *(msg_ins_transfer_prepare*)msg.content;

	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_param.ins_templ_id);
	if (ptempl == NULL)
	{
		LOG_ERROR << "InsTransferPrepare() 找不到对应的副本模板！ins_templ_id:" << ins_param.ins_templ_id;
		return -1;
	}
	ASSERT(ptempl != NULL);

	// 跨服请求不用检查是否在队伍里
	bool check_team = (ins_param.request_type == G2M::IRT_UI_CR_TEAM) ? false : true;
	if (!CheckEnterCondition(ptempl, check_team))
	{
		__PRINTF("玩家%ld不满足副本%d进入条件, 副本模板id=%d！", 
				player_.role_id(), ptempl->ins_map_id, ptempl->templ_id);
		player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_INS_ENTER_COND);
		return 0;
	}

	// instance type
	G2M::EnterInsRequest proto;
	if (ptempl->ins_type == INS_TYPE_SOLO)
	{
		proto.set_ins_type(G2M::IT_SOLO);
	}
	else if (ptempl->ins_type == INS_TYPE_TEAM)
	{
		proto.set_ins_type(G2M::IT_TEAM);
	}
	else if (ptempl->ins_type == INS_TYPE_UNION)
	{
		proto.set_ins_type(G2M::IT_UNION);
	}
	else
	{
		ASSERT(false);
	}

	// request type
	int32_t src_world_id = 0;
	float src_pos_x      = 0.f;
	float src_pos_y      = 0.f;
	switch (ins_param.request_type)
	{
		case G2M::IRT_TRANSFER:
			{
				src_world_id = ptempl->town_portal_map_id;
				src_pos_x    = ptempl->town_portal_pos.x;
				src_pos_y    = ptempl->town_portal_pos.y;
				proto.set_req_type((G2M::InsRequestType)ins_param.request_type);
			}
			break;

		case G2M::IRT_TASK_SOLO:
		case G2M::IRT_UI_SOLO:
		case G2M::IRT_UI_TEAM:
            {
                src_world_id = player_.world_id();
				src_pos_x    = player_.pos().x;
				src_pos_y    = player_.pos().y;
				proto.set_req_type((G2M::InsRequestType)ins_param.request_type);
            }
            break;

		case G2M::IRT_UI_CR_TEAM:
			{
				src_world_id = player_.world_id();
				src_pos_x    = player_.pos().x;
				src_pos_y    = player_.pos().y;
				proto.set_req_type((G2M::InsRequestType)ins_param.request_type);
                BuildPCrossInsMembers(team_crossins_info_.members, proto);
			}
			break;

		default:
			ASSERT(false);
			break;
	}

	// destination world id
	if (ins_param.request_type == G2M::IRT_UI_CR_TEAM)
	{
		int32_t tmp_id = Gmatrix::ConvertToCRWorldID(ptempl->ins_map_id);
		proto.set_des_world_id(tmp_id);
		BuildPCrossInsInfo(player_, proto.mutable_cross_info());
	}
	else
	{
		proto.set_des_world_id(ptempl->ins_map_id);
	}

	proto.set_roleid(player_.role_id());
	proto.set_ins_templ_id(ins_param.ins_templ_id);
	proto.set_src_world_id(src_world_id);
	proto.set_src_pos_x(src_pos_x);
	proto.set_src_pos_y(src_pos_y);
	if (ins_param.request_type == G2M::IRT_TRANSFER)
	{
		FillReenterInfo(ins_param.ins_templ_id, proto);
	}
	player_.sender()->SendToMaster(proto);

	// record request
	cur_request_ins_.ins_tid  = ins_param.ins_templ_id;
	cur_request_ins_.time_out = kInsRequestTimeout;
	return 0;
}

void PlayerInstance::FillReenterInfo(int32_t ins_templ_id, G2M::EnterInsRequest& proto)
{
	const InsDataInfo* ins_data = NULL;
	if (ins_templ_id == cur_ins_.info.ins_templ_id)
	{
		ins_data = &cur_ins_;
	}
	else
	{
		InsDataInfoMap::iterator it = history_ins_.find(ins_templ_id);
		if (it != history_ins_.end())
		{
			ins_data = &(it->second);
		}
	}

	if (ins_data != NULL)
	{
		proto.mutable_reenter()->set_ins_serial_num(ins_data->info.ins_serial_num);
		proto.mutable_reenter()->set_world_id(ins_data->info.world_id);
		proto.mutable_reenter()->set_world_tag(ins_data->info.world_tag);
		proto.mutable_reenter()->set_ins_create_time(ins_data->info.ins_create_time);
	}
}

int PlayerInstance::MSGHandler_EnterInsReply(const MSG& msg)
{
	cur_request_ins_.clear();
	runtime_insinfo_->Clear();

	if (!CanEnterInstance())
		return 0;

	CHECK_CONTENT_PARAM(msg, msg_enter_ins_reply);
	const msg_enter_ins_reply& param = *(msg_enter_ins_reply*)msg.content;

	// error 
	if (param.err_code != M2G::EnterInsReply::SUCCESS)
	{
		if (param.err_code == M2G::EnterInsReply::TEAM_INS_NOT_CREATE)
		{
			player_.sender()->ErrorMessage(G2C::ERR_TEAM_INS_NOT_CREATE);
		}

		if (param.err_code == M2G::EnterInsReply::INS_NOT_EXIST)
		{
			player_.sender()->ErrorMessage(G2C::ERR_INS_NOT_EXIST);
		}

		if (param.err_code == M2G::EnterInsReply::ALREADY_HAS_INS)
		{
			player_.sender()->ErrorMessage(G2C::ERR_ALREADY_HAS_INS);
		}

		CleanUpSpecInsInfo(param.ins_templ_id);
		return 0;
	}

	// find ins template
	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(param.ins_templ_id);
	if (ptempl == NULL)
	{
		LOG_ERROR << "EnterInsReply的ins_templ_id=" << param.ins_templ_id << "居然没找到对应模板？";
		ResetCurInsInfo();
		return -1;
	}

	// create instance 
	int32_t ins_world_id    = param.world_id;
	int32_t ins_world_tag   = param.world_tag;
	int64_t ins_create_time = param.ins_create_time;
	if (param.enter_type == common::protocol::global::ET_INS_CREATE)
	{
		ins_world_id    = ptempl->ins_map_id;
		ins_world_tag   = 0;
		ins_create_time = 0;
		if (!CheckCreateCondition(ptempl))
		{
			__PRINTF("玩家%ld创建副本条件不满足ins_tid:%d,world_id:%d", 
					player_.role_id(), param.ins_templ_id, ins_world_id);
			player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_INS_CREATE_COND);
			return 0;
		}
	}
		
	// enter condition
	bool check_team = (param.enter_type == global::ET_INS_ENTER_CREATE) ? false : true;
	if (!CheckEnterCondition(ptempl, check_team))
	{
		__PRINTF("玩家%ld进入副本条件不满足ins_tid:%d, world_id:%d", 
				player_.role_id(), param.ins_templ_id, ins_world_id);
		player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_INS_ENTER_COND);
		return 0;
	}

	// 做状态检查
	if (!player_.CanSwitch())
	{
		player_.sender()->ErrorMessage(G2C::ERR_CUR_STATE_CANNOT_TRANSPORT);
		return 0;
	}

	///
	/// success! start to transfer
	///
	runtime_insinfo_->set_enter_type((common::protocol::global::InsEnterType)param.enter_type);
	runtime_insinfo_->set_ins_serial_num(param.ins_serial_num);
	runtime_insinfo_->set_ins_templ_id(param.ins_templ_id);
	runtime_insinfo_->set_world_id(ins_world_id);
	runtime_insinfo_->set_world_tag(ins_world_tag);
	runtime_insinfo_->set_ins_create_time(ins_create_time);
	runtime_insinfo_->set_src_world_id(param.src_world_id);
	runtime_insinfo_->set_src_pos_x(param.src_pos_x);
	runtime_insinfo_->set_src_pos_y(param.src_pos_y);
	if (param.leader_id > 0)
	{
		runtime_insinfo_->mutable_team_info()->set_leader(param.leader_id);
		runtime_insinfo_->mutable_team_info()->set_self_pos(param.self_team_pos);
	}

	msg_player_region_transport msg_param;
	msg_param.source_world_id = player_.world_id();
	msg_param.target_world_id = runtime_insinfo_->world_id();
	msg_param.ins_templ_id    = runtime_insinfo_->ins_templ_id();
	msg_param.target_pos.x    = ptempl->entrance_pos.x;
	msg_param.target_pos.y    = ptempl->entrance_pos.y;
	player_.SendMsg(GS_MSG_INS_TRANSFER_START, player_.object_xid(), &msg_param, sizeof(msg_param));
	return 0;
}

bool PlayerInstance::CheckInstanceCond(const playerdef::InsInfoCond& info) const
{
	// find ins template
	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(info.ins_tid);
	if (ptempl == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "进入副本居然没找到对应模板？tid：" << info.ins_tid;
		return false;
	}

	bool check_team    = Gmatrix::IsCrossRealmServer() ? false : true;
	bool is_reenter    = (info.enter_type == playerdef::InsInfoCond::REENTER) ? true : false;
	bool is_create_ins = (info.enter_type == playerdef::InsInfoCond::CREATE) ? true : false;
	// 曾经进入过副本，则无需在检查
	if (is_reenter && CheckCondIsConsumed(info.ins_tid))
	{
		// 检查身份
		if (ptempl->ins_type == INS_TYPE_TEAM)
		{
			if (check_team && !player_.IsInTeam())
				return false;
		}
		if (ptempl->ins_type == INS_TYPE_UNION)
		{
			// ??????????????????
			return false;
		}

		return true;
	}

	// 检查进入条件
	if (!CheckEnterCondition(ptempl, check_team))
	{
		return false;
	}

	// 检查创建条件
	if (is_create_ins && !CheckCreateCondition(ptempl))
	{
		return false;
	}

	return true;
}

bool PlayerInstance::ConsumeInstanceCond(int32_t ins_tid, bool is_create_ins)
{
	ASSERT(ins_tid == cur_ins_.info.ins_templ_id);

	if (cur_ins_.is_consumed)
	{
		return true;
	}

	// find ins template
	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_tid);
	if (ptempl == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "副本居然没找到对应模板？tid：" << ins_tid;
		return false;
	}

	// 扣除进入所需
	ConsumeEnterCond(ptempl);

	// 扣除创建所需
	if (is_create_ins)
	{
		ConsumeCreateCond(ptempl);
	}
	
	cur_ins_.is_consumed = true;
	return true;
}

// 队长发给队员，不包含队长自己
void PlayerInstance::BroadcastAgreeTeammateLC(shared::net::ProtoPacket& packet, RoleID except_id) const
{
	if (!player_.IsTeamLeader())
	{
		LOG_WARN << "玩家不是队长，怎么会广播？";
		return;
	}

    // 发给所有同意的玩家，不包括队长自己
    for (size_t i = 0; i < team_localins_info_.members.size(); ++i)
    {
        if (except_id != 0 && 
            team_localins_info_.members[i].roleid == except_id)
        {
            continue;
        }

        player_.sender()->SendCmdByMaster(team_localins_info_.members[i].roleid, packet);
    }
}

// 队长发给队员，不包含队长自己
void PlayerInstance::BroadcastAgreeTeammateCR(shared::net::ProtoPacket& packet, RoleID except_id) const
{
	if (!player_.IsTeamLeader())
	{
		LOG_WARN << "玩家不是队长，怎么会广播？";
		return;
    }
    // 发给所有同意的玩家，不包括队长自己
    for (size_t i = 0; i < team_crossins_info_.members.size(); ++i)
    {
        if (except_id != 0 && 
            team_crossins_info_.members[i].roleid == except_id)
        {
            continue;
        }

        player_.sender()->SendCmdByMaster(team_crossins_info_.members[i].roleid, packet);
    }
}

void PlayerInstance::BroadcastToAllTeammate(shared::net::ProtoPacket& packet) const
{
	if (!player_.IsTeamLeader())
	{
		LOG_WARN << "玩家不是队长，怎么会广播？";
		return;
	}

	// 发给所有组队里的玩家，不包括队长自己
	std::vector<RoleID> members;
	if (player_.GetTeammates(members))
	{
		for (size_t i = 0; i < members.size(); ++i)
		{
			player_.sender()->SendCmdByMaster(members[i], packet);
		}
	}
}

bool PlayerInstance::CanEnterInstance() const
{
	// 不能从特殊地图跳到副本地图
	if (!IS_NORMAL_MAP(player_.world_id()))
		return false;

	return true;
}


///
/// UI方式进入本服副本
///
void PlayerInstance::CMDHandler_TeamLocalInsInvite(const C2G::TeamLocalInsInvite& cmd)
{
	// 是否能进入副本
	if (!CanEnterInstance())
	{
		__PRINTF("玩家在副本里，不能发起组队本服副本请求TeamLocalInsInvite");
		return;
	}

	if (cur_request_ins_.has_request())
	{
		__PRINTF("玩家还有没有结束的副本请求，不能发起要求");
		return;
	}

	// 检查是不是队长
	if (!player_.IsTeamLeader())
	{
		LOG_WARN << "玩家" << player_.role_id() << "不是队长，不能发起本服组队副本挑战";
		player_.sender()->ErrorMessage(G2C::ERR_IS_NOT_TEAM_LEADER);
		return;
	}
	
	// 查找副本组
	const InstanceGroup* pgroup = s_pDataTempl->QueryDataTempl<InstanceGroup>(cmd.ins_group_id);
	if (pgroup == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "副本组模板居然没有找到？group_id:" << cmd.ins_group_id;
		player_.sender()->ErrorMessage(G2C::ERR_INS_GROUP_NOT_FOUND);
		return;
	}

	// 检查是否有可进入的副本
	bool check_res = false;
	std::vector<int32_t> instid_vec;
	for (size_t i = 0; i < pgroup->ins_array.size(); ++i)
	{
		const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(pgroup->ins_array[i].ins_templ_id);
		if (ptempl != NULL &&
			ptempl->ins_type == INS_TYPE_TEAM &&
			CheckEnterCondition(ptempl))
		{
			check_res = true;
			instid_vec.push_back(ptempl->templ_id);
		}
	}

	// 发送给队友
	if (check_res)
	{
		G2M::TeamLocalInsInvite proto;
		proto.set_roleid(player_.role_id());
		proto.set_ins_group_id(cmd.ins_group_id);
		player_.sender()->SendToMaster(proto);

		// 先清空
		team_localins_info_.clear();
		// 保存队长信息
		team_localins_info_.leader.roleid      = player_.role_id();
		team_localins_info_.leader.ins_tid_vec = instid_vec;
		// 开始保存本服组队本信息
		team_localins_info_.ins_group_id = cmd.ins_group_id;
		team_localins_info_.group_templ  = pgroup;
	}
	else
	{
		__PRINTF("没有可进入的副本，不能发起组队进副本邀请. ins_group_id:%d", cmd.ins_group_id);
	}
}

bool PlayerInstance::CheckLocalInsReply(const msgpack_teammeber_localins_reply& param)
{
	// 检查各种情况
	if (player_.role_id() == param.member_roleid)
	{
		LOG_WARN << "自己收到TeammemberLocalInsReply rold_id:" << player_.role_id();
		return false;
	}
	if (!player_.IsTeammate(param.member_roleid))
	{
		__PRINTF("该玩家已经不是队友，收到teammember localins reply");
		return false;
	}
	if (param.ins_group_id != team_localins_info_.ins_group_id)
	{
		__PRINTF("teammember localins reply 和发邀请的ins_group_id不一致！");
		return false;
	}

	// 查找副本组
	const InstanceGroup* pgroup = team_localins_info_.group_templ;
	if (pgroup == NULL)
	{
		__PRINTF("CheckLocalInsReply()保存的pgroup是NULL");
		return false;
	}
	for (size_t i = 0; i < param.ins_tid_vec.size(); ++i)
	{
		bool found = false;
		for (size_t j = 0; j < pgroup->ins_array.size(); ++j)
		{
			if (param.ins_tid_vec[i] == pgroup->ins_array[j].ins_templ_id)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			__PRINTF("队员%ld本服组队副本reply的ins tid不对！", param.member_roleid);
			return false;
		}
	}

	return true;
}

int PlayerInstance::MSGHandler_TeammemberLocalInsReply(const MSG& msg)
{
	if (!CanEnterInstance())
		return 0;

	if (!player_.IsTeamLeader())
	{
		__PRINTF("玩家已经不是队长，收到teammember localins reply");
		return 0;
	}

	msgpack_teammeber_localins_reply param;
	MsgContentUnmarshal(msg, param);

	//
	// 检查各种情况
	//
	if (!CheckLocalInsReply(param))
	{
		__PRINTF("TeammemberLocalInsReply MSG 条件检查没有通过！");
		return 0;
	}
	
	//
	// success: 需要发送给客户端
	//
	G2C::TeammemberLocalInsReply packet;
	packet.ins_group_id  = param.ins_group_id;
	packet.agreement     = param.agreement;
	packet.member_roleid = param.member_roleid;
	packet.ins_tid_vec   = param.ins_tid_vec;
	player_.sender()->SendCmd(packet);

	//
	// 添加到确认的队员信息中
	//
	if (param.agreement)
	{
		TeamLocalInsInfo::Detail detail;
		detail.roleid      = param.member_roleid;
		detail.ins_tid_vec = param.ins_tid_vec;
        bool filled        = false;
        for (size_t i = 0; i < team_localins_info_.members.size(); ++i)
        {
            if (team_localins_info_.members[i].roleid == detail.roleid)
            {
                team_localins_info_.members[i] = detail;
                filled = true;
            }
        }
        if (!filled)
        {
            team_localins_info_.members.push_back(detail);
        }

		// 通知队友该队员同意挑战副本
		G2C::TeammemberAgreeLocalIns agree;
		agree.ins_group_id  = param.ins_group_id;
		agree.member_roleid = param.member_roleid;
		agree.ins_tid_vec   = param.ins_tid_vec;
		BroadcastAgreeTeammateLC(agree, param.member_roleid);

		// 其他队友的信息发给该队友
		for (size_t i = 0; i < team_localins_info_.members.size(); ++i)
		{
			RoleID roleid = team_localins_info_.members[i].roleid;
			if (roleid == param.member_roleid)
			{
				continue;
			}
			G2C::TeammemberAgreeLocalIns tmp_agree;
			tmp_agree.ins_group_id  = param.ins_group_id;
			tmp_agree.member_roleid = roleid;
			tmp_agree.ins_tid_vec   = team_localins_info_.members[i].ins_tid_vec;
			player_.sender()->SendCmdByMaster(param.member_roleid, tmp_agree);
		}

		// 把自己的信息发给该队友
		G2C::TeammemberAgreeLocalIns leader_agree;
		leader_agree.ins_group_id  = param.ins_group_id;
		leader_agree.member_roleid = team_localins_info_.leader.roleid;
		leader_agree.ins_tid_vec   = team_localins_info_.leader.ins_tid_vec;
		player_.sender()->SendCmdByMaster(param.member_roleid, leader_agree);
	}
	return 0;
}

void PlayerInstance::CMDHandler_LaunchTeamLocalIns(const C2G::LaunchTeamLocalIns& cmd)
{
	if (!CanEnterInstance())
	{
		__PRINTF("玩家在副本地图里不能发起组队本服副本LaunchTeamLocalIns");
		return; 
	}

	if (!player_.IsTeamLeader())
	{
		__PRINTF("玩家已经不是队长，收到LaunchTeamLocalIns");
		return;
	}

	if (cmd.ins_group_id != team_localins_info_.ins_group_id)
	{
		__PRINTF("LaunchTeamLocalIns所对应的ins_group_id不一致");
		return;
	}

	bool found = false;
	const InstanceGroup* pgroup = team_localins_info_.group_templ;
	if (pgroup == NULL)
	{
		__PRINTF("LaunchTeamLocalIns保存的pgroup是NULL");
		return;
	}
	for (size_t i = 0; i < pgroup->ins_array.size(); ++i)
	{
		if (cmd.ins_templ_id == pgroup->ins_array[i].ins_templ_id)
		{
			const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(pgroup->ins_array[i].ins_templ_id);
			if (ptempl != NULL && 
				ptempl->ins_type == INS_TYPE_TEAM &&
				CheckEnterCondition(ptempl))
			{
				found = true;
				break;
			}
		}
	}

	if (found)
	{
		for (size_t i = 0; i < team_localins_info_.members.size(); ++i)
		{
			std::vector<int32_t>& ins_vec     = team_localins_info_.members[i].ins_tid_vec;
			std::vector<int32_t>::iterator it = std::find(ins_vec.begin(), ins_vec.end(), cmd.ins_templ_id);
			if (it != ins_vec.end())
			{
				G2M::TeamLeaderStartLocalIns proto;
				proto.set_member_roleid(team_localins_info_.members[i].roleid);
				proto.set_ins_templ_id(cmd.ins_templ_id);
				player_.sender()->SendToMaster(proto);
			}
		}

		msg_ins_transfer_prepare ins_param;
		ins_param.ins_templ_id = cmd.ins_templ_id;
		ins_param.request_type = G2M::IRT_UI_TEAM;
		player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));
	}
	else
	{
		__PRINTF("LaunchTeamLocalIns ins_templ_id not found!");
		return;
	}
}

void PlayerInstance::CMDHandler_QuitTeamLocalInsRoom(const C2G::QuitTeamLocalInsRoom& cmd)
{
	if (!player_.IsInTeam())
	{
		__PRINTF("CMDHandler_QuitTeamLocalInsRoom已经不在队伍中");
		return;
	}

	if (player_.IsTeamLeader())
	{
		// 队长自己退出广播给其他队员
		G2C::TeammemberQuitLocalIns packet;
		packet.member_roleid = player_.role_id();
		packet.ins_group_id  = team_localins_info_.ins_group_id;
		BroadcastToAllTeammate(packet);

		// 清除保存的信息
		team_localins_info_.clear();
	}
	else
	{
		// 发送给队长
		G2M::TeammemberQuitLocalIns proto;
		proto.set_leader_roleid(player_.GetLeaderId());
		proto.set_member_roleid(player_.role_id());
		proto.set_ins_group_id(cmd.ins_group_id);
		player_.sender()->SendToMaster(proto);
	}

	cur_request_ins_.clear();
}

int PlayerInstance::MSGHandler_QuitTeamLocalIns(const MSG& msg)
{
	if (!player_.IsTeamLeader())
	{
		LOG_WARN << "已经不是队长，收到MSGHandler_QuitTeamLocalIns";
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_quit_team_local_ins);
	msg_quit_team_local_ins& param = *(msg_quit_team_local_ins*)msg.content;

	if (team_localins_info_.ins_group_id != param.ins_group_id)
	{
		LOG_WARN << "收到QuitTeamLocalIns MSG的ins_group_id已经不一致";
		return 0;
	}

	std::vector<TeamLocalInsInfo::Detail>::iterator it = team_localins_info_.members.begin();
	while (it != team_localins_info_.members.end())
	{
		if (param.member_roleid == (*it).roleid)
		{
			// 发协议
			G2C::TeammemberQuitLocalIns packet;
			packet.member_roleid = param.member_roleid;
			packet.ins_group_id  = param.ins_group_id;
			// 发给队长自己
			player_.sender()->SendCmd(packet);
			// 发给其他队员，但不发给退出的队员
			BroadcastAgreeTeammateLC(packet, param.member_roleid);

			// 清除
			it = team_localins_info_.members.erase(it);
		}
		else
		{
			++it;
		}
	}

	return 0;
}

int PlayerInstance::MSGHandler_TeamLocalInsInvite(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_team_local_ins_invite);
	const msg_team_local_ins_invite& param = *(msg_team_local_ins_invite*)msg.content;

	if (!player_.IsTeammate(param.leader_roleid))
	{
		__PRINTF("收到TeamLocalInsInvite，但是不是队友");
		return 0;
	}

	if (player_.GetLeaderId() == 0 ||
		player_.GetLeaderId() != param.leader_roleid)
	{
		__PRINTF("收到TeamLocalInsInvite，但队长不是同一个人");
		return 0;
	}

	if (cur_request_ins_.has_request())
	{
		__PRINTF("收到TeamLocalInsInvite，但已经发起副本request");
		return 0;
	}

	G2C::TeamLocalInsInvite packet;
	packet.ins_group_id = param.ins_group_id;
	player_.sender()->SendCmd(packet);
	return 0;
}

void PlayerInstance::CMDHandler_TeamLocalInsInviteRe(const C2G::TeamLocalInsInvite_Re& cmd)
{
	if (!player_.IsInTeam())
	{
		return;
	}

	G2M::TeamLocalInsInviteRe proto;
	proto.set_leader_roleid(player_.GetLeaderId());
	proto.set_member_roleid(player_.role_id());
	proto.set_agreement(cmd.agreement);
	proto.set_ins_group_id(cmd.ins_group_id);
	for (size_t i = 0; i < cmd.ins_tid_vec.size(); ++i)
	{
		proto.add_ins_tid_array(cmd.ins_tid_vec[i]);
	}
	player_.sender()->SendToMaster(proto);
}

void PlayerInstance::CMDHandler_LaunchSoloLocalIns(const C2G::LaunchSoloLocalIns& cmd)
{
	if (!CanEnterInstance())
	{
		__PRINTF("玩家副本里，不能发起单人副本LaunchSoloLocalIns");
		return;
	}

	// 查找副本组
	const InstanceGroup* pgroup = s_pDataTempl->QueryDataTempl<InstanceGroup>(cmd.ins_group_id);
	if (pgroup == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "副本组模板居然没有找到？group_id:" << cmd.ins_group_id;
		player_.sender()->ErrorMessage(G2C::ERR_INS_GROUP_NOT_FOUND);
		return;
	}

	// 检查是否是可进入的副本
	bool check_res = false;
	for (size_t i = 0; i < pgroup->ins_array.size(); ++i)
	{
		if (cmd.ins_templ_id == pgroup->ins_array[i].ins_templ_id)
		{
			const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(cmd.ins_templ_id);
			if (ptempl != NULL && 
				ptempl->ins_type == InstanceTempl::IT_SOLO &&
				CheckEnterCondition(ptempl))
			{
				check_res = true;
				break;
			}
		}
	}

	if (check_res)
	{
		// success
		msg_ins_transfer_prepare ins_param;
		ins_param.ins_templ_id = cmd.ins_templ_id;
		ins_param.request_type = G2M::IRT_UI_SOLO;
		player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));
	}
	else
	{
		// failure
		player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_INS_ENTER_COND);
	}
}


///
/// UI方式进入跨服副本
///
void PlayerInstance::HandleTeamCrossInsRequest()
{
    if (team_crossins_info_.has_info())
    {
        msg_ins_transfer_prepare ins_param;
        ins_param.ins_templ_id = team_crossins_info_.ins_templ_id;
        ins_param.request_type = G2M::IRT_UI_CR_TEAM;
        player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));

        G2C::TeamCrossInsRequestComplete packet;
        if (team_crossins_info_.members.size() > 0)
        {
            // 发给已同意的队友
            BroadcastAgreeTeammateCR(packet);
        }

        // 发给自己
        player_.sender()->SendCmd(packet);
    }
}

void PlayerInstance::CMDHandler_TeamCrossInsRequest(const C2G::TeamCrossInsRequest& cmd)
{
	if (!CanEnterInstance())
	{
		__PRINTF("玩家在副本里，不能发起跨服副本请求TeamCrossInsRequest");
		return;
	}

    // 当前状态下无法加入副本
    if (!player_.CanSwitch())
    {
        player_.sender()->ErrorMessage(G2C::ERR_CAN_NOT_JOIN_IN_INS);
        return;
    }

	if (cur_request_ins_.has_request())
	{
		__PRINTF("收到TeamCrossInsRequest，但是玩家已经发出副本请求");
		return;
	}

	// 查找副本组
	const InstanceGroup* pgroup = s_pDataTempl->QueryDataTempl<InstanceGroup>(cmd.ins_group_id);
	if (pgroup == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "副本组模板居然没有找到？group_id:" << cmd.ins_group_id;
		player_.sender()->ErrorMessage(G2C::ERR_INS_GROUP_NOT_FOUND);
		return;
	}

	// 检查是否有可进入的副本
	bool check_res = false;
	for (size_t i = 0; i < pgroup->ins_array.size(); ++i)
	{
		if (cmd.ins_templ_id == pgroup->ins_array[i].ins_templ_id)
		{
			check_res = true;
			break;
		}
	}

	// 请求发送给master 
	if (check_res)
	{
		const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(cmd.ins_templ_id);
		if (ptempl == NULL)
		{
			LOG_ERROR << "找不到对应的副本模板 ins_tid:" << cmd.ins_templ_id;
			return;
		}
		if (ptempl->ins_type != INS_TYPE_TEAM)
		{
			LOG_ERROR << "发起跨服组队副本请求，副本模板不是组队副本！ins_tid:" << cmd.ins_templ_id;
			return;
		}
        if (!cmd.solo_request && !player_.IsTeamLeader())
        {
            __PRINTF("玩家不是队长不能发起组队跨服副本请求");
            return;
        }

        // 邀请队友一起报名跨服副本
        if (!cmd.solo_request)
        {
            G2M::TeamCrossInsInvite proto;
            proto.mutable_info()->set_leader_roleid(player_.role_id());
            proto.mutable_info()->set_ins_group_id(cmd.ins_group_id);
            proto.mutable_info()->set_ins_templ_id(cmd.ins_templ_id);
            player_.sender()->SendToMaster(proto);
        }
        
		// 保存跨服副本信息
        team_crossins_info_.clear();
        team_crossins_info_.time_out     = cmd.solo_request ? kTeamCIRMinRecordTime : kTeamCIRMaxRecordTime;
		team_crossins_info_.ins_group_id = cmd.ins_group_id;
		team_crossins_info_.ins_templ_id = cmd.ins_templ_id;
	}
	else
	{
		__PRINTF("没有可进入的副本，不能请求进入跨服副本. group_id:%d templ_id:%d", 
				cmd.ins_group_id, cmd.ins_templ_id);
	}
}

void PlayerInstance::CMDHandler_TeamCrossInsReady(const C2G::TeamCrossInsReady& cmd)
{
	if (team_crossins_info_.ins_group_id == cmd.ins_group_id &&
		team_crossins_info_.ins_templ_id == cmd.ins_templ_id)
	{
		G2M::TeammemberCrossInsReady proto;
		proto.set_roleid(player_.role_id());
		player_.sender()->SendToMaster(proto);

        // 可以清空
        team_crossins_info_.clear();
	}
}

void PlayerInstance::CMDHandler_QuitTeamCrossInsRoom(const C2G::QuitTeamCrossInsRoom& cmd)
{
	if (team_crossins_info_.ins_group_id == cmd.ins_group_id)
	{
		G2M::TeammemberQuitCrossIns proto;
		proto.set_roleid(player_.role_id());
		player_.sender()->SendToMaster(proto);
	}

	cur_request_ins_.clear();
	team_crossins_info_.clear();
}

void PlayerInstance::CMDHandler_PlayerQuitInstance(const C2G::PlayerQuitInstance& cmd)
{
	if (IS_INS_MAP(player_.world_id()))
	{
		player_.SendMsg(GS_MSG_PLAYER_QUIT_INS, player_.object_xid());
	}
}

int PlayerInstance::MSGHandler_TeamCrossInsInvite(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, msg_team_cross_ins_invite);
	const msg_team_cross_ins_invite& param = *(msg_team_cross_ins_invite*)msg.content;

	if (!player_.IsTeammate(param.leader_roleid))
	{
		__PRINTF("收到TeamCrossInsInvite，但是不是队友");
		return 0;
	}

	if (player_.GetLeaderId() == 0 ||
		player_.GetLeaderId() != param.leader_roleid)
	{
		__PRINTF("收到TeamCrossInsInvite，但队长不是同一个人");
		return 0;
	}

	if (cur_request_ins_.has_request())
	{
		__PRINTF("收到TeamCrossInsInvite，但已经发起副本request");
		return 0;
	}

	G2C::TeamCrossInsInvite packet;
	packet.ins_group_id = param.ins_group_id;
    packet.ins_templ_id = param.ins_templ_id;
	player_.sender()->SendCmd(packet);
	return 0;
}

bool PlayerInstance::CheckCrossInsReply(RoleID member_roleid, int32_t ins_group_id, int32_t ins_templ_id)
{
    // 检查各种情况
    if (player_.role_id() == member_roleid)
    {
        LOG_WARN << "自己收到TeamCrossInsInviteReply role_id:" << player_.role_id();
        return false;
    }
    if (!player_.IsTeammate(member_roleid))
    {
        __PRINTF("该玩家已经不是队友，收到team crossins invite reply");
        return false;
    }
    if (team_crossins_info_.ins_group_id != ins_group_id ||
        team_crossins_info_.ins_templ_id != ins_templ_id)
    {
        __PRINTF("team crossins invite reply 和发邀请的ins_group_id或ins_templ_id不一致");
        return false;
    }

    return true;
}

int PlayerInstance::MSGHandler_TeamCrossInsInviteRe(const MSG& msg)
{
    if (!CanEnterInstance())
		return 0;

	if (!player_.IsTeamLeader())
	{
		__PRINTF("玩家已经不是队长，收到team crossins invite reply");
		return 0;
	}

    M2G::TeamCrossInsInviteReply proto;
    ASSERT(proto.ParseFromArray(msg.content, msg.content_len));
    const int64_t member_roleid = proto.info().member_roleid();
    const int32_t ins_group_id  = proto.info().ins_group_id();
    const int32_t ins_templ_id  = proto.info().ins_templ_id();

    //
    // 检查各种情况
    //
    if (!CheckCrossInsReply(member_roleid, ins_group_id, ins_templ_id))
    {
        __PRINTF("TeamCrossInsInviteRe MSG 条件检查没有通过！");
        return 0;
    }

    // 收到一个队员的回复
    team_crossins_info_.reply_count++;

    // 先发给队长自己
    G2C::TeamCrossInsInviteReply reply_packet;
    reply_packet.member_roleid = member_roleid;
    reply_packet.ins_group_id  = ins_group_id;
    reply_packet.ins_templ_id  = ins_templ_id;
    // 该队友不满足条件
    if (!proto.info().match_cond())
    {
        reply_packet.reply_type = G2C::TeamCrossInsInviteReply::RT_NOT_MATCH;
    }
    // 该队友同意
    if (proto.info().agreement())
    {
        reply_packet.reply_type = G2C::TeamCrossInsInviteReply::RT_AGREE;
    }
    else
    {
        reply_packet.reply_type = G2C::TeamCrossInsInviteReply::RT_DISAGREE;
    }
    // 不管是否成功，都需要发送给客户端
    player_.sender()->SendCmd(reply_packet);

    // 对内所有成员都已经回应
    if (team_crossins_info_.reply_count == player_.GetTeammateCount())
    {
        team_crossins_info_.time_out = kTeamCIRMinRecordTime;
    }

    //
    // failure
    //
    if (!proto.info().match_cond() || !proto.info().agreement())
    {
        return 0;
    }

    //
    // success: 添加到确认的队员信息中
    //
    CIRTeammateData teammate_data;
    teammate_data.roleid       = member_roleid;
    teammate_data.src_world_id = proto.info().src_world_id();
    teammate_data.src_pos_x    = proto.info().src_pos_x();
    teammate_data.src_pos_y    = proto.info().src_pos_y();
    teammate_data.detail.CopyFrom(proto.info().detail());
    bool filled = false;
    for (size_t i = 0; i < team_crossins_info_.members.size(); ++i)
    {
        if (team_crossins_info_.members[i].roleid == teammate_data.roleid)
        {
            team_crossins_info_.members[i] = teammate_data;
            filled = true;
        }
    }
    if (!filled)
    {
        team_crossins_info_.members.push_back(teammate_data);
    }

    // 通知队友该队员同意挑战副本
    G2C::TeammemberAgreeCrossIns agree;
    agree.member_roleid = member_roleid;
    agree.ins_group_id  = ins_group_id;
    agree.ins_templ_id  = ins_templ_id;
    BroadcastAgreeTeammateCR(agree, member_roleid);

    // 其他队友的信息发给该队友
    for (size_t i = 0; i < team_crossins_info_.members.size(); ++i)
    {
        RoleID roleid = team_crossins_info_.members[i].roleid;
        if (roleid == member_roleid)
        {
            continue;
        }
        G2C::TeammemberAgreeCrossIns tmp_agree;
        tmp_agree.member_roleid = member_roleid;
        tmp_agree.ins_group_id  = ins_group_id;
        tmp_agree.ins_templ_id  = ins_templ_id;
        player_.sender()->SendCmdByMaster(member_roleid, tmp_agree);
    }

    // 把自己的信息发给该队友
    G2C::TeammemberAgreeCrossIns leader_agree;
    leader_agree.member_roleid = player_.role_id();
    leader_agree.ins_group_id  = ins_group_id;
    leader_agree.ins_templ_id  = ins_templ_id;
    player_.sender()->SendCmdByMaster(member_roleid, leader_agree);
    return 0;
}

void PlayerInstance::CMDHandler_TeamCrossInsInviteRe(const C2G::TeamCrossInsInvite_Re& cmd)
{
    if (!player_.IsInTeam())
	{
		return;
	}

    const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(cmd.ins_templ_id);
	if (ptempl == NULL)
	{
		__PRINTF("TeamCrossInsInvite_Re找不到对应的副本模板！ins_tid:%d", cmd.ins_templ_id);
		return;
	}
	ASSERT(ptempl != NULL);

	G2M::TeamCrossInsInviteReply proto;
    if (CheckEnterCondition(ptempl, false))
    {
        proto.mutable_info()->set_match_cond(true);
    }
    else
    {
        proto.mutable_info()->set_match_cond(false);
        player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_INS_ENTER_COND);
    }

    // 当前状态下无法加入副本
    if (!CanEnterInstance() || !player_.CanSwitch())
    {
        proto.mutable_info()->set_match_cond(false);
        player_.sender()->ErrorMessage(G2C::ERR_CAN_NOT_JOIN_IN_INS);
    }

    ///
    /// success
    ///
    team_crossins_info_.ins_group_id = cmd.ins_group_id;
    team_crossins_info_.ins_templ_id = cmd.ins_templ_id;

    // 发给队长
	proto.mutable_info()->set_leader_roleid(player_.GetLeaderId());
	proto.mutable_info()->set_member_roleid(player_.role_id());
	proto.mutable_info()->set_agreement(cmd.agreement);
	proto.mutable_info()->set_ins_group_id(cmd.ins_group_id);
    proto.mutable_info()->set_ins_templ_id(cmd.ins_templ_id);
    proto.mutable_info()->set_src_world_id(player_.world_id());
    proto.mutable_info()->set_src_pos_x(player_.pos().x);
    proto.mutable_info()->set_src_pos_y(player_.pos().y);
    if (proto.info().match_cond() && cmd.agreement)
    {
        BuildPCrossInsInfo(player_, proto.mutable_info()->mutable_detail());
    }
	player_.sender()->SendToMaster(proto);
}

void PlayerInstance::CMDHandler_QueryInstanceRecord(const C2G::QueryInstanceRecord& cmd)
{
    const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(cmd.ins_tid);
	if (ptempl == NULL)
	{
		__PRINTF("C2G::QueryInstanceRecord找不到对应的副本模板！ins_tid:%d", cmd.ins_tid);
		return;
	}

    G2C::QueryInstanceRecord_Re packet;
    packet.ins_group_id = cmd.ins_group_id;
    packet.ins_tid      = cmd.ins_tid;
    packet.clear_time   = 0;

    QueryCoolDownMap::const_iterator it = query_cooldown_map_.find(cmd.ins_tid);
    if (it != query_cooldown_map_.end() && it->second > 0)
    {
        //__PRINTF("C2G::QueryInstanceRecord还在冷却中....ins_tid:%d", cmd.ins_tid);
        packet.clear_time = -1; // 告诉客户端还在冷却中
        player_.sender()->SendCmd(packet);
        return;
    }

    // 有副本记录的才进行查询
    if (ptempl->has_srv_record)
    {
        globalData::InstanceRecord::RecordValue svr_value;
        bool query_res = s_pGlobalData->Query<globalData::InstanceRecord>(player_.master_id(), cmd.ins_tid, svr_value);
        if (query_res)
        {
            packet.clear_time = svr_value.clear_time;
            for (size_t i = 0; i < svr_value.player_vec.size(); ++i)
            {
                const msgpack_ins_player_info& ins_pinfo = svr_value.player_vec[i];
                G2C::InsRecordPInfo info;
                info.role_id      = ins_pinfo.role_id;
                info.first_name   = ins_pinfo.first_name;
                info.mid_name     = ins_pinfo.mid_name;
                info.last_name    = ins_pinfo.last_name;
                info.level        = ins_pinfo.level;
                info.cls          = ins_pinfo.cls;
                info.combat_value = ins_pinfo.combat_value;
                packet.players.push_back(info);
            }
        }

        // 记录冷却
        query_cooldown_map_[cmd.ins_tid] = kQueryInsInfoCooldown;
    }

    player_.sender()->SendCmd(packet);
}

} // namespace gamed
