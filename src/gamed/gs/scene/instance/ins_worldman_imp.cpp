#include "ins_worldman_imp.h"

#include "gs/netmsg/send_to_master.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/global/game_util.h"
#include "gs/global/game_types.h"
#include "gs/global/global_data.h"
#include "gs/global/randomgen.h"
#include "gs/global/global_data/instance_record.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/instance_templ.h"
#include "gs/scene/instance/ins_player_imp.h"
#include "gs/player/player.h"

// proto
#include "common/protocol/gen/G2M/instance_msg.pb.h"

#include "ins_cluster.h"


namespace gamed {

using namespace dataTempl;
using namespace common::protocol;

namespace {

	G2M::InstanceType GetProtoInsType(const dataTempl::InstanceTempl::InstanceType type)
	{
		if (type == InstanceTempl::IT_SOLO)
			return G2M::IT_SOLO;
		else if (type == InstanceTempl::IT_TEAM)
			return G2M::IT_TEAM;
		else if (type == InstanceTempl::IT_UNION)
			return G2M::IT_UNION;
		else
			ASSERT(false);
	}

} // Anonymous


///
/// InsWorldManImp
/// 
InsWorldManImp::InsWorldManImp(WorldManager& worldMan)
	: BaseWorldManImp(worldMan),
	  state_(IS_INVALID),
	  running_endtime_(0),
	  closing_endtime_(GS_INT32_MAX),
	  max_endtime_(0),
	  remain_secs_(0),
	  player_count_(0),
	  instance_type_(0),
      ins_team_id_(-1),
	  instance_result_(IR_UNDONE),
	  has_key_monster_(false)
{
}

InsWorldManImp::~InsWorldManImp()
{
}

bool InsWorldManImp::OnInit()
{ 
	if (!world_man_.plane()->GetInsInfo(ins_info_))
	{
		LOG_ERROR << "副本imp初始化失败！GetInsInfo()失败！";
		return false;
	}

	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_info_.ins_templ_id);
	if (ptempl == NULL)
	{
		LOG_ERROR << "副本创建没有找到对应的副本模板，templ_id:" << ins_info_.ins_templ_id;
		return false;
	}

	max_endtime_     = g_timer->GetSysTime() + ptempl->ins_max_survival_time;
	remain_secs_     = ptempl->ins_remain_time;
	running_endtime_ = g_timer->GetSysTime() + remain_secs_; // 先按最小时间来
	instance_type_   = ptempl->ins_type;

	if (ptempl->ins_key_monster.size())
	{
		has_key_monster_ = true;
		for (size_t i = 0; i < ptempl->ins_key_monster.size(); ++i)
		{
			key_monster_set_.insert(ptempl->ins_key_monster[i]);
		}
	}

	// script
	std::string scriptDir = Gmatrix::GetInsScriptDir();
	if (!ins_script_if_.Init(world_man_.GetWorldXID(), ptempl->ins_script_id, scriptDir.c_str()))
	{
		LOG_ERROR << "副本脚本接口初始化失败！templ_id:" << ptempl->templ_id;
		return false;
	}

    // init base class
    if (!BaseWorldManImp::OnInit())
    {
        LOG_ERROR << "副本初始化BaseWorldManImp父类失败！templ_id:" << ptempl->templ_id;
        return false;
    }

	UpdateStatus(IS_READY);
	return true; 
}

void InsWorldManImp::UpdateStatus(InstanceStates state)
{
	state_ = state;
}

void InsWorldManImp::OnClockTimeUp(int32_t index)
{
    ins_script_if_.ClockTimeUp(index);
}

//
// 按秒心跳
//
void InsWorldManImp::OnHeartbeat()
{ 
	// refresh result
	RefreshInstanceResult();

	// state switch func
	DriveMachine();

	// clock
    BaseWorldManImp::OnHeartbeat();
}

bool InsWorldManImp::OnInsertPlayer(Player* pplayer)
{
	pplayer->ChangePlayerImp<InsPlayerImp>();

    if (!BaseWorldManImp::OnInsertPlayer(pplayer))
        return false;

	return true;
}

void InsWorldManImp::OnPlayerEnterMap(RoleID roleid)
{
	if ((++player_count_) > 0 && ins_is_running())
	{
		running_endtime_ = max_endtime_;
	}
	
	ins_script_if_.PlayerEnter(roleid);
}

void InsWorldManImp::OnPlayerLeaveMap(RoleID roleid)
{
	ASSERT(player_count_ > 0);
	if ((--player_count_) <= 0 && ins_is_running())
	{
		running_endtime_ = g_timer->GetSysTime() + remain_secs_;
		if (running_endtime_ > max_endtime_)
		{
			running_endtime_ = max_endtime_;
		}
	}

    BaseWorldManImp::OnPlayerLeaveMap(roleid);
}

