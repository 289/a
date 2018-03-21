#ifndef GAMED_GS_SCENE_INSTANCE_INS_PLAYER_IMP_H_
#define GAMED_GS_SCENE_INSTANCE_INS_PLAYER_IMP_H_

#include "gs/player/player.h"
#include "gs/scene/world_def.h"


namespace dataTempl {
	class InstanceTempl;
} // namespace dataTempl


namespace gamed {

struct msg_ins_finish_result;

/**
 * @brief InsPlayerImp
 */
class InsPlayerImp : public PlayerImp
{
	static const int kLogoutCountdownTime    = 55; // 单位s, 要小于InsWorldManImp::kKickoutCloseTime
	static const int kNotMeetCondKickoutTime = 3;  // 玩家不满足条件时，踢人时间
	static const int kQuitInsKickoutTime     = 2;  // 玩家主动退出副本，踢人时间
public:
	InsPlayerImp(Player& player);
	virtual ~InsPlayerImp();

	virtual void OnEnterWorld();
	virtual void OnLeaveWorld();
	virtual void OnHeartbeat();

	virtual void OnGetAllData();
	virtual void OnChangeGsComplete();
	virtual void OnWorldClosing(const MSG& msg);
	virtual int  OnMessageHandler(const MSG& msg);


private:
	void InsChangeMapLogout();
	void KickoutPlayerCountdown(int countdown_secs);
	void KickoutPlayer(int countdown_secs);
	bool CheckPlayerCondition() const;
	void PlayerQuitInsMap();
	void ContinuousKickout();
	void CalcPlayerRecord(const msg_ins_finish_result& param);
	void HandleInsQueryInfo();


private:
	world::instance_info ins_info_;         // 只有init的时候会设置，副本生命周期内不会变化
	int32_t              logout_countdown_; // 用于倒计时踢人
	int32_t              ins_team_id_;
	bool                 is_quitting_;
    bool                 ins_end_;
	const dataTempl::InstanceTempl* ins_templ_;
};

} // namespace gamed

#endif // GAMED_GS_SCENE_INSTANCE_INS_PLAYER_IMP_H_
