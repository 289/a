#include "base_wm_imp.h"

#include "gamed/client_proto/G2C_proto.h"
#include "gamed/client_proto/G2C_error.h"

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/monster_templ.h"
#include "gs/netmsg/send_to_link.h"
#include "gs/global/timer.h"
#include "gs/global/game_util.h"
#include "gs/global/message.h"
#include "gs/global/msg_plane.h"
#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/randomgen.h"
#include "gs/global/team_util.h"
#include "gs/player/player.h"

// proto
#include "common/protocol/gen/M2G/team_msg.pb.h"


namespace gamed {

using namespace mapDataSvr;
using namespace dataTempl;
using namespace common::protocol;

#define FIND_PLAYER_IN_TEAM_POS(roleid, teaminfo, pos) \
    { \
        teaminfo = NULL; \
        MapTeamInfoMap::iterator it = team_info_map_.begin(); \
        for (; it != team_info_map_.end(); ++it)  \
        { \
            if (it->second.find_member(roleid, pos)) { \
                teaminfo = &(it->second); \
            } \
        } \
    }

#define FIND_PLAYER_IN_TEAM(roleid, teaminfo) \
    { \
        int pos = 0; \
        FIND_PLAYER_IN_TEAM_POS(roleid, teaminfo, pos); \
    }

#define FIND_PLAYER_IN_TEAM_RETURN(roleid, teaminfo) \
    { \
        FIND_PLAYER_IN_TEAM(roleid, teaminfo); \
        if (teaminfo == NULL) \
            return; \
    }


//
// BaseWorldManImp
//
BaseWorldManImp::BaseWorldManImp(WorldManager& worldMan)
	: WorldManagerImp(worldMan),
      cur_team_id_(0),
      query_pinfo_timeout_(GS_INT32_MAX),
	  check_clock_timeup_(GS_INT32_MAX)
{
}

BaseWorldManImp::~BaseWorldManImp()
{
    cur_team_id_          = 0;
	query_pinfo_timeout_  = GS_INT32_MAX;
}

bool BaseWorldManImp::OnInit()
{
    // set world has map team
    world_man_.SetMapTeamFlag();

    // set world has map counter
    world_man_.SetMapCounterFlag();

    // set world has gather event
    world_man_.SetMapGatherFlag();

	// query timeout
    ResetQueryPInfoTimeout();
    return true;
}

void BaseWorldManImp::OnHeartbeat()
{
    // clock
	time_t now = g_timer->GetSysTime();
	CheckClockTimeup(now);
}

bool BaseWorldManImp::OnInsertPlayer(Player* pplayer)
{
    // lock
	MutexLockGuard lock(mutex_team_);
    if (!PlayerEnterMapTeam(pplayer))
    {
        return false;
    }

    return true;
}

void BaseWorldManImp::OnPlayerLeaveMap(RoleID roleid)
{
    // lock
	MutexLockGuard lock(mutex_team_);
    PlayerOffline(roleid);
}

int BaseWorldManImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
    {
        case GS_PLANE_MSG_MAP_MONSTER_KILLED:
			{
				msgpack_map_monster_killed param;
				MsgContentUnmarshal(msg, param);
				for (size_t i = 0; i < param.monster_list.size(); ++i)
				{
					MonsterKilled(param.monster_list[i].monster_tid, 
                                  param.monster_list[i].monster_count);
				}
			}
			break;

        case GS_PLANE_MSG_MODIFY_MAP_COUNTER:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_modify_map_counter);
				const plane_msg_modify_map_counter& param = *(plane_msg_modify_map_counter*)msg.content;
				ModifyCounter(param.op_type, param.index, param.value);
			}
			break;

        case GS_PLANE_MSG_MAP_COUNTER_SUBSCRIBE:
            {
                CHECK_CONTENT_PARAM(msg, plane_msg_map_counter_subscribe);
                const plane_msg_map_counter_subscribe& param = *(plane_msg_map_counter_subscribe*)msg.content;
                MapCounterSubscribe(msg.source.id, param.index, param.is_subscribe);
            }
            break;

        case GS_PLANE_MSG_LOCK_MAP_COUNTER:
            {
                CHECK_CONTENT_PARAM(msg, plane_msg_lock_map_counter);
                const plane_msg_lock_map_counter& param = *(plane_msg_lock_map_counter*)msg.content;
                LockCounter(param.index, param.value);
            }
            break;

        case GS_PLANE_MSG_UNLOCK_MAP_COUNTER:
            {
                UnlockCounter(msg.param);
            }
            break;

		case GS_PLANE_MSG_ACTIVE_MAP_CLOCK:
			{
				CHECK_CONTENT_PARAM(msg, plane_msg_active_map_clock);
				const plane_msg_active_map_clock& param = *(plane_msg_active_map_clock*)msg.content;
				ActivateClock(param.index, param.times, param.seconds);
			}
			break;

		case GS_PLANE_MSG_DEACTIVE_MAP_CLOCK:
			{
				DeactivateClock(msg.param);
			}
			break;

        case GS_PLANE_MSG_MAP_PLAYER_GATHER_MINE:
            {
                ASSERT(msg.source.IsPlayer());
                PlayerGatherMine(msg.source.id, msg.param);
            }
            break;

        case GS_PLANE_MSG_MAP_DELIVER_TASK:
            {
                DeliverTaskToAll(msg.param);
            }
            break;

        case GS_PLANE_MSG_MAP_PROMPT_MESSAGE:
            {
                MapPromptMessage(msg);
            }
            break;

        case GS_PLANE_MSG_MAP_COUNTDOWN:
            {
                ShowMapCountDown(msg);
            }
            break;

        case GS_PLANE_MSG_QUERY_MAP_TEAM_INFO:
			{
				ASSERT(msg.source.IsPlayer());
				MutexLockGuard lock(mutex_team_);
				QueryMapTeamInfo(msg.source);
			}
			break;

		case GS_PLANE_MSG_PLAYER_QUIT_MAP:
			{
				ASSERT(msg.source.IsPlayer());
				MutexLockGuard lock(mutex_team_);
				PlayerQuitMap(msg.source);
			}
			break;

		case GS_PLANE_MSG_MAP_TEAM_CHANGE_POS:
			{
				ASSERT(msg.source.IsPlayer());
				MutexLockGuard lock(mutex_team_);
				PlayerChangePos(msg);
			}
			break;

		case GS_PLANE_MSG_MAP_TEAM_CHANGE_LEADER:
			{
				ASSERT(msg.source.IsPlayer());
				MutexLockGuard lock(mutex_team_);
				PlayerChangeLeader(msg);
			}
			break;

        case GS_PLANE_MSG_MAP_QUERY_PINFO_RE:
			{
                ASSERT(msg.source.IsPlayer());
				CHECK_CONTENT_PARAM(msg, plane_msg_map_query_pinfo_re);
				MutexLockGuard lock(mutex_team_);
				const plane_msg_map_query_pinfo_re& param = *(const plane_msg_map_query_pinfo_re*)msg.content;
				UpdatePlayerInfo(msg.source, param.combat_value);
			}
			break;

        case GS_PLANE_MSG_MT_APPLY_FOR_JOIN:
            {
                ASSERT(msg.source.IsPlayer());
				MutexLockGuard lock(mutex_team_);
                PlayerApplyForJoinTeam(msg);
            }
            break;

        case GS_PLANE_MSG_MT_LEAVE_TEAM:
            {
                ASSERT(msg.source.IsPlayer());
				MutexLockGuard lock(mutex_team_);
                PlayerLeaveMapTeam(msg.source.id);
            }
            break;

        case GS_PLANE_MSG_MT_KICKOUT_MEMBER:
            {
                ASSERT(msg.source.IsPlayer());
                MutexLockGuard lock(mutex_team_);
                PlayerKickoutMember(msg.source.id, msg.param);
            }
            break;

        case GS_PLANE_MSG_SPOT_MAPELEM_TELEPORT:
            {
                ASSERT(world_man_.GetWorldXID() == msg.source);
                HandleSpotMapElemTeleport(msg);
            }
            break;

        case GS_PLANE_MSG_SPOT_MONSTER_MOVE:
            {
                ASSERT(world_man_.GetWorldXID() == msg.source);
                HandleSpotMonsterMove(msg);
            }
            break;

        case GS_PLANE_MSG_SPOT_MONSTER_SPEED:
            {
                ASSERT(world_man_.GetWorldXID() == msg.source);
                HandleSpotMonsterSpeed(msg);
            }
            break;

        case GS_PLANE_MSG_REACH_DESTINATION:
            {
                HandleReachDestination(msg);
            }
            break;

        default:
            return -1;
    }
    return 0;
}

