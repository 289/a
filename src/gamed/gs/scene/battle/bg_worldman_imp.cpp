#include "bg_worldman_imp.h"

#include "gs/netmsg/send_to_master.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/battleground_templ.h"
#include "gs/global/game_util.h"
#include "gs/global/timer.h"
#include "gs/global/gmatrix.h"
#include "gs/player/player.h"

// proto
#include "common/protocol/gen/G2M/battleground_msg.pb.h"

#include "bg_player_imp.h"
#include "bg_pve_rally.h"
#include "bg_pve_worldboss.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;

namespace {

	G2M::BGType GetProtoBGType(const dataTempl::BattleGroundTempl::BGType type)
	{
		if (type == BattleGroundTempl::BGT_PVE_RALLY)
			return G2M::BGT_PVE_RALLY;
        else if (type == BattleGroundTempl::BGT_PVE_WORLD_BOSS)
            return G2M::BGT_PVE_WORLD_BOSS;
		else
			ASSERT(false);
	}

    // 断线后重新登陆是否可以超过战场的玩家上限，因为战场有补人机制，玩家再上线可能会超上限
    bool EnterWorldOverPlayerCountLimit(int bg_type)
    {
        if (bg_type == dataTempl::BattleGroundTempl::BGT_PVE_RALLY ||
            bg_type == dataTempl::BattleGroundTempl::BGT_PVE_WORLD_BOSS)
            return true;

        return false;
    }

} // Anonymous


///
/// BGWorldManImp
///
BGWorldManImp::BGWorldManImp(WorldManager& worldMan)
	: BaseWorldManImp(worldMan),
	  state_(BS_INVALID),
      running_endtime_(0),
      closing_endtime_(GS_INT32_MAX),
      max_endtime_(0),
      player_count_(0),
      player_min_count_(1),
      player_max_count_(0),
      bg_type_(0),
      supplement_time_(0),
      is_auto_map_team_(false),
      bg_result_(BGR_UNDONE),
      bg_lite_(NULL)
{
}

BGWorldManImp::~BGWorldManImp()
{
    DELETE_SET_NULL(bg_lite_);
}

bool BGWorldManImp::OnInit()
{ 
	if (!world_man_.plane()->GetBGInfo(bg_info_))
	{
		LOG_ERROR << "战场imp初始化失败！GetBGInfo()失败！";
		return false;
	}

	const BattleGroundTempl* ptempl = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(bg_info_.bg_templ_id);
	if (ptempl == NULL)
	{
		LOG_ERROR << "战场创建没有找到对应的战场模板，templ_id:" << bg_info_.bg_templ_id;
		return false;
	}

	max_endtime_      = g_timer->GetSysTime() + ptempl->bg_max_survival_time;
	running_endtime_  = max_endtime_;
    supplement_time_  = g_timer->GetSysTime() + ptempl->bg_supplement_time;
    is_auto_map_team_ = ptempl->auto_create_team;
	bg_type_          = ptempl->bg_type;
    player_min_count_ = ptempl->bg_player_lower_limit;
    player_max_count_ = ptempl->bg_player_upper_limit;
	
	// script
	std::string scriptDir = Gmatrix::GetBGScriptDir();
	if (!bg_script_if_.Init(world_man_.GetWorldXID(), ptempl->bg_script_id, scriptDir.c_str()))
	{
		LOG_ERROR << "战场脚本接口初始化失败！templ_id:" << ptempl->templ_id;
		return false;
	}

    // init base class
    if (!BaseWorldManImp::OnInit())
    {
        LOG_ERROR << "战场初始化BaseWorldManImp父类失败！templ_id:" << ptempl->templ_id;
        return false;
    }

    // init bg imp
    switch (bg_type_)
    {
        case dataTempl::BattleGroundTempl::BGT_PVE_RALLY:
            bg_lite_ = new BGPveRally(*this);
            break;

        case dataTempl::BattleGroundTempl::BGT_PVE_WORLD_BOSS:
            bg_lite_ = new BGPveWorldBoss(*this);
            break;

        default:
            bg_lite_ = new BattleGroundLite(*this);
            break;
    }
    if (!bg_lite_->OnInit())
    {
        LOG_ERROR << "战场的bg_lite_初始化失败！templ_id:" << ptempl->templ_id;
        return false;
    }

	UpdateStatus(BS_READY);

    // to master
    SendNotifyBGStatus(G2M::NotifyBGStatus::BG_CREATE);
	return true; 
}

