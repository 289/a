#include "global_counter.h"

#include "shared/logsys/logging.h"
#include "gs/netmsg/send_to_master.h"

#include "gmatrix.h"


namespace gamed {

using namespace shared;

namespace {
	static const int kMaxCounterIndex = 8192;

    int64_t MakeListnerKey(int32_t master_id, int32_t index)
    {
        return makeInt64(master_id, index);
    }

    int32_t GET_MASTER_ID(int64_t key)
    {
        return high32bits(key);
    }

    int32_t GET_INDEX(int64_t key)
    {
        return low32bits(key);
    }

} // Anonymous


///
/// GlobalCounter
///
GlobalCounter::GlobalCounter()
{
}

GlobalCounter::~GlobalCounter()
{
}

void GlobalCounter::SetCounterValue(int32_t master_id, const std::vector<Entry>& ent_vec)
{
	RWLockWriteGuard wrlock(counter_rwlock_);
	for (size_t i = 0; i < ent_vec.size(); ++i)
	{
		int32_t index = ent_vec[i].index;
		int32_t value = ent_vec[i].value;
		counter_map_[master_id][index] = value;
	}
}

void GlobalCounter::ModifyGCounter(int32_t master_id, int32_t index, int32_t delta, bool sync_master)
{
	if (delta == 0 || index <= 0 || index >= kMaxCounterIndex || master_id <= 0)
	{
		LOG_WARN << "全局计数器修改,传入参数有误：delta=" << delta << " index=" << index
			<< " master_id=" << master_id;
		return;
	}

	RWLockWriteGuard wrlock(counter_rwlock_);
	MasterCounterMap::iterator it = counter_map_.find(master_id);
	if (it == counter_map_.end())
	{
		LOG_WARN << "没有找到对应的服务器master_id:" << master_id;
		return;
	}
	it->second[index] += delta;

    if (sync_master)
    {
	    // send to master
	    NetToMaster::ModifyGlobalCounter(master_id, index, delta);
    }
}

void GlobalCounter::SetGCounter(int32_t master_id, int32_t index, int32_t value, bool sync_master)
{
    if (index <= 0 || index >= kMaxCounterIndex || master_id <= 0)
	{
		LOG_WARN << "全局计数器赋值,传入参数有误：value=" << value << " index=" << index
			<< " master_id=" << master_id;
		return;
	}

    RWLockWriteGuard wrlock(counter_rwlock_);
	MasterCounterMap::iterator it = counter_map_.find(master_id);
	if (it == counter_map_.end())
	{
		LOG_WARN << "没有找到对应的服务器master_id:" << master_id;
		return;
	}
    if (it->second[index] == value)
        return;
	it->second[index] = value;

    if (sync_master)
    {
	    // send to master
	    NetToMaster::SetGlobalCounter(master_id, index, value);
    }
}

bool GlobalCounter::GetGCounter(int32_t master_id, int32_t index, int32_t& value)
{
	RWLockReadGuard rdlock(counter_rwlock_);
	MasterCounterMap::const_iterator it = counter_map_.find(master_id);
	if (it == counter_map_.end())
	{
		LOG_WARN << "没有找到对应的服务器master_id:" << master_id;
		return false;
	}

	value = 0;
	CounterValueMap::const_iterator it_value = it->second.find(index);
	if (it_value != it->second.end())
	{
		value = it_value->second;
	}
	return true;
}

void GlobalCounter::ClearByMasterId(int32_t master_id)
{
    RWLockWriteGuard wrlock(counter_rwlock_);
	MasterCounterMap::iterator it = counter_map_.find(master_id);
    if (it != counter_map_.end())
    {
        it->second.clear();
        counter_map_.erase(it);
    }
}


///
/// GlobalCounterMan
///
GlobalCounterMan::GlobalCounterMan()
    : hb_ticker_(0),
      hb_delta_(TICK_PER_SEC),
      change_flag_(false)
{
}

GlobalCounterMan::~GlobalCounterMan()
{
}

void GlobalCounterMan::HeartbeatTick()
{
    if (++hb_ticker_ >= (TICK_PER_SEC + hb_delta_))
    {
        if (change_flag_)
        {
            swap_change_.clear();
            // lock
            {
                MutexLockGuard lock(lock_change_);
                swap_change_.swap(change_list_);
                change_list_.clear();
                change_flag_ = false;
            }
            HandleChangeList(swap_change_);
        }
        hb_ticker_ = 0;
        hb_delta_  = (hb_delta_ + 1) % TICK_PER_SEC;
    }
}

void GlobalCounterMan::SetCounterValue(int32_t master_id, const std::vector<GlobalCounter::Entry>& ent_vec)
{
    counter_.SetCounterValue(master_id, ent_vec);

    // change
    for (size_t i = 0; i < ent_vec.size(); ++i)
    {
        GlobalCounterChange(master_id, ent_vec[i].index);
    }
}

void GlobalCounterMan::ModifyGCounter(int32_t master_id, int32_t index, int32_t delta, bool sync_master)
{
    counter_.ModifyGCounter(master_id, index, delta, sync_master);

    // change
    GlobalCounterChange(master_id, index);
}

void GlobalCounterMan::SetGCounter(int32_t master_id, int32_t index, int32_t value, bool sync_master)
{
    counter_.SetGCounter(master_id, index, value, sync_master);

    // change
    GlobalCounterChange(master_id, index);
}

bool GlobalCounterMan::GetGCounter(int32_t master_id, int32_t index, int32_t& value)
{
    return counter_.GetGCounter(master_id, index, value);
}

void GlobalCounterMan::ClearByMasterId(int32_t master_id)
{
    counter_.ClearByMasterId(master_id);

    // change lock
    {
        MutexLockGuard lock(lock_change_);
        change_list_.clear();
        change_flag_ = false;
    }

    // listener lock
    {
        MutexLockGuard lock(lock_listener_);
        listener_map_.clear();
        unregister_map_.clear();
    }
}

void GlobalCounterMan::GlobalCounterChange(int32_t master_id, int32_t index)
{
    MutexLockGuard lock(lock_change_);
    change_list_.push_back(MakeListnerKey(master_id, index));
    change_flag_ = true;
}

void GlobalCounterMan::HandleChangeList(const std::vector<int64_t>& change_list)
{
    for (size_t i = 0; i < change_list.size(); ++i)
    {
        int32_t master_id = GET_MASTER_ID(change_list[i]);
        int32_t index     = GET_INDEX(change_list[i]);
        int32_t value     = 0;

        if (!GetGCounter(master_id, index, value))
            continue;

        NodifyListener(master_id, index, value);
    }
}

void GlobalCounterMan::NodifyListener(int32_t master_id, int32_t index, int32_t value)
{
    int64_t key = MakeListnerKey(master_id, index);

    MutexLockGuard lock(lock_listener_);
    UnregisterMap::iterator re_it = unregister_map_.begin();
    for (; re_it != unregister_map_.end(); ++re_it)
    {
        int64_t tmp_roleid   = re_it->first;
        int32_t tmp_masterid = re_it->second;
        ListenerMap::iterator lis_it = listener_map_.begin();
        for (; lis_it != listener_map_.end(); ++lis_it)
        {
            if (tmp_masterid == GET_MASTER_ID(lis_it->first))
            {
                lis_it->second.erase(tmp_roleid);
            }
        }
    }
    unregister_map_.clear();

    ListenerMap::iterator it = listener_map_.find(key);
    if (it != listener_map_.end())
    {
        RoleIDSet::iterator it_set = it->second.begin();
        for (; it_set != it->second.end(); ++it_set)
        {
            msg_global_counter_change param;
            param.index = index;
            param.value = value;

            MSG msg;
            BuildMessage(msg, GS_MSG_GLOBAL_COUNTER_CHANGE, XID(*it_set, XID::TYPE_PLAYER), XID(-1, -1), 0, &param, sizeof(param));
            Gmatrix::SendObjectMsg(msg, true);
        }
    }
}

void GlobalCounterMan::RegisterListener(int32_t master_id, int64_t roleid, int32_t index)
{
    int64_t key = MakeListnerKey(master_id, index);

    MutexLockGuard lock(lock_listener_);
    listener_map_[key].insert(roleid);
}

void GlobalCounterMan::UnregisterListener(int32_t master_id, int64_t roleid, int32_t index)
{
    int64_t key = MakeListnerKey(master_id, index);

    MutexLockGuard lock(lock_listener_);
    ListenerMap::iterator it = listener_map_.find(key);
    if (it != listener_map_.end())
    {
        it->second.erase(roleid);
    }
}

void GlobalCounterMan::UnregisterListenerByID(int32_t master_id, int64_t roleid)
{
    MutexLockGuard lock(lock_listener_);
    unregister_map_[roleid] = master_id;
}

} // namespace gamed
