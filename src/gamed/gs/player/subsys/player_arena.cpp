#include "player_arena.h"

#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/player/subsys_if.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/global_config.h"

namespace gamed {

using namespace std;
using namespace dataTempl;

#define GetArenaTempl(id) s_pDataTempl->QueryDataTempl<ArenaTempl>(id)

static int32_t CalcDayResetTime(time_t now)
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

static int32_t CalcWeekResetTime(time_t now)
{
	struct tm date;
#ifdef PLATFORM_WINDOWS
	localtime_s(&date, &now);
#else // !PLATFORM_WINDOWS
	localtime_r(&now, &date);
#endif // PLATFORM_WINDOWS
    date.tm_mday -= date.tm_wday;
    date.tm_hour = 5;
    date.tm_min = 0;
    date.tm_sec = 0;
    return mktime(&date);
}

PlayerArena::PlayerArena(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_ARENA, player)
{
    //SAVE_LOAD_REGISTER(common::PlayerArenaData, PlayerArena::SaveToDB, PlayerArena::LoadFromDB);
}

PlayerArena::~PlayerArena()
{
}

bool PlayerArena::SaveToDB(common::PlayerArenaData* pData)
{
    *pData = arena_data;
    return true;
}

bool PlayerArena::LoadFromDB(const common::PlayerArenaData& data)
{
    arena_data = data;

    //const GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
    time_t now = g_timer->GetSysTime();
    int32_t last_login_time = player_.last_login_time();
    int32_t day_reset_time = CalcDayResetTime(now);
    if (last_login_time < day_reset_time)
    {
        arena_data.today_score = 0;
        //arena_data.free_ticket = pTpl->arena_config.free_ticket_per_day;
    }
    int32_t week_reset_time = CalcWeekResetTime(now);
    if (last_login_time < week_reset_time)
    {
        arena_data.week_score = 0;
    }
    return true;
}

void PlayerArena::RegisterCmdHandler()
{
    //REGISTER_NORMAL_CMD_HANDLER(C2G::ArenaODQueryData, PlayerArena::CMDHandler_ArenaODQueryData);
    //REGISTER_NORMAL_CMD_HANDLER(C2G::ArenaODInviteAssist, PlayerArena::CMDHandler_ArenaODInviteAssist);
    //REGISTER_NORMAL_CMD_HANDLER(C2G::ArenaODAssistSelect, PlayerArena::CMDHandler_ArenaODAssistSelect);
    //REGISTER_NORMAL_CMD_HANDLER(C2G::ArenaODCombatStart, PlayerArena::CMDHandler_ArenaODCombatStart);
}

void PlayerArena::RegisterMsgHandler()
{
}
/*
void PlayerArena::CMDHandler_ArenaODQueryData(const C2G::ArenaODQueryData& cmd)
{
}

void PlayerArena::CMDHandler_ArenaODInviteAssist(const C2G::ArenaODInviteAssist& cmd)
{
}

void PlayerArena::CMDHandler_ArenaODAssistSelect(const C2G::ArenaODAssistSelect& cmd)
{
}

void PlayerArena::CMDHandler_ArenaODBuyTicket(const C2G::ArenaODBuyTicket& cmd)
{
}

void PlayerArena::CMDHandler_ArenaODCombatStart(const C2G::ArenaODCombatStart& cmd)
{
}
*/
} // namespace gamed
