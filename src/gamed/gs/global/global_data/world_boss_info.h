#ifndef GAMED_GS_GLOBAL_DATA_WORLD_BOSS_INFO_H_
#define GAMED_GS_GLOBAL_DATA_WORLD_BOSS_INFO_H_

#include <map>
#include <time.h>

#include "base_record.h"


namespace gamed {
namespace globalData {

/**
 * @brief WorldBossInfo
 *  （1）该类是一个线程不安全的类，TODO:在global_data.cpp的CreateEntry()函数里添加
 *  （2）世界BOSS的信息，比如当日已经开启几次等
 */
class WorldBossInfo : public BaseRecord
{
public:
    struct DailyInfo
    {
        DailyInfo() 
            : record_time(0), record_count(0)
        { }

        int32_t record_time;  // 上次记录的开始时间
        int32_t record_count; // 当天该世界BOSS的已经开启几次
    };

    struct TimeValue
    {
        TimeValue()
            : timestamp(0), cur_time(0)
        { }

        time_t timestamp; // 世界BOSS的出生时间
        time_t cur_time;  // 当前时间
    };

    struct QueryValue
    {
        time_t cur_time;  // 当前时间
        DailyInfo out_daily;
    };
    

public:
    WorldBossInfo(int32_t masterid);
	virtual ~WorldBossInfo();
	
	static int32_t Type();

    // **** thread unsafe ****
	virtual bool SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len);
	virtual void ClearAll();

	// **** thread unsafe ****
	bool Query(int64_t monster_tid, QueryValue& value) const;
    void Modify(int64_t monster_tid, const TimeValue& value, bool need_sync = true);
    void Delete(int64_t monster_tid, const TimeValue* value, bool need_sync = true);

    
private:
    struct RecordValue
    {
        RecordValue()
            : timestamp(0)
        { }

        time_t timestamp; // 世界BOSS的出生时间
        DailyInfo daily;
    };


private:
    void    SyncToMaster(int64_t monster_tid, bool is_add, const RecordValue& value) const;
    void    Modify(int64_t monster_tid, const RecordValue& value, bool need_sync);


private:
	int32_t master_id_;

    typedef std::map<int64_t/*monster_tid*/, RecordValue> RecordMap;
    RecordMap    record_map_;
}; 

} // namespace globalData
} // namespace gamed

#endif // GAMED_GS_GLOBAL_DATA_WORLD_BOSS_INFO_H_
