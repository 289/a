#include "player_gevent.h"

#include "gs/global/timer.h"
#include "gs/global/glogger.h"
#include "gs/global/dbgprt.h"
#include "gs/global/time_slot.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/template/data_templ/gevent_group_templ.h"
#include "gs/template/data_templ/gevent_templ.h"
#include "gs/template/data_templ/templ_manager.h"

namespace gamed
{

using namespace std;
using namespace dataTempl;

#define GetGeventTempl(id) s_pDataTempl->QueryDataTempl<GeventTempl>(id)
#define GetGeventGroupTempl(id) s_pDataTempl->QueryDataTempl<GeventGroupTempl>(id)

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

PlayerGevent::PlayerGevent(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_GEVENT, player), reset_time_(0)
{
	SAVE_LOAD_REGISTER(common::PlayerGeventData, PlayerGevent::SaveToDB, PlayerGevent::LoadFromDB);
}

PlayerGevent::~PlayerGevent()
{
}

void PlayerGevent::ResetGevent(int32_t now)
{
    if (now <= reset_time_)
    {
        return;
    }

    GeventEntryMap::iterator it = gevent_map_.begin();
    for (; it != gevent_map_.end();)
    {
        if (it->second.last_join_time <= reset_time_ && now > reset_time_)
        {
            gevent_map_.erase(it++);
        }
        else
        {
            ++it;
        }
    }
    int32_t diff = now - reset_time_;
    if (diff > 0)
    {
        reset_time_ += SEC_PER_DAY * ((now - reset_time_) / SEC_PER_DAY + 1);
    }
}

bool PlayerGevent::SaveToDB(common::PlayerGeventData* pData)
{
    GeventEntryMap::iterator it = gevent_map_.begin();
    for (; it != gevent_map_.end(); ++it)
    {
        it->second.gevent_gid = it->first;
        pData->gevent_list.push_back(it->second);
    }
    return true;
}

bool PlayerGevent::LoadFromDB(const common::PlayerGeventData& data)
{
    time_t now = g_timer->GetSysTime();
    reset_time_ = CalcResetTime(now);
    for (size_t i = 0; i < data.gevent_list.size(); ++i)
    {
        const common::PlayerGeventData::gevent_entry& entry = data.gevent_list[i];
        gevent_map_[entry.gevent_gid] = entry;
    }
    ResetGevent(now);
    return true;
}

void PlayerGevent::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::JoinGevent, PlayerGevent::CMDHandler_JoinGevent);
}

void PlayerGevent::PlayerGetGeventData() const
{
    G2C::GeventData packet;
    GeventEntryMap::const_iterator it = gevent_map_.begin();
    for (; it != gevent_map_.end(); ++it)
    {
        G2C::GeventInfo info;
        info.gevent_gid = it->second.gevent_gid;
        info.last_join_time = it->second.last_join_time;
        info.num = it->second.num;
        packet.gevent_list.push_back(info);
    }
    player_.sender()->SendCmd(packet);
}

void PlayerGevent::SendJoinReply(int8_t result, int32_t gevent_gid, int32_t gevent_id) const
{
    G2C::JoinGevent_Re packet;
    packet.result = result;
    packet.gevent_gid = gevent_gid;
    packet.gevent_id = gevent_id;
    player_.sender()->SendCmd(packet);
}

static const GeventTempl* SelectGevent(Player& player, const GeventGroupTempl* group)
{
    for (size_t i = 0; i < group->gevent_list.size(); ++i)
    {
        const GeventTempl* templ = GetGeventTempl(group->gevent_list[i]);
        if (templ == NULL)
        {
            break;
        }
        if (player.level() >= templ->min_level && player.level() <= templ->max_level)
        {
            return templ;
        }
    }
    return NULL;
}