bool BaseWorldManImp::CheckCounterIndex(int32_t index)
{
	if (index <= 0 || index >= kMaxCounterCount)
        return false;

    return true;
}

void BaseWorldManImp::ModifyCounter(int32_t op_type, int32_t index, int32_t value)
{
	if (!CheckCounterIndex(index))
	{
		LOG_WARN << "脚本修改地图计数器有误！world_id：" << world_man_.GetWorldID();
		return;
	}

    if (counter_list_[index].is_locked)
        return;

	switch (op_type)
	{
		case MMC_INCREASE:
			{
				counter_list_[index].value += value;
			}
			break;

		case MMC_DECREASE:
			{
				counter_list_[index].value -= value;
			}
			break;

		case MMC_ASSIGNMENT:
			{
				counter_list_[index].value = value;
			}
			break;

		default:
			LOG_WARN << "脚本修改地图计数器有误！op_type error -- world_id：" << world_man_.GetWorldID();
			return;
	}

    OnModifyCounter(index, counter_list_[index].value);

    // notify player
    NotifyPlayerCounterChange(index, counter_list_[index].value);
}

void BaseWorldManImp::LockCounter(int32_t index, int32_t value)
{
    if (!CheckCounterIndex(index))
    {
        LOG_ERROR << "LockMapCounter index error！world_id:" << world_man_.GetWorldID()
            << " index:" << index;
        return;
    }

    counter_list_[index].is_locked = true;
    counter_list_[index].value     = value;

    // notify player
    NotifyPlayerCounterChange(index, counter_list_[index].value);
}

void BaseWorldManImp::UnlockCounter(int32_t index)
{
   if (!CheckCounterIndex(index))
   {
        LOG_ERROR << "UnlockMapCounter index error！world_id:" << world_man_.GetWorldID()
            << " index:" << index;
        return;
   }

   counter_list_[index].is_locked = false;
}

int32_t BaseWorldManImp::GetCounter(int32_t index) const
{
    if (!CheckCounterIndex(index))
        return -1;

    return counter_list_[index].value;
}

bool BaseWorldManImp::IsCounterLocked(int32_t index) const
{
    if (!CheckCounterIndex(index))
        return false;

    return counter_list_[index].is_locked;
}

void BaseWorldManImp::CheckClockTimeup(time_t now)
{
	if (--check_clock_timeup_ <= 0)
	{
		int64_t next = clock_timer_.GetExpired(now, expired_vec_);
		for (size_t i = 0; i < expired_vec_.size(); ++i)
		{
			ClockInfo& info = *(expired_vec_[i].second);
            OnClockTimeUp(get_clock_index(info));

			--info.times;
			if (info.times <= 0)
			{
				info.times   = 0;
				info.seconds = 0;
			}
			else
			{
				int64_t when = now + info.seconds;
				next = clock_timer_.AddEntry(when, &info);
			}
		}
		expired_vec_.clear();
		SetClockNextTime(now, next);
	}
}

