#include "player_battleground.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/battleground_templ.h"
#include "gs/global/timer.h"
#include "gs/global/time_slot.h"
#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/global/randomgen.h"
#include "gs/global/global_data.h"
#include "gs/obj/faction.h"
#include "gs/scene/world.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"

// proto
#include "common/protocol/gen/global/battleground_msg.pb.h"
#include "common/protocol/gen/G2M/battleground_msg.pb.h"
#include "common/protocol/gen/M2G/battleground_msg.pb.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;

namespace {

	static const int64_t kHistoryMaxRemainMsecs = 7*24*3600*1000; // 单位是毫秒

    bool FormatCopyFromDB(PlayerBattleGround::BGDataInfo& bg_info, const common::BattleGroundData::CurBGInfo& data)
	{
		// find bg template
		const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(data.bg_templ_id);
		if (ptempl == NULL)
		{
			bg_info.clear();
			return false;
		}

		bg_info.info.bg_serial_num   = data.bg_serial_num;
		bg_info.info.bg_templ_id     = data.bg_templ_id;
		bg_info.info.world_id        = data.world_id;
		bg_info.info.world_tag       = data.world_tag;
		bg_info.info.bg_create_time  = data.bg_create_time;
		bg_info.is_consumed          = data.is_consumed_cond;
		bg_info.self_pos.x           = data.bg_pos_x;
		bg_info.self_pos.y           = data.bg_pos_y;
		bg_info.bg_type              = ptempl->bg_type;
		return true;
	}

    bool FormatCopyToDB(const PlayerBattleGround::BGDataInfo& bg_info, common::BattleGroundData::CurBGInfo* pdata)
    {
        pdata->bg_serial_num    = bg_info.info.bg_serial_num;
        pdata->bg_templ_id      = bg_info.info.bg_templ_id;
        pdata->world_id         = bg_info.info.world_id;
        pdata->world_tag        = bg_info.info.world_tag;
        pdata->bg_create_time   = bg_info.info.bg_create_time;
        pdata->is_consumed_cond = bg_info.is_consumed;
        pdata->bg_pos_x         = bg_info.self_pos.x;
        pdata->bg_pos_y         = bg_info.self_pos.y;
        return true;
    }

    bool CheckBGCreateTime(int64_t now, int64_t bg_create_time)
	{
		int64_t timespan = now - bg_create_time;
		if (timespan >= 0 && timespan < kHistoryMaxRemainMsecs)
			return true;

		return false;
	}
    
} // Anonymous 

///
/// PlayerBattleGround
///
PlayerBattleGround::PlayerBattleGround(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_BATTLEGROUND, player)
{
	ResetCurBGInfo();
	runtime_bginfo_ = new global::BattleGroundInfo();
	ASSERT(runtime_bginfo_);

    SAVE_LOAD_REGISTER(common::BattleGroundData, PlayerBattleGround::SaveToDB, PlayerBattleGround::LoadFromDB);
}

PlayerBattleGround::~PlayerBattleGround()
{
}

void PlayerBattleGround::OnRelease()
{
	ResetCurBGInfo();
	cur_request_bg_.clear();

	DELETE_SET_NULL(runtime_bginfo_);
}


bool PlayerBattleGround::SaveToDB(common::BattleGroundData* pdata)
{
    // 如果当前地图就是所在的副本地图才保存cur项
	if (cur_bg_.info.has_info() && 
        cur_bg_.info.world_id == player_.world_id())
	{
		// 如果在跨服服务器，存盘时对world_id进行调整
        BGDataInfo tmp_bg    = cur_bg_;
		tmp_bg.info.world_id = Gmatrix::GetRealWorldID(tmp_bg.info.world_id);
		FormatCopyToDB(tmp_bg, &pdata->cur_bg);
    }
    
    HistoryDataMap::iterator it = history_bg_.begin();
	for (; it != history_bg_.end(); ++it)
	{
		pdata->history_bg.push_back(it->second);
	}

    return true;
}

