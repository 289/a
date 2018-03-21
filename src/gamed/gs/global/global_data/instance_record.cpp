#include "instance_record.h"

#include "shared/logsys/logging.h"

#include "common/obj_data/global_data_def.h"
#include "common/protocol/gen/global/global_data_change.pb.h"

#include "gs/global/game_util.h"
#include "gs/netmsg/send_to_master.h"


namespace gamed {
namespace globalData {

using namespace shared;
using namespace common::protocol;

namespace {

	void BuildDataFromCommon(msgpack_ins_player_info& info, 
			                 const common::InstanceRecordData::MemberInfo& member)
	{
		info.role_id      = member.role_id;
		info.first_name   = member.first_name;
		info.mid_name     = member.mid_name;
		info.last_name    = member.last_name;
		info.level        = member.level;
		info.cls          = member.cls;
		info.combat_value = member.combat_value;
	}

	void CopyDataToCommon(const msgpack_ins_player_info& info, 
			              common::InstanceRecordData::MemberInfo& member)
	{
		member.role_id      = info.role_id;
		member.first_name   = info.first_name;
		member.mid_name     = info.mid_name;
		member.last_name    = info.last_name;
		member.level        = info.level;
		member.cls          = info.cls;
		member.combat_value = info.combat_value;
	}

} // Anonymous

InstanceRecord::InstanceRecord(int32_t masterid)
	: BaseRecord(Type()),
      master_id_(masterid)
{
}

InstanceRecord::~InstanceRecord()
{
	master_id_ = -1;
}

int32_t InstanceRecord::Type()
{
    return common::GDT_INS_RECORD_DATA;
}

bool InstanceRecord::SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len)
{
	if (is_remove)
	{
		Delete(key, NULL, false); // master发过来的数据，不要再同步给master
	}
	else
    {
        common::InstanceRecordData record_data;
        record_data.AppendBuffer((const char*)content, len);
        try
        {
            record_data.Unmarshal();
        }
        catch (...)
        {
            return false;
        }

        if (record_data.ins_templ_id != key)
        {
            LOG_ERROR << "InstanceRecord::SetGlobalData() error!";
            return false;
        }

        RecordValue value;
        value.clear_time = record_data.clear_time;
        for (size_t i = 0; i < record_data.detail.size(); ++i)
        {
            msgpack_ins_player_info info;
			BuildDataFromCommon(info, record_data.detail[i]);
			value.player_vec.push_back(info);
		}
		Modify(key, value, false); // master发过来的数据，不要再同步给master
	}

	return true;
}

void InstanceRecord::ClearAll()
{
	ins_record_map_.clear();
}

bool InstanceRecord::Query(int64_t ins_tid, RecordValue& value) const
{
	InsRecordMap::const_iterator it = ins_record_map_.find(ins_tid);
	if (it != ins_record_map_.end())
	{
		value = it->second;
		return true;
	}
	return false;
}

void InstanceRecord::Modify(int64_t ins_tid, const RecordValue& value, bool need_sync)
{
	bool is_add = false;
	InsRecordMap::iterator it = ins_record_map_.find(ins_tid);
	if (it == ins_record_map_.end())
	{
		// not found
		is_add = true;
		ins_record_map_[ins_tid].clear_time = GS_INT32_MAX;
		ins_record_map_[ins_tid].player_vec.clear();
	}

	RecordValue& ent = ins_record_map_[ins_tid];
	if (ent.clear_time > value.clear_time)
	{
		ent.clear_time = value.clear_time;
		ent.player_vec = value.player_vec;

        // 需要同步给master
        if (need_sync)
        {
            SyncToMaster(ins_tid, is_add, value);
        }
    }
    else if (ent.clear_time < value.clear_time && !need_sync)
    {
        // master上保存的数据已经过时
        RecordValue new_value;
        new_value.clear_time = ent.clear_time;
        new_value.player_vec = ent.player_vec;
        SyncToMaster(ins_tid, is_add, new_value);
        ASSERT(!is_add); // 不可能是新增项
    }
}

void InstanceRecord::Delete(int64_t ins_tid, const char* none, bool need_sync)
{
	InsRecordMap::iterator it = ins_record_map_.find(ins_tid);
	if (it != ins_record_map_.end())
	{
		// 同步给master
		if (need_sync)
		{
			global::GlobalDataChange proto;
			proto.set_gdtype(GetType());
			proto.set_key(ins_tid);
			proto.set_optype(global::GlobalDataChange::OP_DELETE);
			NetToMaster::SendProtocol(master_id_, proto);
		}

		// remove
		ins_record_map_.erase(it);
	}
}

void InstanceRecord::SyncToMaster(int64_t ins_tid, bool is_add, const RecordValue& value)
{
    common::InstanceRecordData record;
    record.ins_templ_id = ins_tid;
    record.clear_time   = value.clear_time;
    for (size_t i = 0; i < value.player_vec.size(); ++i)
    {
        common::InstanceRecordData::MemberInfo member;
        CopyDataToCommon(value.player_vec[i], member);
        record.detail.push_back(member);
    }

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
    proto.set_key(ins_tid);
    proto.set_optype(optype);
    proto.set_content(record.GetContent(), record.GetSize());
    NetToMaster::SendProtocol(master_id_, proto);
}

} // namespace globalData
} // namespace gamed
