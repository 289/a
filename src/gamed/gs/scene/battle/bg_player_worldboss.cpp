#include "bg_player_worldboss.h"


namespace gamed {

BGPlayerWorldBoss::BGPlayerWorldBoss(BGPlayerImp& imp)
    : BGPlayerLite(imp)
{
}

BGPlayerWorldBoss::~BGPlayerWorldBoss()
{
}

void BGPlayerWorldBoss::OnHandleBGFinish()
{
    bg_player_imp_.KickoutPlayerCountdown(kKickoutCloseTime);
}

} // namespace gamed
