#include "base_datatempl.h"

#include "player_class_templ.h"
#include "monster_templ.h"
#include "service_npc_templ.h"
#include "global_config.h"
#include "monster_drop_templ.h"
#include "equip_templ.h"
#include "card_templ.h"
#include "refine_templ.h"
#include "normal_item_templ.h"
#include "pet_templ.h"
#include "golem_templ.h"
#include "global_drop_templ.h"
#include "instance_templ.h"
#include "skill_tree_templ.h"
#include "hidden_item_templ.h"
#include "task_item_templ.h"
#include "mine_templ.h"
#include "mall_templ.h"
#include "mall_class_templ.h"
#include "shop_templ.h"
#include "fill_blank_cfg_tbl.h"
#include "global_counter.h"
#include "instance_group.h"
#include "task_scroll_templ.h"
#include "pet_item_templ.h"
#include "talent_templ.h"
#include "talent_group_templ.h"
#include "skill_item_templ.h"
#include "title_templ.h"
#include "reputation_templ.h"
#include "talent_item_templ.h"
#include "battleground_templ.h"
#include "enhance_templ.h"
#include "enhance_group_templ.h"
#include "cooldown_group.h"
#include "star_templ.h"
#include "player_counter_templ.h"
#include "gevent_group_templ.h"
#include "gevent_templ.h"
#include "mount_templ.h"
#include "mount_equip_templ.h"
#include "world_boss_award.h"
#include "gift_bag_templ.h"
#include "boss_challenge_templ.h"
#include "info_prompt_templ.h"


