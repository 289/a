#include "player_world_boss.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/monster_templ.h"
#include "gs/global/global_data.h"
#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"


namespace gamed {

using namespace dataTempl;

PlayerWorldBoss::PlayerWorldBoss(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_WORLD_BOSS, player),
      query_cooldown_(0)
{
}

PlayerWorldBoss::~PlayerWorldBoss()
{
}

void PlayerWorldBoss::OnHeartbeat(time_t cur_time)
{
    if (query_cooldown_ > 0)
    {
        --query_cooldown_;
    }
}

bool PlayerWorldBoss::CheckCoolDown() const
{
    if (query_cooldown_ > 0)
        return false;
    return true;
}

bool PlayerWorldBoss::QueryGlobalData(int32_t monster_tid, G2C::WBDamageList& list)
{
    const MonsterTempl* pmonster = s_pDataTempl->QueryDataTempl<MonsterTempl>(monster_tid);
    if (pmonster == NULL)
    {
        player_.sender()->ErrorMessage(G2C::ERR_WORLD_BOSS_NOT_FOUND);
        return false;
    }

    globalData::WorldBossRecord::QueryValue query_value;
    query_value.role_id     = player_.role_id();
    query_value.query_count = kMaxQueryRankingCount;
    if (!s_pGlobalData->Query<globalData::WorldBossRecord>(player_.master_id(), monster_tid, query_value))
    {
        query_value.out_ranking = -1;
        query_value.out_damage  = -1;
    }

    // packet
    list.monster_tid = monster_tid;
    list.ranking     = query_value.out_ranking;
    list.damage      = query_value.out_damage;
    for (size_t i = 0; i < query_value.info_vec.size(); ++i)
    {
        const globalData::WorldBossRecord::PInfo& info = query_value.info_vec[i];
        G2C::WBDamageRecord tmprecord;
        tmprecord.damage     = info.damage;
        tmprecord.roleid     = info.role_id;
        tmprecord.first_name = info.first_name;
        tmprecord.mid_name   = info.mid_name;
        tmprecord.last_name  = info.last_name;
        list.records.push_back(tmprecord);
    }

    return true;
}

void PlayerWorldBoss::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::QueryWorldBossRecord, PlayerWorldBoss::CMDHandler_QueryWorldBossRecord);
}

void PlayerWorldBoss::RegisterMsgHandler()
{
    REGISTER_MSG_HANDLER(GS_MSG_WORLD_BOSS_DEAD, PlayerWorldBoss::MSGHandler_WorldBossDead);
}

void PlayerWorldBoss::CMDHandler_QueryWorldBossRecord(const C2G::QueryWorldBossRecord& cmd)
{
    if (!CheckCoolDown())
        return;
    
    G2C::QueryWorldBossRecord_Re packet;
    if (QueryGlobalData(cmd.monster_tid, packet.list))
    {
        player_.sender()->SendCmd(packet);
    }
}

int PlayerWorldBoss::MSGHandler_WorldBossDead(const MSG& msg)
{
    G2C::WBCombatEndRecord packet;
    if (QueryGlobalData(msg.param, packet.list))
    {
        player_.sender()->SendCmd(packet);
    }
    return 0;
}

} // namespace gamed
