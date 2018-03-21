#include "bg_pve_worldboss.h"

#include "gs/global/message.h"
#include "gs/global/msg_plane.h"


namespace gamed {

BGPveWorldBoss::BGPveWorldBoss(BGWorldManImp& worldManImp)
    : BattleGroundLite(worldManImp)
{
}

BGPveWorldBoss::~BGPveWorldBoss()
{
}

int BGPveWorldBoss::OnMessageHandler(const MSG& msg)
{
    switch (msg.message)
	{
        case GS_PLANE_MSG_WORLD_BOSS_DEAD:
            {
                HandleWorldBossDead(msg.param);
            }
            break;

        default:
            return -1;
    }

    return 0;
}

void BGPveWorldBoss::HandleWorldBossDead(int32_t monster_tid)
{
    worldman_imp_.SetBattleGroundFinish(BGWorldManImp::BGR_SYS_WORLD_BOSS_DEAD);
}

} // namespace gamed