int InsWorldManImp::OnMessageHandler(const MSG& msg) 
{ 
	switch (msg.message)
	{
        case GS_PLANE_MSG_SYS_CLOSE_INS:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_sys_close_ins);
				SysCloseInstance(msg);
			}
			break;

        default:
            if (BaseWorldManImp::OnMessageHandler(msg) != 0)
            {
                ASSERT(false && "无法处理未知类型的inter-message");
                return -1;
            }
			return 0;
	}
	return 0; 
}

void InsWorldManImp::DriveMachine()
{
	switch (state_)
	{
		case IS_READY:
			{
				OnReady();
			}
			break;

		case IS_RUNNING:
			{
				OnRunning();
				// 时间到了，或者结果已出
				if (running_endtime_ < g_timer->GetSysTime() || instance_result_ != IR_UNDONE)
				{
					OnClose();
				}
			}
			break;

		case IS_CLOSING:
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

void InsWorldManImp::OnReady()
{
	G2M::NotifyInsStatus proto;
	proto.set_ins_type(GetProtoInsType((dataTempl::InstanceTempl::InstanceType)instance_type_));
	proto.set_ins_status(G2M::NotifyInsStatus::INS_CREATE);
	proto.set_world_id(Gmatrix::GetRealWorldID(ins_info_.world_id));
	proto.set_world_tag(ins_info_.world_tag);
	proto.set_ins_templ_id(ins_info_.ins_templ_id);
	proto.set_ins_create_time(ins_info_.ins_create_time);
	proto.set_ins_serial_num(ins_info_.ins_serial_num);
	NetToMaster::SendToAllMaster(proto);

	ins_script_if_.InstanceStart();

	UpdateStatus(IS_RUNNING);
}

void InsWorldManImp::OnRunning()
{
	QueryPlayerInfoInTeam();
}

void InsWorldManImp::OnClose()
{
	G2M::NotifyInsStatus proto;
	proto.set_ins_type(GetProtoInsType((dataTempl::InstanceTempl::InstanceType)instance_type_));
	proto.set_ins_status(G2M::NotifyInsStatus::INS_CLOSE);
	proto.set_world_id(Gmatrix::GetRealWorldID(ins_info_.world_id));
	proto.set_world_tag(ins_info_.world_tag);
	proto.set_ins_templ_id(ins_info_.ins_templ_id);
	proto.set_ins_create_time(ins_info_.ins_create_time);
	proto.set_ins_serial_num(ins_info_.ins_serial_num);
	NetToMaster::SendToAllMaster(proto);

	closing_endtime_ = g_timer->GetSysTime() + kKickoutCloseTime;
	UpdateStatus(IS_CLOSING);

	// 计算服务器记录
	CalcInsResult();
	
    // 从cluster里删掉，让玩家不能再进来
    world_man_.CloseWorldMan();
}

void InsWorldManImp::OnRemove()
{
	world_man_.RemoveSelfFromCluster();

	UpdateStatus(IS_INVALID);
}

void InsWorldManImp::RefreshInstanceResult()
{
	if (has_key_monster_ && key_monster_set_.size() == 0)
	{
		instance_result_ = IR_KEY_MONSTER_ALL_KILLED;
	}
}

void InsWorldManImp::OnMonsterKilled(int32_t monster_tid, int32_t count)
{
	ins_script_if_.MonsterDead(monster_tid, count);

	if (has_key_monster_)
	{
		key_monster_set_.erase(monster_tid);
	}
}

bool InsWorldManImp::IsPlayersVictory() const
{
	if (instance_result_ == IR_KEY_MONSTER_ALL_KILLED ||
		instance_result_ == IR_TASK_SET_P_SUCCESS ||
		instance_result_ == IR_SCRIPT_SET_P_SUCCESS)
	{
		return true;
	}

	return false;
}

void InsWorldManImp::OnModifyCounter(int32_t index, int32_t value)
{
    ins_script_if_.CounterChange(index, value);
}

void InsWorldManImp::SysCloseInstance(const MSG& msg)
{
	if (instance_result_ != IR_UNDONE)
	{
		// 副本已经结束
		return;
	}

	const plane_msg_sys_close_ins& param = *(plane_msg_sys_close_ins*)msg.content;
	switch (param.sys_type)
	{
		case plane_msg_sys_close_ins::ST_TASK:
			{
				if (param.ins_result == plane_msg_sys_close_ins::PLAYER_VICTORY)
				{
					instance_result_ = IR_TASK_SET_P_SUCCESS;
				}
				else
				{
					instance_result_ = IR_TASK_SET_P_FAILURE;
				}
			}
			break;

		case plane_msg_sys_close_ins::ST_SCRIPT:
			{
				if (param.ins_result == plane_msg_sys_close_ins::PLAYER_VICTORY)
				{
					instance_result_ = IR_SCRIPT_SET_P_SUCCESS;
				}
				else
				{
					instance_result_ = IR_SCRIPT_SET_P_FAILURE;
				}
			}
			break;

		default:
			ASSERT(false);
			break;
	}
}

void InsWorldManImp::CalcInsResult()
{
	const InstanceTempl* ptempl = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_info_.ins_templ_id);
	ASSERT(ptempl != NULL);

	msg_ins_finish_result param;
	param.clear_time = g_timer->GetSysTime() - world_man_.plane()->get_create_time();
	param.ins_result = IsPlayersVictory() ? ICR_PLAYER_WIN : ICR_PLAYER_FAIL;
	param.is_svr_record = false;
	ASSERT(param.clear_time > 0);

    std::vector<MapTeamInfo> info_list;
    GetAllTeamInfo(info_list);

	// 有金牌记录的副本才能有服务器记录
	if (ptempl->has_srv_record && 
		param.clear_time < ptempl->gold_record_time)
	{
		world::instance_info ins_info;
		ASSERT(world_man_.plane()->GetInsInfo(ins_info));

        // 获取所有的master
        std::set<int32_t> master_set;
        Gmatrix::GetAllMasterId(master_set);

        for (size_t j = 0; j < info_list.size(); ++j)
        {
            const MapTeamInfo& team_info = info_list[j];
            for (size_t i = 0; i < team_info.members.size(); ++i)
            {
                if (team_info.members[i].is_vacancy())
                    continue;

                SendInsResult(team_info.members[i].roleid, param);
            }

            // 把破纪录的消息发给所有master
            std::set<int32_t>::const_iterator it_master = master_set.begin();
            for (; it_master != master_set.end(); ++it_master)
            {
                globalData::InstanceRecord::RecordValue svr_value;
                param.is_svr_record    = false;
                int32_t masterid       = *it_master;
                int32_t ins_tid        = ins_info_.ins_templ_id;
                bool query_res         = s_pGlobalData->Query<globalData::InstanceRecord>(masterid, ins_tid, svr_value);
                int32_t svr_clear_time = svr_value.clear_time;
                if (!query_res || (query_res && param.clear_time < svr_clear_time))
                {
                    param.is_svr_record  = true;
                    param.svr_clear_time = (svr_clear_time > 0) ? svr_clear_time : 0; // 考虑第一次破纪录的情况
                    BreakServerRecord(team_info, masterid, ins_tid, param.clear_time);
                }
            }
        }
	}
	else
	{
        for (size_t j = 0; j < info_list.size(); ++j)
        {
            const MapTeamInfo& team_info = info_list[j];
            for (size_t i = 0; i < team_info.members.size(); ++i)
            {
                if (team_info.members[i].is_vacancy())
                    continue;

                param.is_svr_record = false;
                SendInsResult(team_info.members[i].roleid, param);
            }
        }
	}
}

void InsWorldManImp::SendInsResult(RoleID roleid, const msg_ins_finish_result& param)
{
	XID player;
	MAKE_XID(roleid, player);
	SendPlayerMsg(GS_MSG_INS_FINISH_RESULT, player, 0, &param, sizeof(param));
}

void InsWorldManImp::BreakServerRecord(const MapTeamInfo& team_info, int32_t masterid, int32_t ins_tid, int32_t clear_time)
{
	globalData::InstanceRecord::RecordValue value;
	value.clear_time = clear_time;

	for (size_t i = 0; i < team_info.members.size(); ++i)
	{
		if (team_info.members[i].is_vacancy())
			continue;

		const map_team_member_info& player = team_info.members[i];
		msgpack_ins_player_info record_info;
		record_info.role_id      = player.roleid;
		record_info.first_name   = player.first_name;
		record_info.mid_name     = player.mid_name;
		record_info.last_name    = player.last_name;
		record_info.level        = player.level;
		record_info.cls          = player.cls;
		record_info.combat_value = player.combat_value;
		value.player_vec.push_back(record_info);
	}

	s_pGlobalData->Modify<globalData::InstanceRecord>(masterid, ins_tid, value);
}

void InsWorldManImp::OnPlayerGatherMine(int64_t roleid, int32_t mine_tid)
{
    ins_script_if_.PlayerGatherMine(roleid, mine_tid);
}

void InsWorldManImp::OnPlayerQuitMap(int32_t member_count)
{
    // 检查副本已经没有玩家（全部主动退出副本）
    if (member_count <= 0)
    {
        instance_result_ = IR_ALL_PLAYER_QUIT;
    }
}

void InsWorldManImp::OnReachDestination(int32_t elem_id, const A2DVECTOR& pos)
{
    ins_script_if_.ReachDestination(elem_id, pos);
}

bool InsWorldManImp::IsAutoMapTeam() const
{
    return true;
}

int InsWorldManImp::GetAutoMapTeamID(const Player* pplayer) const
{
    return ins_team_id_;
}

void InsWorldManImp::NewMapTeamCreated(const MapTeamInfo& info)
{
    ASSERT(ins_team_id_ <= 0);
    ins_team_id_ = info.team_id;
}

} // namespace gamed

