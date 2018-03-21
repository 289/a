#ifndef COMMON_OBJDATAPOOL_GLOBAL_DATA_DEF_H_
#define COMMON_OBJDATAPOOL_GLOBAL_DATA_DEF_H_

#include "common/obj_data/attr_type_def.h"
#include "common/obj_data/obj_attr_packet.h"
#include "common/obj_data/obj_packet_util.h"


namespace common {

enum GLOBAL_DATA_TYPE
{
// 0
	GDT_INS_RECORD_DATA = GLOBAL_DATA_TYPE_LOWER_LIMIT,
    GDT_WB_RECORD_DATA,
    GDT_WB_INFO_DATA,

// max
	MAX_GLOBAL_DATA_TYPE = GLOBAL_DATA_TYPE_UPPER_LIMIT
};

///
/// 以下实现类需要在cpp里做INIT
///

/**
 * @brief 副本本服的世界记录
 *  （1）需要在cpp里做INIT才能生效
 */
class InstanceRecordData : public ObjectAttrPacket
{
	DECLARE_GLOBALDATA(InstanceRecordData, GDT_INS_RECORD_DATA);
public:
	static const int kMaxMemberCount = 4;

	struct MemberInfo
	{
        MemberInfo() 
            : role_id(0), level(0), cls(0), combat_value(0)
        { }

		int64_t role_id;
		std::string first_name;
		std::string mid_name;
		std::string last_name;
		int32_t level;
		int32_t cls;
		int32_t combat_value;
		NESTED_DEFINE(role_id, first_name, mid_name, last_name, level, cls, combat_value);
	};

	int32_t ins_templ_id; // 副本id，作为key，一个副本只记录一条最快通关记录
	int32_t clear_time;   // 通关时间，单位s
	shared::net::BoundArray<MemberInfo, kMaxMemberCount> detail; // 通关队员信息，最多4个
	OBJECTATTR_DEFINE(ins_templ_id, clear_time, detail);

	virtual void Release()
	{
		ins_templ_id = 0;
		clear_time   = 0;
		detail.clear();
	}
};


/**
 * @brief 世界BOSS伤害排行数据
 *  （1）需要在cpp里做INIT才能生效
 */
class WorldBossRecordData : public ObjectAttrPacket
{
    DECLARE_GLOBALDATA(WorldBossRecordData, GDT_WB_RECORD_DATA);
public:
    // 一个monster_tid对应一个WorldBossRecordData
    struct Entry
    {
        Entry() 
            : monster_tid(0), timestamp(0), role_id(0), damage(0) 
        { }

        int32_t monster_tid;
        int32_t timestamp;
        int64_t role_id;
        int64_t damage;
        std::string pinfo; // 对应scalable里的PlayerBriefInfo
        NESTED_DEFINE(monster_tid, timestamp, role_id, damage, pinfo);
    };

    std::vector<Entry> ranking_list;
	OBJECTATTR_DEFINE(ranking_list);

    virtual void Release()
    {
        ranking_list.clear();
    }
};


/**
 * @brief 世界BOSS的信息数据（比如每天已创建次数）
 */
class WorldBossInfoData : public ObjectAttrPacket
{
    DECLARE_GLOBALDATA(WorldBossInfoData, GDT_WB_INFO_DATA);
public:
    // 每日记录信息
    struct DailyInfo
    {
        DailyInfo() { clear(); }

        void clear()
        {
            record_time  = 0;
            record_count = 0;
        }

        int32_t record_time;  // 上次记录的开始时间
        int32_t record_count; // 当天该世界BOSS的已经开启几次
        NESTED_DEFINE(record_time, record_count);
    };

    int32_t monster_tid; // 世界BOSS怪物模板id
    int32_t timestamp;   // 世界BOSS出生的时间戳
    DailyInfo daily;     // 每日记录
	OBJECTATTR_DEFINE(monster_tid, timestamp, daily);

    virtual void Release()
    {
        monster_tid = 0;
        timestamp   = 0;
        daily.clear();
    }
};


/**
 * @brief 一个工具类
 */
class GlobalDataDeclare
{
public:
	static bool Marshal(ObjectAttrPacket& packet);
	template <typename T>
	static T* Unmarshal(uint16_t cmd_type_no, const char* buf, int len);
};

template <typename T>
T* GlobalDataDeclare::Unmarshal(uint16_t cmd_type_no, const char* buf, int len)
{
	ObjectAttrPacket* packet = ObjAttrPacketManager::UnmarshalPacket(cmd_type_no, buf, len);
	if (packet != NULL)
	{
		return dynamic_cast<T*>(packet);
	}
	return NULL;
}

} // namespace common

#endif // COMMON_OBJDATAPOOL_GLOBAL_DATA_DEF_H_
