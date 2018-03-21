#include "world_boss_record.h"

#include "shared/logsys/logging.h"
#include "common/obj_data/global_data_def.h"
#include "common/protocol/gen/global/global_data_change.pb.h"
#include "common/obj_data/gen/player/player_info.pb.h"

#include "gs/netmsg/send_to_master.h"


namespace gamed {
namespace globalData {

using namespace common;
using namespace common::protocol;

namespace {

    void BuildPBriefInfo(const WorldBossRecord::PInfo& info, scalable::PlayerBriefInfo& brief)
    {
        brief.set_roleid(info.role_id);
        brief.set_first_name(info.first_name);
        brief.set_mid_name(info.mid_name);
        brief.set_last_name(info.last_name);
        brief.set_level(info.level);
        brief.set_cls(info.cls);
        brief.set_combat_value(info.combat_value);
        brief.set_gender(info.gender);
    }

    void BuildPInfoFromBrief(int64_t damage, const scalable::PlayerBriefInfo& brief, WorldBossRecord::PInfo& info)
    {
        info.damage       = damage;
        info.role_id      = brief.roleid();
        info.first_name   = brief.first_name();
        info.mid_name     = brief.mid_name();
        info.last_name    = brief.last_name();
        info.level        = brief.level();
        info.cls          = brief.cls();
        info.combat_value = brief.combat_value();
        info.gender       = brief.gender();
    }

} // Anonymous


WorldBossRecord::WorldBossRecord(int32_t masterid)
	: BaseRecord(Type()),
      master_id_(masterid)
{
}

WorldBossRecord::~WorldBossRecord()
{
	master_id_ = -1;
}

int32_t WorldBossRecord::Type()
{
	return common::GDT_WB_RECORD_DATA;
}

int64_t WorldBossRecord::FindPlayerDamage(int64_t monster_tid, int64_t role_id) const
{
    WBPDamageMap::const_iterator it_dam = wb_pdamage_map_.find(monster_tid);
    if (it_dam == wb_pdamage_map_.end())
        return -1;

    PlayerDamageMap::const_iterator it_player = it_dam->second.find(role_id);
    if (it_player == it_dam->second.end())
        return -1;

    return it_player->second;
}

bool WorldBossRecord::SetGlobalData(int64_t key, bool is_remove, const void* content, int32_t len)
{
    if (is_remove)
	{
		Delete(key, NULL, false); // master发过来的数据，不要再同步给master
	}
	else
    {
        common::WorldBossRecordData record;
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
        for (size_t i = 0; i < record.ranking_list.size(); ++i)
        {
            const common::WorldBossRecordData::Entry& ent = record.ranking_list[i];
            if (ent.monster_tid != key)
            {
                LOG_ERROR << "WorldBossRecord::SetGlobalData() error!";
                continue;
            }

            scalable::PlayerBriefInfo brief;
            if (!brief.ParseFromArray(ent.pinfo.c_str(), ent.pinfo.size()))
            {
                LOG_ERROR << "scalable::PlayerBriefInfo ParseFromArray error!";
                continue;
            }

            PInfo tmpinfo;
            BuildPInfoFromBrief(ent.damage, brief, tmpinfo);
            if (value.timestamp == 0)
            {
                value.timestamp = ent.timestamp;
            }
            else
            {
                ASSERT(value.timestamp == ent.timestamp);
            }
            value.info_vec.push_back(tmpinfo);
        }

		Modify(key, value, false); // master发过来的数据，不要再同步给master
    }

    return true;
}

void WorldBossRecord::ClearAll()
{
    wb_ranking_map_.clear();
    wb_pdamage_map_.clear();
    wb_timestamp_map_.clear();
}

void WorldBossRecord::SyncToMaster(int64_t monster_tid, bool is_add, const RecordValue& value) const
{
    common::WorldBossRecordData record;
    for (size_t i = 0; i < value.info_vec.size(); ++i)
    {
        const PInfo& info = value.info_vec[i];
        common::WorldBossRecordData::Entry ent;
        ent.timestamp   = value.timestamp;
        ent.monster_tid = monster_tid;
        ent.role_id     = info.role_id;
        ent.damage      = info.damage;
        scalable::PlayerBriefInfo brief;
        BuildPBriefInfo(info, brief);

        ent.pinfo.resize(brief.ByteSize());
        if (!brief.SerializeToArray((void*)ent.pinfo.c_str(), ent.pinfo.size())) 
        {
            LOG_ERROR << "scalable::PlayerBriefInfo SerializeToArray error!";
            continue;
        }
        record.ranking_list.push_back(ent);
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
    proto.set_key(monster_tid);
    proto.set_optype(optype);
    proto.set_content(record.GetContent(), record.GetSize());
    NetToMaster::SendProtocol(master_id_, proto);
}

bool WorldBossRecord::Query(int64_t monster_tid, QueryValue& value) const
{
    WBRankingMap::const_iterator it_rank = wb_ranking_map_.find(monster_tid);
    if (it_rank == wb_ranking_map_.end())
    {
        value.clear();
        return false;
    }

    WBTimestampMap::const_iterator it_time = wb_timestamp_map_.find(monster_tid);
    ASSERT(it_time != wb_timestamp_map_.end());
    
    const RecordSet& ranking = it_rank->second;
    RecordSet::const_iterator it_record = ranking.begin();
    for (size_t i = 0; i < value.query_count && it_record != ranking.end(); ++i, ++it_record)
    {
        value.info_vec.push_back((*it_record).second);
    }

    value.out_timestamp = it_time->second;
    value.out_ranking   = 0;
    value.out_damage    = 0;
    int64_t damage = FindPlayerDamage(monster_tid, value.role_id);
    if (damage > 0)
    {
        Entry ent;
        ent.first = damage;
        RecordSetPair ret = ranking.equal_range(ent);
        for (RecordSet::iterator it = ret.first; it != ret.second; ++it)
        {
            if ((*it).second.role_id == value.role_id)
            {
                value.out_ranking = std::distance(ranking.begin(), it) + 1;
                value.out_damage  = (*it).second.damage;
                break;
            }
        }
    }

    return true;
}

bool WorldBossRecord::CheckTimestamp(int64_t monster_tid, time_t timestamp)
{
    WBTimestampMap::iterator it_time = wb_timestamp_map_.find(monster_tid);
    if (it_time != wb_timestamp_map_.end())
    {
        // found
        if (timestamp > it_time->second)
        {
            Delete(monster_tid, NULL, true);
        }
        else if (timestamp < it_time->second)
        {
            return false;
        }
    }

    wb_timestamp_map_[monster_tid] = timestamp;
    return true;
}

void WorldBossRecord::Modify(int64_t monster_tid, const RecordValue& value, bool need_sync)
{
    if (value.info_vec.empty())
    {
        LOG_WARN << "WorldBossRecord::Modify RecordValue empty!";
        return;
    }

    if (!CheckTimestamp(monster_tid, value.timestamp))
    {
        LOG_WARN << "WorldBossRecord::Modify CheckTimestamp() failure!";
        return;
    }
    
    bool is_add = false;
    WBRankingMap::iterator it_rank = wb_ranking_map_.find(monster_tid);
    if (it_rank == wb_ranking_map_.end())
    {
        // not found
        is_add = true;
        wb_ranking_map_[monster_tid].clear();
        it_rank = wb_ranking_map_.find(monster_tid);
        ASSERT(it_rank != wb_ranking_map_.end());
    }
    
    // 刷新排行榜数据
    PlayerDamageMap& pdamage = wb_pdamage_map_[monster_tid];
    RecordSet& ranking       = it_rank->second;
    for (size_t i = 0; i < value.info_vec.size(); ++i)
    {
        const PInfo& pinfo = value.info_vec[i];
        PlayerDamageMap::iterator it_player = pdamage.find(pinfo.role_id);
        if (it_player != pdamage.end())
        {
            Entry ent;
            ent.first = it_player->second;
            RecordSetPair ret = ranking.equal_range(ent);
            for (RecordSet::iterator it_pair = ret.first; it_pair != ret.second;)
            {
                if ((*it_pair).second.role_id == pinfo.role_id) {
                    ranking.erase(it_pair++);
                }
                else {
                    ++it_pair;
                }
            }
        }
        ranking.insert(std::make_pair(pinfo.damage, pinfo));
        pdamage[pinfo.role_id] = pinfo.damage;
    }

    if (need_sync)
    {
        SyncToMaster(monster_tid, is_add, value);
    }
}

void WorldBossRecord::Delete(int64_t monster_tid, const DeleteValue* value, bool need_sync)
{
    WBRankingMap::iterator it = wb_ranking_map_.find(monster_tid);
    if (it != wb_ranking_map_.end())
    {
        if (value != NULL)
        {
            if (value->timestamp < wb_timestamp_map_[monster_tid])
                return;
        }

        // 同步给master
		if (need_sync)
		{
			global::GlobalDataChange proto;
			proto.set_gdtype(GetType());
			proto.set_key(monster_tid);
			proto.set_optype(global::GlobalDataChange::OP_DELETE);
			NetToMaster::SendProtocol(master_id_, proto);
		}

        // remove
        wb_ranking_map_.erase(monster_tid);
        wb_pdamage_map_.erase(monster_tid);
        wb_timestamp_map_.erase(monster_tid);
    }
    else
    {
        WBPDamageMap::const_iterator it_dam = wb_pdamage_map_.find(monster_tid);
        ASSERT(it_dam == wb_pdamage_map_.end());
        WBTimestampMap::const_iterator it_time = wb_timestamp_map_.find(monster_tid);
        ASSERT(it_time == wb_timestamp_map_.end());
    }
}

} // namespace globalData
} // namespace gamed
