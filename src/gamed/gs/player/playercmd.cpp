#include "player_ctrl.h"

#include "shared/logsys/logging.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/movement/pmv_session.h"
#include "gs/movement/move_util.h"
#include "gs/global/dbgprt.h"
#include "gs/global/timer.h"
#include "gs/item/item.h"

#include "player.h"
#include "player_sender.h"


namespace gamed {

using namespace mapDataSvr;

void PlayerController::PacketDefaultHandler(const PacketRef packet)
{
	LOG_ERROR << "收到客户端发来的协议[" << packet.GetType() << "] 没有对应的处理函数";
}

#define NORMAL_CMD_HANDLER(packet, handler) \
	ASSERT(packet::TypeNumber() >= C2G_CMD_LOWER_LIMIT && packet::TypeNumber() <= C2G_CMD_UPPER_LIMIT); \
	cmd_disp_.Register<packet>(BIND_MEM_CB(&handler, this));


///
/// register normal cmd handlers
/// 
void PlayerController::StartupNormalCmdDispRegister()
{
	NORMAL_CMD_HANDLER(C2G::StartMove, PlayerController::StartMove);
	NORMAL_CMD_HANDLER(C2G::MoveContinue, PlayerController::MoveContinue);
	NORMAL_CMD_HANDLER(C2G::StopMove, PlayerController::StopMove);
	NORMAL_CMD_HANDLER(C2G::GetAllData, PlayerController::GetAllData);
	NORMAL_CMD_HANDLER(C2G::SelectResurrectPos, PlayerController::SelectResurrectPos);
	NORMAL_CMD_HANDLER(C2G::JoinTeam, PlayerController::JoinTeam);
	NORMAL_CMD_HANDLER(C2G::JoinTeamRes, PlayerController::JoinTeamRes);
	NORMAL_CMD_HANDLER(C2G::OpenCatVision, PlayerController::OpenCatVision);
	NORMAL_CMD_HANDLER(C2G::CloseCatVision, PlayerController::CloseCatVision);
	NORMAL_CMD_HANDLER(C2G::GetElsePlayerExtProp, PlayerController::GetElsePlayerExtProp);
	NORMAL_CMD_HANDLER(C2G::GetElsePlayerEquipCRC, PlayerController::GetElsePlayerEquipCRC);
	NORMAL_CMD_HANDLER(C2G::GetElsePlayerEquipment, PlayerController::GetElsePlayerEquipment);
	NORMAL_CMD_HANDLER(C2G::GetStaticRoleInfo, PlayerController::GetStaticRoleInfo);
	NORMAL_CMD_HANDLER(C2G::QueryNpcZoneInfo, PlayerController::QueryNpcZoneInfo);
	NORMAL_CMD_HANDLER(C2G::QueryServerTime, PlayerController::QueryServerTime);
}


///
/// -------- normal cmd --------
///
bool PlayerController::HandleMoveInCombat(uint16_t seq_from_cli, uint16_t speed, uint8_t mode)
{
	if (player_.InCombat())
	{
		player_.PullBackToValidPos(seq_from_cli);
		player_.sender()->MoveStop(player_.pos(), speed, mode, player_.dir());
		return true;
	}
	return false;
}

void PlayerController::StartMove(const C2G::StartMove& cmd)
{
	A2DVECTOR dest(cmd.dest.x, cmd.dest.y);
	PStartMoveSession* psession = new PStartMoveSession(&player_);
	psession->SetStartMoveInfo(dest, 
			                   cmd.speed, 
							   cmd.mode);
	player_.AddSessionAndStart(psession);
}

void PlayerController::MoveContinue(const C2G::MoveContinue& cmd)
{
	if (HandleMoveInCombat(cmd.cmd_seq, cmd.speed, cmd.mode)) 
		return;
	
	uint16_t use_time = cmd.use_time;
	MOVE_PRINTF("时间:%d", use_time);

	A2DVECTOR cur_pos(cmd.cur_pos.x, cmd.cur_pos.y);
	A2DVECTOR dest(cmd.dest.x, cmd.dest.y);
	PMoveSession* psession = new PMoveSession(&player_);
	psession->SetMoveInfo(cmd.cmd_seq,
			              cmd.speed,
						  cmd.mode,
						  cur_pos,
						  dest,
						  cmd.use_time);
	player_.AddSessionAndStart(psession);
}

void PlayerController::StopMove(const C2G::StopMove& cmd)
{
	if (HandleMoveInCombat(cmd.cmd_seq, cmd.speed, cmd.mode))
		return;

	A2DVECTOR pos(cmd.pos.x, cmd.pos.y);
	PStopMoveSession* psession = new PStopMoveSession(&player_);
	psession->SetStopMoveInfo(cmd.cmd_seq,
			                  cmd.speed,
						      cmd.mode,
						      pos,
						      cmd.dir,
						      cmd.use_time);
	player_.AddSessionAndStart(psession);
}

void PlayerController::GetAllData(const C2G::GetAllData& cmd)
{
	//登陆成功后，客户端请求数据
	//后面会有更多角色数据同步给客户端

    player_.PlayerGetUIConf();
	player_.PlayerGetCash();
	player_.PlayerGetExtendProp();
	player_.PlayerGetInventoryAll();
	player_.PlayerGetStaticRoleInfo();
	player_.PlayerGetBaseInfo();
	player_.PlayerGetCoolDownData();
	player_.PlayerGetSkillData();
	player_.PlayerGetPetData();
	player_.PlayerGetOwnMoney();
	player_.PlayerGetOwnScore();
	player_.PlayerGetBuddyInfo();
	player_.PlayerAnnounceNewMail();
	player_.PlayerGetInsTeamInfo();
	player_.PlayerGetTalentData();
    player_.PlayerGetFriendData();
    player_.PlayerGetTitleData();
    player_.PlayerGetReputationData();
    player_.PlayerGetPropReinforce();
    player_.PlayerGetAchieveData();
    player_.PlayerGetStarData();
    player_.PlayerGetEnhanceData();
    player_.PlayerGetPCounterList();
    player_.PlayerGetPunchCardData();
    player_.PlayerGetMountData();
    player_.PlayerGetGeventData();
    player_.PlayerGetParticipation(); // 需要放在活动系统后
    player_.PlayerGetMoveSpeed();
    player_.PlayerGetGameVersion();
    player_.PlayerGetBossChallengeData();

	// Player里统一的getalldata
	player_.OnGetAllData();

	//注意：考虑客户端初始化顺序，战斗数据需要最后发送;
	player_.PlayerGetCombatData();
	player_.PlayerGetTaskData();

    player_.PlayerGetLoginData();
}

void PlayerController::SelectResurrectPos(const C2G::SelectResurrectPos& cmd)
{
	player_.Resurrect(cmd.pos);
}

void PlayerController::JoinTeam(const C2G::JoinTeam& cmd)
{
	player_.JoinTeamTransmit(cmd.other_roleid);
}

void PlayerController::JoinTeamRes(const C2G::JoinTeamRes& cmd)
{
	player_.JoinTeamResTransmit(cmd.invite, cmd.accept, cmd.requester);
}

void PlayerController::OpenCatVision(const C2G::OpenCatVision& cmd)
{
	player_.OpenCatVision();
}

void PlayerController::CloseCatVision(const C2G::CloseCatVision& cmd)
{
	player_.CloseCatVision();
}

void PlayerController::GetElsePlayerExtProp(const C2G::GetElsePlayerExtProp& cmd)
{
	player_.GetElsePlayerExtProp(cmd.else_player_roleid);
}

void PlayerController::GetElsePlayerEquipCRC(const C2G::GetElsePlayerEquipCRC& cmd)
{
	player_.GetElsePlayerEquipCRC(cmd.else_player_roleid);
}

void PlayerController::GetElsePlayerEquipment(const C2G::GetElsePlayerEquipment& cmd)
{
	player_.GetElsePlayerEquipment(cmd.else_player_roleid);
}

void PlayerController::GetStaticRoleInfo(const C2G::GetStaticRoleInfo& cmd)
{
	XID target;
	MAKE_XID(cmd.roleid, target);

	if (target.IsPlayer())
	{
		msg_get_static_role_info param;
		param.requester   = player_.role_id();
		param.link_id     = player_.link_id();
		param.sid_in_link = player_.sid_in_link();
		player_.SendMsg(GS_MSG_GET_STATIC_ROLE_INFO, target, &param, sizeof(param));
	}
}

void PlayerController::QueryNpcZoneInfo(const C2G::QueryNpcZoneInfo& cmd)
{
	const BaseMapData* pdata = s_pMapData->QueryBaseMapDataTempl(cmd.elem_id);
	if (pdata == NULL || pdata->map_id != player_.world_id())
	{
		__PRINTF("QueryNpcZoneInfo()没有找到对应的地图元素，或者不是本地图的地图元素");
		return;
	}

	plane_msg_query_npc_zone_info param;
	param.elem_id     = cmd.elem_id;
	param.link_id     = player_.link_id();
	param.sid_in_link = player_.sid_in_link();
	player_.SendPlaneMsg(GS_PLANE_MSG_QUERY_NPC_ZONE_INFO, &param, sizeof(param));
}

void PlayerController::QueryServerTime(const C2G::QueryServerTime& cmd)
{
	G2C::ServerTime packet;
	packet.timestamp = g_timer->GetSysTime();
	player_.sender()->SendCmd(packet);
}

} // namespace gamed
