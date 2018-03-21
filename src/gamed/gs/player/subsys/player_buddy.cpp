#include "player_buddy.h"

#include "gamed/client_proto/G2C_proto.h"
#include "gs/player/player_sender.h"


#define PLAYER_SELF_ID -1
#define PLAYER_TID 0
#define BUDDY_ID_INIT_VALUE 1


namespace gamed {

// 直接用0~3的下标来访问members，改kTeamMemberCount值需谨慎
SHARED_STATIC_ASSERT(PlayerBuddy::kTeamMemberCount == 4);


///
/// TeamInfo
///
PlayerBuddy::TeamInfo::TeamInfo() 
{
	Release();
}

void PlayerBuddy::TeamInfo::Release()
{
	leader  = 0;
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		members[i].id     = 0;
		members[i].tpl_id = 0;
	}
}


///
/// PlayerBuddy
///
PlayerBuddy::PlayerBuddy(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_BUDDY, player),
	  next_buddy_id_(BUDDY_ID_INIT_VALUE),
	  is_getalldata_complete_(false)
{
}

PlayerBuddy::~PlayerBuddy()
{
}

void PlayerBuddy::OnRelease()
{
	next_buddy_id_          = BUDDY_ID_INIT_VALUE;
	is_getalldata_complete_ = false;
	ClearTeamInfo();
}

void PlayerBuddy::ClearTeamInfo()
{
	team_info_.Release();
}

bool PlayerBuddy::GetBuddyMembers(std::vector<playerdef::BuddyMemberInfo>& buddy_vec) const
{
	if (!HasBuddyTeam())
	{
		buddy_vec.clear();
		return false;
	}

	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (team_info_.members[i].id > 0)
		{
			playerdef::BuddyMemberInfo tmpinfo;
			tmpinfo.tpl_id    = team_info_.members[i].tpl_id;
			tmpinfo.pos_index = i;
			buddy_vec.push_back(tmpinfo);
		}
	}

	return true;
}

int PlayerBuddy::GetSelfPos() const
{
	if (!HasBuddyTeam())
		return -1;

	int index = -1;
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (team_info_.members[i].id == PLAYER_SELF_ID)
		{
			index = i;
			break;
		}
	}

	return index;
}

bool PlayerBuddy::HasBuddyTeam() const
{
	return team_info_.leader != 0;
}

bool PlayerBuddy::CanJoinPlayerTeam() const
{
	if (HasBuddyTeam())
		return false;

	return true;
}

void PlayerBuddy::StartRecvBuddyInfo()
{
	is_getalldata_complete_ = true;

	if (waiting_buddy_.size())
	{
		WaitingBuddyVec tmpvec;
		tmpvec.swap(waiting_buddy_);
		for (size_t i = 0; i < tmpvec.size(); ++i)
		{
			ObtainBuddy(tmpvec[i]);
		}
	}

	// 非正常地图里不能带组队npc，解散队伍
    if (!IS_NORMAL_MAP(player_.world_id()))
    {
        SendLeaveTeam(PLAYER_SELF_ID);
    }
}

void PlayerBuddy::ObtainBuddy(int32_t buddy_tid)
{
	// 非正常地图里不能带组队npc
    if (!IS_NORMAL_MAP(player_.world_id()))
        return;

	// 检测客户端是否初始化完成
	if (!is_getalldata_complete_)
	{
		waiting_buddy_.push_back(buddy_tid);
	}
	else
	{
		// 有队伍
		if (HasBuddyTeam())
		{
			JoinTeam(buddy_tid);
		}
		else // 无队伍
		{
			// 主动离开玩家队伍
			player_.LeaveTeamBySelf();
			// 创建npc伙伴队伍
			CreateTeam(buddy_tid);
		}
	}
}

void PlayerBuddy::CreateTeam(int32_t buddy_tid)
{
	team_info_.leader            = player_.role_id();
	team_info_.members[0].id     = get_next_buddy_id();
	team_info_.members[0].tpl_id = buddy_tid;
	team_info_.members[1].id     = 0;
	team_info_.members[1].tpl_id = 0;
	team_info_.members[2].id     = 0;
	team_info_.members[2].tpl_id = 0;
	team_info_.members[3].id     = PLAYER_SELF_ID;
	team_info_.members[3].tpl_id = PLAYER_TID;

	G2C::BuddyTeamInfo packet;
	packet.pos1.buddy_id         = team_info_.members[0].id;
	packet.pos1.buddy_templ_id   = team_info_.members[0].tpl_id;
	packet.pos2.buddy_id         = team_info_.members[1].id;
	packet.pos2.buddy_templ_id   = team_info_.members[1].tpl_id;
	packet.pos3.buddy_id         = team_info_.members[2].id;
	packet.pos3.buddy_templ_id   = team_info_.members[2].tpl_id;
	packet.pos4.buddy_id         = team_info_.members[3].id;
	packet.pos4.buddy_templ_id   = team_info_.members[3].tpl_id;

	player_.sender()->SendCmd(packet);
}