void BaseWorldManImp::ActivateClock(int32_t index, int32_t times, int32_t secs)
{
	if (index <= 0 || index >= kMaxClockCount)
	{
		LOG_WARN << "地图计时器index错误，world_id" << world_man_.GetWorldID();
		return;
	}

	time_t now   = g_timer->GetSysTime();
	int32_t when = now + secs;
	clock_list_[index].times   = times;
	clock_list_[index].seconds = secs;
	int64_t next = clock_timer_.AddEntry(when, &clock_list_[index]);
	SetClockNextTime(now, next);
}

void BaseWorldManImp::DeactivateClock(int32_t index)
{
	if (index <= 0 || index >= kMaxClockCount)
	{
		LOG_WARN << "地图计时器index错误，world_id" << world_man_.GetWorldID();
		return;
	}

	int64_t next = clock_timer_.Cancel(&clock_list_[index]);
	SetClockNextTime(g_timer->GetSysTime(), next);
}

void BaseWorldManImp::SetClockNextTime(time_t now, int64_t nextTime)
{
	check_clock_timeup_ = static_cast<int32_t>(nextTime - now);
}

void BaseWorldManImp::MonsterKilled(int32_t monster_tid, int32_t count)
{
    OnMonsterKilled(monster_tid, count);
}

void BaseWorldManImp::PlayerGatherMine(int64_t roleid, int32_t mine_tid)
{
    OnPlayerGatherMine(roleid, mine_tid);
}

void BaseWorldManImp::CopyPlayerInfo(const Player* pplayer, map_team_member_info& info)
{
	info.masterid     = pplayer->master_id();
	info.roleid       = pplayer->role_id();
	info.gender       = pplayer->gender();
	info.cls          = pplayer->role_class();
	info.online       = true;
	info.level        = pplayer->level();
	info.combat_value = pplayer->CalcCombatValue();
	info.first_name   = pplayer->first_name();
	info.mid_name     = pplayer->middle_name();
	info.last_name    = pplayer->last_name();
}

void BaseWorldManImp::CopyPlayerInfo(const map_team_player_info& join_info, map_team_member_info& info)
{
    info.masterid     = join_info.masterid;
	info.roleid       = join_info.role_id;
	info.gender       = join_info.gender;
	info.cls          = join_info.cls;
	info.online       = true;
	info.level        = join_info.level;
	info.combat_value = join_info.combat_value;
	info.first_name   = join_info.first_name;
	info.mid_name     = join_info.mid_name;
	info.last_name    = join_info.last_name;
}

// thread safe
void BaseWorldManImp::SendPlayerMsg(int message, const XID& player, int64_t param, const void* buf, size_t len)
{
	MSG msg;
	BuildMessage(msg, message, player, world_man_.GetWorldXID(), param, buf, len);
	Gmatrix::SendObjectMsg(msg, true);
}

void BaseWorldManImp::SendObjectMsg(int message, const XID& obj, int64_t param, const void* buf, size_t len)
{
    MSG msg;
	BuildMessage(msg, message, obj, world_man_.GetWorldXID(), param, buf, len);
	Gmatrix::SendObjectMsg(msg);
}

void BaseWorldManImp::SendPlayerPacket(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet)
{
    NetToLink::SendS2CGameData(linkid, roleid, sid_in_link, packet);
}

// needed lock outside
void BaseWorldManImp::SendMemberMsg(const MapTeamInfo* team_info, int message, int64_t param, const void* buf, size_t len, RoleID except_id)
{
	for (size_t i = 0; i < team_info->members.size(); ++i)
	{
		RoleID roleid = team_info->members[i].roleid;
		if (roleid <= 0 || 
			(except_id != 0 && roleid == except_id) ||
			!team_info->members[i].online)
		{
			continue;
		}

		XID target;
		MAKE_XID(roleid, target);

		MSG msg;
		BuildMessage(msg, message, target, world_man_.GetWorldXID(), param, buf, len);
		Gmatrix::SendObjectMsg(msg, true);
	}
}

void BaseWorldManImp::SendPlayerError(RoleID roleid, int err_no)
{
    XID target;
    MAKE_XID(roleid, target);

    MSG msg;
    BuildMessage(msg, GS_MSG_ERROR_MESSAGE, target, world_man_.GetWorldXID(), err_no);
    Gmatrix::SendObjectMsg(msg);
}

void BaseWorldManImp::QueryMapTeamInfo(const XID& player)
{
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM_RETURN(player.id, team_info);

    shared::net::ByteBuffer buf;
    MsgContentMarshal(*team_info, buf);

    // send msg
    SendPlayerMsg(GS_MSG_MAP_TEAM_INFO, player, 0, buf.contents(), buf.size());
}

// needed lock outside
void BaseWorldManImp::SyncMapTeamInfo(Player* pplayer, MapTeamInfo* team_info)
{
    if (team_info == NULL)
        return;

	shared::net::ByteBuffer buf;
	MsgContentMarshal(*team_info, buf);
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_MAP_TEAM_INFO, pplayer->object_xid(), world_man_.GetWorldXID(), 0, buf.contents(), buf.size());
	pplayer->SyncMapTeamInfo(msg);
}

// needed lock outside
void BaseWorldManImp::SyncMapTeamInfo(RoleID roleid, MapTeamInfo* team_info)
{
    if (team_info == NULL)
        return;

    XID target;
    MAKE_XID(roleid, target);

	shared::net::ByteBuffer buf;
	MsgContentMarshal(*team_info, buf);
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_MAP_TEAM_INFO, target, world_man_.GetWorldXID(), 0, buf.contents(), buf.size());
    Gmatrix::SendObjectMsg(msg, true);
}

