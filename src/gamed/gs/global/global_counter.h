#ifndef GAMED_GS_GLOBAL_GLOBAL_COUNTER_H_
#define GAMED_GS_GLOBAL_GLOBAL_COUNTER_H_

#include <map>
#include <set>
#include <vector>

#include "shared/base/singleton.h"
#include "shared/base/rwlock.h"
#include "shared/base/mutex.h"


namespace gamed {

/**
 * @brief 全局计数器
 */
class GlobalCounter
{
public:
    GlobalCounter();
	~GlobalCounter();

	struct Entry
	{
		int32_t index;
		int32_t value;
	};

	// ---- thread safe ----
	void SetCounterValue(int32_t master_id, const std::vector<Entry>& ent_vec);

	// ---- thread safe ----
	void ModifyGCounter(int32_t master_id, int32_t index, int32_t delta, bool sync_master);
	void SetGCounter(int32_t master_id, int32_t index, int32_t value, bool sync_master);
	bool GetGCounter(int32_t master_id, int32_t index, int32_t& value);

    // ---- thread safe ----
    void ClearByMasterId(int32_t master_id);
	

private:
	shared::RWLock    counter_rwlock_;
	// index, value
	typedef std::map<int32_t, int32_t> CounterValueMap;
	// master_id, counter map
	typedef std::map<int32_t, CounterValueMap> MasterCounterMap;
	MasterCounterMap  counter_map_;
};


/**
 * @brief 全局计数器管理类
 */
class GlobalCounterMan : public shared::Singleton<GlobalCounterMan>
{
	friend class shared::Singleton<GlobalCounterMan>;
public:
	static inline GlobalCounterMan* GetInstance() {
		return &(get_mutable_instance());
	}

    // ---- thread safe ----
	void SetCounterValue(int32_t master_id, const std::vector<GlobalCounter::Entry>& ent_vec);

	// ---- thread safe ----
	void ModifyGCounter(int32_t master_id, int32_t index, int32_t delta, bool sync_master = true);
	void SetGCounter(int32_t master_id, int32_t index, int32_t value, bool sync_master = true);
	bool GetGCounter(int32_t master_id, int32_t index, int32_t& value);

    // ---- thread safe ----
	void HeartbeatTick(); // Tick心跳，每秒20次
    void ClearByMasterId(int32_t master_id);
    void RegisterListener(int32_t master_id, int64_t roleid, int32_t index);
    void UnregisterListener(int32_t master_id, int64_t roleid, int32_t index);
    void UnregisterListenerByID(int32_t master_id, int64_t roleid);

protected:
    GlobalCounterMan();
    ~GlobalCounterMan();

private:
    void    GlobalCounterChange(int32_t master_id, int32_t index);
    void    HandleChangeList(const std::vector<int64_t>& change_list);
    void    NodifyListener(int32_t master_id, int32_t index, int32_t value);

private:
    GlobalCounter  counter_;
	int32_t        hb_ticker_;   // 心跳计数
    int32_t        hb_delta_;    // 心跳的偏移值（做不规律心跳）

    shared::MutexLock lock_change_;
    bool           change_flag_; // 0表示没有变化，非0表示有变化
    typedef std::vector<int64_t> ChangeListVec;
    ChangeListVec  change_list_;
    ChangeListVec  swap_change_;

    typedef std::set<int64_t> RoleIDSet;
    typedef std::map<int64_t, RoleIDSet> ListenerMap;
    ListenerMap    listener_map_;
    shared::MutexLock lock_listener_;

    typedef std::map<int64_t/*roleid*/, int32_t/*masterid*/> UnregisterMap;
    UnregisterMap  unregister_map_;
};

#define s_pGCounter gamed::GlobalCounterMan::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GLOBAL_COUNTER_H_
