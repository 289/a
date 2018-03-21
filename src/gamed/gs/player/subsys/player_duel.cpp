#include "player_duel.h"

#include "gs/global/dbgprt.h"
#include "gs/scene/world.h"
#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"


namespace gamed {

namespace {

    inline void BuildDuelMemberInfo(const std::vector<duel_team_info>& data, std::vector<G2C::DuelMemberInfo>& info_vec)
    {
        for (size_t i = 0; i < data.size(); ++i)
        {
            G2C::DuelMemberInfo tmpinfo;
            tmpinfo.role_id      = data[i].role_id;
            tmpinfo.combat_value = data[i].combat_value;
            info_vec.push_back(tmpinfo);
        }
    }

} // Anonymous

///
/// PlayerDuel
///
PlayerDuel::PlayerDuel(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_DUEL, player)
{
}

PlayerDuel::~PlayerDuel()
{
}

void PlayerDuel::OnRelease()
{
    duel_requester_.clear();
    waiting_requester_.clear();
    request_info_.clear();
}

void PlayerDuel::OnHeartbeat(time_t cur_time)
{
    if (request_info_.is_countdown())
    {
        if (--request_info_.time_out <= 0)
        {
            HandleDuelStartCombat();
        }
    }

    if (duel_requester_.is_countdown())
    {
        if (--duel_requester_.time_out <= 0)
        {
            duel_requester_.id = 0;
        }
    }
}

void PlayerDuel::DuelCombatEnd(const playerdef::PVPEndInfo& info, playerdef::PVPEndExtraData& data)
{
    if (info.creator == player_.role_id())
    {
        // ???????????????????? 喊话
    }

    data.remain_hp = 1;

    // clear info
    duel_requester_.id = 0;
    request_info_.clear();
}

void PlayerDuel::HandleDuelStartCombat()
{
    // 再次做检查
    if (!CheckDuelCondition(request_info_.duel_roleid))
    {
        request_info_.clear();
        return;
    }
     
    //
    // 检查成功
    //
    int32_t combat_scene_id = player_.GetCombatSceneID();
    playerdef::StartPVPResult result;
    if (player_.StartPVPCombat(combat_scene_id, playerdef::PVP_TYPE_DUEL, result))
    {
        XID target = MAKE_XID(request_info_.duel_roleid);
        ASSERT(target.IsPlayer());
        player_.SendMsg(GS_MSG_START_JOIN_DUEL_COMBAT, target, result.combat_id);
    }
    else
    {
        request_info_.clear();
        player_.sender()->ErrorMessage(G2C::ERR_CREATE_COMBAT_FAILURE);
    }
}

void PlayerDuel::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::DuelRequest, PlayerDuel::CMDHandler_DuelRequest);
    REGISTER_NORMAL_CMD_HANDLER(C2G::DuelRequest_Re, PlayerDuel::CMDHandler_DuelRequestReply);
    REGISTER_NORMAL_CMD_HANDLER(C2G::TeammateDuelRequest_Re, PlayerDuel::CMDHandler_TeammateDuelRequestReply);
}

void PlayerDuel::RegisterMsgHandler()
{
    REGISTER_MSG_HANDLER(GS_MSG_DUEL_REQUEST, PlayerDuel::MSGHandler_DuelRequest);
    REGISTER_MSG_HANDLER(GS_MSG_DUEL_REQUEST_RE, PlayerDuel::MSGHandler_DuelRequestReply);
    REGISTER_MSG_HANDLER(GS_MSG_TEAMMATE_DUEL_REQUEST, PlayerDuel::MSGHandler_TeammateDuelRequest);
    REGISTER_MSG_HANDLER(GS_MSG_TEAMMATE_DUEL_REQUEST_RE, PlayerDuel::MSGHandler_TeammateDuelRequestReply);
    REGISTER_MSG_HANDLER(GS_MSG_DUEL_PREPARE, PlayerDuel::MSGHandler_DuelPrepare);
    REGISTER_MSG_HANDLER(GS_MSG_START_JOIN_DUEL_COMBAT, PlayerDuel::MSGHandler_StartJoinDuelCombat);
}

bool PlayerDuel::CheckDuelCondition(RoleID duel_roleid)
{
    world::player_base_info info;
    if (player_.world_plane()->QueryPlayer(duel_roleid, info))
    {
        // 检查对方是否已经在战斗中
        if (!info.can_combat)
        {
            player_.sender()->ErrorMessage(G2C::ERR_ALREADY_IN_COMBAT);
            return false;
        }

        // 检查距离
        if (info.pos.squared_distance(player_.pos()) > kMaxPlayerDuelDisSquare)
        {
            player_.sender()->ErrorMessage(G2C::ERR_THE_PEER_IS_LEAVE);
            return false;
        }
    }
    else
    {
        player_.sender()->ErrorMessage(G2C::ERR_THE_PEER_IS_LEAVE);
        return false;
    }

    return true;
}

