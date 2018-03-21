#ifndef GAMED_GS_SUBSYS_PLAYER_TEAM_H_
#define GAMED_GS_SUBSYS_PLAYER_TEAM_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player组队子系统
 */
class PlayerTeam : public PlayerSubSystem
{
public:
	static const int kTeamMemberCount = 4;

public:
	PlayerTeam(Player& player);
	virtual ~PlayerTeam();

	virtual void OnRelease();
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	bool IsInTeam() const;
	bool IsInTeam(int32_t teamid) const;
	bool IsTeamLeader() const;
	bool IsTeammate(RoleID roleid) const; // roleid不包含自己
	bool GetTeammates(std::vector<RoleID>& members_vec) const; // 获取队友，不包含自己
	int  GetSelfPos(); // 返回值小于0表示没有队伍
	int  GetTeamId() const;
	void LeaveTeamBySelf();
    int  GetTeammateCount() const; // 不包含自己
	RoleID GetLeaderId() const;
    void TeamLeaderConvene(RoleID leader, int32_t world_id, const A2DVECTOR& pos);


protected:
	int    MSGHandler_JoinTeam(const MSG&);
	int    MSGHandler_LeaveTeam(const MSG&);
	int    MSGHandler_ChangeTeamLeader(const MSG&);
	int    MSGHandler_ChangeTeamPos(const MSG&);
	int    MSGHandler_TeamInfo(const MSG&);
	int    MSGHandler_QueryTeamMember(const MSG&);

    void   CMDHandler_ConveneTeammate(const C2G::ConveneTeammate&);
    void   CMDHandler_ConveneResponse(const C2G::ConveneResponse&);


private:
	typedef int32_t TeamID;
	// copyable
	struct TeamInfo
	{
		TeamInfo();
		void Release();

		TeamID  team_id;
		RoleID  leader;
		RoleID  members[kTeamMemberCount];
	};

    struct LeaderConvene
    {
        LeaderConvene()
            : convened_leader(0),
              world_id(0)
        { }

        bool has_convene() const
        {
            return convened_leader != 0;
        }

        void clear()
        {
            convened_leader = 0;
            world_id        = 0;
            pos.x           = 0;
            pos.y           = 0;
        }

        RoleID convened_leader;
        MapID  world_id;
        A2DVECTOR pos;
    };

	void   RefreshTeamInfo(int message);  // 发生错误时重新同步队伍信息
	void   ClearTeamInfo();
	bool   IsTeamLeader(RoleID leader) const;
	inline bool CheckMembersIndex(int index) const;
	inline bool FindMember(RoleID roleid, int& index) const; // 包含自己


private:
	TeamInfo      team_info_;
    LeaderConvene leader_con_; // 暂存队长召集的信息
};

///
/// inline func
///
inline bool PlayerTeam::CheckMembersIndex(int index) const
{	
	if (index < 0 || index >= kTeamMemberCount)
		return false;
	return true;
}

inline bool PlayerTeam::FindMember(RoleID roleid, int& index) const
{
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (roleid == team_info_.members[i])
		{
			index = i;
			return true;
		}
	}
	return false;
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_TEAM_H_
