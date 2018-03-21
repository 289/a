#include "player_map_team.h"

#include "shared/net/packet/bytebuffer.h"
#include "gamed/client_proto/G2C_proto.h"
#include "gs/global/dbgprt.h"
#include "gs/scene/world.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

SHARED_STATIC_ASSERT(PlayerMapTeam::kTeamMemberCount == 4);

namespace {

	void BuildG2CMemberInfo(const map_team_member_info& info, G2C::MapTeamMemberInfo& member)
	{
		member.roleid    = info.roleid;
		member.first_name = info.first_name;
		member.mid_name = info.mid_name;
		member.last_name = info.last_name;
		member.gender    = info.gender;
		member.roleclass = info.cls;
		member.online    = info.online;
	}

} // Anonymous 


///
/// PlayerMapTeam
///
PlayerMapTeam::PlayerMapTeam(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_MAP_TEAM, player),
	  already_getalldata_(false)
{
}

PlayerMapTeam::~PlayerMapTeam()
{
}

void PlayerMapTeam::OnRelease()
{
	already_getalldata_ = false;
	ClearTeamInfo();
}

void PlayerMapTeam::OnLeaveWorld()
{
    G2C::MapTeamLeave packet;
    packet.leave_roleid = player_.role_id();
    SendToClient(packet);
}

void PlayerMapTeam::ClearTeamInfo()
{
	team_info_.clear();
}

bool PlayerMapTeam::IsInTeam() const
{
	return team_info_.has_team();
}

bool PlayerMapTeam::IsInTeam(int32_t teamid) const
{
	if (!IsInTeam())
		return false;

	return team_info_.team_id == teamid;
}

bool PlayerMapTeam::IsTeamLeader() const
{
	return team_info_.leader == player_.role_id();
}

bool PlayerMapTeam::IsTeammate(RoleID roleid) const
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

bool PlayerMapTeam::GetTeammates(std::vector<RoleID>& members_vec) const
{
	if (!IsInTeam())
		return false;

	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		RoleID roleid = team_info_.members[i].roleid;
		if (roleid != 0 && roleid != player_.role_id())
		{
			members_vec.push_back(roleid);
		}
	}
	return true;
}

// 不包含自己
int PlayerMapTeam::GetTeammateCount() const
{
    int count = 0;
    if (IsInTeam())
    {
        for (int i = 0; i < kTeamMemberCount; ++i)
        {
            RoleID roleid = team_info_.members[i].roleid;
            if (roleid != 0 && roleid != player_.role_id())
            {
                ++count;
            }
        }
    }
    return count;
}

int PlayerMapTeam::GetSelfPos()
{
	if (!IsInTeam())
		return -1;

	int index = -1;
	if (!FindMember(player_.role_id(), index))
	{
		LOG_ERROR << "副本组队没有找到自己？";
		return -1;
	}
	return index;
}

int PlayerMapTeam::GetTeamId() const
{
	if (!IsInTeam())
		return -1;

	return team_info_.team_id;
}

RoleID PlayerMapTeam::GetLeaderId() const
{
	return team_info_.leader;
}

void PlayerMapTeam::SendMapTeamInfo() const
{
    if (!IsInTeam())
        return;

    G2C::MapTeamInfo packet;
    packet.teamid   = team_info_.team_id;
    packet.leaderid = team_info_.leader;
    for (size_t i = 0; i < team_info_.members.size(); ++i)
    {
        G2C::MapTeamMemberInfo info;
        BuildG2CMemberInfo(team_info_.members[i], info);
        packet.members.push_back(info);
    }
    SendToClient(packet);
}

void PlayerMapTeam::StartRecvMapTeamInfo()
{
	already_getalldata_ = true;
	SendMapTeamInfo();
}

void PlayerMapTeam::RefreshTeamInfo(int message)
{
	__PRINTF("PlayerMapTeam::RefreshTeamInfo() player:%ld message:%d", player_.role_id(), message);
	SendPlaneMsg(GS_PLANE_MSG_QUERY_MAP_TEAM_INFO);
}

void PlayerMapTeam::SendToClient(shared::net::ProtoPacket& packet) const
{
	if (already_getalldata_ && team_info_.has_team() && player_.world_plane()->HasMapTeam())
	{
		player_.sender()->SendCmd(packet);
	}
}

void PlayerMapTeam::SendPlaneMsg(int message, int64_t param, const void* buf, size_t len)
{
	if (player_.world_plane()->HasMapTeam())
	{
		player_.SendPlaneMsg(message, param, buf, len);
	}
}

void PlayerMapTeam::ErrorMsgToOtherJoin(int err_no)
{
    player_.sender()->ErrorMessageToOther(other_join_req_.requester,
                                          other_join_req_.info.link_id,
                                          other_join_req_.info.sid_in_link,
                                          err_no);
}