// needed lock outside
void BaseWorldManImp::PlayerJoinTeam(const Player* pplayer, MapTeamInfo* team_info, size_t pos)
{
	// copy info
	CopyPlayerInfo(pplayer, team_info->members[pos]);

    // send to team members
    SendMemberJoinTeam(team_info, pos, pplayer->role_id());
}

void BaseWorldManImp::PlayerJoinTeam(const map_team_player_info& info, MapTeamInfo* team_info, size_t pos)
{
    // copy info
    CopyPlayerInfo(info, team_info->members[pos]);

    // send to team members
    SendMemberJoinTeam(team_info, pos, info.role_id);
}

void BaseWorldManImp::SendMemberJoinTeam(const MapTeamInfo* team_info, size_t pos, RoleID except_id)
{
    // send to team members
	msgpack_map_team_join param;
	param.pos  = pos;
	param.info = team_info->members[pos];
	shared::net::ByteBuffer buf;
	MsgContentMarshal(param, buf);
	SendMemberMsg(team_info, GS_MSG_MAP_TEAM_JOIN, 0, buf.contents(), buf.size(), except_id);
}

// lock outside
void BaseWorldManImp::PlayerQuitMap(const XID& player)
{
    PlayerQuitTeam(player);

    // 主要要减掉自己
    int player_stay = world_man_.GetPlayerCount() - 1;
    OnPlayerQuitMap(player_stay);
}

void BaseWorldManImp::PlayerQuitTeam(const XID& player)
{
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM_RETURN(player.id, team_info);
    
    // 玩家主动退出副本
    PlayerLeaveMapTeam(player.id);
}

void BaseWorldManImp::PlayerChangePos(const MSG& msg)
{
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM_RETURN(msg.source.id, team_info);

    CHECK_CONTENT_PARAM(msg, plane_msg_map_team_change_pos);
	const plane_msg_map_team_change_pos& param = *(plane_msg_map_team_change_pos*)msg.content;
	
	if (!check_team_index(param.src_index) ||
		!check_team_index(param.des_index))
	{
		LOG_WARN << "PlayerChangePos invalid src_index:" << param.src_index <<
			" or des_index:" << param.des_index;
		return;
	}

	int src_index = param.src_index;
	int des_index = param.des_index;
	if (!team_info->is_leader(msg.source.id))
	{
		if (team_info->members[src_index].roleid != msg.source.id ||
			!team_info->members[des_index].is_vacancy())
		{
			LOG_WARN << "PlayerChangePos member changepos error, src_index:" << src_index 
				<< " des_index:" << des_index;
			return;
		}
	}

	map_team_member_info tmpinfo;
	tmpinfo = team_info->members[des_index];
	team_info->members[des_index] = team_info->members[src_index];
	team_info->members[src_index] = tmpinfo;
	
	// send msg
	msg_map_team_change_pos send_param;
	send_param.src_index = src_index;
	send_param.des_index = des_index;
	SendMemberMsg(team_info, GS_MSG_MAP_TEAM_CHANGE_POS, 0, &send_param, sizeof(send_param));
}

void BaseWorldManImp::PlayerChangeLeader(const MSG& msg)
{
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM_RETURN(msg.source.id, team_info);

    // ???????????????????????????
	RoleID newleader = msg.param;
	int pos = 0;
	if (!team_info->is_leader(msg.source.id) ||
		!team_info->find_member(newleader, pos))
	{
		return;
	}

	// send msg
	ChangeLeader(newleader, team_info);
}

void BaseWorldManImp::ChangeLeader(RoleID newleader, MapTeamInfo* pinfo)
{
	pinfo->leader = newleader;
	SendMemberMsg(pinfo, GS_MSG_MAP_TEAM_CHANGE_LEADER, newleader);
}

BaseWorldManImp::MapTeamInfo* BaseWorldManImp::CreateNewTeam(size_t pos, const Player* pplayer)
{
    MapTeamInfo tmpinfo;
    tmpinfo.team_id = get_next_map_team_id();
    tmpinfo.leader  = pplayer->role_id();
    CopyPlayerInfo(pplayer, tmpinfo.members[pos]);
    team_info_map_[tmpinfo.team_id] = tmpinfo;

    NewMapTeamCreated(tmpinfo);
    return &(team_info_map_[tmpinfo.team_id]);
}

BaseWorldManImp::MapTeamInfo* BaseWorldManImp::CreateNewTeam(size_t pos, const map_team_player_info& info)
{
    MapTeamInfo tmpinfo;
    tmpinfo.team_id = get_next_map_team_id();
    tmpinfo.leader  = info.role_id;
    CopyPlayerInfo(info, tmpinfo.members[pos]);
    team_info_map_[tmpinfo.team_id] = tmpinfo;

    NewMapTeamCreated(tmpinfo);
    return &(team_info_map_[tmpinfo.team_id]);
}

BaseWorldManImp::MapTeamInfo* BaseWorldManImp::CreateNewTeam(const MapTeamInfo& info)
{
    MapTeamInfo tmpinfo = info;
    tmpinfo.team_id = get_next_map_team_id();
    tmpinfo.leader  = info.leader;
    team_info_map_[tmpinfo.team_id] = tmpinfo;

    NewMapTeamCreated(tmpinfo);
    return &(team_info_map_[tmpinfo.team_id]);
}