void PlayerDuel::HandleTeamDuelRequest(RoleID duel_roleid)
{
    if (!CheckDuelCondition(duel_roleid))
        return;

    XID target = MAKE_XID(duel_roleid);
    msg_duel_request param;
    param.requester       = player_.role_id();
    param.is_team_req     = true;
    param.requester_pos_x = player_.pos().x;
    param.requester_pos_y = player_.pos().y;
    player_.SendMsg(GS_MSG_DUEL_REQUEST, target, &param, sizeof(param));
}

void PlayerDuel::HandleSoloDuelRequest(RoleID duel_roleid)
{
    if (!CheckDuelCondition(duel_roleid))
        return;

    XID target = MAKE_XID(duel_roleid);
    msg_duel_request param;
    param.requester       = player_.role_id();
    param.is_team_req     = false;
    param.requester_pos_x = player_.pos().x;
    param.requester_pos_y = player_.pos().y;
    player_.SendMsg(GS_MSG_DUEL_REQUEST, target, &param, sizeof(param));
}

void PlayerDuel::RequestTransmitToLeader(RoleID duel_roleid)
{
    RoleID leader = player_.GetLeaderId();
    world::player_base_info info;
    if (!player_.world_plane()->QueryPlayer(leader, info) ||
        maths::squared_distance(player_.pos(), info.pos) > kMaxPlayerDuelDisSquare)
    {
        // 距离太远
        player_.sender()->ErrorMessage(G2C::ERR_LEADER_TOO_FAR);
        return;
    }

    if (!info.can_combat)
    {
        player_.sender()->ErrorMessage(G2C::ERR_LEADER_BUSY);
        return;
    }

    XID target = MAKE_XID(leader);
    player_.SendMsg(GS_MSG_TEAMMATE_DUEL_REQUEST, target, duel_roleid);
}

void PlayerDuel::CMDHandler_DuelRequest(const C2G::DuelRequest& cmd)
{
    if (!player_.CanCombat())
        return;

    if (player_.IsInTeam())
    {
        if (!player_.IsTeamLeader())
        {
            RequestTransmitToLeader(cmd.duel_roleid);
            return;
        }

        if (player_.IsTeammate(cmd.duel_roleid))
        {
            // 队友之间不能决斗
            player_.sender()->ErrorMessage(G2C::ERR_TEAMMATE_CAN_NOT_DUEL);
            return;
        }

        HandleTeamDuelRequest(cmd.duel_roleid);
    }
    else
    {
        HandleSoloDuelRequest(cmd.duel_roleid);
    }
}

void PlayerDuel::CMDHandler_DuelRequestReply(const C2G::DuelRequest_Re& cmd)
{
    DuelRequesterSet::iterator it = waiting_requester_.find(cmd.requester_roleid);
    if (it == waiting_requester_.end())
    {
        __PRINTF("CMDHandler_DuelRequestReply error cmd.requester_roleid=%ld", cmd.requester_roleid);
        return;
    }
    waiting_requester_.erase(it);

    if (duel_requester_.id != 0)
    {
        __PRINTF("CMDHandler_DuelRequestReply duel_requester_.id != 0");
        player_.sender()->ErrorMessage(G2C::ERR_DUEL_ALREADY_START);
        return;
    }

    // 检查玩家距离
    world::player_base_info info;
    if (player_.world_plane()->QueryPlayer(cmd.requester_roleid, info))
    {
        if (maths::squared_distance(player_.pos(), info.pos) > kMaxPlayerDuelDisSquare)
        {
            player_.sender()->ErrorMessage(G2C::ERR_TOO_FAR_FOR_DUEL);
            return;
        }
    }
    else
    {
        player_.sender()->ErrorMessage(G2C::ERR_THE_PEER_IS_LEAVE);
        return;
    }

    // success
    msg_duel_request_re send_param;
    if (cmd.agreement && player_.CanCombat())
    {
        send_param.duel_result   = DRR_SUCCESS;
        duel_requester_.id       = cmd.requester_roleid;
        duel_requester_.time_out = kDuelPrepareTime + 2;
        waiting_requester_.clear();
    }
    else
    {
        send_param.duel_result = DRR_NOT_AGREE;
    }

    XID target = MAKE_XID(cmd.requester_roleid);
    player_.SendMsg(GS_MSG_DUEL_REQUEST_RE, target, &send_param, sizeof(send_param));
}

