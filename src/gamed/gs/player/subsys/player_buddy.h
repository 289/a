#ifndef GAMED_GS_SUBSYS_PLAYER_BUDDY_H_
#define GAMED_GS_SUBSYS_PLAYER_BUDDY_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief: player伙伴系统（包括npc组队等）
 */
class PlayerBuddy : public PlayerSubSystem
{
public:
	static const int kTeamMemberCount = 4;

public:
	PlayerBuddy(Player& player);
	virtual ~PlayerBuddy();

	virtual void OnRelease();
	virtual void RegisterCmdHandler();

	void ObtainBuddy(int32_t buddy_tid);
	void TakeoutBuddy(int32_t buddy_tid);

	bool CanJoinPlayerTeam() const;
	bool HasBuddyTeam() const;
	int  GetSelfPos() const; // 返回值小于0表示没有队伍
	bool GetBuddyMembers(std::vector<playerdef::BuddyMemberInfo>& buddy_vec) const; // 不包括玩家自己
	void StartRecvBuddyInfo();


protected:
	void CMDHandler_ChangeBuddyTeamPos(const C2G::ChangeBuddyTeamPos&);


private:
	// copyable
	struct MemberInfo
	{
		MemberInfo() : id(0), tpl_id(0) { }
		int32_t id;     // 0代表该位置为空，负数代表是玩家自己，正数代表是伙伴
		int32_t tpl_id; // template id
	};
	struct TeamInfo
	{
		TeamInfo();
		void Release();

		RoleID     leader;
		MemberInfo members[kTeamMemberCount];
	};

	void CreateTeam(int32_t buddy_tid);
	void JoinTeam(int32_t buddy_tid);
	void LeaveTeam(int32_t buddy_tid);
	void CheckTeamExist();
	void ClearTeamInfo();
	void SendJoinTeam(int32_t id, int32_t index, int tpl_id);
	void SendLeaveTeam(int32_t id);

	inline int32_t get_next_buddy_id();


private:
	int32_t  next_buddy_id_;
	TeamInfo team_info_;
	bool     is_getalldata_complete_;

	typedef std::vector<int32_t> WaitingBuddyVec;
	WaitingBuddyVec waiting_buddy_;
};

///
/// inline func
///
inline int32_t PlayerBuddy::get_next_buddy_id()
{
	return next_buddy_id_++;
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_BUDDY_H_
