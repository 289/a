#ifndef GAMED_GS_SUBSYS_PLAYER_BOSS_CHALLENGE_H_
#define GAMED_GS_SUBSYS_PLAYER_BOSS_CHALLENGE_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"
#include "gamed/client_proto/G2C_proto.h"

namespace gamed
{

class PlayerBossChallenge : public PlayerSubSystem
{
public:
    PlayerBossChallenge(Player& player);
    virtual ~PlayerBossChallenge();

	bool SaveToDB(common::PlayerBossChallengeData* pData);
	bool LoadFromDB(const common::PlayerBossChallengeData& data);

	virtual void RegisterCmdHandler();

    void PlayerGetBossChallengeData() const;
    void WinBossChallenge(int8_t result, int32_t challenge_id, int32_t monster_gid);

    // 仅限Debug使用
    void ClearBossChallenge(int32_t challenge_id);
protected:
    //
    // CMD处理函数
    //
    void CMDHandler_BossChallenge(const C2G::BossChallenge&);
    void CMDHandler_GetBossChallengeAward(const C2G::GetBossChallengeAward&);
    void CMDHandler_GetClearChallengeAward(const C2G::GetClearChallengeAward&);
private:
    int32_t GetBossInfo(int32_t challenge_id, int32_t monster_gid) const;
private:
    G2C::BossChallengeData data_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_BOSS_CHALLENGE_H_