int PlayerDuel::MSGHandler_DuelRequest(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, msg_duel_request);
	const msg_duel_request& param = *(const msg_duel_request*)msg.content;

    if (duel_requester_.id != 0)
    {
        __PRINTF("玩家作为决斗接收方已经开始决斗！");
        world::player_base_info info;
		if (player_.world_plane()->QueryPlayer(msg.source.id, info))
		{
            player_.sender()->ErrorMessageToOther(info.xid.id, info.link_id, info.sid_in_link, 
                                                  G2C::ERR_DUEL_ALREADY_START);
        }
        return 0;
    }

    A2DVECTOR requester_pos;
    requester_pos.x = param.requester_pos_x;
    requester_pos.y = param.requester_pos_y;

    if (player_.IsInTeam() && !player_.IsTeamLeader())
    {
        RoleID leader = player_.GetLeaderId();
        world::player_base_info info;
        if (!player_.world_plane()->QueryPlayer(leader, info) ||
            maths::squared_distance(requester_pos, info.pos) > kMaxPlayerDuelDisSquare)
        {
            // 距离太远，队长不在附近
            msg_duel_request_re send_param;
            send_param.duel_result = DRR_LEADER_NOT_AROUND;
            XID target = MAKE_XID(param.requester);
            player_.SendMsg(GS_MSG_DUEL_REQUEST_RE, target, &send_param, sizeof(send_param));
            return 0;
        }

        // 队长在周围，转给队长
        player_.SendMsg(GS_MSG_DUEL_REQUEST, info.xid, &param, sizeof(param));
    }
    else
    {
        if (maths::squared_distance(requester_pos, player_.pos()) > kMaxPlayerDuelDisSquare)
        {
            // 距离太远
            msg_duel_request_re send_param;
            send_param.duel_result = DRR_TOO_FAR;
            XID target = MAKE_XID(param.requester);
            player_.SendMsg(GS_MSG_DUEL_REQUEST_RE, target, &send_param, sizeof(send_param));
            return 0;
        }

        G2C::DuelRequest packet;
        packet.duel_roleid  = param.requester;
        packet.is_team_duel = param.is_team_req;
        player_.sender()->SendCmd(packet);

        // 记录发起者
        waiting_requester_.insert(param.requester);
    }
    return 0;
}

void PlayerDuel::CMDHandler_TeammateDuelRequestReply(const C2G::TeammateDuelRequest_Re& cmd)
{
    if (!player_.IsTeamLeader())
    {
        __PRINTF("玩家已经不能队长CMDHandler_TeammateDuelRequestReply");
        return;
    }

    if (!player_.IsTeammate(cmd.teammate_roleid))
    {
        __PRINTF("该玩家已经不是队友CMDHandler_TeammateDuelRequestReply");
        return;
    }

    msg_teammate_duel_request_re param;
    param.agreement   = cmd.agreement;
    param.duel_roleid = cmd.duel_roleid;
    XID target = MAKE_XID(cmd.teammate_roleid);
    player_.SendMsg(GS_MSG_TEAMMATE_DUEL_REQUEST_RE, target, &param, sizeof(param));
}

int PlayerDuel::MSGHandler_DuelRequestReply(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, msg_duel_request_re);
    const msg_duel_request_re& param = *(const msg_duel_request_re*)msg.content;

    if (request_info_.is_countdown())
    {
        __PRINTF("玩家作为决斗发起方已经开始倒计时！");
        world::player_base_info info;
		if (player_.world_plane()->QueryPlayer(msg.source.id, info))
		{
            player_.sender()->ErrorMessageToOther(info.xid.id, info.link_id, info.sid_in_link, 
                                                  G2C::ERR_DUEL_ALREADY_START);
        }
        return 0;
    }

    G2C::DuelRequest_Re packet;
    switch (param.duel_result)
    {
        case DRR_SUCCESS:
            packet.result = G2C::DuelRequest_Re::RT_SUCCESS;
            break;

        case DRR_NOT_AGREE:
            packet.result = G2C::DuelRequest_Re::RT_NOT_AGREE;
            break;

        case DRR_LEADER_NOT_AROUND:
            packet.result = G2C::DuelRequest_Re::RT_LEADER_NOT_AROUND;
            break;

        case DRR_TOO_FAR:
            packet.result = G2C::DuelRequest_Re::RT_TOO_FAR;
            break;

        default:
            ASSERT(false);
            return -1;
    }

    if (param.duel_result == DRR_SUCCESS)
    {
        request_info_.time_out    = kDuelPrepareTime;
        request_info_.duel_roleid = msg.source.id;
        SendDuelPrepareMsg(msg.source);
    }

    player_.sender()->SendCmd(packet);
    return 0;
}

