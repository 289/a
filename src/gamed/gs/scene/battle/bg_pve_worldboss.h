#ifndef GAMED_GS_SCENE_BATTLE_BG_PVE_WORLDBOSS_H_
#define GAMED_GS_SCENE_BATTLE_BG_PVE_WORLDBOSS_H_

#include "bg_worldman_imp.h"


namespace gamed {

class BGPveWorldBoss : public BattleGroundLite
{
public:
    BGPveWorldBoss(BGWorldManImp& worldManImp);
    virtual ~BGPveWorldBoss();

	virtual int OnMessageHandler(const MSG& msg);


private:
    void HandleWorldBossDead(int32_t monster_tid);

private:
};

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_PVE_WORLDBOSS_H_