bool PlayerBattleGround::LoadFromDB(const common::BattleGroundData& data)
{
	int64_t now = g_timer->GetSysTimeMsecs();

	if (CheckBGCreateTime(now, data.cur_bg.bg_create_time))
    {
        if (FormatCopyFromDB(cur_bg_, data.cur_bg))
		{
			if (IS_CR_BG(cur_bg_.info.world_id))
			{
                // 从跨服world_id转成正常的地图id
				cur_bg_.info.world_id = WID_TO_MAPID(cur_bg_.info.world_id);
			}
		}
		else
		{
			cur_bg_.clear();
		}
    }
    
    for (size_t i = 0; i < data.history_bg.size(); ++i)
	{
		if (CheckBGCreateTime(now, data.history_bg[i].checkin_per_day))
		{
            history_bg_[data.history_bg[i].bg_templ_id] = data.history_bg[i];
		}
	}

    return true;
}

void PlayerBattleGround::OnHeartbeat(time_t cur_time)
{
    if (cur_request_bg_.is_countdown())
	{
		if (--cur_request_bg_.time_out <= 0)
		{
			cur_request_bg_.clear();
		}
	}
}

void PlayerBattleGround::OnEnterWorld()
{
    if (!IS_BG_MAP(player_.world_id()) && cur_bg_.info.has_info())
	{
		ResetCurBGInfo();
	}
}

void PlayerBattleGround::OnLeaveWorld()
{
    if (IS_BG_MAP(player_.world_id()))
	{
        // 如果不是玩家主动退出战场，则记录玩家在战场里的位置
        // 玩家主动退出战场会清空cur_bg_ , 见bg_player_imp.cpp
		if (cur_bg_.info.has_info())
        {
		    cur_bg_.self_pos = player_.pos();
        }
	}
}

void PlayerBattleGround::ResetCurBGInfo()
{
	cur_bg_.clear();
}

bool PlayerBattleGround::CanEnterBattleGround() const
{
	// 不能从特殊地图跳到战场地图
	if (!IS_NORMAL_MAP(player_.world_id()))
		return false;

	return true;
}

bool PlayerBattleGround::CheckEnterCondition(const dataTempl::BattleGroundTempl* pdata) const
{
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

    // 检查每日的进入次数
    if (pdata->entrance_limit_per_day > 0)
    {
        HistoryDataMap::const_iterator it = history_bg_.find(pdata->templ_id);
        if (it != history_bg_.end())
        {
            time_t now     = g_timer->GetSysTime();
            time_t checkin = it->second.checkin_per_day;
            struct tm tmNow, tmCheckin;
            ::localtime_r(&now, &tmNow);
            ::localtime_r(&checkin, &tmCheckin);
            // 年月日都相等
            if (tmNow.tm_year == tmCheckin.tm_year &&
                tmNow.tm_mon  == tmCheckin.tm_mon &&
                tmNow.tm_mday == tmCheckin.tm_mday)
            {
                if (it->second.entertimes_per_day > pdata->entrance_limit_per_day)
                    return false;
            }
        }
    }

    // 世界BOSS的每日开启次数
    if (pdata->bg_type == dataTempl::BattleGroundTempl::BGT_PVE_WORLD_BOSS &&
        pdata->worldboss_data.wb_tid > 0)
    {
        int32_t master_id = player_.master_id();
        int32_t boss_tid  = pdata->worldboss_data.wb_tid;
        globalData::WorldBossInfo::QueryValue value;
        value.cur_time = g_timer->GetSysTime();
        if (s_pGlobalData->Query<globalData::WorldBossInfo>(master_id, boss_tid, value))
        {
            if (value.out_daily.record_count >= pdata->worldboss_data.enter_limit_per_day)
            {
                player_.sender()->ErrorMessage(G2C::ERR_WORLD_BOSS_DAILY_LIMIT);
                return false;
            }
        }
    }

    return true;
}

bool PlayerBattleGround::CheckCondIsConsumed(int32_t bg_templ_id) const
{
    // 是否已经检查
	if (cur_bg_.info.has_info() &&
		cur_bg_.info.bg_templ_id == bg_templ_id &&
		cur_bg_.is_consumed)
	{
		return true;
	}
	
	return false;
}

void PlayerBattleGround::CleanUpSpecBGInfo(int32_t bg_tid)
{
    if (cur_bg_.info.bg_templ_id == bg_tid)
	{
		ResetCurBGInfo();
	}
}

bool PlayerBattleGround::GetRuntimeBGInfo(MapID target_world_id, global::BattleGroundInfo& bginfo) const
{
    if (target_world_id == runtime_bginfo_->world_id())
	{
		bginfo.CopyFrom(*runtime_bginfo_);
		return true;
	}
	return false;
}

