#include "timer.h"
#include "cool_down.h"


namespace gamed {

bool CoolDownMan::TestCoolDown(CoolDownID id)
{
	CoolDownMap::iterator it = cd_map_.find(id);
	if (it != cd_map_.end())
	{
		if(it->second.expire_time > g_timer->GetSysTimeMsecs())
			return false;
		cd_map_.erase(it);
	}
	return true;
}

bool CoolDownMan::SetCoolDown(CoolDownID id, int interval)
{
    if (interval <= 0)
    {
        return false;
    }

	int64_t expire_time = g_timer->GetSysTimeMsecs() + interval;
	CoolDownMap::iterator it = cd_map_.find(id);
	if (it == cd_map_.end())
	{
		cd_map_[id] = cd_entry(interval, expire_time);
	}
	else
	{
		it->second.interval = interval;
		it->second.expire_time = expire_time;
	}
	return true;
}

bool CoolDownMan::ClrCoolDown(CoolDownID id)
{
	CoolDownMap::iterator it = cd_map_.find(id);
	if (it == cd_map_.end())
	{
		return false;
	}

	cd_map_.erase(it);
	return true;
}

void CoolDownMan::LoadFromData(const CoolDownMap& cd_list)
{
    cd_map_ = cd_list;
	ClearExpiredCoolDown();
}

void CoolDownMan::SaveToData(CoolDownMap& cd_list)
{
	cd_list.clear();
	ClearExpiredCoolDown();
    cd_list = cd_map_;
}

void CoolDownMan::ClearExpiredCoolDown()
{
	int64_t now = g_timer->GetSysTimeMsecs();
	CoolDownMap::iterator it = cd_map_.begin();
	while (it != cd_map_.end())
	{
		if (it->second.expire_time > now)
		{
			++it;
		}
		else
		{
			cd_map_.erase(it++);
		}
	}
}

void CoolDownMan::Clear()
{
	cd_map_.clear();
}

}; // namespace gamed