namespace dataTempl {

///
/// init template
///
// 1
INIT_STATIC_DATATEMPLATE(PlayerClassTempl, TEMPL_TYPE_PLAYER_CLASS_TEMPL);
INIT_STATIC_DATATEMPLATE(MonsterTempl, TEMPL_TYPE_MONSTER_TEMPL);
INIT_STATIC_DATATEMPLATE(ServiceNpcTempl, TEMPL_TYPE_SERVICE_NPC_TEMPL);
INIT_STATIC_DATATEMPLATE(GlobalConfigTempl, TEMPL_TYPE_GLOBAL_CONFIG);

// 5
INIT_STATIC_DATATEMPLATE(EquipTempl, TEMPL_TYPE_EQUIP);
INIT_STATIC_DATATEMPLATE(CardTempl, TEMPL_TYPE_CARD);
INIT_STATIC_DATATEMPLATE(RefineTempl, TEMPL_TYPE_REFINE_CONFIG);
INIT_STATIC_DATATEMPLATE(MonsterDropTempl, TEMPL_TYPE_MONSTER_DROP_TEMPL);
INIT_STATIC_DATATEMPLATE(NormalItemTempl, TEMPL_TYPE_NORMAL_ITEM);

// 10
INIT_STATIC_DATATEMPLATE(PetTempl, TEMPL_TYPE_PET);
INIT_STATIC_DATATEMPLATE(GolemTempl, TEMPL_TYPE_GOLEM);
INIT_STATIC_DATATEMPLATE(GlobalDropTempl, TEMPL_TYPE_GLOBAL_DROP);
INIT_STATIC_DATATEMPLATE(InstanceTempl, TEMPL_TYPE_INSTANCE_TEMPL);
INIT_STATIC_DATATEMPLATE(SkillTreeTempl, TEMPL_TYPE_SKILL_TREE);

// 15
INIT_STATIC_DATATEMPLATE(HiddenItemTempl, TEMPL_TYPE_HIDDEN_ITEM);
INIT_STATIC_DATATEMPLATE(TaskItemTempl, TEMPL_TYPE_TASK_ITEM);
INIT_STATIC_DATATEMPLATE(MineTempl, TEMPL_TYPE_MINE_TEMPL);
INIT_STATIC_DATATEMPLATE(MallTempl, TEMPL_TYPE_MALL);
INIT_STATIC_DATATEMPLATE(MallClassTempl, TEMPL_TYPE_MALL_CLASS);

// 20
INIT_STATIC_DATATEMPLATE(ShopTempl, TEMPL_TYPE_SHOP);
INIT_STATIC_DATATEMPLATE(FillBlankCfgTblTempl, TEMPL_TYPE_FILL_BLANK_CFG_TBL);
INIT_STATIC_DATATEMPLATE(GlobalCounter, TEMPL_TYPE_GLOBAL_COUNTER);
INIT_STATIC_DATATEMPLATE(InstanceGroup, TEMPL_TYPE_INSTANCE_GROUP);
INIT_STATIC_DATATEMPLATE(TaskScrollTempl, TEMPL_TYPE_TASK_SCROLL);

// 25
INIT_STATIC_DATATEMPLATE(PetItemTempl, TEMPL_TYPE_PET_ITEM);
INIT_STATIC_DATATEMPLATE(TalentTempl, TEMPL_TYPE_TALENT);
INIT_STATIC_DATATEMPLATE(TalentGroupTempl, TEMPL_TYPE_TALENT_GROUP);
INIT_STATIC_DATATEMPLATE(SkillItemTempl, TEMPL_TYPE_SKILL_ITEM);
INIT_STATIC_DATATEMPLATE(TitleTempl, TEMPL_TYPE_TITLE);

// 30
INIT_STATIC_DATATEMPLATE(ReputationTempl, TEMPL_TYPE_REPUTATION);
INIT_STATIC_DATATEMPLATE(TalentItemTempl, TEMPL_TYPE_TALENT_ITEM);
INIT_STATIC_DATATEMPLATE(BattleGroundTempl, TEMPL_TYPE_BATTLEGROUND);
INIT_STATIC_DATATEMPLATE(EnhanceTempl, TEMPL_TYPE_ENHANCE);
INIT_STATIC_DATATEMPLATE(EnhanceGroupTempl, TEMPL_TYPE_ENHANCE_GROUP);

// 35
INIT_STATIC_DATATEMPLATE(CoolDownGroupTempl, TEMPL_TYPE_COOLDOWN_GROUP);
INIT_STATIC_DATATEMPLATE(StarTempl, TEMPL_TYPE_STAR);
INIT_STATIC_DATATEMPLATE(PlayerCounterTempl, TEMPL_TYPE_PLAYER_COUNTER);
INIT_STATIC_DATATEMPLATE(GeventGroupTempl, TEMPL_TYPE_GEVENT_GROUP);
INIT_STATIC_DATATEMPLATE(GeventTempl, TEMPL_TYPE_GEVENT);

// 40
INIT_STATIC_DATATEMPLATE(MountTempl, TEMPL_TYPE_MOUNT);
INIT_STATIC_DATATEMPLATE(MountEquipTempl, TEMPL_TYPE_MOUNT_EQUIP);
INIT_STATIC_DATATEMPLATE(WorldBossAwardTempl, TEMPL_TYPE_WORLDBOSS_AWARD);
INIT_STATIC_DATATEMPLATE(GiftBagTempl, TEMPL_TYPE_GIFT_BAG);
INIT_STATIC_DATATEMPLATE(BossChallengeTempl, TEMPL_TYPE_BOSS_CHALLENGE);

// 45
INIT_STATIC_DATATEMPLATE(InfoPromptTempl, TEMPL_TYPE_INFO_PROMPT);



///
/// TimeSegment
///
TimeSegment::TimeSegment()
	: is_valid(0),
	  start_date(0),
	  end_date(0),
	  start_time(0),
	  end_time(0),
	  is_day_of_month(1)
{
}

void TimeSegment::Pack(shared::net::ByteBuffer& buf)
{
    PACK_NESTED_VALUE(is_valid, start_date, end_date, start_time, end_time, is_day_of_month, days, months);
}

void TimeSegment::UnPack(shared::net::ByteBuffer& buf)
{
    UNPACK_NESTED_VALUE(is_valid, start_date, end_date, start_time, end_time, is_day_of_month, days, months);
    CorrectionDateTime(*this);
    assert(CheckTimeSegment(*this));
}

bool CheckTimeSegment(const TimeSegment& time_seg)
{
	const int sec_limit = 23*3600 + 59*60 + 59;

	//
	// start_date/end_date
	//
	if (time_seg.start_date <= 0 && time_seg.end_date > 0)
		return false;
	if (time_seg.start_date > 0 && time_seg.end_date <= 0)
		return false;
	if (time_seg.start_date > 0 && time_seg.start_date > time_seg.end_date)
		return false;

	//
	// start_time/end_time
	//
	if (time_seg.start_time < 0 && time_seg.end_time > 0)
		return false;
	if (time_seg.start_time > 0 && time_seg.end_time < 0)
		return false;
	if (time_seg.start_time < 0 || time_seg.start_time > sec_limit || 
		time_seg.end_time < 0 || time_seg.end_time > sec_limit)
		return false;
	if (time_seg.start_time > 0 && time_seg.start_time > time_seg.end_time)
		return false;

	//
	// days
	//
	if (!time_seg.is_day_of_month && time_seg.days.size() > 7)
		return false;

	for (size_t i = 0; i < time_seg.days.size(); ++i)
	{
		if (time_seg.is_day_of_month)
		{
			if (time_seg.days[i] < 1 || time_seg.days[i] > 31)
				return false;
		}
		else
		{
			if (time_seg.days[i] < 0 || time_seg.days[i] > 6)
				return false;
		}
	}

	//
	// months
	//
	for (size_t i = 0; i < time_seg.months.size(); ++i)
	{
		if (time_seg.months[i] < 0 || time_seg.months[i] > 11)
			return false;
	}

	return true;
}

void CorrectionDateTime(TimeSegment& time_seg)
{
	if (!time_seg.is_valid)
		return;

	//
	// 时间调整
	//
	if (time_seg.start_date > 0 && time_seg.end_date > 0)
	{
		time_t seconds = time(NULL);
		struct tm tm_time;
#ifdef PLATFORM_WINDOWS
		localtime_s(&tm_time, (const time_t*)&seconds);
		struct tm tm_gmt;
		gmtime_s(&tm_gmt, (const time_t*)&seconds);
		int tz_zone = tm_time.tm_hour - tm_gmt.tm_hour;
		int tz_adjust = -(tz_zone * 3600);
#else // !PLATFORM_WINDOWS
		localtime_r(&seconds, &tm_time);
		int tz_adjust = -tm_time.tm_gmtoff;
#endif // PLATFORM_WINDOWS
		time_seg.start_date += tz_adjust;
		time_seg.end_date   += tz_adjust;
	}
}


///
/// class BaseDataTempl
///
void BaseDataTempl::Marshal()
{
    MARSHAL_TEMPLVALUE(templ_id);
	OnMarshal();
}

void BaseDataTempl::Unmarshal()
{
    UNMARSHAL_TEMPLVALUE(templ_id);
	OnUnmarshal();
}

bool BaseDataTempl::CheckDataValidity()
{
	return OnCheckDataValidity();
}


///
/// class ItemDataTempl
///
void ItemDataTempl::Marshal()
{
    BaseDataTempl::Marshal();
	MARSHAL_TEMPLVALUE(pile_limit, proc_type, recycle_price, quality);
	MARSHAL_TEMPLVALUE(visible_name, visible_desp, icon_file_path);
}

void ItemDataTempl::Unmarshal()
{
    BaseDataTempl::Unmarshal();
	UNMARSHAL_TEMPLVALUE(pile_limit, proc_type, recycle_price, quality);
	UNMARSHAL_TEMPLVALUE(visible_name, visible_desp, icon_file_path);
}

bool ItemDataTempl::CheckDataValidity()
{
	if (pile_limit <= 0 || proc_type < 0 || recycle_price < 0 || quality <= 0)
		return false;
	return OnCheckDataValidity();
}


///
/// class BaseDataTemplManager
///
BaseDataTempl* BaseDataTemplManager::CreatePacket(BaseDataTempl::Type id)
{
	return BaseDataTemplManager::GetInstance()->OnCreatePacket(id);
}

bool BaseDataTemplManager::InsertPacket(uint16_t type, BaseDataTempl* packet)
{
	return BaseDataTemplManager::GetInstance()->OnInsertPacket(type, packet);
}

bool BaseDataTemplManager::IsValidType(int32_t type)
{
	if (type > TEMPL_TYPE_INVALID && type < TEMPL_TYPE_MAX)
	{
		return true;
	}

	return false;
}

BaseDataTempl* BaseDataTemplManager::OnCreatePacket(BaseDataTempl::Type id)
{
	BaseDataTemplMap::iterator it = packet_map_.find(id);
	if (packet_map_.end() == it) return NULL;

	return dynamic_cast<BaseDataTempl*>(it->second->Clone());
}
	
bool BaseDataTemplManager::OnInsertPacket(uint16_t type, BaseDataTempl* packet)
{
	if (!packet_map_.insert(std::make_pair(type, packet)).second)
	{
		assert(false);
		return false;
	}

	return true;
}

} // namespace dataTempl