void PlayerDuel::SendDuelPrepareMsg(const XID& target)
{
    msgpack_duel_prepare param;
    QueryDuelMembers(param.members);

    shared::net::ByteBuffer buf;
    MsgContentMarshal(param, buf);
    player_.SendMsg(GS_MSG_DUEL_PREPARE, target, buf.contents(), buf.size());
}

int PlayerDuel::MSGHandler_TeammateDuelRequest(const MSG& msg)
{
    if (!player_.IsTeammate(msg.source.id))
    {
        __PRINTF("不是队友怎么会收到TeammateDuelRequest MSG？");
        return 0;
    }

    G2C::TeammateDuelRequest packet;
    packet.duel_roleid     = msg.param;
    packet.teammate_roleid = msg.source.id;
    player_.sender()->SendCmd(packet);
    return 0;
}

int PlayerDuel::MSGHandler_TeammateDuelRequestReply(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, msg_teammate_duel_request_re);
    const msg_teammate_duel_request_re& param = *(const msg_teammate_duel_request_re*)msg.content;

    G2C::TeammateDuelRequest_Re packet;
    packet.agreement = param.agreement;
    player_.sender()->SendCmd(packet);

    if (param.agreement)
    {
        HandleTeamDuelRequest(param.duel_roleid);
    }
    return 0;
}

int PlayerDuel::MSGHandler_DuelPrepare(const MSG& msg)
{
    // unmarshal
    msgpack_duel_prepare param;
    MsgContentUnmarshal(msg, param);

    // 查找队友
    DuelTeamInfoVec duel_team;
    QueryDuelMembers(duel_team);

    // 发给本队成员
    {
        G2C::DuelPrepare packet;
        packet.countdown_secs = kDuelPrepareTime - 1;
        BuildDuelMemberInfo(duel_team, packet.own_side);
        BuildDuelMemberInfo(param.members, packet.enemy_side);
        for (size_t i = 0; i < duel_team.size(); ++i)
        {
            const duel_team_info& info = duel_team[i];
            player_.sender()->PlayerDuelPacket(info.role_id, info.link_id, info.sid_in_link, packet);
        }
    }

    // 发给对方成员
    {
        G2C::DuelPrepare packet;
        packet.countdown_secs = kDuelPrepareTime - 1;
        BuildDuelMemberInfo(param.members, packet.own_side);
        BuildDuelMemberInfo(duel_team, packet.enemy_side);
        for (size_t i = 0; i < param.members.size(); ++i)
        {
            const duel_team_info& info = param.members[i];
            player_.sender()->PlayerDuelPacket(info.role_id, info.link_id, info.sid_in_link, packet);
        }
    }

    return 0;
}

void PlayerDuel::QueryDuelMembers(DuelTeamInfoVec& team_info)
{
    // 把自己添加到信息里
    duel_team_info tmpdata;
    tmpdata.role_id      = player_.role_id();
    tmpdata.link_id      = player_.link_id();
    tmpdata.sid_in_link  = player_.sid_in_link();
    tmpdata.combat_value = player_.GetCombatValue();
    team_info.push_back(tmpdata);

    // 查找队友
    std::vector<RoleID> members_vec;
	if (!player_.GetTeammates(members_vec))
		return;

    for (size_t i = 0; i < members_vec.size(); ++i)
	{
		world::player_base_info info;
		if (player_.world_plane()->QueryPlayer(members_vec[i], info))
		{
			// 检查距离
			if (info.can_combat && info.pos.squared_distance(player_.pos()) < kMaxPlayerDuelDisSquare)
			{
                duel_team_info data;
                data.role_id      = info.xid.id;
                data.link_id      = info.link_id;
                data.sid_in_link  = info.sid_in_link;
                data.combat_value = info.combat_value;
                team_info.push_back(data);
			}
		}
	}
}

int PlayerDuel::MSGHandler_StartJoinDuelCombat(const MSG& msg)
{
    if (duel_requester_.id != msg.source.id)
    {
        __PRINTF("决斗请求者不是之前那个？MSGHandler_StartJoinDuelCombat");
        return 0;
    }
    
    if (!player_.FirstJoinPVPCombat((int32_t)msg.param))
    {
        __PRINTF("决斗FirstJoinPVPCombat失败，MSGHandler_StartJoinDuelCombat");
        player_.sender()->ErrorMessage(G2C::ERR_JOIN_DUEL_COMBAT_FAIL);
        return 0;
    }

    return 0;
}

} // namespace gamed
