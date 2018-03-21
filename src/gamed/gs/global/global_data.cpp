#include "global_data.h"

#include "shared/logsys/logging.h"
#include "shared/base/base_define.h"


namespace gamed {

using namespace shared;
using namespace globalData;

GlobalData::GlobalData()
{
}

GlobalData::~GlobalData()
{
	RWLockWriteGuard wrlock(data_lock_);
	GlobalDataMap::iterator it = entry_data_map_.begin();
	for (; it != entry_data_map_.end(); ++it)
	{
        for (size_t i = 0; i < it->second.records.size(); ++i)
        {
		    SAFE_DELETE(it->second.records[i]);
        }
	}
}

GlobalData::Entry GlobalData::CreateEntry(int32_t masterid)
{
	Entry ent;
	ent.records.push_back(static_cast<BaseRecord*>(new InstanceRecord(masterid)));
    ent.records.push_back(static_cast<BaseRecord*>(new WorldBossRecord(masterid)));
    ent.records.push_back(static_cast<BaseRecord*>(new WorldBossInfo(masterid)));
	return ent;
}

void GlobalData::ClearByMasterId(int32_t masterid)
{
	RWLockWriteGuard wrlock(data_lock_);
	GlobalDataMap::iterator it = entry_data_map_.find(masterid);
	if (it != entry_data_map_.end())
	{
		// delete
        for (size_t i = 0; i < it->second.records.size(); ++i)
        {
            it->second.records[i]->ClearAll();
		    SAFE_DELETE(it->second.records[i]);
        }

		// erase
		entry_data_map_.erase(it);
	}
}

void GlobalData::SetGlobalData(int32_t masterid, 
		                       int32_t gdType, 
							   int64_t key,
							   bool is_remove, 
							   const void* content, 
							   int32_t len)
{
	RWLockWriteGuard wrlock(data_lock_);
	GlobalDataMap::iterator it = entry_data_map_.find(masterid);
    if (it == entry_data_map_.end())
    {
        Entry ent = CreateEntry(masterid);
		entry_data_map_[masterid] = ent;
        it = entry_data_map_.find(masterid);
        ASSERT(it != entry_data_map_.end());
    }

    BaseRecord* pBase = MatchRecord(gdType, it->second);
    if (pBase)
    {
        pBase->SetGlobalData(key, is_remove, content, len);
    }
    else
    {
        LOG_ERROR << "GlobalData::SetGlobalData() error! gdType:" << gdType;
    }
}

BaseRecord* GlobalData::MatchRecord(int32_t type, const Entry& ent) const
{
    for (size_t i = 0; i < ent.records.size(); ++i)
    {
        BaseRecord* pBase = ent.records[i];
        if (type == pBase->GetType())
        {
            return pBase;
        }
    }

    return NULL;
}

} // namespace gamed
