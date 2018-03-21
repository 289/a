#include "templ_manager.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared/net/packet/codec_templ.h"

#include "global_config.h"
#include "normal_item_templ.h"
#include "equip_templ.h"
#include "card_templ.h"
#include "hidden_item_templ.h"
#include "task_item_templ.h"
#include "task_scroll_templ.h"
#include "pet_item_templ.h"
#include "skill_item_templ.h"
#include "cooldown_group.h"
#include "talent_item_templ.h"
#include "instance_group.h"
#include "instance_templ.h"
#include "mount_templ.h"
#include "monster_templ.h"
#include "world_boss_award.h"
#include "gevent_group_templ.h"
#include "battleground_templ.h"


namespace dataTempl {

DataTemplManager::DataTemplManager()
	: codec_(NULL),
	  global_config_templ_(NULL)
{
	codec_ = new TemplPacketCodec<BaseDataTempl>(DATATEMPLATE_VERSION, 
			                                     BaseDataTemplManager::CreatePacket, 
												 BaseDataTemplManager::IsValidType);
	assert(codec_);
}

DataTemplManager::~DataTemplManager()
{
	IdToTemplMap::const_iterator it = id_query_map_.begin();
	for (; it != id_query_map_.end(); ++it)
	{
		delete it->second;
	}
	id_query_map_.clear();

	DELETE_SET_NULL(codec_);

	global_config_templ_ = NULL;
}

bool DataTemplManager::ReadFromFile(const char* file)
{
	FILE* pfile       = NULL;
	int32_t file_size = 0;
	int result        = 0;

	if ((pfile = fopen(file, "rb")) == NULL)
		return false;

	// get file size
	fseek(pfile, 0L, SEEK_END);
	file_size = ftell(pfile);
	rewind(pfile);

	// malloc buffer
	shared::net::Buffer buffer;
	buffer.EnsureWritableBytes(file_size);

	// copy whole file
	result = fread(buffer.BeginWrite(), 1, file_size, pfile);
	if (result != file_size)
	{
		fclose(pfile);
		return false;
	}
	buffer.HasWritten(result);
	
	// parse
	if (!ReadFromBuffer(buffer))
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

bool DataTemplManager::ReadFromBuffer(const shared::net::Buffer& buffer)
{
	// parse
	std::vector<BaseDataTempl*> tempvec;
	if (TPC_SUCCESS != codec_->ParseBuffer(&buffer, tempvec))
	{
		return false;
	}

	// push to map
	for (size_t i = 0; i < tempvec.size(); ++i)
	{
		if (!tempvec[i]->CheckDataValidity())
        {
            LOG_ERROR << "templ_id: " << tempvec[i]->templ_id << " CheckDataValidity() failure!";
            assert(false);
            return false;
        }
		if (!id_query_map_.insert(std::pair<TemplID, const BaseDataTempl*>(tempvec[i]->templ_id, tempvec[i])).second)
        {
            LOG_ERROR << "templ_id repetition! id: " << tempvec[i]->templ_id;
            assert(false);
            return false;
        }
		InsertToSpecificMap(tempvec[i]->templ_id, tempvec[i]);
	}
	tempvec.clear();

	// Has only one data of global_config template
	assert(specific_type_query_map_[TEMPL_TYPE_GLOBAL_CONFIG].size() == 1);
	global_config_templ_ = dynamic_cast<const GlobalConfigTempl*>(specific_type_query_map_[TEMPL_TYPE_GLOBAL_CONFIG][0]);
	assert(global_config_templ_ != NULL);
	return true;
}
	
bool DataTemplManager::WriteToFile(const char* file, std::vector<BaseDataTempl*>& vec_templ)
{
	for (size_t i = 0; i < vec_templ.size(); ++i)
	{
		if (!vec_templ[i]->CheckDataValidity()) 
		{
			return false;
		}
	}

	shared::net::Buffer buffer;
	if (TPC_SUCCESS != codec_->AssemblingEmptyBuffer(&buffer, vec_templ))
	{
		return false;
	}
	
	FILE* pfile = NULL;
	if ((pfile = fopen(file, "wb+")) == NULL)
		return false;

	size_t result = fwrite(buffer.peek(), 1, buffer.ReadableBytes(), pfile);
	if (result != buffer.ReadableBytes())
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

void DataTemplManager::InsertToSpecificMap(TemplID templ_id, const BaseDataTempl* ptempl)
{
	specific_type_query_map_[ptempl->GetType()].push_back(ptempl);
}


template <class T1, class T2>
class Collector
{
	std::vector<const T2*>& list;

public:
	Collector(std::vector<const T2*> &l) :
		list(l)
	{
	}

	void operator() (const T1* rhs)
	{
		list.push_back(dynamic_cast<const T2*>(rhs));
	}
};

void DataTemplManager::CollectAllItemTempl(std::vector<const ItemDataTempl*>& list) const
{

#define COLLECT_SPEC_ITEM(itemname,ItemName) \
		{ \
			std::vector<const ItemName##Templ*> itemname##_vec; \
			QueryDataTemplByType<ItemName##Templ>(itemname##_vec); \
			std::for_each(itemname##_vec.begin(), itemname##_vec.end(), Collector<ItemName##Templ, ItemDataTempl>(list)); \
		}

	COLLECT_SPEC_ITEM(normal_item, NormalItem);
	COLLECT_SPEC_ITEM(equip, Equip);
	COLLECT_SPEC_ITEM(card, Card);
	COLLECT_SPEC_ITEM(hidden_item, HiddenItem);
	COLLECT_SPEC_ITEM(task_item, TaskItem);
	COLLECT_SPEC_ITEM(task_scroll, TaskScroll);
	COLLECT_SPEC_ITEM(pet_item, PetItem);
	COLLECT_SPEC_ITEM(skill_item, SkillItem);
	COLLECT_SPEC_ITEM(talent_item, TalentItem);
	COLLECT_SPEC_ITEM(mount, Mount);

#undef COLLECT_SPEC_ITEM

}

const BaseDataTempl* DataTemplManager::QueryBaseDataTempl(TemplID tpl_id) const
{
	IdToTemplMap::const_iterator it = id_query_map_.find(tpl_id);
	if(it == id_query_map_.end())
		return NULL;
	return it->second;
}

const GlobalConfigTempl* DataTemplManager::QueryGlobalConfigTempl() const
{
	return global_config_templ_;
}

bool DataTemplManager::CheckAllTemplate() const
{
	if (!CheckInstanceGroup())
		return false;

    if (!CheckInstanceTempl())
        return false;

    if (!CheckItemCoolDownGroup())
        return false;

    if (!CheckWorldBossAward())
        return false;

    if (!CheckGeventGroup())
        return false;

    if (!CheckBattleGround())
        return false;

	return true;
}

bool DataTemplManager::CheckInstanceGroup() const
{
	std::vector<const InstanceGroup*> ins_group_vec;
	QueryDataTemplByType(ins_group_vec);

	for (size_t i = 0; i < ins_group_vec.size(); ++i)
	{
		int ins_type = -1;
		const InstanceGroup* ins_group = ins_group_vec[i];
		const InstanceGroup::InsInfoArray& ins_array = ins_group->ins_array;
		for (size_t j = 0; j < ins_array.size(); ++j)
		{
			const InstanceTempl* ins_templ = QueryDataTempl<InstanceTempl>(ins_array[j].ins_templ_id);
			if (ins_templ != NULL)
			{
				ins_type = (ins_type == -1) ? ins_templ->ins_type : ins_type;
				if (ins_type != ins_templ->ins_type)
				{
					LOG_ERROR << "InstanceGroup: " << ins_group->templ_id << " error! ins_type inconformity";
					return false;
				}
			}
			else
			{
				LOG_ERROR << "InstanceGroup: " << ins_group->templ_id << " error! ins_templ not found - id:" 
					<< ins_array[j].ins_templ_id;
				return false;
			}
		}
	}

	return true;
}

bool DataTemplManager::CheckInstanceTempl() const
{
    std::vector<const InstanceTempl*> ins_templ_vec;
    QueryDataTemplByType(ins_templ_vec);

    for (size_t i = 0; i < ins_templ_vec.size(); ++i)
    {
        const InstanceTempl* ins_tpl = ins_templ_vec[i];
        if (ins_tpl->gevent_group_id > 0)
        {
            const GeventGroupTempl* ptempl = QueryDataTempl<GeventGroupTempl>(ins_tpl->gevent_group_id);
            if (ptempl == NULL)
            {
                LOG_ERROR << "InstanceTempl: " << ins_tpl->templ_id << "error! gevent_group not found - id:" 
                    << ins_tpl->gevent_group_id;
                return false;
            }
        }
    }

    return true;
}

bool DataTemplManager::CheckWorldBossAward() const
{
    std::vector<const MonsterTempl*> monster_vec;
	QueryDataTemplByType(monster_vec);
    for (size_t i = 0; i < monster_vec.size(); ++i)
    {
        int32_t award_tid = monster_vec[i]->world_boss_award_tid;
        if (award_tid > 0)
        {
            const WorldBossAwardTempl* ptpl = QueryDataTempl<WorldBossAwardTempl>(award_tid);
            if (ptpl == NULL)
                return false;
        }
    }

    std::vector<const WorldBossAwardTempl*> wb_award_vec;
    QueryDataTemplByType(wb_award_vec);
    for (size_t i = 0; i < wb_award_vec.size(); ++i)
    {
        const WorldBossAwardTempl* wb_tpl = wb_award_vec[i];
        for (size_t j = 0; j < wb_tpl->ranking.size(); ++j)
        {
            const WorldBossAwardTempl::Entry& ent = wb_tpl->ranking[j];
            for (size_t k = 0; k < ent.item_list.size(); ++k)
            {
                int32_t item_tid = ent.item_list[k].tid;
                int32_t count    = ent.item_list[k].count;
                const ItemDataTempl* ptpl = QueryDataTempl<ItemDataTempl>(item_tid);
                if (ptpl == NULL)
                    return false;
                if (count > ptpl->pile_limit)
                    return false;
            }
        }
    }

    return true;
}

bool DataTemplManager::CheckItemCoolDownGroup() const
{
    int ret = 0;
    if ((ret = CheckItemCDGroup<SkillItemTempl>()) != 0)
    {
        LOG_ERROR << "CheckItemCoolDownGroup() error! tid: " << ret << " not found";
        return false;
    }

    if ((ret = CheckItemCDGroup<TaskScrollTempl>()) != 0)
    {
        LOG_ERROR << "CheckItemCoolDownGroup() error! tid: " << ret << " not found";
        return false;
    }

    return true;
}

int DataTemplManager::CheckCoolDownGroup(TemplID cd_group_id) const
{
    const CoolDownGroupTempl* ptpl = QueryDataTempl<CoolDownGroupTempl>(cd_group_id);
    if (ptpl == NULL)
        return -1;

    return 0;
}

bool DataTemplManager::CheckGeventGroup() const
{
    std::vector<const GeventGroupTempl*> gevent_vec;
    QueryDataTemplByType(gevent_vec);
    for (size_t i = 0; i < gevent_vec.size(); ++i)
    {
        const GeventGroupTempl* ptempl = gevent_vec[i];
        if (ptempl->world_boss_tid > 0)
        {
            const MonsterTempl* pmonster = QueryDataTempl<MonsterTempl>(ptempl->world_boss_tid);
            if (pmonster == NULL)
            {
                LOG_ERROR << "WorldBoss not found! tid=" << ptempl->world_boss_tid;
                return false;
            }
        }
    }

    return true;
}

bool DataTemplManager::CheckBattleGround() const
{
    std::vector<const BattleGroundTempl*> battle_vec;
    QueryDataTemplByType(battle_vec);
    for (size_t i = 0; i < battle_vec.size(); ++i)
    {
        const BattleGroundTempl* ptempl = battle_vec[i];
        if (ptempl->worldboss_data.wb_tid > 0)
        {
            const MonsterTempl* pmonster = QueryDataTempl<MonsterTempl>(ptempl->worldboss_data.wb_tid);
            if (pmonster == NULL)
            {
                LOG_ERROR << "BattleGround worldboss not found! tid=" << ptempl->worldboss_data.wb_tid;
                return false;
            }
        }
        if (ptempl->gevent_group_id > 0)
        {
            const GeventGroupTempl* pgevent = QueryDataTempl<GeventGroupTempl>(ptempl->gevent_group_id);
            if (pgevent == NULL)
            {
                LOG_ERROR << "BattleGround gevent_group not found! tid=" << ptempl->gevent_group_id;
                return false;
            }
        }
    }

    return true;
}

} // namespace gamed
