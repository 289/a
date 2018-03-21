#ifndef GAMED_GS_GLOBAL_GLOBAL_DATA_H_
#define GAMED_GS_GLOBAL_GLOBAL_DATA_H_

#include "shared/base/singleton.h"
#include "shared/base/rwlock.h"

#include "gs/global/global_data/instance_record.h"
#include "gs/global/global_data/world_boss_record.h"
#include "gs/global/global_data/world_boss_info.h"


namespace gamed {

class GlobalData : public shared::Singleton<GlobalData>
{
	friend class shared::Singleton<GlobalData>;
public:
	static inline GlobalData* GetInstance() {
		return &(get_mutable_instance());
	}

	// ---- thread unsafe ----
	void SetGlobalData(int32_t masterid, int32_t gdType, int64_t key, 
			           bool is_remove, const void* content, int32_t len);
	void ClearByMasterId(int32_t masterid);

	// ---- thread safe ----
	template <typename T, typename D>
	bool Query(int32_t masterid, int64_t key, D& data) const;

	// ---- thread safe ----
	template <typename T, typename V>
	void Modify(int32_t masterid, int64_t key, const V& value);

	// ---- thread safe ----
	template <typename T, typename V>
	void Delete(int32_t masterid, int64_t key, const V* value);
	

protected:
	GlobalData();
	~GlobalData();


private:
    typedef std::vector<globalData::BaseRecord*> RecordPtrVec;
	struct Entry
	{
        RecordPtrVec records;
	};
	Entry CreateEntry(int32_t masterid);
    globalData::BaseRecord* MatchRecord(int32_t type, const Entry& ent) const;


private:
	mutable shared::RWLock data_lock_;
	// masterid, Entry
	typedef std::map<int32_t, Entry> GlobalDataMap;
	GlobalDataMap  entry_data_map_;
};


///
/// template func
///
template <typename T, typename D>
bool GlobalData::Query(int32_t masterid, int64_t key, D& data) const
{
	shared::RWLockReadGuard rdlock(data_lock_);
	GlobalDataMap::const_iterator it = entry_data_map_.find(masterid);
	if (it != entry_data_map_.end())
	{
        const T* pRecord = dynamic_cast<const T*>(MatchRecord(T::Type(), it->second));
        if (pRecord) {
            return pRecord->Query(key, data);
        }
	}
	return false;
}

template <typename T, typename V>
void GlobalData::Modify(int32_t masterid, int64_t key, const V& value)
{
	shared::RWLockWriteGuard wrlock(data_lock_);
	GlobalDataMap::iterator it = entry_data_map_.find(masterid);
    if (it == entry_data_map_.end())
    {
        Entry ent = CreateEntry(masterid);
		entry_data_map_[masterid] = ent;
        it = entry_data_map_.find(masterid);
        ASSERT(it != entry_data_map_.end());
    }

    T* pRecord = dynamic_cast<T*>(MatchRecord(T::Type(), it->second));
    if (pRecord) {
        pRecord->Modify(key, value);
    }
}

template <typename T, typename V>
void GlobalData::Delete(int32_t masterid, int64_t key, const V* value)
{
	shared::RWLockWriteGuard wrlock(data_lock_);
	GlobalDataMap::iterator it = entry_data_map_.find(masterid);
	if (it != entry_data_map_.end())
	{
        T* pRecord = dynamic_cast<T*>(MatchRecord(T::Type(), it->second));
        if (pRecord) {
            pRecord->Delete(key, value);
        }
	}
}

#define s_pGlobalData gamed::GlobalData::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GLOBAL_DATA_H_
