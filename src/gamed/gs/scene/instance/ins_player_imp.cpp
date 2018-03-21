#include "ins_player_imp.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/instance_templ.h"

#include "gs/global/game_util.h"
#include "gs/global/timer.h"
#include "gs/player/player_sender.h"
#include "gs/scene/world.h"


namespace gamed {

using namespace dataTempl;

InsPlayerImp::InsPlayerImp(Player& player)
	: PlayerImp(player),
	  logout_countdown_(GS_INT32_MAX),
	  ins_team_id_(0),
	  is_quitting_(false),
      ins_end_(false),
	  ins_templ_(NULL)
{
}

InsPlayerImp::~InsPlayerImp()
{
}

int InsPlayerImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_PLAYER_QUIT_INS:
			{
				ASSERT(player_.object_xid() == msg.source);
				PlayerQuitInsMap();
			}
			break;

		case GS_MSG_INS_FINISH_RESULT:
			{
				CHECK_CONTENT_PARAM(msg, msg_ins_finish_result);
				const msg_ins_finish_result& param = *(const msg_ins_finish_result*)msg.content;
				CalcPlayerRecord(param);
                ins_end_ = true;
			}
			break;

		case GS_MSG_MAP_QUERY_PLAYER_INFO:
			{
				HandleInsQueryInfo();
			}
			break;

		default:
			return -1;
	}

	return 0;
}

void InsPlayerImp::OnEnterWorld()
{
	if (!player_.world_plane()->GetInsInfo(ins_info_))
	{
		LOG_ERROR << "副本player_imp进入世界失败！GetInsInfo()失败！";
		ASSERT(false);
		return;
	}

	// find ins template
	ins_templ_ = s_pDataTempl->QueryDataTempl<InstanceTempl>(ins_info_.ins_templ_id);
	ASSERT(ins_templ_);

	if (ins_templ_->ins_type == dataTempl::InstanceTempl::IT_TEAM)
	{
		// 登录的时候就已经带了组队信息，所以在EnterWorld里直接取team_id没有问题
		ins_team_id_ = player_.GetTeamId();
	}
}

void InsPlayerImp::OnLeaveWorld()
{
    // 停止战斗，离开地图不需要终止战斗
    //player_.TerminateCombat();

    // 离开副本
    player_.sender()->PlayerLeaveInsMap(ins_info_.ins_templ_id);

    // 如果副本结束则清除自己保存的副本信息
    if (ins_end_)
    {
        player_.ResetInstanceInfo();
    }
}

void InsPlayerImp::OnHeartbeat()
{
	if (!is_quitting_ && !CheckPlayerCondition())
	{
		KickoutPlayerCountdown(kNotMeetCondKickoutTime);
	}

	// 持续踢人
	ContinuousKickout();

	// 检查是否踢出玩家，放在最后检查
	if (is_quitting_ && --logout_countdown_ < 0)
	{
		InsChangeMapLogout();
	}
}

void InsPlayerImp::OnGetAllData()
{
	int32_t ins_create_time = player_.world_plane()->get_create_time();
	player_.sender()->PlayerEnterInsMap(ins_create_time, ins_info_.ins_templ_id);
}

void InsPlayerImp::OnChangeGsComplete()
{
	int32_t ins_create_time = player_.world_plane()->get_create_time();
	player_.sender()->PlayerEnterInsMap(ins_create_time, ins_info_.ins_templ_id);
}

void InsPlayerImp::OnWorldClosing(const MSG& msg)
{
	KickoutPlayerCountdown(kLogoutCountdownTime);
}

void InsPlayerImp::CalcPlayerRecord(const msg_ins_finish_result& param)
{
	if (param.ins_result == ICR_PLAYER_WIN)
	{
		ASSERT(param.clear_time > 0);
		// player win
		player_.CalcInstanceRecord(ins_info_.ins_templ_id, param.clear_time, param.is_svr_record, param.svr_clear_time);
        player_.FinishInstance(ins_info_.ins_templ_id);
	}
	else
	{
		// player failure
		G2C::InstanceEnd packet;
		packet.ins_templ_id = ins_info_.ins_templ_id;
		packet.result       = G2C::InstanceEnd::IR_FAILURE;
        packet.award_class  = G2C::InstanceEnd::AC_NONE;
		player_.sender()->SendCmd(packet);
	}
}

void InsPlayerImp::ContinuousKickout()
{
	if (is_quitting_ && logout_countdown_ < 0)
	{
		// 持续踢人
		KickoutPlayer(1);
	}
}

void InsPlayerImp::KickoutPlayerCountdown(int countdown_secs)
{
	if (!is_quitting_)
	{
		KickoutPlayer(countdown_secs);
	}
    else if (is_quitting_ && (logout_countdown_ > countdown_secs))
    {
        logout_countdown_ = countdown_secs;
    }
}

void InsPlayerImp::KickoutPlayer(int countdown_secs)
{
	// 禁止被动标记
	player_.RejectPassivityFlag();

	// 停止战斗
	player_.TerminateCombat();

    // 强制复活，否则无法传送
    player_.ForceResurrect();

	// countdown
	logout_countdown_ = countdown_secs;
	player_.sender()->MapKickoutCountdown(logout_countdown_);

	// 可能多次调用，但只需发一次给地图
	if (!is_quitting_)
	{
		// notify world
		player_.SendPlaneMsg(GS_PLANE_MSG_PLAYER_QUIT_MAP);
	}

	// 最后设置quit标记
	is_quitting_ = true;
}

void InsPlayerImp::InsChangeMapLogout()
{
	int32_t tmp_worldid; 
	A2DVECTOR tmp_pos;

	msg_player_region_transport param;
	param.source_world_id = player_.world_id();
	ASSERT(player_.GetSpecSavePos(tmp_worldid, tmp_pos));
	ASSERT(IS_NORMAL_MAP(tmp_worldid));
	param.target_world_id = tmp_worldid;
	param.target_pos.x    = tmp_pos.x;
	param.target_pos.y    = tmp_pos.y;
	player_.SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, player_.object_xid(), &param, sizeof(param));
}

bool InsPlayerImp::CheckPlayerCondition() const
{
    if (ins_templ_->ins_type == dataTempl::InstanceTempl::IT_TEAM)
    {
        if (!player_.IsInTeam(ins_team_id_))
            return false;
    }
    return true;
}

void InsPlayerImp::PlayerQuitInsMap()
{
    if (player_.CanSwitch())
    {
        // 主动退出则不保存该副本的信息
        player_.ResetInstanceInfo();
	    KickoutPlayerCountdown(kQuitInsKickoutTime);
    }
}

void InsPlayerImp::HandleInsQueryInfo()
{
	plane_msg_map_query_pinfo_re param;
	param.combat_value = player_.CalcCombatValue();
	player_.SendPlaneMsg(GS_PLANE_MSG_MAP_QUERY_PINFO_RE, &param, sizeof(param));
}

} // namespace gamed