bool PlayerBattleGround::IsChangeMapFillMapTeam(MapID target_world_id) const
{
    if (target_world_id == runtime_bginfo_->world_id())
    {
        // find battleground template
        const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(runtime_bginfo_->bg_templ_id());
        if (ptempl == NULL)
        {
            LOG_WARN << "玩家" << player_.role_id() << "进入战场居然没找到对应模板？tid：" << runtime_bginfo_->bg_templ_id();
            return false;
        }

        // 现在只有世界boss战场需要把大世界的组队带入战场
        if (ptempl->bg_type == dataTempl::BattleGroundTempl::BGT_PVE_WORLD_BOSS)
        {
            return true;
        }
    }
    return false;
}

bool PlayerBattleGround::CheckBattleGroundCond(const playerdef::BGInfoCond& info) const
{
    // find battleground template
	const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(info.bg_tid);
	if (ptempl == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "进入战场居然没找到对应模板？tid：" << info.bg_tid;
		return false;
	}

	bool is_reenter = (info.enter_type == playerdef::BGInfoCond::REENTER) ? true : false;
	// 曾经进入过副本，则无需在检查
	if (is_reenter && CheckCondIsConsumed(info.bg_tid))
	{
		return true;
	}

	// 检查进入条件
	if (!CheckEnterCondition(ptempl))
	{
		return false;
	}

    return true;
}

void PlayerBattleGround::ResetBattleGroundInfo()
{
	ResetCurBGInfo();
}

void PlayerBattleGround::SetBattleGroundInfo(const world::battleground_info& info)
{
	ASSERT(IS_BG_MAP(info.world_id));
	if (cur_bg_.info != info)
	{
		// find battleground template
		const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(info.bg_templ_id);
		ASSERT(ptempl != NULL);
		cur_bg_.info        = info;
		cur_bg_.is_consumed = false;
		cur_bg_.bg_type     = ptempl->bg_type;
        cur_bg_.self_pos    = GetPositionByFaction(ptempl, BG_FACTION_INDEX_A);
	}
}

A2DVECTOR PlayerBattleGround::GetPositionByFaction(const dataTempl::BattleGroundTempl* ptempl, int faction) const
{
    int size = ptempl->faction_list[faction].entrance_pos.size();
    int index = mrand::Rand(0, size - 1);
    A2DVECTOR pos;
    pos.x = ptempl->faction_list[faction].entrance_pos[index].x;
    pos.y = ptempl->faction_list[faction].entrance_pos[index].y;
    return pos;
}

bool PlayerBattleGround::GetBattleGroundPos(A2DVECTOR& pos) const
{
	if (cur_bg_.info.has_info())
	{
		pos = cur_bg_.self_pos;
		return true;
	}
	return false;
}

// needed check condition outside
void PlayerBattleGround::ConsumeEnterCond(const dataTempl::BattleGroundTempl* pdata)
{
    // 检查物品
    if (pdata->entrance_item_proc == dataTempl::BattleGroundTempl::IPT_TAKE_OUT)
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

bool PlayerBattleGround::ConsumeBattleGroundCond(int32_t bg_tid)
{
	ASSERT(bg_tid == cur_bg_.info.bg_templ_id);

	if (cur_bg_.is_consumed)
	{
		return true;
	}

	// find battleground template
	const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(bg_tid);
	if (ptempl == NULL)
	{
		LOG_WARN << "玩家" << player_.role_id() << "战场居然没找到对应模板？tid：" << bg_tid;
		return false;
	}

	// 扣除进入所需
	ConsumeEnterCond(ptempl);

    // 记录进入次数
    if (ptempl->entrance_limit_per_day > 0)
    {
        BattleGroundData::HistoryData& ent = history_bg_[ptempl->templ_id];
        ent.bg_templ_id = ptempl->templ_id;

        time_t now     = g_timer->GetSysTime();
        time_t checkin = ent.checkin_per_day;
        struct tm tmNow, tmCheckin;
        ::localtime_r(&now, &tmNow);
        ::localtime_r(&checkin, &tmCheckin);
        // 年月日都相等
        if (ent.checkin_per_day > 0 &&
            tmNow.tm_year == tmCheckin.tm_year &&
            tmNow.tm_mon  == tmCheckin.tm_mon &&
            tmNow.tm_mday == tmCheckin.tm_mday)
        {
            ent.entertimes_per_day++;
        }
        else
        {
            ent.checkin_per_day    = now;
            ent.entertimes_per_day = 1;
        }
    }

    // 记录已经扣除
	cur_bg_.is_consumed = true;
	return true;
}

bool PlayerBattleGround::QueryBGEntrancePos(A2DVECTOR& pos) const
{
    if (cur_bg_.info.has_info())
	{
        pos = cur_bg_.self_pos;
        return true;
	}
	return false;
}

void PlayerBattleGround::EnterPveBattleGround(int32_t bg_tid) const
{
    const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(bg_tid);
    if (ptempl == NULL)
    {
        __PRINTF("EnterPveBattleGround没有找到战场模板！bg_tid:%d", bg_tid);
        return;
    }
    msg_bg_transfer_prepare send_param;
    send_param.bg_templ_id  = bg_tid;
    send_param.request_type = G2M::BGRT_UI_PVE_RALLY; // 现在只有一种请求方式，主要用来区分返回点
    player_.SendMsg(GS_MSG_BG_TRANSFER_PREPARE, player_.object_xid(), &send_param, sizeof(send_param));
}

void PlayerBattleGround::QuitBattleGround(int32_t bg_tid) const
{
    if (IS_BG_MAP(player_.world_id()))
    {
        if (cur_bg_.info.has_info() && cur_bg_.info.bg_templ_id == bg_tid)
        {
            player_.SendMsg(GS_MSG_PLAYER_QUIT_BG, player_.object_xid());
        }
    }
}

void PlayerBattleGround::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::PlayerQuitBattleGround, PlayerBattleGround::CMDHandler_PlayerQuitBattleGround);
    REGISTER_NORMAL_CMD_HANDLER(C2G::GetBattleGroundData, PlayerBattleGround::CMDHandler_GetBattleGroundData);
}

