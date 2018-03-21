#include "player_counter.h"

#include "common/obj_data/gen/player/counter_data.pb.h"
#include "gamed/client_proto/G2C_proto.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/player_counter_templ.h"
#include "gs/global/glogger.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

#define CT_REGISTER G2C::PlayerCounterChange::CT_REGISTER
#define CT_UNREGISTER G2C::PlayerCounterChange::CT_UNREGISTER
#define CT_MODIFY G2C::PlayerCounterChange::CT_MODIFY

namespace {

    inline bool CheckCounterValidity(int32_t tid)
    {
        const dataTempl::PlayerCounterTempl* ptpl = s_pDataTempl->QueryDataTempl<dataTempl::PlayerCounterTempl>(tid);
        if (ptpl == NULL)
            return false;

        if (ptpl->validity == dataTempl::PlayerCounterTempl::IS_INVALID)
            return false;

        return true;
    }

} // Anonymous

///
/// PlayerCounter
///
PlayerCounter::PlayerCounter(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_PLAYER_COUNTER, player)
{
    SAVE_LOAD_REGISTER(common::PlayerCounterData, PlayerCounter::SaveToDB, PlayerCounter::LoadFromDB);
}

PlayerCounter::~PlayerCounter()
{
}

bool PlayerCounter::SaveToDB(common::PlayerCounterData* pData)
{
    if (counter_map_.size() > 0)
    {
        common::scalable::CounterData scalable_data;
        CounterMap::const_iterator it = counter_map_.begin();
        for (; it != counter_map_.end(); ++it)
        {
            if (!CheckCounterValidity(it->first))
                continue;

            common::scalable::CounterData::Entry* ent = scalable_data.add_counter_list();
            ent->set_tid(it->first);
            ent->set_value(it->second);
        }

        std::string& tmpbuf = pData->info;
        tmpbuf.resize(scalable_data.ByteSize());
        if (!scalable_data.SerializeToArray((void*)tmpbuf.c_str(), tmpbuf.size()))
        {
            GLog::log("PlayerCounter::SaveToDB Serialize error! roleid:%ld", player_.role_id());
            return false;
        }
    }
    return true;
}

bool PlayerCounter::LoadFromDB(const common::PlayerCounterData& data)
{
    if (data.info.size() > 0)
    {
        common::scalable::CounterData scalable_data;
        const std::string& bufRef = data.info;
        if (!scalable_data.ParseFromArray(bufRef.c_str(), bufRef.size()))
        {
            GLog::log("PlayerCounter::LoadFromDB error! roleid: ", player_.role_id());
            return false;
        }

        for (int i = 0; i < scalable_data.counter_list_size(); ++i)
        {
            if (!CheckCounterValidity(scalable_data.counter_list(i).tid()))
                continue;

            counter_map_[scalable_data.counter_list(i).tid()] = scalable_data.counter_list(i).value();
        }
    }
    return true;
}

bool PlayerCounter::RegisterCounter(int32_t tid)
{
    // check template id
    if (!CheckCounterValidity(tid))
        return false;

    const dataTempl::PlayerCounterTempl* ptpl = s_pDataTempl->QueryDataTempl<dataTempl::PlayerCounterTempl>(tid);
    ASSERT(ptpl != NULL);

    CounterMap::const_iterator it = counter_map_.find(tid);
    if (it != counter_map_.end())
        return false;

    counter_map_[tid] = ptpl->initial_value;

    // change
    NotifyCounterChange(CT_REGISTER, tid, counter_map_[tid]);
    return true;
}

bool PlayerCounter::UnregisterCounter(int32_t tid)
{
    CounterMap::iterator it = counter_map_.find(tid);
    if (it == counter_map_.end())
        return false;

    counter_map_.erase(it);

    // change
    NotifyCounterChange(CT_UNREGISTER, tid, 0);
    return true;
}

bool PlayerCounter::ModifyCounter(int32_t tid, int32_t delta)
{
    CounterMap::iterator it = counter_map_.find(tid);
    if (it == counter_map_.end())
        return false;

    it->second += delta;

    // change
    NotifyCounterChange(CT_MODIFY, tid, it->second);
    return true;
}

bool PlayerCounter::GetCounter(int32_t tid, int32_t& value) const
{
    CounterMap::const_iterator it = counter_map_.find(tid);
    if (it == counter_map_.end())
        return false;

    value = it->second;
    return true;
}

bool PlayerCounter::SetCounter(int32_t tid, int32_t value)
{
    CounterMap::iterator it = counter_map_.find(tid);
    if (it == counter_map_.end())
        return false;

    it->second = value;

    // change
    NotifyCounterChange(CT_MODIFY, tid, it->second);
    return true;
}

void PlayerCounter::NotifyCounterChange(int8_t type, int32_t tid, int32_t value)
{
    G2C::PlayerCounterChange packet;
    packet.type  = type;
    packet.tid   = tid;
    packet.value = value;
    player_.sender()->SendCmd(packet);
}

void PlayerCounter::SendCounterList() const
{
    G2C::PlayerCounterList packet;
    CounterMap::const_iterator it = counter_map_.begin();
    for (; it != counter_map_.end(); ++it)
    {
        G2C::PlayerCounterList::Entry ent;
        ent.tid   = it->first;
        ent.value = it->second;
        packet.counter_list.push_back(ent);
    }
    player_.sender()->SendCmd(packet);
}

} // namespace gamed