void PlayerGevent::CMDHandler_JoinGevent(const C2G::JoinGevent& packet)
{
    const GeventGroupTempl* group_templ = GetGeventGroupTempl(packet.gevent_gid);
    if (group_templ == NULL)
    {
        return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_GID, packet.gevent_gid);
    }

    ResetGevent(g_timer->GetSysTime());

    if (group_templ->max_num != -1 && gevent_map_[packet.gevent_gid].num >= group_templ->max_num)
    {
        return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_NUM, packet.gevent_gid);
    }

    size_t i = 0;
    for (i = 0; i< group_templ->open_time.size(); ++i)
    {
        if (group_templ->open_time[i].is_valid)
        {
            TimeSlot time_slot;
            BuildTimeSlot(time_slot, group_templ->open_time[i]);
            if (time_slot.IsMatch(g_timer->GetSysTime()))
            {
                break;
            }
        }
    }
    if (i != 0 && i == group_templ->open_time.size())
    {
        return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_TIME, packet.gevent_gid);
    }

    const GeventTempl* templ = SelectGevent(player_, group_templ);
    if (templ == NULL)
    {
        return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_LEVEL, packet.gevent_gid);
    }

    if (templ->join_type == GEVENT_JOIN_TASK)
    {
        for (size_t i = 0; i < templ->task_list.size(); ++i)
        {
            if (templ->task_list[i] != 0 && !player_.CanDeliverTask(templ->task_list[i]))
            {
                return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_TASK, packet.gevent_gid);
            }
        }
        for (size_t i = 0; i < templ->task_list.size(); ++i)
        {
            if (templ->task_list[i] != 0)
            {
                player_.DeliverTask(templ->task_list[i]);
            }
        }
        return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_SUCC, packet.gevent_gid, templ->templ_id);
    }

    if (!IS_NORMAL_MAP(player_.world_id()))
    {
        return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_MAP, packet.gevent_gid);
    }

    if (templ->join_type == GEVENT_JOIN_INSTANCE)
    {
        player_.GeventTransferSoloIns(templ->ins_id);
    }
    else if (templ->join_type == GEVENT_JOIN_BATTLEGROUND)
    {
        player_.EnterPveBattleGround(templ->battleground_id);
    }
    return SendJoinReply(G2C::JoinGevent_Re::JOIN_GEVENT_SUCC, packet.gevent_gid, templ->templ_id);
}

void PlayerGevent::CompleteGevent(int32_t gevent_gid)
{
    const GeventGroupTempl* group_templ = GetGeventGroupTempl(gevent_gid);
    if (group_templ == NULL)
    {
        return;
    }

    ResetGevent(g_timer->GetSysTime());

    common::PlayerGeventData::gevent_entry& entry = gevent_map_[gevent_gid];
    if (entry.num < group_templ->max_num || group_templ->max_num == -1)
    {
        player_.ModifyParticipation(group_templ->participation);
        entry.gevent_gid = gevent_gid;
        entry.last_join_time = g_timer->GetSysTime();
        ++entry.num;

        NotifyDataChange(entry);
    }
}

void PlayerGevent::NotifyDataChange(const common::PlayerGeventData::gevent_entry& entry) const
{
    G2C::GeventDataChange packet;
    packet.gevent_data.gevent_gid = entry.gevent_gid;
    packet.gevent_data.last_join_time = entry.last_join_time;
    packet.gevent_data.num = entry.num;

    player_.sender()->SendCmd(packet);
}

void PlayerGevent::SetGeventNum(int32_t gevent_gid, int32_t num)
{
    const GeventGroupTempl* group_templ = GetGeventGroupTempl(gevent_gid);
    if (group_templ == NULL)
    {
        return;
    }

    common::PlayerGeventData::gevent_entry& entry = gevent_map_[gevent_gid];
    entry.gevent_gid = gevent_gid;
    entry.last_join_time = g_timer->GetSysTime();
    entry.num = num;

    NotifyDataChange(entry);
}

} // namespace gamed