void PlayerBuddy::JoinTeam(int32_t buddy_tid)
{
	int index = -1;
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (team_info_.members[i].id == 0)
		{
			index = i;
			team_info_.members[i].id     = get_next_buddy_id();
			team_info_.members[i].tpl_id = buddy_tid;
			break;
		}
	}

	// 进入队伍成功
	if (index >= 0)
	{
		SendJoinTeam(team_info_.members[index].id, index, buddy_tid);
	}
	else
	{
		waiting_buddy_.push_back(buddy_tid);
	}
}

void PlayerBuddy::TakeoutBuddy(int32_t buddy_tid)
{
	if (!HasBuddyTeam())
	{
        if (IS_NORMAL_MAP(player_.world_id()))
        {
            LOG_ERROR << "TakeoutBuddy() error! has not buddy team yet!";
        }
		return;
	}

	LeaveTeam(buddy_tid);

	// 检测队伍里是不是已经没有buddy，即解散队伍
	CheckTeamExist();
}

void PlayerBuddy::LeaveTeam(int32_t buddy_tid)
{
	int32_t index = -1;
	int32_t tmpid = 0;
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (team_info_.members[i].tpl_id == buddy_tid)
		{
			index = i;
			tmpid = team_info_.members[i].id;
			team_info_.members[i].id     = 0;
			team_info_.members[i].tpl_id = 0;	
			break;
		}
	}

	if (tmpid != 0)
	{
		SendLeaveTeam(tmpid);

		ASSERT(index >= 0 && index < kTeamMemberCount);
		if (waiting_buddy_.size())
		{
			int32_t tpl_id = waiting_buddy_.front();
			waiting_buddy_.erase(waiting_buddy_.begin());
			team_info_.members[index].id     = get_next_buddy_id();
			team_info_.members[index].tpl_id = tpl_id;

			SendJoinTeam(team_info_.members[index].id, index, tpl_id);
		}
	}
	else
	{
		WaitingBuddyVec::iterator it = waiting_buddy_.begin();
		while (it != waiting_buddy_.end())
		{
			if ((*it) == buddy_tid)
			{
				tmpid = -2;
				waiting_buddy_.erase(it);
				break;
			}
				
			++it;
		}
	}

	if (tmpid == 0)
	{
		LOG_ERROR << "LeaveTeam() error! buddy_tid" << buddy_tid << " not found";
	}
}

void PlayerBuddy::SendJoinTeam(int32_t id, int32_t index, int tpl_id)
{
	G2C::BuddyJoinTeam packet;
	packet.buddy_id       = id;
	packet.pos_index      = index;
	packet.buddy_templ_id = tpl_id;
	player_.sender()->SendCmd(packet);
}

void PlayerBuddy::SendLeaveTeam(int32_t id)
{
	G2C::BuddyLeaveTeam packet;
	packet.buddy_id = id;
	player_.sender()->SendCmd(packet);
}

void PlayerBuddy::CheckTeamExist()
{
	int32_t tmpid = 0;
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (team_info_.members[i].id > 0)
		{
			tmpid = team_info_.members[i].id;
			break;
		}
	}

	if (tmpid == 0)
	{
		ClearTeamInfo();
		SendLeaveTeam(PLAYER_SELF_ID);
	}
}

void PlayerBuddy::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::ChangeBuddyTeamPos, PlayerBuddy::CMDHandler_ChangeBuddyTeamPos);
}

void PlayerBuddy::CMDHandler_ChangeBuddyTeamPos(const C2G::ChangeBuddyTeamPos& cmd)
{
	if ((cmd.src_index < 0 || cmd.src_index >= kTeamMemberCount)
	 || (cmd.des_index < 0 || cmd.des_index >= kTeamMemberCount)
	 || (cmd.src_index == cmd.des_index))
	{
		return;
	}

	MemberInfo tmpinfo                = team_info_.members[cmd.des_index];	
	team_info_.members[cmd.des_index] = team_info_.members[cmd.src_index];
	team_info_.members[cmd.src_index] = tmpinfo;

	G2C::ChangeBuddyTeamPos_Re packet;
	packet.src_index = cmd.src_index;
	packet.des_index = cmd.des_index;
	player_.sender()->SendCmd(packet);
}

} // namespace gamed
