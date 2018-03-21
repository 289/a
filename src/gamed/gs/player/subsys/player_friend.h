#ifndef GAMED_GS_SUBSYS_PLAYER_FRIEND_H_
#define GAMED_GS_SUBSYS_PLAYER_FRIEND_H_

#include "gs/player/player_subsys.h"

namespace gamed
{

/**
 * @brief：player好友子系统
 */
class PlayerFriend : public PlayerSubSystem
{
public:
	PlayerFriend(Player& player);
	virtual ~PlayerFriend();

	bool SaveToDB(common::PlayerFriendData* pData);
	bool LoadFromDB(const common::PlayerFriendData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    bool IsFriend(int64_t roleid) const;    
    void PlayerGetFriendData();
    int32_t GetFriendNum() const;
    int32_t GetEnemyNum() const;
    void AddNPCFriend(int32_t id);
    void DelNPCFriend(int32_t id);
    void OnlineNPCFriend(int32_t id);
    void OfflineNPCFriend(int32_t id);
protected:
	//
	// CMD处理函数
	//
	void CMDHandler_AddFriend(const C2G::AddFriend&);
	void CMDHandler_DeleteFriend(const C2G::DeleteFriend&);
	void CMDHandler_QueryRoleInfo(const C2G::QueryRoleInfo&);
private:
    typedef std::map<int64_t, int8_t> FriendMap;
    FriendMap friend_list_;
    FriendMap enemy_list_;
    FriendMap black_list_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_FRIEND_H_
