#ifndef TASK_BASIC_COND_H_
#define TASK_BASIC_COND_H_

#include "basic_info.h"
#include "item_info.h"
#include "zone_info.h"
#include "monster_info.h"
#include "shared/base/time_segment.h"

namespace task
{

class LevelCond
{
public:
	LevelCond()
		: min_level(0), max_level(0)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t min_level;
	int32_t max_level;

	NESTED_DEFINE(min_level, max_level);
};

class GoldCond
{
public:
	GoldCond()
		: gold(0), take_away(true)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t gold;
	bool take_away;

	NESTED_DEFINE(gold, take_away);
};

class CashCond
{
public:
	CashCond()
		: cash(0), take_away(true)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t cash;
	bool take_away;

	NESTED_DEFINE(cash, take_away);
};

class ScoreCond
{
public:
	ScoreCond()
		: score(0), take_away(true)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t score;
	bool take_away;

	NESTED_DEFINE(score, take_away);
};

class ItemCond
{
public:
	ItemCond()
		: take_away(true), all_match(true)
	{
	}

	inline bool CheckDataValidity() const;

	ItemInfoVec item_list;
	bool take_away;
    bool all_match;

	NESTED_DEFINE(item_list, take_away, all_match);
};

enum Gender
{
	GENDER_NONE = -1, // 无性别
	GENDER_MALE,
	GENDER_FEMALE,
};

class GenderCond
{
public:
	GenderCond()
		: gender(GENDER_NONE)
	{
	}

	inline bool CheckDataValidity() const;

	int8_t gender;

	NESTED_DEFINE(gender);
};

class RoleClassCond
{
public:
	inline bool CheckDataValidity() const;

	typedef std::vector<int8_t> RoleClassVec;
	RoleClassVec class_list;

	NESTED_DEFINE(class_list);
};

class ZoneCond
{
public:
	inline bool CheckDataValidity() const;

	ZoneOpVec zone_list;

	NESTED_DEFINE(zone_list);
};

class TimeCond
{
public:
	TimeCond()
		: time(0), record_offline(false)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t time;
	bool record_offline;

	NESTED_DEFINE(time, record_offline);
};

class TimeSegmentCond
{
public:
	inline bool CheckDataValidity() const;

	shared::TimeSegment segment;

	void Pack(shared::net::ByteBuffer& buf)
	{
		buf << segment.start_date_ << segment.end_date_;
		buf << static_cast<uint32_t>(segment.entries_.size());
		for (size_t i = 0; i < segment.entries_.size(); ++i)
		{
			const shared::TimeEntry& entry = segment.entries_[i];
			buf << static_cast<uint32_t>(entry.months_.size());
			std::set<int8_t>::const_iterator it = entry.months_.begin();
			for (; it != entry.months_.end(); ++it)
			{
				buf << *it;
			}
			buf << entry.day_of_month_;
			buf << static_cast<uint32_t>(entry.days_.size());
			it = entry.days_.begin();
			for (; it != entry.days_.end(); ++it)
			{
				buf << *it;
			}
			buf << entry.start_time_ << entry.end_time_;
		}
	}

	void UnPack(shared::net::ByteBuffer& buf)
	{
		int8_t value = 0;
		uint32_t entry_size = 0;
		buf >> segment.start_date_ >> segment.end_date_;		
		buf >> entry_size;
		segment.entries_.clear();
		for (uint32_t i = 0; i < entry_size; ++i)
		{
			uint32_t j = 0;
			uint32_t size = 0;
			shared::TimeEntry entry;
			buf >> size;
			entry.months_.clear();
			for (j = 0; j < size; ++j)
			{
				buf >> value;
				entry.months_.insert(value);
			}
			buf >> entry.day_of_month_;
			buf >> size;
			for (j = 0; j < size; ++j)
			{
				buf >> value;
				entry.days_.insert(value);
			}
			buf >> entry.start_time_ >> entry.end_time_;
			segment.entries_.push_back(entry);
		}
	}
};

class MonsterCond
{
public:
	inline bool CheckDataValidity() const;

	MonsterKilledVec monster_list;

	NESTED_DEFINE(monster_list);
};

class TaskCond
{
public:
	inline bool CheckDataValidity() const;

	TaskInfoVec task_list;

	NESTED_DEFINE(task_list);
};

class InstanceCond
{
public:
    InstanceCond()
        : all_match(true)
    {
    }

	inline bool CheckDataValidity() const;

    bool all_match;
	InstanceInfoVec instance_list;

	NESTED_DEFINE(all_match, instance_list);
};

class CounterCond
{
public:
	CounterCond()
		: all_match(true)
	{
	}

	inline bool CheckDataValidity() const;

	bool all_match;
	CounterVec counter_list;

	NESTED_DEFINE(all_match, counter_list);
};

class ReputationCond
{
public:
	inline bool CheckDataValidity() const;

	ReputationVec reputation_list;

	NESTED_DEFINE(reputation_list);
};

class TalentCond
{
public:
	inline bool CheckDataValidity() const;

	TalentVec talent_list;

	NESTED_DEFINE(talent_list);
};

class BuffCond
{
public:
    inline bool CheckDataValidity() const;

    BuffVec buff_list;

    NESTED_DEFINE(buff_list);
};

class CombatValueCond
{
public:
    inline bool CheckDataValidity() const;

    CombatValueVec value_list;

    NESTED_DEFINE(value_list);
};

class PetPowerCond
{
public:
    PetPowerCond() 
        : value(0)
    {
    }

    inline bool CheckDataValidity() const;

    int32_t value;

    NESTED_DEFINE(value);
};

class PetCond
{
public:
    inline bool CheckDataValidity() const;

    PetVec pet_list;

    NESTED_DEFINE(pet_list);
};

class CardCond
{
public:
    inline bool CheckDataValidity() const;

    CardVec card_list;

    NESTED_DEFINE(card_list);
};

class EnhanceCond
{
public:
    inline bool CheckDataValidity() const;

    EnhanceVec enhance_list;

    NESTED_DEFINE(enhance_list);
};

class EquipCond
{
public:
    inline bool CheckDataValidity() const;

    EquipVec equip_list;

    NESTED_DEFINE(equip_list);
};

class StarCond
{
public:
    StarCond()
        : full_activate_num(0)
    {
    }

    inline bool CheckDataValidity() const;

    int32_t full_activate_num;

    NESTED_DEFINE(full_activate_num);
};

class CatVisionCond : public LevelCond
{
};

#include "basic_cond-inl.h"

} // namespace task

#endif // TASK_BASIC_COND_H_
