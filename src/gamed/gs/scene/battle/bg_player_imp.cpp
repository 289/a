#include "bg_player_imp.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/battleground_templ.h"

#include "gs/global/game_util.h"
#include "gs/global/glogger.h"
#include "gs/scene/world.h"
#include "gs/player/player_sender.h"

#include "bg_player_worldboss.h"


namespace gamed {

using namespace dataTempl;

BGPlayerImp::BGPlayerImp(Player& player)
	: PlayerImp(player),
	  logout_countdown_(GS_INT32_MAX),
	  is_quitting_(false),
      bg_end_(false),
	  bg_templ_(NULL),
      plite_(NULL)
{
}

BGPlayerImp::~BGPlayerImp()
{
    DELETE_SET_NULL(plite_);
}

int BGPlayerImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_PLAYER_QUIT_BG:
			{
				ASSERT(player_.object_xid() == msg.source);
				PlayerQuitBGMap();
			}
			break;

		case GS_MSG_BG_FINISH_RESULT:
			{
				CHECK_CONTENT_PARAM(msg, msg_bg_finish_result);
				const msg_bg_finish_result& param = *(const msg_bg_finish_result*)msg.content;
				HandleBGFinish(param);
                bg_end_ = true;
			}
			break;

		case GS_MSG_MAP_QUERY_PLAYER_INFO:
			{
				HandleBGQueryInfo();
			}
			break;

		default:
			return -1;
	}

	return 0;
}

void BGPlayerImp::OnEnterWorld()
{
	if (!player_.world_plane()->GetBGInfo(bg_info_))
	{
		LOG_ERROR << "战场player_imp进入世界失败！GetBGInfo()失败！";
		ASSERT(false);
		return;
	}

	// find bg template
	bg_templ_ = s_pDataTempl->QueryDataTempl<BattleGroundTempl>(bg_info_.bg_templ_id);
	ASSERT(bg_templ_);
    switch (bg_templ_->bg_type)
    {
        case dataTempl::BattleGroundTempl::BGT_PVE_RALLY:
            plite_ = new BGPlayerLite(*this);
            break;
        
        case dataTempl::BattleGroundTempl::BGT_PVE_WORLD_BOSS:
            plite_ = new BGPlayerWorldBoss(*this);
            break;

        default:
            GLog::log("BGPlayerImp::OnEnterWorld() new BGPlayerLite没有找到对应类型！type:%d", bg_templ_->bg_type);
            plite_ = new BGPlayerLite(*this);
            break;
    }
}

void BGPlayerImp::OnLeaveWorld()
{
    // 停止战斗，离开地图不需要终止战斗
    //player_.TerminateCombat();

    // 离开副本
    player_.sender()->PlayerLeaveBGMap(bg_info_.bg_templ_id);

    // 如果副本结束则清除自己保存的副本信息
    if (bg_end_)
    {
        player_.ResetBattleGroundInfo();
    }
}

void BGPlayerImp::OnHeartbeat()
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
		BGChangeMapLogout();
	}
}

void BGPlayerImp::OnGetAllData()
{
	int32_t bg_create_time = player_.world_plane()->get_create_time();
	player_.sender()->PlayerEnterBGMap(bg_create_time, bg_info_.bg_templ_id);
}

void BGPlayerImp::OnChangeGsComplete()
{
	int32_t bg_create_time = player_.world_plane()->get_create_time();
	player_.sender()->PlayerEnterBGMap(bg_create_time, bg_info_.bg_templ_id);
}

void BGPlayerImp::OnWorldClosing(const MSG& msg)
{
	KickoutPlayerCountdown(kLogoutCountdownTime);
}

void BGPlayerImp::HandleBGQueryInfo()
{
	plane_msg_map_query_pinfo_re param;
	param.combat_value = player_.CalcCombatValue();
	player_.SendPlaneMsg(GS_PLANE_MSG_MAP_QUERY_PINFO_RE, &param, sizeof(param));
}

void BGPlayerImp::PlayerQuitBGMap()
{
    if (player_.CanSwitch())
    {
        // 主动退出则不保存该副本的信息
        player_.ResetBattleGroundInfo();
        KickoutPlayerCountdown(kQuitBGKickoutTime);
    }
}

bool BGPlayerImp::CheckPlayerCondition() const
{
    if (!plite_->OnCheckPlayerCond())
        return false;

	return true;
}

void BGPlayerImp::KickoutPlayerCountdown(int countdown_secs)
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

void BGPlayerImp::KickoutPlayer(int countdown_secs)
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

void BGPlayerImp::ContinuousKickout()
{
	if (is_quitting_ && logout_countdown_ < 0)
	{
		// 持续踢人
		KickoutPlayer(1);
	}
}

void BGPlayerImp::BGChangeMapLogout()
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

void BGPlayerImp::HandleBGFinish(const msg_bg_finish_result& param)
{
    plite_->OnHandleBGFinish();
}

} // namespace gamed
