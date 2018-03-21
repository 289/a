#ifndef GAMED_GS_SCENE_BATTLE_BG_PLAYER_WORLDBOSS_H_
#define GAMED_GS_SCENE_BATTLE_BG_PLAYER_WORLDBOSS_H_

#include "bg_player_imp.h"


namespace gamed {

class BGPlayerWorldBoss : public BGPlayerLite
{
    static const int kKickoutCloseTime = 10; // 单位秒
public:
    BGPlayerWorldBoss(BGPlayerImp& imp);
    virtual ~BGPlayerWorldBoss();
    
    virtual void OnHandleBGFinish();

private:
};

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_PLAYER_WORLDBOSS_H_
