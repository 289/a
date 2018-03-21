#include "player_boss_challenge.h"

#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/player/subsys_if.h"
#include "gs/item/item.h"
#include "gs/template/data_templ/boss_challenge_templ.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/player/player_sender.h"
#include "game_module/combat/include/combat_def.h"
#include "common/obj_data/gen/player/challenge_data.pb.h"

namespace gamed
{

using namespace std;
using namespace common;
using namespace dataTempl;

#define GetBossTempl(id) s_pDataTempl->QueryDataTempl<BossChallengeTempl>(id)

PlayerBossChallenge::PlayerBossChallenge(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_BOSS_CHALLENGE, player)
{
	SAVE_LOAD_REGISTER(PlayerBossChallengeData, PlayerBossChallenge::SaveToDB, PlayerBossChallenge::LoadFromDB);
}

PlayerBossChallenge::~PlayerBossChallenge()
{
}

bool PlayerBossChallenge::SaveToDB(PlayerBossChallengeData* pData)
{
    if (data_.boss.empty())
    {
        return true;
    }
    scalable::ChallengeData tmp;
    G2C::BossChallengeData::BossGroupMap::const_iterator bit = data_.boss.begin();
    for (; bit != data_.boss.end(); ++bit)
    {
        scalable::ChallengeData_ChallengeInfo* info = tmp.add_challenge_list();
        info->set_challenge_id(bit->first);
        G2C::BossChallengeData::BossMap::const_iterator it = bit->second.begin();
        for (; it != bit->second.end(); ++it)
        {
            scalable::ChallengeData_BossEntry* entry = info->add_boss_list();
            entry->set_monster_gid(it->first);
            entry->set_challenge_info(it->second);
        }
    }
    pData->challenge_data.resize(tmp.ByteSize()); 
    if (!tmp.SerializeToArray((void*)pData->challenge_data.c_str(), pData->challenge_data.size()))
    {
        GLog::log("PlayerTitle::Save Serialize error! roleid:%ld", player_.role_id());
        return false;
    }
    return true;
}

bool PlayerBossChallenge::LoadFromDB(const PlayerBossChallengeData& data)
{
    if (data.challenge_data.size() > 0)
    {
        scalable::ChallengeData tmp;
        if (!tmp.ParseFromArray(data.challenge_data.c_str(), data.challenge_data.size()))
        {
            GLog::log("PlayerBossChallenge::Load error! roleid: ", player_.role_id());
            return false;
        }
        for (int32_t i = 0; i < tmp.challenge_list_size(); ++i)
        {
            const scalable::ChallengeData_ChallengeInfo& info = tmp.challenge_list(i);
            G2C::BossChallengeData::BossMap& boss_map = data_.boss[info.challenge_id()];
            for (int32_t j = 0; j < info.boss_list_size(); ++j)
            {
                const scalable::ChallengeData_BossEntry& entry = info.boss_list(j);
                boss_map[entry.monster_gid()] = entry.challenge_info();
            }
        }
    }
    return true;
}

void PlayerBossChallenge::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::BossChallenge, PlayerBossChallenge::CMDHandler_BossChallenge);
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetBossChallengeAward, PlayerBossChallenge::CMDHandler_GetBossChallengeAward);
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetClearChallengeAward, PlayerBossChallenge::CMDHandler_GetClearChallengeAward);
}

void PlayerBossChallenge::PlayerGetBossChallengeData() const
{
    G2C::BossChallengeData packet;
    packet = data_;
    player_.sender()->SendCmd(packet);
}

int32_t PlayerBossChallenge::GetBossInfo(int32_t challenge_id, int32_t monster_gid) const
{
    G2C::BossChallengeData::BossGroupMap::const_iterator git = data_.boss.find(challenge_id);
    if (git == data_.boss.end())
    {
        return 0;
    }
    G2C::BossChallengeData::BossMap::const_iterator bit = git->second.find(monster_gid);
    return bit == git->second.end() ? 0 : bit->second;
}

void PlayerBossChallenge::WinBossChallenge(int8_t result, int32_t challenge_id, int32_t monster_gid)
{
    const BossChallengeTempl* templ = GetBossTempl(challenge_id);
    if (templ == NULL)
    {
        return;
    }
    // 本怪物过关
    if (result == combat::RESULT_WIN)
    {
        data_.boss[challenge_id][monster_gid] |= 0x01;
    }
    // 检查是否已经通关
    size_t i = 0;
    for (; i < templ->boss_list.size(); ++i)
    {
        if (GetBossInfo(challenge_id, templ->boss_list[i].monster_gid) == 0)
        {
            break;
        }
    }
    if (i == templ->boss_list.size())
    {
        data_.boss[challenge_id][0] |= 0x01;
    }
    G2C::BossChallenge_Re packet;
    packet.result = result;
    packet.challenge_id = challenge_id;
    packet.monster_gid = monster_gid;
    player_.sender()->SendCmd(packet);
}