bool BaseWorldManImp::PlayerEnterMapTeam(Player* pplayer)
{
    MapTeamInfo* team_info = NULL;
    FIND_PLAYER_IN_TEAM(pplayer->role_id(), team_info);

	// has no team before
	if (team_info == NULL && IsAutoMapTeam())
	{
        int32_t auto_teamid = GetAutoMapTeamID(pplayer);
        if (auto_teamid <= 0)
        {
            int pos = (pplayer->GetPosInTeam() > 0) ? pplayer->GetPosInTeam() : 0;
            auto_teamid = CreateNewTeam(pos, pplayer)->team_id;
        }

        MapTeamInfoMap::iterator it = team_info_map_.find(auto_teamid);
        if (it != team_info_map_.end())
        {
            team_info = &(it->second);
        }
    }

    // already has team
    if (team_info != NULL)
	{
		int pos = 0;
		// already in-team
		if (team_info->find_member(pplayer->role_id(), pos))
		{
			team_info->members[pos].online = true;

			msg_map_team_status_change param;
			param.roleid = pplayer->role_id();
			param.online = true;
			SendMemberMsg(team_info, GS_MSG_MAP_TEAM_STATUS_CHANGE, 0, &param, sizeof(param), pplayer->role_id());
		}
		else // new team member
		{
			if (team_info->member_count() >= kTeamMemberCount)
                return false;

			pos = (pplayer->GetPosInTeam() > 0) ? pplayer->GetPosInTeam() : 0;
			pos = team_info->get_gap_in_team(pos);
			if (pos < 0) 
				return false;

			// new player join team
			PlayerJoinTeam(pplayer, team_info, pos);

			// change leader
			if (pplayer->role_id() == pplayer->GetLeaderId() &&
				!team_info->is_leader(pplayer->role_id()))
			{
				ChangeLeader(pplayer->role_id(), team_info);
			}
		}
	}

    if (team_info != NULL)
    {
        if (!team_info->is_leader_online())
        {
            ChangeLeader(pplayer->role_id(), team_info);
        }
        // 整理地图组队的队员位置
        TidyMapTeamPos(team_info, pplayer->role_id());
        // sync team info
	    SyncMapTeamInfo(pplayer, team_info);
    }
	return true;
}

void BaseWorldManImp::PlayerOffline(RoleID playerid)
{
    int pos;
    MapTeamInfo* team_info = NULL;
    FIND_PLAYER_IN_TEAM_POS(playerid, team_info, pos);

    if (team_info != NULL)
    {
        team_info->members[pos].online = false;

        msg_map_team_status_change param;
        param.roleid = playerid;
        param.online = false;
        SendMemberMsg(team_info, GS_MSG_MAP_TEAM_STATUS_CHANGE, 0, &param, sizeof(param), playerid);

        if (playerid == team_info->leader)
        {
            RoleID newleader = 0;
            if (team_info->get_first_online(newleader))
            {
                ASSERT(playerid != newleader);
                ChangeLeader(newleader, team_info);
            }
        }
    }
}

void BaseWorldManImp::PlayerLeaveMapTeam(RoleID playerid)
{
	int pos;
    MapTeamInfo* team_info = NULL;
    FIND_PLAYER_IN_TEAM_POS(playerid, team_info, pos);

	if (team_info != NULL)
	{
        // clear player data in team
        team_info->members[pos].clear();

        // send msg
        SendMemberMsg(static_cast<const MapTeamInfo*>(team_info), GS_MSG_MAP_TEAM_LEAVE, playerid);

        // leave team to kicked player
        XID target;
        MAKE_XID(playerid, target);
        SendPlayerMsg(GS_MSG_MAP_TEAM_LEAVE, target, playerid);

        if (playerid == team_info->leader)
		{
			RoleID newleader = 0;
			if (team_info->get_first_online(newleader))
			{
				ASSERT(playerid != newleader);
				ChangeLeader(newleader, team_info);
			}
		}

        // only one player left, dismiss team
        if (team_info->member_count() == 1)
        {
            DismissTeam(team_info->team_id);
            return;
        }

        // 整理地图组队的队员位置
        TidyMapTeamPos(team_info, 0);
	}
}

// needed lock outside
void BaseWorldManImp::DismissTeam(int32_t team_id)
{
    MapTeamInfoMap::iterator it = team_info_map_.find(team_id);
    if (it != team_info_map_.end())
    {
        MapTeamInfo* team_info = &it->second;
        for (size_t i = 0; i < team_info->members.size(); ++i)
        {
            RoleID roleid = team_info->members[i].roleid;
            if (roleid <= 0 || !team_info->members[i].online)
                continue;

            XID target;
            MAKE_XID(roleid, target);
            SendPlayerMsg(GS_MSG_MAP_TEAM_LEAVE, target, roleid);
        }
        team_info_map_.erase(it);
    }
}

void BaseWorldManImp::ResetQueryPInfoTimeout()
{
	query_pinfo_timeout_ = mrand::Rand(20, 51);
}

void BaseWorldManImp::QueryPlayerInfoInTeam()
{
	if (--query_pinfo_timeout_ < 0)
	{
	    MutexLockGuard lock(mutex_team_);
        MapTeamInfoMap::const_iterator it = team_info_map_.begin();
        for (; it != team_info_map_.end(); ++it)
        { 
            const MapTeamInfo* team_info = &(it->second);
		    SendMemberMsg(team_info, GS_MSG_MAP_QUERY_PLAYER_INFO);
        }
		ResetQueryPInfoTimeout();
	}
}

void BaseWorldManImp::UpdatePlayerInfo(const XID& player, int32_t combat_value)
{
	int pos = 0;
    MapTeamInfo* team_info = NULL;
    FIND_PLAYER_IN_TEAM_POS(player.id, team_info, pos);
	if (team_info == NULL)
	{
		LOG_WARN << "地图收到query_player_info_re，但玩家已不再队伍里roleid:" << player.id;
		return;
	}

	if (team_info->members[pos].combat_value < combat_value)
	{
		team_info->members[pos].combat_value = combat_value;
	}
}

void BaseWorldManImp::GetAllTeamInfo(std::vector<MapTeamInfo>& info_list) const
{
    if (mutex_team_.IsLockedByThisThread())
    {
        return GetAllTeamInfoReal(info_list);
    }
    else // need to lock
    {
	    MutexLockGuard lock(mutex_team_);
        return GetAllTeamInfoReal(info_list);
    }
}

