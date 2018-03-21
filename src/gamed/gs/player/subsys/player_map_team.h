#ifndef GAMED_GS_SUBSYS_PLAYER_MAP_TEAM_H_
#define GAMED_GS_SUBSYS_PLAYER_MAP_TEAM_H_

#include "shared/net/packet/packet_util.h"
#include "gs/global/msg_pack_def.h"

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief: 副本队伍子系统
 *  （1）进入副本会自动组成一队
 */
class PlayerMapTeam : public PlayerSubSystem
{
public:
	static const int kTeamMemberCount = 4;

public:
	PlayerMapTeam(Player& player);
	virtual ~PlayerMapTeam();

	virtual void OnRelease();
	virtual void OnLeaveWorld();
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	bool IsInTeam() const;
	bool IsInTeam(int32_t teamid) const;
	bool IsTeamLeader() const;
	bool IsTeammate(RoleID roleid) const; // roleid不包含自己
	bool GetTeammates(std::vector<RoleID>& members_vec) const; // 获取队友，不包含自己
	int  GetSelfPos(); // 返回值小于0表示没有队伍
	int  GetTeamId() const;
	RoleID GetLeaderId() const;
    int  GetTeammateCount() const; // 不包含自己
	void StartRecvMapTeamInfo();


protected:
	void CMDHandler_MapTeamChangePos(const C2G::MapTeamChangePos&);
	void CMDHandler_MapTeamChangeLeader(const C2G::MapTeamChangeLeader&);
	void CMDHandler_MapTeamQueryMember(const C2G::MapTeamQueryMember&);
    void CMDHandler_MapTeamJoinTeam(const C2G::MapTeamJoinTeam&);
    void CMDHandler_MapTeamLeaveTeam(const C2G::MapTeamLeaveTeam&);
    void CMDHandler_MapTeamKickoutMember(const C2G::MapTeamKickoutMember&);
    void CMDHandler_MapTeamJoinTeamRes(const C2G::MapTeamJoinTeamRes&);

	int  MSGHandler_MapTeamInfo(const MSG&);
	int  MSGHandler_MapTeamJoin(const MSG&);
	int  MSGHandler_MapTeamLeave(const MSG&);
	int  MSGHandler_MapTeamChangePos(const MSG&);
	int  MSGHandler_MapTeamChangeLeader(const MSG&);
	int  MSGHandler_MapTeamQueryMember(const MSG&);
	int  MSGHandler_MapTeamStatusChange(const MSG&);
    int  MSGHandler_MapTeamJoinTeam(const MSG&); // 有玩家发起邀请或申请
    int  MSGHandler_MapTeamTidyPos(const MSG&);

private:
    struct OtherJoinRequest
    {
        OtherJoinRequest() { clear(); }

        void clear()
        {
            requester    = 0;
        }

        bool check_requester(RoleID rid) const
        {
            if (requester != 0 && rid != requester)
                return false;
            return true;
        }

        RoleID  requester;
        map_team_player_info info;
    };

private:
	void RefreshTeamInfo(int message);  // 发生错误时重新同步队伍信息
	void ClearTeamInfo();
	void SendMapTeamInfo() const;
	void SendToClient(shared::net::ProtoPacket& packet) const;
	void SendPlaneMsg(int message, int64_t param = 0, const void* buf = NULL, size_t len = 0);
    void ErrorMsgToOtherJoin(int err_no);
    void BuildMapTeamPlayerInfo(map_team_player_info& info);
	
    inline bool CheckMembersIndex(int index) const;
	inline bool FindMember(RoleID roleid, int& index) const; // 包含自己


private:
	bool already_getalldata_;
	msgpack_map_team_info team_info_;

    // join request
    OtherJoinRequest other_join_req_;
};

///
/// inline func
///
inline bool PlayerMapTeam::CheckMembersIndex(int index) const
{	
	if (index < 0 || index >= kTeamMemberCount)
		return false;
	return true;
}

inline bool PlayerMapTeam::FindMember(RoleID roleid, int& index) const
{
	for (int i = 0; i < kTeamMemberCount; ++i)
	{
		if (roleid == team_info_.members[i].roleid)
		{
			index = i;
			return true;
		}
	}
	return false;
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_MAP_TEAM_H_
