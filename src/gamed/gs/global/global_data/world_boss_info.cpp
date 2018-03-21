#include "world_boss_info.h"

#include "common/obj_data/global_data_def.h"
#include "common/protocol/gen/global/global_data_change.pb.h"

#include "gs/global/date_time.h"
#include "gs/netmsg/send_to_master.h"


namespace gamed {
namespace globalData {

using namespace common;
using namespace common::protocol;

namespace {

    inline bool at_the_same_day(time_t now, time_t old_time)
    {
        struct tm now_tt = GetLocalTimeTM(now);
        struct tm old_tt = GetLocalTimeTM(old_time);
        if (old_tt.tm_year == now_tt.tm_year &&
            old_tt.tm_mon  == now_tt.tm_mon &&
            old_tt.tm_mday == now_tt.tm_mday)
        {
            return true;
        }

        return false;
    }

} // Anonymous


WorldBossInfo::WorldBossInfo(int32_t masterid)
	: BaseRecord(Type()),
      master_id_(masterid)
{
}

WorldBossInfo::~WorldBossInfo()
{
	master_id_ = -1;
}

int32_t WorldBossInfo::Type()
{
	return common::GDT_WB_INFO_DATA;
}

bool WorldBossInfo::SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len)
{
    if (is_remove)
	{
		Delete(key, NULL, false); // master发过来的数据，不要再同步给master
	}
	else
    {
        common::WorldBossInfoData record;
        record.AppendBuffer((const char*)content, len);
        try
        {
            record.Unmarshal();
        }
        catch (...)
        {
            return false;
        }

        RecordValue value;
        value.timestamp          = record.timestamp;
        value.daily.record_time  = record.daily.record_time;
        value.daily.record_count = record.daily.record_count;
		Modify(key, value, false); // master发过来的数据，不要再同步给master
    }

    return true;
}

void WorldBossInfo::ClearAll()
{
    record_map_.clear();
}

void WorldBossInfo::SyncToMaster(int64_t monster_tid, bool is_add, const RecordValue& value) const
{
    common::WorldBossInfoData record;
    record.monster_tid        = monster_tid;
    record.timestamp          = value.timestamp;
    record.daily.record_time  = value.daily.record_time;
    record.daily.record_count = value.daily.record_count;
    
    try
    {
        record.Marshal();
    }
    catch (...)
    {
        ASSERT(false);
        return;
    }

    // add or modify
    global::GlobalDataChange::OpType optype;
    if (is_add)
    {
        optype = global::GlobalDataChange::OP_ADD;
    }
    else
    {
        optype = global::GlobalDataChange::OP_MODIFY;
    }

    global::GlobalDataChange proto;
    proto.set_gdtype(GetType());
    proto.set_key(monster_tid);
    proto.set_optype(optype);
    proto.set_content(record.GetContent(), record.GetSize());
    NetToMaster::SendProtocol(master_id_, proto);
}

bool WorldBossInfo::Query(int64_t monster_tid, QueryValue& value) const
{
    RecordMap::const_iterator it = record_map_.find(monster_tid);
    if (it == record_map_.end())
        return false;

    value.out_daily = it->second.daily;
    if (!at_the_same_day(it->second.daily.record_time, value.cur_time))
    {
        value.out_daily.record_count = 0;
    }
    return true;
}

void WorldBossInfo::Modify(int64_t monster_tid, const TimeValue& value, bool need_sync)
{
    RecordValue record_value;
    RecordMap::iterator it = record_map_.find(monster_tid);
    if (it == record_map_.end())
    {
        record_value.timestamp          = value.timestamp;
        record_value.daily.record_time  = value.cur_time;
        record_value.daily.record_count = 1;
    }
    else
    {
        const RecordValue& ent = it->second;
        if (value.timestamp < ent.timestamp)
            return;

        record_value = ent;
        if (at_the_same_day(value.cur_time, record_value.daily.record_time))
        {
            record_value.daily.record_count++;
        }
        else // 跨天
        {
            record_value.daily.record_count = 1;
        }
    }

    Modify(monster_tid, record_value, need_sync);
}

void WorldBossInfo::Modify(int64_t monster_tid, const RecordValue& value, bool need_sync)
{
    bool is_add = false;
    RecordMap::iterator it = record_map_.find(monster_tid);
    if (it == record_map_.end())
    {
        is_add = true;
        record_map_[monster_tid] = value;
    }
    else
    {
        it->second = value;
    }

    if (need_sync)
    {
        SyncToMaster(monster_tid, is_add, value);
    }
}

void WorldBossInfo::Delete(int64_t monster_tid, const TimeValue* value, bool need_sync)
{
    RecordMap::iterator it = record_map_.find(monster_tid);
    if (it == record_map_.end())
        return;

    if (value != NULL && value->timestamp < it->second.timestamp)
        return;

    // 同一天不能删除
    if (value != NULL && at_the_same_day(it->second.daily.record_time, value->cur_time))
        return;

    // 同步给master
    if (need_sync)
    {
        global::GlobalDataChange proto;
        proto.set_gdtype(GetType());
        proto.set_key(monster_tid);
        proto.set_optype(global::GlobalDataChange::OP_DELETE);
        NetToMaster::SendProtocol(master_id_, proto);
    }

    record_map_.erase(monster_tid);
}

} // namespace globalData
} // namespace gamed