void BaseWorldManImp::GetAllTeamInfoReal(std::vector<MapTeamInfo>& info_list) const
{
    MapTeamInfoMap::const_iterator it = team_info_map_.begin();
    for (; it != team_info_map_.end(); ++it)
    { 
        info_list.push_back(it->second);
    }
}

void BaseWorldManImp::DeliverTaskToAll(int32_t task_id)
{
    if (task_id > 0)
    {
        std::vector<XID> player_vec;
        world_man_.GetAllPlayerInWorld(player_vec);
        for (size_t i = 0; i < player_vec.size(); ++i)
        {
            SendPlayerMsg(GS_MSG_WORLD_DELIVER_TASK, player_vec[i], task_id);
        }
    }
}

void BaseWorldManImp::MapPromptMessage(const MSG& msg)
{
    if (world_man_.IsClosed())
        return;

    CHECK_CONTENT_PARAM(msg, plane_msg_map_prompt_message);
    const plane_msg_map_prompt_message& param = *(plane_msg_map_prompt_message*)msg.content;

    if (IS_INS_MAP(world_man_.GetWorldID()))
    {
        world::instance_info info;
        if (!world_man_.GetInsInfo(info))
        {
            LOG_ERROR << "没有对应的副本信息？？";
            return;
        }

        G2C::InsPromptMessage packet;
        packet.info.templ_id = info.ins_templ_id;
        packet.info.index    = param.index;
        packet.info.delay    = param.delay;
        packet.info.duration = param.duration;
        BroadcastToAllPlayers(packet);
    }
    else if(IS_BG_MAP(world_man_.GetWorldID()))
    {
        world::battleground_info info;
        if (!world_man_.GetBGInfo(info))
        {
            LOG_ERROR << "没有对应的战场信息？？";
            return;
        }

        G2C::BGPromptMessage packet;
        packet.info.templ_id = info.bg_templ_id;
        packet.info.index    = param.index;
        packet.info.delay    = param.delay;
        packet.info.duration = param.duration;
        BroadcastToAllPlayers(packet);
    }
    else
    {
        LOG_WARN << "还有别的类型地图可以发PromptMessage？world_id: " << world_man_.GetWorldID();
        return;
    }
}

void BaseWorldManImp::ShowMapCountDown(const MSG& msg)
{
    if (world_man_.IsClosed())
        return;

    CHECK_CONTENT_PARAM(msg, plane_msg_map_countdown);
    const plane_msg_map_countdown& param = *(plane_msg_map_countdown*)msg.content;

    G2C::ShowMapCountDown packet;
    packet.countdown = param.countdown;
    packet.screen_x  = param.screen_x;
    packet.screen_y  = param.screen_y;
    BroadcastToAllPlayers(packet);
}

void BaseWorldManImp::MapCounterSubscribe(RoleID roleid, int32_t index, bool is_subscribe)
{
    if (index <= 0)
    {
        CounterSubscribeMap::iterator it = counter_subscribe_map_.begin();
        for (; it != counter_subscribe_map_.end(); ++it)
        {
            it->second.erase(roleid);
        }
        return;
    }

    if (!CheckCounterIndex(index))
    {
        LOG_ERROR << "订阅地图计数器出错！index:" << index;
        return;
    }

    if (is_subscribe)
    {
        counter_subscribe_map_[index].insert(roleid); 
        int32_t value = GetCounter(index);
        NotifyMapCounterChange(roleid, index, value);
    }
    else
    {
        CounterSubscribeMap::iterator it = counter_subscribe_map_.find(index);
        if (it != counter_subscribe_map_.end())
        {
            it->second.erase(roleid);
        }
    }
}

void BaseWorldManImp::NotifyPlayerCounterChange(int32_t index, int32_t value)
{
    if (world_man_.IsClosed())
        return;

    CounterSubscribeMap::const_iterator it = counter_subscribe_map_.find(index);
    if (it != counter_subscribe_map_.end())
    {
        RoleIDSet::const_iterator set_it = it->second.begin();
        for (; set_it != it->second.end(); ++set_it)
        {
            NotifyMapCounterChange(*set_it, index, value);
        }
    }
}

void BaseWorldManImp::NotifyMapCounterChange(RoleID roleid, int32_t index, int32_t value)
{
    if (world_man_.IsClosed())
        return;

    if (value >= 0)
    {
        XID target;
        MAKE_XID(roleid, target);

        msg_map_counter_change param;
        param.index = index;
        param.value = value;
        world_man_.SendObjectMSG(GS_MSG_MAP_COUNTER_CHANGE, target, &param, sizeof(param));
    }
}

void BaseWorldManImp::PlayerApplyForJoinTeam(const MSG& msg)
{
    msgpack_map_team_apply_for_join param;
	MsgContentUnmarshal(msg, param);

    // 检查申请者是不是已经在队伍中
    MapTeamInfo* tmpinfo = NULL;
    FIND_PLAYER_IN_TEAM(param.applicant, tmpinfo);
    if (tmpinfo != NULL)
    {
        SendPlayerError(param.applicant, G2C::ERR_ALREADY_IN_TEAM);
        return;
    }

    // 检查被申请的队伍是不是已经没有空位
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM(param.respondent, team_info);

    // 双方都没有队伍, 创建队伍
    if (team_info == NULL && tmpinfo == NULL)
    {
        team_info = CreateNewTeam(0, param.resp_info);
        SyncMapTeamInfo(param.respondent, team_info);
    }

    // 队伍已经找到
    if (team_info->member_count() >= kTeamMemberCount)
    {
        SendPlayerError(param.applicant, G2C::ERR_THE_TEAM_IS_FULL);
        return;
    }
    size_t pos = team_info->get_vacancy();
    if (pos < 0) 
    {
        LOG_WARN << "apply for join map team fail! get_vacancy() fail";
        return;
    }

    ///
    /// success
    ///

    // new player join team
    PlayerJoinTeam(param.info, team_info, pos);

    // msg to applyer
    SyncMapTeamInfo(param.applicant, team_info);
}