void PlayerMapTeam::BuildMapTeamPlayerInfo(map_team_player_info& info)
{
    info.role_id      = player_.role_id();
    info.masterid     = player_.master_id();
    info.link_id      = player_.link_id();
    info.sid_in_link  = player_.sid_in_link();
    info.gender       = player_.gender();
    info.cls          = player_.role_class();
    info.level        = player_.level();
    info.combat_value = player_.CalcCombatValue();
    info.first_name   = player_.first_name();
    info.mid_name     = player_.middle_name();
    info.last_name    = player_.last_name();
}

void PlayerMapTeam::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamChangePos, PlayerMapTeam::CMDHandler_MapTeamChangePos);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamChangeLeader, PlayerMapTeam::CMDHandler_MapTeamChangeLeader);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamQueryMember, PlayerMapTeam::CMDHandler_MapTeamQueryMember);
    REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamJoinTeam, PlayerMapTeam::CMDHandler_MapTeamJoinTeam);
    REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamLeaveTeam, PlayerMapTeam::CMDHandler_MapTeamLeaveTeam);
    REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamKickoutMember, PlayerMapTeam::CMDHandler_MapTeamKickoutMember);
    REGISTER_NORMAL_CMD_HANDLER(C2G::MapTeamJoinTeamRes, PlayerMapTeam::CMDHandler_MapTeamJoinTeamRes);
}

void PlayerMapTeam::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_INFO, PlayerMapTeam::MSGHandler_MapTeamInfo);
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_JOIN, PlayerMapTeam::MSGHandler_MapTeamJoin);
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_LEAVE, PlayerMapTeam::MSGHandler_MapTeamLeave);
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_CHANGE_POS, PlayerMapTeam::MSGHandler_MapTeamChangePos);
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_CHANGE_LEADER, PlayerMapTeam::MSGHandler_MapTeamChangeLeader);
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_QUERY_MEMBER, PlayerMapTeam::MSGHandler_MapTeamQueryMember);
	REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_STATUS_CHANGE, PlayerMapTeam::MSGHandler_MapTeamStatusChange);
    REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_JOIN_TEAM, PlayerMapTeam::MSGHandler_MapTeamJoinTeam);
    REGISTER_MSG_HANDLER(GS_MSG_MAP_TEAM_TIDY_POS, PlayerMapTeam::MSGHandler_MapTeamTidyPos);
}

void PlayerMapTeam::CMDHandler_MapTeamChangePos(const C2G::MapTeamChangePos& cmd)
{
	if (!IsInTeam())
		return;

	if (!CheckMembersIndex(cmd.src_index) || 
		!CheckMembersIndex(cmd.des_index))
	{
		return;
	}

	if (!IsTeamLeader() && 
		!team_info_.members[cmd.des_index].is_vacancy())
	{
		return;
	}

	plane_msg_map_team_change_pos param;
	param.src_index = cmd.src_index;
	param.des_index = cmd.des_index;
	SendPlaneMsg(GS_PLANE_MSG_MAP_TEAM_CHANGE_POS, 0, &param, sizeof(param));
}

void PlayerMapTeam::CMDHandler_MapTeamChangeLeader(const C2G::MapTeamChangeLeader& cmd)
{
	G2C::MapTeamChangeLeader_Re packet;
	packet.new_leader = 0;

	if (!IsInTeam())
	{
		packet.error = G2C::CLE_TEAM_NOTEXIST;
		SendToClient(packet);
		return;
	}
	
	if (!IsTeamLeader())
	{
		packet.error = G2C::CLE_NOT_LEADER;
		SendToClient(packet);
		return;
	}
	
	if (!IsTeammate(cmd.new_leader))
	{
		packet.error = G2C::CLE_NOT_TEAMMATE;
		SendToClient(packet);
		return;
	}

	SendPlaneMsg(GS_PLANE_MSG_MAP_TEAM_CHANGE_LEADER, cmd.new_leader);
}

void PlayerMapTeam::CMDHandler_MapTeamQueryMember(const C2G::MapTeamQueryMember& cmd)
{
	if (!IsInTeam())
		return;

	msg_map_team_query_member send_param;
	send_param.linkid      = player_.link_id();
	send_param.sid_in_link = player_.sid_in_link();
	for (size_t i = 0; i < team_info_.members.size(); ++i)
	{
		RoleID roleid = team_info_.members[i].roleid;
		if (roleid == player_.role_id() || roleid <= 0)
			continue;

		XID target;
		MAKE_XID(roleid, target);
		player_.SendMsg(GS_MSG_MAP_TEAM_QUERY_MEMBER, target, &send_param, sizeof(send_param));
	}
}