void BGWorldManImp::UpdateStatus(BGStates state)
{
	state_ = state;
}

void BGWorldManImp::OnClockTimeUp(int32_t index)
{
    bg_script_if_.ClockTimeUp(index);
}

//
// 按秒心跳
//
void BGWorldManImp::OnHeartbeat()
{ 
	// state switch func
	DriveMachine();

	// clock
    BaseWorldManImp::OnHeartbeat();

    // sub imp
    bg_lite_->OnHeartbeat();
}

bool BGWorldManImp::OnInsertPlayer(Player* pplayer)
{
	pplayer->ChangePlayerImp<BGPlayerImp>();

    if (player_count_ >= player_max_count_)
        return false;

    if (!BaseWorldManImp::OnInsertPlayer(pplayer))
        return false;

	return true;
}

void BGWorldManImp::OnPlayerEnterMap(RoleID roleid)
{
    ASSERT(player_count_ < player_max_count_);
	if ((++player_count_) >= player_min_count_ && bg_is_running())
	{
		running_endtime_ = max_endtime_;
	}
	
	bg_script_if_.PlayerEnter(roleid);

    // to master
    SendNotifyBGPlayerCount();
}

void BGWorldManImp::OnPlayerLeaveMap(RoleID roleid)
{
	ASSERT(player_count_ > 0);
	if ((--player_count_) < player_min_count_ && bg_is_running())
	{
		running_endtime_ = g_timer->GetSysTime() + kMaxRemainTime;
		if (running_endtime_ > max_endtime_)
		{
			running_endtime_ = max_endtime_;
		}
	}

    BaseWorldManImp::OnPlayerLeaveMap(roleid);

    // to master
    SendNotifyBGPlayerCount();
}

int BGWorldManImp::OnMessageHandler(const MSG& msg) 
{ 
	switch (msg.message)
	{
        case GS_PLANE_MSG_SYS_CLOSE_BG:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_sys_close_bg);
				SysCloseBattleGround(msg);
			}
			break;

        default:
            if (BaseWorldManImp::OnMessageHandler(msg) != 0 && bg_lite_->OnMessageHandler(msg) != 0)
            {
                ASSERT(false && "无法处理未知类型的inter-message");
                return -1;
            }
			return 0;
	}
	return 0; 
}

void BGWorldManImp::DriveMachine()
{
	switch (state_)
	{
		case BS_READY:
			{
				OnReady();
			}
			break;

		case BS_RUNNING:
			{
				OnRunning();
				// 时间到了，或者结果已出
				if (running_endtime_ < g_timer->GetSysTime() || bg_result_ != BGR_UNDONE)
				{
					OnClose();
				}
			}
			break;

		case BS_CLOSING:
			{
				// 这个期间踢出玩家
				// 在玩家Heartbeat逻辑里完成规定时间内的踢人
				if (closing_endtime_ < g_timer->GetSysTime())
				{
					OnRemove();
				}
			}
			break;

		default:
			ASSERT(false);
	}
}

void BGWorldManImp::OnReady()
{
    // script
	bg_script_if_.BattleGroundStart();

    // update
	UpdateStatus(BS_RUNNING);
}

void BGWorldManImp::OnRunning()
{
    QueryPlayerInfoInTeam();
}

void BGWorldManImp::OnClose()
{
    // to master
    SendNotifyBGStatus(G2M::NotifyBGStatus::BG_CLOSE);

    closing_endtime_ = g_timer->GetSysTime() + kKickoutCloseTime;
	UpdateStatus(BS_CLOSING);

    // 计算结果
    HandleBGResult();

    // 从cluster里删掉，让玩家不能再进来
    world_man_.CloseWorldMan();
}

void BGWorldManImp::OnRemove()
{
	world_man_.RemoveSelfFromCluster();

	UpdateStatus(BS_INVALID);
}

void BGWorldManImp::OnMonsterKilled(int32_t monster_tid, int32_t count)
{
	bg_script_if_.MonsterDead(monster_tid, count);
}