void BaseWorldManImp::PlayerKickoutMember(RoleID leader, RoleID kicked_roleid)
{
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM_RETURN(leader, team_info);

    if (!team_info->is_leader(leader))
    {
        LOG_WARN << "kickout member, but is not leader:" << leader;
        return;
    }

    int pos;
    if (!team_info->find_member(kicked_roleid, pos))
    {
        LOG_WARN << "can not find the teammember! member:" << kicked_roleid;
        return;
    }

    // leave team to members left
    PlayerLeaveMapTeam(kicked_roleid);
}

void BaseWorldManImp::HandleSpotMapElemTeleport(const MSG& msg)
{
    if (world_man_.IsClosed())
        return;

    CHECK_CONTENT_PARAM(msg, plane_msg_spot_mapelem_teleport);
	const plane_msg_spot_mapelem_teleport& param = *(plane_msg_spot_mapelem_teleport*)msg.content;

    // 检查是否是定点地图元素
    const BaseMapData* pbaseData = s_pMapData->QueryBaseMapDataTempl(param.elem_id);
    if (pbaseData == NULL)
    {
        __PRINTF("SpotMapElemTeleport mapElement not found! elem_id:%d", param.elem_id);
        return;
    }
    if (pbaseData->GetType() != mapDataSvr::MAPDATA_TYPE_SPOT_MINE && 
        pbaseData->GetType() != mapDataSvr::MAPDATA_TYPE_SPOT_MONSTER)
    {
        __PRINTF("SpotMapElemTeleport elem is not a spot mapElement! elem_id:%d", param.elem_id);
        return;
    }

    // 检查目标位置是否合法
    A2DVECTOR target_pos(param.pos_x, param.pos_y);
    if (!world_man_.PosInWorld(target_pos))
    {
        __PRINTF("SpotMapElemTeleport pos is not in world! x:%f y:%f", target_pos.x, target_pos.y);
        return;
    }
    if (!world_man_.IsWalkablePos(target_pos))
    {
        __PRINTF("SpotMapElemTeleport pos is not walkable! x:%f y:%f", target_pos.x, target_pos.y);
        return;
    }

    // 查找该地图元素是否已经激活
    std::vector<XID> obj_vec;
    if (!world_man_.QueryMapElemInfo(param.elem_id, obj_vec))
    {
        __PRINTF("SpotMapElemTeleport elem is not found!");
        return;
    }
    if (obj_vec.size() != 1)
    {
        LOG_ERROR << "Spot Object size greater than 1 !!! elem_id:" << param.elem_id;
        return;
    }

    msg_object_teleport send_param;
    send_param.elem_id = param.elem_id;
    send_param.pos_x   = param.pos_x;
    send_param.pos_y   = param.pos_y;
    send_param.dir     = param.dir;
    for (size_t i = 0; i < obj_vec.size(); ++i)
    {
        SendObjectMsg(GS_MSG_OBJECT_TELEPORT, obj_vec[i], 0, &send_param, sizeof(send_param));
    }
}

void BaseWorldManImp::HandleSpotMonsterMove(const MSG& msg)
{
    if (world_man_.IsClosed())
        return;

    CHECK_CONTENT_PARAM(msg, plane_msg_spot_monster_move);
	const plane_msg_spot_monster_move& param = *(plane_msg_spot_monster_move*)msg.content;

    // 检查是否是定点怪
    const SpotMonster* ptempl = s_pMapData->QueryMapDataTempl<SpotMonster>(param.elem_id);
    if (ptempl == NULL)
    {
        __PRINTF("SpotMonsterMove mapElement not found! elem_id:%d", param.elem_id);
        return;
    }
    // 检查是怪，不是npc
    int32_t monster_tid = ptempl->associated_templ_id;
    const MonsterTempl* pData = s_pDataTempl->QueryDataTempl<MonsterTempl>(monster_tid);
    if (pData == NULL)
    {
        __PRINTF("SpotMonsterMove spotmonster is not monster! tid:%d", monster_tid);
        return;
    }

    // 检查目标位置是否合法
    A2DVECTOR target_pos(param.pos_x, param.pos_y);
    if (!world_man_.PosInWorld(target_pos))
    {
        __PRINTF("SpotMonsterMove pos is not in world! x:%f y:%f", target_pos.x, target_pos.y);
        return;
    }
    if (!world_man_.IsWalkablePos(target_pos))
    {
        __PRINTF("SpotMonsterMove pos is not walkable! x:%f y:%f", target_pos.x, target_pos.y);
        return;
    }

    // 查找该地图元素是否已经激活
    std::vector<XID> obj_vec;
    if (!world_man_.QueryMapElemInfo(param.elem_id, obj_vec))
    {
        __PRINTF("SpotMonsterMove elem is not found!");
        return;
    }
    if (obj_vec.size() != 1)
    {
        LOG_ERROR << "Spot Object size greater than 1 !!! elem_id:" << param.elem_id;
        return;
    }

    msg_monster_move send_param;
    send_param.elem_id = param.elem_id;
    send_param.pos_x   = param.pos_x;
    send_param.pos_y   = param.pos_y;
    send_param.speed   = param.speed;
    for (size_t i = 0; i < obj_vec.size(); ++i)
    {
        SendObjectMsg(GS_MSG_MONSTER_MOVE, obj_vec[i], 0, &send_param, sizeof(send_param));
    }
}