void PlayerMapTeam::CMDHandler_MapTeamJoinTeam(const C2G::MapTeamJoinTeam& cmd)
{
    if (IS_INS_MAP(player_.world_id()))
    {
        player_.sender()->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
        return;
    }

    msgpack_map_team_join_team param;
    if (IsInTeam()) 
    {
        param.invite = true;
    }
    else 
    {
        param.invite = false;
    }
    BuildMapTeamPlayerInfo(param.info);

    XID target;
    MAKE_XID(cmd.other_roleid, target);
    if (target.IsPlayer())
    {
        shared::net::ByteBuffer buf;
        MsgContentMarshal(param, buf);
        player_.SendMsg(GS_MSG_MAP_TEAM_JOIN_TEAM, target, buf.contents(), buf.size());
    }
}

void PlayerMapTeam::CMDHandler_MapTeamJoinTeamRes(const C2G::MapTeamJoinTeamRes& cmd)
{
    if (IS_INS_MAP(player_.world_id()))
    {
        player_.sender()->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
        return;
    }

    // 检查是不是原来那个请求者
    if (!other_join_req_.check_requester(cmd.requester))
    {
        return;
    }

    msgpack_map_team_apply_for_join apply_param;

    // 对方是提出邀请
    if (cmd.invite)
    {
        if (IsInTeam())
        {
            player_.sender()->ErrorMessage(G2C::ERR_ALREADY_IN_TEAM);
            return;
        }

        if (!cmd.accept)
        {
            ErrorMsgToOtherJoin(G2C::ERR_TEAM_APPLY_NOT_AGREE);
            return;
        }

        // 自己同意入队
        apply_param.applicant  = player_.role_id();
        apply_param.respondent = cmd.requester;
        apply_param.resp_info  = other_join_req_.info;
        BuildMapTeamPlayerInfo(apply_param.info);
    }
    else // 对方提出是申请
    {
        if (!IsInTeam())
        {
            player_.sender()->ErrorMessage(G2C::ERR_ALREADY_NOT_IN_TEAM);
            return;
        }

        if (team_info_.get_vacancy() <= 0)
        {
            ErrorMsgToOtherJoin(G2C::ERR_THE_TEAM_IS_FULL);
            return;
        }

        if (!cmd.accept)
        {
            ErrorMsgToOtherJoin(G2C::ERR_TEAM_INVITE_NOT_AGREE);
            return;
        }

        // 同意对方入队
        apply_param.applicant  = cmd.requester;
        apply_param.respondent = player_.role_id();
        apply_param.info       = other_join_req_.info;
        BuildMapTeamPlayerInfo(apply_param.resp_info);
    }

    shared::net::ByteBuffer buf;
    MsgContentMarshal(apply_param, buf);
    player_.SendPlaneMsg(GS_PLANE_MSG_MT_APPLY_FOR_JOIN, buf.contents(), buf.size());
}

void PlayerMapTeam::CMDHandler_MapTeamLeaveTeam(const C2G::MapTeamLeaveTeam& cmd)
{
    if (IS_INS_MAP(player_.world_id()))
    {
        player_.sender()->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
        return;
    }

    if (!IsInTeam())
        return;

    player_.SendPlaneMsg(GS_PLANE_MSG_MT_LEAVE_TEAM);
}

void PlayerMapTeam::CMDHandler_MapTeamKickoutMember(const C2G::MapTeamKickoutMember& cmd)
{
    if (IS_INS_MAP(player_.world_id()))
    {
        player_.sender()->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
        return;
    }

    if (!IsInTeam())
        return;

    if (!IsTeamLeader())
        return;

    if (!IsTeammate(cmd.kicked_roleid))
        return;

    player_.SendPlaneMsg(GS_PLANE_MSG_MT_KICKOUT_MEMBER, cmd.kicked_roleid);
}

