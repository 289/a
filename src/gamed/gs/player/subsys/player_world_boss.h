#ifndef GAMED_GS_SUBSYS_PLAYER_WORLD_BOSS_H_
#define GAMED_GS_SUBSYS_PLAYER_WORLD_BOSS_H_

#include "gs/player/player_subsys.h"


namespace G2C {
    class WBDamageList;
} // namespace G2C


namespace gamed {

/**
 * @brief：player世界BOSS子系统
 */
class PlayerWorldBoss : public PlayerSubSystem
{
    static const int kQueryCoolDownSecs    = 4;  // 单位秒
    static const int kMaxQueryRankingCount = 10; // 查询排行榜内前多少名的数据
public:
	PlayerWorldBoss(Player& player);
	virtual ~PlayerWorldBoss();

	virtual void OnHeartbeat(time_t cur_time);
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

protected:
	void CMDHandler_QueryWorldBossRecord(const C2G::QueryWorldBossRecord&);
    int  MSGHandler_WorldBossDead(const MSG&);

private:
    bool QueryGlobalData(int32_t monster_tid, G2C::WBDamageList& list);
    bool CheckCoolDown() const;

private:
    int  query_cooldown_;
}; 

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_WORLD_BOSS_H_