void BaseWorldManImp::HandleSpotMonsterSpeed(const MSG& msg)
{
    if (world_man_.IsClosed())
        return;

    CHECK_CONTENT_PARAM(msg, plane_msg_spot_monster_speed);
	const plane_msg_spot_monster_speed& param = *(plane_msg_spot_monster_speed*)msg.content;

    // 检查是否是定点怪
    const SpotMonster* ptempl = s_pMapData->QueryMapDataTempl<SpotMonster>(param.elem_id);
    if (ptempl == NULL)
    {
        __PRINTF("SetSpotMonsterSpeed mapElement not found! elem_id:%d", param.elem_id);
        return;
    }
    // 检查是怪，不是npc
    int32_t monster_tid = ptempl->associated_templ_id;
    const MonsterTempl* pData = s_pDataTempl->QueryDataTempl<MonsterTempl>(monster_tid);
    if (pData == NULL)
    {
        __PRINTF("SetSpotMonsterSpeed spotmonster is not monster! tid:%d", monster_tid);
        return;
    }

    // 查找该地图元素是否已经激活
    std::vector<XID> obj_vec;
    if (!world_man_.QueryMapElemInfo(param.elem_id, obj_vec))
    {
        __PRINTF("SpotMonsterMove elem is not found!");
        return;
    }
    if (obj_vec.size() != 1)
    {
        LOG_ERROR << "Spot Object size greater than 1 !!! elem_id:" << param.elem_id;
        return;
    }

    msg_monster_speed send_param;
    send_param.elem_id = param.elem_id;
    send_param.speed   = param.speed;
    for (size_t i = 0; i < obj_vec.size(); ++i)
    {
        SendObjectMsg(GS_MSG_MONSTER_SPEED, obj_vec[i], 0, &send_param, sizeof(send_param));
    }
}

void BaseWorldManImp::HandleReachDestination(const MSG& msg)
{
    CHECK_CONTENT_PARAM(msg, plane_msg_reach_destination);
    const plane_msg_reach_destination& param = *(const plane_msg_reach_destination*)msg.content;

    A2DVECTOR pos(param.pos_x, param.pos_y);
    OnReachDestination(param.elem_id, pos);
}

void BaseWorldManImp::BroadcastToAllPlayers(PacketRef packet)
{
    std::vector<world::player_link_info> player_vec;
    world_man_.GetAllPlayerLinkInfo(player_vec);

    for (size_t i = 0; i < player_vec.size(); ++i)
    {
        const world::player_link_info& info = player_vec[i];
        SendPlayerPacket(info.role_id, info.link_id, info.sid_in_link, packet);
    }
}

void BaseWorldManImp::OnSetMapTeamInfo(void* buf, int len)
{
    M2G::MapTeamInfo proto;
    ASSERT(proto.ParseFromArray(buf, len));

    // lock
	MutexLockGuard lock(mutex_team_);
    MapTeamInfo* team_info = NULL; 
    FIND_PLAYER_IN_TEAM(proto.roleid(), team_info);
    if (team_info == NULL)
    {
        MapTeamInfo tmpinfo;
        tmpinfo.team_id = -1;
        tmpinfo.leader  = 0;
        for (int i = 0; i < proto.members_size(); ++i)
        {
            const M2G::MapTeamMemberInfo& ent = proto.members(i);
            if (ent.roleid() != proto.roleid())
            {
                FIND_PLAYER_IN_TEAM(ent.roleid(), team_info);
                if (team_info != NULL) // 该队员已经有队伍
                    continue;
            }

            // 是不是队长
            if (proto.leader() == ent.roleid())
            {
                tmpinfo.leader = proto.leader();
            }

            // 添加该组队成员
            map_team_member_info member;
            member.masterid     = ent.masterid();
            member.roleid       = ent.roleid();
            member.gender       = ent.gender();
            member.cls          = ent.cls();
            member.online       = (ent.roleid() == proto.roleid()) ? true : false;
            member.level        = ent.level();
            member.combat_value = ent.combat_value();
            member.first_name   = ent.first_name();
            member.mid_name     = ent.mid_name();
            member.last_name    = ent.last_name();
            tmpinfo.members[ent.pos()] = member;
        }
        // 如果没有找到队长，把该玩家设为队长
        tmpinfo.leader = (tmpinfo.leader <= 0) ? proto.roleid() : tmpinfo.leader;
        CreateNewTeam(tmpinfo);
    }
    else
    {
        // 队伍已经创建
        return;
    }
}

void BaseWorldManImp::TidyMapTeamPos(MapTeamInfo* team_info, RoleID except_id)
{
    team::TeamPosVec team_raw;
    team::TeamPosVec result;
    for (size_t i = 0; i < team_info->members.size(); ++i)
    {
        team::TeamPosDetail detail;
        const map_team_member_info& info = team_info->members[i];
        if (info.roleid > 0)
        {
            detail.roleid       = info.roleid;
            detail.cls          = info.cls;
            detail.combat_value = info.combat_value;
        }
        team_raw[i] = detail;
    }

    MapTeamInfo tmp_info;
    // 如果位置重新调整，则通知队伍成员
    if (team::TidyTeamPos(team_raw, result))
    {
        for (size_t i = 0; i < result.size(); ++i)
        {
            if (result[i].roleid <= 0)
                continue;

            int index = i;
            for (size_t k = 0; k < team_info->members.size(); ++k)
            {
                const map_team_member_info& info = team_info->members[k];
                if (result[index].roleid == info.roleid)
                {
                    tmp_info.members[index] = info;
                }
            }
        }
        team_info->members = tmp_info.members;

        // send to members
        msgpack_map_team_tidy_pos send_param;
        for (size_t i = 0; i < team_info->members.size(); ++i)
        {
            send_param.members[i] = team_info->members[i].roleid;
        }
        shared::net::ByteBuffer buf;
        MsgContentMarshal(send_param, buf);
        SendMemberMsg(team_info, GS_MSG_MAP_TEAM_TIDY_POS, 0, buf.contents(), buf.size(), except_id);
    }
}

} // namespace gamed