void PlayerBattleGround::RegisterMsgHandler()
{
    REGISTER_MSG_HANDLER(GS_MSG_BG_TRANSFER_PREPARE, PlayerBattleGround::MSGHandler_BGTransferPrepare);
    REGISTER_MSG_HANDLER(GS_MSG_ENTER_BG_REPLY, PlayerBattleGround::MSGHandler_EnterBGReply);
}

void PlayerBattleGround::CMDHandler_PlayerQuitBattleGround(const C2G::PlayerQuitBattleGround& cmd)
{
    if (IS_BG_MAP(player_.world_id()))
	{
		player_.SendMsg(GS_MSG_PLAYER_QUIT_BG, player_.object_xid());
	}
}

void PlayerBattleGround::CMDHandler_GetBattleGroundData(const C2G::GetBattleGroundData& cmd)
{
    if (!IS_BG_MAP(player_.world_id()))
	{
		LOG_WARN << "玩家" << player_.role_id() << "不在战场，但获取战场信息！C2G::GetBattleGroundData";
		return;
	}

	if (!cur_bg_.info.has_info() || cur_bg_.info.world_id != player_.world_id())
	{
		LOG_ERROR << "GetBattleGroundData cur_bg_.info.world_id != player_.world_id()";
		return;
	}

	//
	// 发战场信息
	//
	player_.sender()->GetBattleGroundDataRe(cur_bg_.info.bg_templ_id, Gmatrix::IsCrossRealmServer());
}

