#include "player_team.h"

#include "shared/base/assertx.h"

#include "gs/global/dbgprt.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

// MSGHandler_TeamInfo()里直接用0~3的下标来访问members，改kTeamMemberCount值需谨慎
SHARED_STATIC_ASSERT(PlayerTeam::kTeamMemberCount == 4);


///
/// TeamInfo
///
PlayerTeam::TeamInfo::TeamInfo() 
{
	Release();
}

void PlayerTeam::TeamInfo::Release()
{
	team_id = 0;
	leader  = 0;
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		members[i] = 0;
	}
}


///
/// PlayerTeam
///
PlayerTeam::PlayerTeam(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_TEAM, player)
{
}

PlayerTeam::~PlayerTeam()
{
}

void PlayerTeam::OnRelease()
{
	ClearTeamInfo();
}

void PlayerTeam::ClearTeamInfo()
{
	player_.visible_state().ClrTeamFlag();
	team_info_.Release();
    leader_con_.clear();
}

bool PlayerTeam::IsInTeam() const
{
	return team_info_.team_id > 0;
}

bool PlayerTeam::IsInTeam(int32_t teamid) const
{
	if (!IsInTeam())
		return false;

	return team_info_.team_id == teamid;
}

bool PlayerTeam::IsTeamLeader() const
{
	return team_info_.leader == player_.role_id();
}

bool PlayerTeam::IsTeamLeader(RoleID leader) const
{
    return team_info_.leader == leader;
}

bool PlayerTeam::IsTeammate(RoleID roleid) const
{
	if (!IsInTeam())
		return false;

	// 不包含自己
	if (player_.role_id() == roleid)
		return false;

	int index = 0;
	if (FindMember(roleid, index))
		return true;

	return false;
}

bool PlayerTeam::GetTeammates(std::vector<RoleID>& members_vec) const
{
	if (!IsInTeam())
		return false;

	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		RoleID roleid = team_info_.members[i];
		if (roleid != 0 && roleid != player_.role_id())
		{
			members_vec.push_back(roleid);
		}
	}
	return true;
}

int PlayerTeam::GetTeammateCount() const
{
    int count = 0;
    if (IsInTeam())
    {
        for (int i = 0; i < kTeamMemberCount; ++i)
        {
            RoleID roleid = team_info_.members[i];
            if (roleid != 0 && roleid != player_.role_id())
            {
                ++count;
            }
        }
    }
    return count;
}

int PlayerTeam::GetSelfPos()
{
	if (!IsInTeam())
		return -1;

	int index = -1;
	if (!FindMember(player_.role_id(), index))
	{
		RefreshTeamInfo(-1);
		return -1;
	}
	return index;
}

int PlayerTeam::GetTeamId() const
{
	if (!IsInTeam())
		return -1;

	return team_info_.team_id;
}

void PlayerTeam::LeaveTeamBySelf()
{
	player_.sender()->LeaveTeam();
	ClearTeamInfo();
}

RoleID PlayerTeam::GetLeaderId() const
{
	return team_info_.leader;
}

void PlayerTeam::RefreshTeamInfo(int message)
{
	__PRINTF("RefreshTeamInfo() player:%ld message:%d", player_.role_id(), message);
	player_.sender()->QueryTeamInfo();
}

void PlayerTeam::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::ConveneTeammate, PlayerTeam::CMDHandler_ConveneTeammate);
    REGISTER_NORMAL_CMD_HANDLER(C2G::ConveneResponse, PlayerTeam::CMDHandler_ConveneResponse);
}

void PlayerTeam::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_JOIN_TEAM, PlayerTeam::MSGHandler_JoinTeam);
	REGISTER_MSG_HANDLER(GS_MSG_LEAVE_TEAM, PlayerTeam::MSGHandler_LeaveTeam);
	REGISTER_MSG_HANDLER(GS_MSG_CHANGE_TEAM_LEADER, PlayerTeam::MSGHandler_ChangeTeamLeader);
	REGISTER_MSG_HANDLER(GS_MSG_CHANGE_TEAM_POS, PlayerTeam::MSGHandler_ChangeTeamPos);
	REGISTER_MSG_HANDLER(GS_MSG_TEAM_INFO, PlayerTeam::MSGHandler_TeamInfo);
	REGISTER_MSG_HANDLER(GS_MSG_QUERY_TEAM_MEMBER, PlayerTeam::MSGHandler_QueryTeamMember);
}

/// MsgHandler
int PlayerTeam::MSGHandler_JoinTeam(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_join_team);
	msg_join_team& param = *(msg_join_team*)msg.content;
	if (!IsInTeam() || !CheckMembersIndex(param.pos))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	// 自己不会收到JoinTeam
	if (player_.role_id() == param.new_memberid ||
		team_info_.members[param.pos] > 0)//team_info_.members[param.pos])
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	team_info_.members[param.pos] = param.new_memberid;
	return 0;
}