bool BGWorldManImp::IsAutoMapTeam() const
{
    return is_auto_map_team_;
}

int BGWorldManImp::GetAutoMapTeamID(const Player* pplayer) const
{
    int team_id = 0;
    std::vector<MapTeamInfo> info_list;
    GetAllTeamInfo(info_list);

    for (size_t i = 0; i < info_list.size(); ++i)
    {
        if (info_list[i].get_vacancy() >= 0)
        {
            team_id = info_list[i].team_id;
            break;
        }
    }
    return team_id;
}

void BGWorldManImp::NewMapTeamCreated(const MapTeamInfo& info)
{
}

void BGWorldManImp::OnModifyCounter(int32_t index, int32_t value)
{
    bg_script_if_.CounterChange(index, value);
}

void BGWorldManImp::SysCloseBattleGround(const MSG& msg)
{
	if (bg_result_ != BGR_UNDONE)
	{
		// 战场已经结束
		return;
	}

	const plane_msg_sys_close_bg& param = *(plane_msg_sys_close_bg*)msg.content;
	switch (param.sys_type)
	{
		case plane_msg_sys_close_bg::ST_TASK:
			{
                bg_result_ = BGR_SYS_TASK_CLOSE;
			}
			break;

		case plane_msg_sys_close_bg::ST_SCRIPT:
			{
                bg_result_ = BGR_SYS_SCRIPT_CLOSE;
			}
			break;

		default:
			ASSERT(false);
			break;
	}
}

void BGWorldManImp::OnPlayerGatherMine(int64_t roleid, int32_t mine_tid)
{
    bg_script_if_.PlayerGatherMine(roleid, mine_tid);
}

void BGWorldManImp::OnPlayerQuitMap(int32_t member_count)
{
    // 检查战场已经没有玩家（全部主动退出战场）
    if (member_count <= 0)
    {
        bg_result_ = BGR_ALL_PLAYER_QUIT;
    }
}

void BGWorldManImp::OnReachDestination(int32_t elem_id, const A2DVECTOR& pos)
{
    bg_script_if_.ReachDestination(elem_id, pos);
}

void BGWorldManImp::SendNotifyBGStatus(int status)
{
    G2M::NotifyBGStatus proto;
	proto.set_bg_type(GetProtoBGType((dataTempl::BattleGroundTempl::BGType)bg_type_));
	proto.set_bg_status((G2M::NotifyBGStatus::BGStatus)status);
	proto.set_world_id(Gmatrix::GetRealWorldID(bg_info_.world_id));
	proto.set_world_tag(bg_info_.world_tag);
	proto.set_bg_templ_id(bg_info_.bg_templ_id);
	proto.set_bg_create_time(bg_info_.bg_create_time);
	proto.set_bg_serial_num(bg_info_.bg_serial_num);
    proto.set_bg_player_max(player_max_count_);
    proto.set_supplement_time(supplement_time_);
	NetToMaster::SendToAllMaster(proto);
}

void BGWorldManImp::SendNotifyBGPlayerCount()
{
    G2M::NotifyBGPlayerCount proto;
    proto.set_bg_serial_num(bg_info_.bg_serial_num);
    proto.set_world_id(Gmatrix::GetRealWorldID(bg_info_.world_id));
	proto.set_world_tag(bg_info_.world_tag);
    proto.set_player_count(player_count_);
    NetToMaster::SendToAllMaster(proto);
}

bool BGWorldManImp::OnCheckPlayerCountLimit(world::BGEnterType enterType) const
{
    if (enterType == world::BGET_ENTER_WORLD)
    {
        return EnterWorldOverPlayerCountLimit(bg_type_);
    }

    if (player_count_ < player_max_count_)
        return true;

    return false;
}

void BGWorldManImp::HandleBGResult()
{
    msg_bg_finish_result param;
    param.clear_time = g_timer->GetSysTime() - world_man_.plane()->get_create_time();
	world_man_.BroadcastToAllPlayer(GS_MSG_BG_FINISH_RESULT, &param, sizeof(param));
}

void BGWorldManImp::SetBattleGroundFinish(BGResult res)
{
    bg_result_ = res;
}

} // namespace gamed