int PlayerBattleGround::MSGHandler_BGTransferPrepare(const MSG& msg)
{
    if (!CanEnterBattleGround())
		return 0;

	if (cur_request_bg_.has_request())
		return 0;

	CHECK_CONTENT_PARAM(msg, msg_bg_transfer_prepare);
	const msg_bg_transfer_prepare& bg_param = *(msg_bg_transfer_prepare*)msg.content;

    const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(bg_param.bg_templ_id);
	if (ptempl == NULL)
	{
		LOG_ERROR << "BGTransferPrepare() 找不到对应的战场模板！bg_templ_id:" << bg_param.bg_templ_id;
		return -1;
	}
	ASSERT(ptempl != NULL);

    if (!CheckEnterCondition(ptempl))
	{
		__PRINTF("玩家%ld不满足战场%d进入条件, 战场模板id=%d！", 
				player_.role_id(), ptempl->bg_map_id, ptempl->templ_id);
		player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_BG_ENTER_COND);
		return 0;
	}

    // battleground type
    int32_t tmp_id = ptempl->bg_map_id;
	G2M::EnterBGRequest proto;
    if (ptempl->bg_type == dataTempl::BattleGroundTempl::BGT_PVE_RALLY)
    {
        proto.set_bg_type(G2M::BGT_PVE_RALLY);
        // 获取跨服map_id
        tmp_id = Gmatrix::ConvertToCRWorldID(ptempl->bg_map_id);
    }
    else if (ptempl->bg_type == dataTempl::BattleGroundTempl::BGT_PVE_WORLD_BOSS)
    {
        proto.set_bg_type(G2M::BGT_PVE_WORLD_BOSS);
        // 获取本服map_id
        tmp_id = ptempl->bg_map_id;
    }
    else
    {
        ASSERT(false);
    }

    // request type
	int32_t src_world_id = 0;
	float src_pos_x      = 0.f;
	float src_pos_y      = 0.f;
    switch (bg_param.request_type)
	{
		case G2M::BGRT_UI_PVE_RALLY:
            {
                src_world_id = player_.world_id();
				src_pos_x    = player_.pos().x;
				src_pos_y    = player_.pos().y;
				proto.set_req_type((G2M::BGRequestType)bg_param.request_type);
            }
            break;
		
		default:
			ASSERT(false);
			break;
	}

    // set proto
    proto.set_des_world_id(tmp_id);
    proto.set_roleid(player_.role_id());
	proto.set_bg_templ_id(bg_param.bg_templ_id);
	proto.set_src_world_id(src_world_id);
	proto.set_src_pos_x(src_pos_x);
	proto.set_src_pos_y(src_pos_y);
	player_.sender()->SendToMaster(proto);

	// record request
	cur_request_bg_.bg_tid   = bg_param.bg_templ_id;
	cur_request_bg_.time_out = kBGRequestTimeout;
    return 0;
}

int PlayerBattleGround::MSGHandler_EnterBGReply(const MSG& msg)
{
    cur_request_bg_.clear();
	runtime_bginfo_->Clear();

    if (!CanEnterBattleGround())
		return 0;

    M2G::EnterBGReply proto;
    ASSERT(proto.ParseFromArray(msg.content, msg.content_len));

    // error 
	if (proto.err_code() != M2G::EnterBGReply::SUCCESS)
	{
		CleanUpSpecBGInfo(proto.bg_templ_id());
		return 0;
	}

    // find battleground template
	const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(proto.bg_templ_id());
	if (ptempl == NULL)
	{
		LOG_ERROR << "EnterBGReply的bg_templ_id=" << proto.bg_templ_id() << "居然没找到对应模板？";
		ResetCurBGInfo();
		return -1;
	}

	// enter condition
    int32_t bg_world_id = proto.des_world_id();
    if (WID_TO_MAPID(bg_world_id) != ptempl->bg_map_id)
    {
        LOG_ERROR << "EnterBGReply收到的des_world_id与模板里不符，des_wid:" << bg_world_id 
            << " templ_id:" << ptempl->templ_id;
        return -1;
    }
    if (!CheckEnterCondition(ptempl))
	{
		__PRINTF("玩家%ld进入战场条件不满足bg_tid:%d, world_id:%d", 
				player_.role_id(), proto.bg_templ_id(), bg_world_id);
		player_.sender()->ErrorMessage(G2C::ERR_DONOT_MATCH_BG_ENTER_COND);
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
	runtime_bginfo_->set_enter_type((common::protocol::global::BGEnterType)proto.enter_type());
	runtime_bginfo_->set_bg_serial_num(proto.bg_serial_num());
	runtime_bginfo_->set_bg_templ_id(proto.bg_templ_id());
	runtime_bginfo_->set_world_id(bg_world_id);
    runtime_bginfo_->set_world_tag(0);
    runtime_bginfo_->set_bg_create_time(0);
	runtime_bginfo_->set_src_world_id(proto.src_world_id());
	runtime_bginfo_->set_src_pos_x(proto.src_pos_x());
	runtime_bginfo_->set_src_pos_y(proto.src_pos_y());

	msg_player_region_transport msg_param;
	msg_param.source_world_id = player_.world_id();
	msg_param.target_world_id = runtime_bginfo_->world_id();
	msg_param.bg_templ_id     = runtime_bginfo_->bg_templ_id();
	msg_param.target_pos.x    = 0;
	msg_param.target_pos.y    = 0;
	player_.SendMsg(GS_MSG_BG_TRANSFER_START, player_.object_xid(), &msg_param, sizeof(msg_param));
    return 0;
}

} // namespace gamed
