#ifndef GAMED_GS_GLOBAL_DATA_INSTANCE_RECORD_H_
#define GAMED_GS_GLOBAL_DATA_INSTANCE_RECORD_H_

#include <stdlib.h>
#include <string>

#include "shared/net/packet/packet_util.h"
#include "gs/global/msg_pack_def.h"

#include "base_record.h"


namespace gamed {
namespace globalData {

/**
 * @brief InstanceRecord
 *  （1）该类是一个线程不安全的类，TODO:在global_data.cpp的CreateEntry()函数里添加
 */
class InstanceRecord : public BaseRecord
{
public:
	typedef std::vector<msgpack_ins_player_info> PlayerInfoVec;

	struct RecordValue
	{
		int32_t clear_time;
		PlayerInfoVec player_vec;
	};

public:
	InstanceRecord(int32_t masterid);
	virtual ~InstanceRecord();
	
	static int32_t Type();

	// **** thread unsafe ****
	virtual bool SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len);
	virtual void ClearAll();

	// **** thread unsafe ****
	bool Query(int64_t ins_tid, RecordValue& value) const;
	void Modify(int64_t ins_tid, const RecordValue& value, bool need_sync = true);
	void Delete(int64_t ins_tid, const char* none, bool need_sync = true);

private:
    void SyncToMaster(int64_t ins_tid, bool is_add, const RecordValue& value);

private:
	int32_t master_id_;

	// ins_templ_id, RecordValue
	typedef std::map<int64_t, RecordValue> InsRecordMap;
	InsRecordMap  ins_record_map_;
};

} // namespace globalData
} // namespace gamed

#endif // GAMED_GS_GLOBAL_DATA_INSTANCE_RECORD_H_