int PlayerMapTeam::MSGHandler_MapTeamInfo(const MSG& msg)
{
	team_info_.clear();
	MsgContentUnmarshal(msg, team_info_);

	if (already_getalldata_) 
	{
		SendMapTeamInfo();
	}
	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamJoin(const MSG& msg)
{
	msgpack_map_team_join param;
	MsgContentUnmarshal(msg, param);

	if (!IsInTeam() || !CheckMembersIndex(param.pos))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	// 自己不会收到JoinTeam
	if (player_.role_id() == param.info.roleid ||
		team_info_.members[param.pos].roleid > 0)
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	team_info_.members[param.pos] = param.info;

	// send to client
	G2C::MapTeamJoin packet;
	packet.member_pos = param.pos;
	BuildG2CMemberInfo(team_info_.members[param.pos], packet.new_member);
	SendToClient(packet);
	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamLeave(const MSG& msg)
{
	RoleID leave_roleid = msg.param;
	if (!IsInTeam())
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	// send to client
    G2C::MapTeamLeave packet;
	packet.leave_roleid = leave_roleid;

	int index = 0;
	if (FindMember(leave_roleid, index))
	{
		// 自己离队
		if (player_.role_id() == leave_roleid)
		{
		    SendToClient(packet);
			ClearTeamInfo();
		}
		else // 队友离队
		{
		    SendToClient(packet);
			team_info_.members[index].clear();
		}
	}
	else
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamChangePos(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_map_team_change_pos);

	msg_map_team_change_pos& param = *(msg_map_team_change_pos*)msg.content;
	if (!IsInTeam() || !CheckMembersIndex(param.src_index) || !CheckMembersIndex(param.des_index))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	map_team_member_info tmpinfo = team_info_.members[param.des_index];
	team_info_.members[param.des_index] = team_info_.members[param.src_index];
	team_info_.members[param.src_index] = tmpinfo;

	// send to client
	G2C::MapTeamChangePos_Re packet;
	packet.src_index = param.src_index;
	packet.des_index = param.des_index;
	SendToClient(packet);
	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamChangeLeader(const MSG& msg)
{
	RoleID newleader = msg.param;
	if (!IsInTeam() || (!IsTeammate(newleader) && player_.role_id() != newleader))
	{
		RefreshTeamInfo(msg.message);
		return -1;
	}

	// success
	team_info_.leader = newleader;

	// send to client
	G2C::MapTeamChangeLeader_Re packet;
	packet.error      = G2C::CLE_SUCC;
	packet.new_leader = newleader;
	SendToClient(packet);
	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamQueryMember(const MSG& msg)
{
	if (!IsInTeam() || !IsTeammate(msg.source.id))
	{
		__PRINTF("不在队伍里或不是队友，收到MSGHandler_MapTeamQueryMember");
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_map_team_query_member);
	const msg_map_team_query_member& param = *(msg_map_team_query_member*)msg.content;
	player_.sender()->MapTeamQueryMemberRe(msg.source.id, param.linkid, param.sid_in_link); 
	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamStatusChange(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_map_team_status_change);
	const msg_map_team_status_change& param = *(msg_map_team_status_change*)msg.content;

	int pos = 0;
	if (team_info_.find_member(param.roleid, pos))
	{
		team_info_.members[pos].online = param.online;
		G2C::MapTeamStatusChange packet;
		packet.roleid = param.roleid;
		packet.online = param.online;
		SendToClient(packet);
	}
	else
	{
		RefreshTeamInfo(msg.message);
	}

	return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamJoinTeam(const MSG& msg)
{
    msgpack_map_team_join_team param;
	MsgContentUnmarshal(msg, param);

    // 对方是邀请
    if (param.invite)
    {
        if (IsInTeam())
        {
            player_.sender()->ErrorMessageToOther(msg.source.id, param.info.link_id, 
                    param.info.sid_in_link, G2C::ERR_THE_PLAYER_HAS_TEAM);
            return 0;
        }
    }
    else
    {
        if (!IsInTeam())
        {
            param.invite = true;
        }
    }

    other_join_req_.requester = msg.source.id;
    other_join_req_.info      = param.info;
    
    G2C::MapTeamJoinTeamRequest send_packet;
    send_packet.invite     = param.invite;
    send_packet.requester  = msg.source.id;
    send_packet.first_name = param.info.first_name;
    send_packet.mid_name   = param.info.mid_name;
    send_packet.last_name  = param.info.last_name;
    player_.sender()->SendCmd(send_packet); // 不能用SendToClient()，因为是组队请求
    return 0;
}

int PlayerMapTeam::MSGHandler_MapTeamTidyPos(const MSG& msg)
{
    msgpack_map_team_tidy_pos param;
	MsgContentUnmarshal(msg, param);

    G2C::MapTeamTidyPos packet;
    packet.members.resize(param.members.size());

    msgpack_map_team_info tmp_info;
    for (size_t i = 0; i < param.members.size(); ++i)
    {
        packet.members[i] = param.members[i];
        if (param.members[i] <= 0)
            continue;

        int index = i;
        for (size_t k = 0; k < team_info_.members.size(); ++k)
        {
            const map_team_member_info& info = team_info_.members[k];
            if (param.members[index] == info.roleid)
            {
                tmp_info.members[index] = info;
            }
        }
    }

    // 同步team的最新站位到team_info_
    for (size_t i = 0; i < tmp_info.members.size(); ++i)
    {
        team_info_.members[i] = tmp_info.members[i];
    }

    // 同步给客户端
    SendToClient(packet);
    return 0;
}

} // namespace gamed
