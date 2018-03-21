#ifndef GAMED_GS_GLOBAL_DATA_WORLD_BOSS_RECORD_H_
#define GAMED_GS_GLOBAL_DATA_WORLD_BOSS_RECORD_H_

#include <set>
#include <map>
#include <string>
#include <vector>

#include "base_record.h"


namespace gamed {
namespace globalData {

/**
 * @brief WorldBossRecord
 *  （1）该类是一个线程不安全的类，TODO:在global_data.cpp的CreateEntry()函数里添加
 *  （2）世界BOSS伤害排行榜的全局数据
 */
class WorldBossRecord : public BaseRecord
{
public:
    struct PInfo
    {
        PInfo()
            : damage(0),
              role_id(0),
              level(0),
              cls(0),
              combat_value(0),
              gender(0)
        { }

        int64_t damage;
        int64_t role_id;
        std::string first_name;
        std::string mid_name;
        std::string last_name;
        int32_t level;
        int32_t cls;
        int32_t combat_value;
        int32_t gender;
    };

    struct RecordValue
    {
        RecordValue() : timestamp(0) { }

        time_t timestamp;
        std::vector<PInfo> info_vec;
    };

    struct QueryValue
    {
        QueryValue() { clear(); }

        void clear()
        {
            role_id       = 0;
            out_ranking   = 0;
            out_damage    = 0;
            out_timestamp = 0;
            query_count   = 0;
            info_vec.clear();
        }

        int64_t role_id;      // 该玩家的id
        size_t out_ranking;   // 该值用于返回该玩家的具体排名
        size_t out_damage;    // 该值用于返回该玩家的具体伤害
        time_t out_timestamp; // 世界boss的时间戳（怪物出生时间）
        size_t query_count;   // 查询排名前多少位的玩家
        std::vector<PInfo> info_vec; // 玩家信息
    };

    struct DeleteValue
    {
        time_t timestamp; // 世界boss的时间戳（怪物出生时间）
    };


public:
    WorldBossRecord(int32_t masterid);
	virtual ~WorldBossRecord();
	
	static int32_t Type();

    // **** thread unsafe ****
	virtual bool SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len);
	virtual void ClearAll();

	// **** thread unsafe ****
	bool Query(int64_t monster_tid, QueryValue& value) const;
	void Modify(int64_t monster_tid, const RecordValue& value, bool need_sync = true);
	void Delete(int64_t monster_tid, const DeleteValue* value, bool need_sync = true);


private:
    void    SyncToMaster(int64_t monster_tid, bool is_add, const RecordValue& value) const;
    int64_t FindPlayerDamage(int64_t monster_tid, int64_t role_id) const; // 返回小于等于0表示错误
    bool    CheckTimestamp(int64_t monster_tid, time_t timestamp);

    typedef std::pair<int64_t/*damage*/, PInfo> Entry;
    struct entryCompare 
    {
        // greater 
        bool operator()(const Entry& lhs, const Entry& rhs) const
        {
            return lhs.first > rhs.first;
        }
    };

    
private:
	int32_t master_id_;

    typedef std::multiset<Entry, entryCompare> RecordSet;
    typedef std::pair<RecordSet::iterator, RecordSet::iterator> RecordSetPair;
    typedef std::map<int64_t/*monster_tid*/, RecordSet> WBRankingMap;
    WBRankingMap    wb_ranking_map_;

    typedef std::map<int64_t/*roleid*/, int64_t/*damage*/> PlayerDamageMap;
    typedef std::map<int64_t/*monster_tid*/, PlayerDamageMap> WBPDamageMap;
    WBPDamageMap    wb_pdamage_map_;

    typedef std::map<int64_t/*monster_tid*/, time_t/*timestamp*/> WBTimestampMap;
    WBTimestampMap  wb_timestamp_map_;
};

} // namespace globalData
} // namespace gamed

#endif // GAMED_GS_GLOBAL_DATA_WORLD_BOSS_RECORD_H_
