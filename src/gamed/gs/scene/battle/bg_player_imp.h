#ifndef GAMED_GS_SCENE_BATTLE_BG_PLAYER_IMP_H_
#define GAMED_GS_SCENE_BATTLE_BG_PLAYER_IMP_H_

#include "gs/player/player.h"
#include "gs/scene/world_def.h"


namespace dataTempl {
    class BattleGroundTempl;
} // namespace dataTempl


namespace gamed {

class BGPlayerLite;

/**
 * @brief BGPlayerImp
 */
class BGPlayerImp : public PlayerImp
{
    // 友元类只能访问public及protected函数
    friend class BGPlayerWorldBoss;

	static const int kLogoutCountdownTime    = 45; // 单位s, 要小于BGWorldManImp::kKickoutCloseTime
	static const int kNotMeetCondKickoutTime = 3;  // 玩家不满足条件时，踢人时间
	static const int kQuitBGKickoutTime      = 2;  // 玩家主动退出战场，踢人时间
public:
	BGPlayerImp(Player& player);
	virtual ~BGPlayerImp();

	virtual void OnEnterWorld();
	virtual void OnLeaveWorld();
	virtual void OnHeartbeat();

	virtual void OnGetAllData();
	virtual void OnChangeGsComplete();
	virtual void OnWorldClosing(const MSG& msg);
	virtual int  OnMessageHandler(const MSG& msg); // 子类实现该函数时，必须手工调用父类该函数


protected:
	void KickoutPlayerCountdown(int countdown_secs);


private:
	void BGChangeMapLogout();
	void KickoutPlayer(int countdown_secs);
	bool CheckPlayerCondition() const;
	void PlayerQuitBGMap();
	void ContinuousKickout();
    void HandleBGQueryInfo();
	void HandleBGFinish(const msg_bg_finish_result& param);


private:
	world::battleground_info bg_info_; // 只有init的时候会设置，战场生命周期内不会变化
	int32_t    logout_countdown_;      // 用于倒计时踢人
	bool       is_quitting_;
    bool       bg_end_;
	const dataTempl::BattleGroundTempl* bg_templ_;

    BGPlayerLite* plite_;
};


/**
 * @brief 不同类型战场里玩家的多态基类
 */
class BGPlayerLite
{
public:
    BGPlayerLite(BGPlayerImp& imp) 
        : bg_player_imp_(imp)
    { }
    virtual ~BGPlayerLite() { }

	virtual bool OnCheckPlayerCond() const { return true; }
    virtual void OnHandleBGFinish()        { }

protected:
    BGPlayerImp& bg_player_imp_;
};

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_PLAYER_IMP_H_