void PlayerBossChallenge::CMDHandler_BossChallenge(const C2G::BossChallenge& packet)
{
    // 检查挑战组中是否有对应的怪物且目前可以挑战
    const BossChallengeTempl* templ = GetBossTempl(packet.challenge_id);
    if (templ == NULL)
    {
        return;
    }
    for (size_t i = 0; i < templ->boss_list.size(); ++i)
    {
        const BossChallengeTempl::BossEntry& boss = templ->boss_list[i];
        if (GetBossInfo(packet.challenge_id, boss.monster_gid) == 0)
        {
            if (boss.monster_gid == packet.monster_gid)
            {
                // 服务器发起战斗
                msg_sys_trigger_combat param;
                param.challenge_id = packet.challenge_id;
                param.task_id = 0;
                param.monster_group_id = packet.monster_gid;
                param.battle_scene_id = boss.battle_scene_id;
                player_.SendMsg(GS_MSG_SYS_TRIGGER_COMBAT, player_.object_xid(), &param, sizeof(param));
            }
            return;
        }
    }
}

void PlayerBossChallenge::CMDHandler_GetBossChallengeAward(const C2G::GetBossChallengeAward& packet)
{
    // 检查该BOSS是否已经完成挑战并且没有领奖
    G2C::GetBossChallengeAward_Re reply;
    reply.challenge_id = packet.challenge_id;
    reply.monster_gid = packet.monster_gid;
    int32_t value = GetBossInfo(packet.challenge_id, packet.monster_gid);
    if (value == 0)
    {
        reply.result = G2C::GetBossChallengeAward_Re::GET_BOSS_AWARD_NONE;
        player_.sender()->SendCmd(reply);
        return;
    }
    else if ((value & 0x02) != 0)
    {
        reply.result = G2C::GetBossChallengeAward_Re::GET_BOSS_AWARD_ALREADY;
        player_.sender()->SendCmd(reply);
        return;
    }
    // 检查是否有足够的包裹空间
    const BossChallengeTempl* templ = GetBossTempl(packet.challenge_id);
    if (templ == NULL)
    {
        return;
    }
    const BossChallengeTempl::BossEntry* entry = NULL;
    for (size_t i = 0; i < templ->boss_list.size(); ++i)
    {
        if (templ->boss_list[i].monster_gid == packet.monster_gid)
        {
            entry = &(templ->boss_list[i]);
            break;
        }
    }
    if (entry == NULL)
    {
        return;
    }
    if (!player_.HasSlot(Item::INVENTORY, entry->win_award_item.size()))
    {
        reply.result = G2C::GetBossChallengeAward_Re::GET_BOSS_AWARD_FULL;
        player_.sender()->SendCmd(reply);
        return;
    }
    // 发奖并设置领奖标志位
    player_.GainMoney(entry->win_award_money);
    player_.GainScore(entry->win_award_score);
    for (size_t i = 0; i < entry->win_award_item.size(); ++i)
    {
        const BossChallengeTempl::ItemEntry& item = entry->win_award_item[i];
        player_.GainItem(item.item_id, item.item_num);
    }
    data_.boss[packet.challenge_id][packet.monster_gid] |= 0x02;
    reply.result = G2C::GetBossChallengeAward_Re::GET_BOSS_AWARD_SUCC;
    player_.sender()->SendCmd(reply);
}

void PlayerBossChallenge::CMDHandler_GetClearChallengeAward(const C2G::GetClearChallengeAward& packet)
{
    G2C::GetClearChallengeAward_Re reply;
    reply.challenge_id = packet.challenge_id;
    int32_t value = GetBossInfo(packet.challenge_id, 0);
    // 检查是否已经通关
    if ((value & 0x01) == 0)
    {
        reply.result = G2C::GetClearChallengeAward_Re::GET_CLEAR_AWARD_NONE;
        player_.sender()->SendCmd(reply);
        return;
    }
    // 检查是否已经领取奖励
    if ((value & 0x02) != 0)
    {
        reply.result = G2C::GetClearChallengeAward_Re::GET_CLEAR_AWARD_ALREADY;
        player_.sender()->SendCmd(reply);
        return;
    }
    const BossChallengeTempl* templ = GetBossTempl(packet.challenge_id);
    if (!player_.HasSlot(Item::INVENTORY, templ->clear_award_item.size()))
    {
        reply.result = G2C::GetClearChallengeAward_Re::GET_CLEAR_AWARD_FULL;
        player_.sender()->SendCmd(reply);
        return;
    }
    // 发奖并设置领奖标志位
    player_.GainMoney(templ->clear_award_money);
    player_.GainScore(templ->clear_award_score);
    for (size_t i = 0; i < templ->clear_award_item.size(); ++i)
    {
        const BossChallengeTempl::ItemEntry& item = templ->clear_award_item[i];
        player_.GainItem(item.item_id, item.item_num);
    }
    data_.boss[packet.challenge_id][0] |= 0x02;
    reply.result = G2C::GetClearChallengeAward_Re::GET_CLEAR_AWARD_SUCC;
    player_.sender()->SendCmd(reply);
}

void PlayerBossChallenge::ClearBossChallenge(int32_t challenge_id)
{
    data_.boss.erase(challenge_id);
}

} // namespace gamed 
