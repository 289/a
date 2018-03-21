#include "player_participation.h"

#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/player/subsys_if.h"
#include "gs/item/item.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/player/player_sender.h"

namespace gamed
{

using namespace std;
using namespace dataTempl;

#define BIT_PER_BYTE 8

static int32_t CalcResetTime(time_t now)
{
	struct tm date;
#ifdef PLATFORM_WINDOWS
	localtime_s(&date, &now);
#else // !PLATFORM_WINDOWS
	localtime_r(&now, &date);
#endif // PLATFORM_WINDOWS
    date.tm_hour = 5;
    date.tm_min = 0;
    date.tm_sec = 0;
    return mktime(&date);
}

PlayerParticipation::PlayerParticipation(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_PARTICIPATION, player), reset_time_(0)
{
	SAVE_LOAD_REGISTER(common::PlayerParticipationData, PlayerParticipation::SaveToDB, PlayerParticipation::LoadFromDB);
}

PlayerParticipation::~PlayerParticipation()
{
}

void PlayerParticipation::ResetParticipation(int32_t now)
{
    if (now <= reset_time_)
    {
        return;
    }
    if (player_.last_login_time() <= reset_time_ && now > reset_time_)
    {
        data_.participation = 0;
        ClearParticipationAward();
    }
    int32_t diff = now - reset_time_;
    if (diff > 0)
    {
        reset_time_ += SEC_PER_DAY * ((now - reset_time_) / SEC_PER_DAY + 1);
    }
}

bool PlayerParticipation::SaveToDB(common::PlayerParticipationData* pData)
{
    *pData = data_;
    return true;
}

bool PlayerParticipation::LoadFromDB(const common::PlayerParticipationData& data)
{
    time_t now = g_timer->GetSysTime();
    reset_time_ = CalcResetTime(now);

    data_ = data;
    const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
    int32_t size = pTpl->participation_config.kMaxAwardNum / BIT_PER_BYTE;
    if (pTpl->participation_config.kMaxAwardNum % BIT_PER_BYTE != 0)
    {
        size += 1;
    }
    data_.award_info.resize(size);

    ResetParticipation(now);
    return true;
}

void PlayerParticipation::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::GetParticipationAward, PlayerParticipation::CMDHandler_GetParticipationAward);
}

void PlayerParticipation::PlayerGetParticipationData() const
{
    G2C::ParticipationData packet;
    packet.participation = data_.participation;
    packet.award_info = data_.award_info;
    player_.sender()->SendCmd(packet);
}

void PlayerParticipation::ModifyParticipation(int32_t value)
{
    if (value < 0)
    {
        return;
    }

    ResetParticipation(g_timer->GetSysTime());

    const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
    if (data_.participation + value > pTpl->participation_config.max_participation)
    {
        value = pTpl->participation_config.max_participation - data_.participation;
    }
    SetParticipation(data_.participation + value);
}

void PlayerParticipation::SetParticipation(int32_t value)
{
    if (value < 0 || data_.participation == value)
    {
        return;
    }

    data_.participation = value;
    G2C::ParticipationChange reply;
    reply.new_value = value;
    player_.sender()->SendCmd(reply);
}

void PlayerParticipation::ClearParticipationAward()
{
    data_.award_info.clear();
    const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
    size_t size = pTpl->participation_config.kMaxAwardNum / BIT_PER_BYTE;
    if (pTpl->participation_config.kMaxAwardNum % BIT_PER_BYTE != 0)
    {
        size += 1;
    }
    data_.award_info.resize(size);
}

void PlayerParticipation::CMDHandler_GetParticipationAward(const C2G::GetParticipationAward& cmd)
{
    const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
    if (cmd.award_index < 0 || cmd.award_index > (int32_t)pTpl->participation_config.awards.size())
    {
        return;
    }

    ResetParticipation(g_timer->GetSysTime());

    // 检查活跃度是否足够
    const ParticipationAward& award = pTpl->participation_config.awards[cmd.award_index];
    if (data_.participation < award.participation)
    {
        return;
    }
    // 检查是否领过奖
    int32_t group_index = cmd.award_index / BIT_PER_BYTE;
    ASSERT(group_index < (int32_t)data_.award_info.size());
    char& value = data_.award_info[group_index];
    char flag = 0x01 << (cmd.award_index % BIT_PER_BYTE);
    if ((value & flag) != 0)
    {
        return;
    }
    // 检查包裹是否已满
    G2C::GetParticipationAward_Re reply;
    reply.award_index = cmd.award_index;
    if (!player_.HasSlot(Item::INVENTORY, award.items.size()))
    {
        reply.result = G2C::GetParticipationAward_Re::GET_AWARD_FULL;
        player_.sender()->SendCmd(reply);
        return;
    }
    // 发奖
    value |= flag;
    if (award.exp != 0)
    {
        player_.IncExp(award.exp);
    }
    if (award.money != 0)
    {
        player_.GainMoney(award.money);
    }
    if (award.score != 0)
    {
        player_.GainScore(award.score);
    }
    for (size_t i = 0; i < award.items.size(); ++i)
    {
        if (award.items[i].item_id != 0)
        {
            player_.GainItem(award.items[i].item_id, award.items[i].item_count);
        }
    }
    reply.result = G2C::GetParticipationAward_Re::GET_AWARD_SUCC;
    player_.sender()->SendCmd(reply);
}

} // namespace gamed