int PlayerTeam::MSGHandler_LeaveTeam(const MSG& msg)
{
	RoleID leave_roleid = msg.param;
	// 自己离队，直接清除
	if (player_.role_id() == leave_roleid)
	{
		ClearTeamInfo();
		return 0;
	}

	if (!IsInTeam())
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	int index = 0;
	if (FindMember(leave_roleid, index))
	{
		// 自己离队
		if (player_.role_id() == leave_roleid)
		{
			ClearTeamInfo();
		}
		else // 队友离队
		{
			team_info_.members[index] = 0;
		}
	}
	else
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}
	return 0;
}

int PlayerTeam::MSGHandler_ChangeTeamLeader(const MSG& msg)
{
	RoleID new_leaderid = msg.param;
	if (!IsInTeam())
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	// 不是自己也不是队友
	if (player_.role_id() != new_leaderid && !IsTeammate(new_leaderid))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	team_info_.leader = new_leaderid;
	return 0;
}

int PlayerTeam::MSGHandler_ChangeTeamPos(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_change_team_pos);
	msg_change_team_pos& param = *(msg_change_team_pos*)msg.content;
	if (!IsInTeam() || !CheckMembersIndex(param.src_index) || !CheckMembersIndex(param.des_index))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	RoleID tmpid = team_info_.members[param.des_index];
	team_info_.members[param.des_index] = team_info_.members[param.src_index];
	team_info_.members[param.src_index] = tmpid;
	return 0;
}

int PlayerTeam::MSGHandler_TeamInfo(const MSG& msg)
{
	// 已经和npc组队，则自己离开队伍
	if (!player_.CanJoinPlayerTeam())
	{
		LeaveTeamBySelf();
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_team_info);
	msg_team_info& param  = *(msg_team_info*)msg.content;
	team_info_.team_id    = param.teamid;
	team_info_.leader     = param.leaderid;
	team_info_.members[0] = param.pos1;
	team_info_.members[1] = param.pos2;
	team_info_.members[2] = param.pos3;
	team_info_.members[3] = param.pos4;

	player_.visible_state().SetTeamFlag(param.teamid);
	return 0;
}

int PlayerTeam::MSGHandler_QueryTeamMember(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_query_team_member);
	msg_query_team_member& param = *(msg_query_team_member*)msg.content;

	// 自己没必要查，不是队友不能查
	if (player_.role_id() == param.query_roleid || !IsTeammate(param.query_roleid))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	player_.sender()->TeamMemberQueryRe(param.query_roleid, param.link_id, param.client_sid_in_link);
	return 0;
}

void PlayerTeam::CMDHandler_ConveneTeammate(const C2G::ConveneTeammate& cmd)
{
    // ????????????????? check cool down
    
    if (!IsTeamLeader())
    {
        player_.sender()->ErrorMessage(G2C::ERR_IS_NOT_TEAM_LEADER);
        return;
    }

    if (!IS_NORMAL_MAP(player_.world_id()))
    {
        player_.sender()->ErrorMessage(G2C::ERR_THIS_MAP_CAN_NOT_EXE);
        return;
    }

    if (GetTeammateCount() <= 1)
    {
        // 队伍里只有自己一个人
        return;
    }

    player_.sender()->ConveneTeammate();
}

void PlayerTeam::CMDHandler_ConveneResponse(const C2G::ConveneResponse& cmd)
{
    if (!leader_con_.has_convene())
    {
        player_.sender()->ErrorMessage(G2C::ERR_TEAM_LEADER_CONVENE);
        return;
    }

    if (!IsInTeam())
    {
        player_.sender()->ErrorMessage(G2C::ERR_NOT_IN_TEAM);
        leader_con_.clear();
        return;
    }

    if (!IsTeamLeader(leader_con_.convened_leader))
    {
        player_.sender()->ErrorMessage(G2C::ERR_TEAM_LEADER_CHANGE);
        leader_con_.clear();
        return;
    }

    if (cmd.agree)
    {
        msg_player_region_transport param;
        param.source_world_id = player_.world_id();
        param.target_world_id = leader_con_.world_id;
        param.target_pos.x    = leader_con_.pos.x;
        param.target_pos.y    = leader_con_.pos.y;
        player_.SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, player_.object_xid(), &param, sizeof(param));
    }

    // 同意与否都重置召集信息
    leader_con_.clear();
}

void PlayerTeam::TeamLeaderConvene(RoleID leader, int32_t world_id, const A2DVECTOR& pos)
{
    if (!IsTeamLeader(leader))
        return;

    if (!IS_NORMAL_MAP(player_.world_id()))
        return;

    leader_con_.convened_leader = leader;
    leader_con_.world_id        = world_id;
    leader_con_.pos             = pos;

    G2C::TeamLeaderConvene packet;
    player_.sender()->SendCmd(packet);
}

} // namespace gamed

