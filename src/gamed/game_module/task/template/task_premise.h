#ifndef TASK_TASK_PREMISE_H_
#define TASK_TASK_PREMISE_H_

#include "basic_cond.h"

namespace task
{

class TaskPremise
{
public:
	inline bool CheckDataValidity() const;

	LevelCond           level;
	GoldCond            gold;
	CashCond            cash;
	ScoreCond           score;
	ItemCond            item;
	GenderCond          gender;
	RoleClassCond       roleclass;
	ZoneCond            zone;
	TaskCond            premise_task;
	TaskCond            mutex_task;
	InstanceCond        instance;
	CounterCond         global_counter;
	TimeSegmentCond     segment;
    ReputationCond      reputation;
    BuffCond            buff;
    CombatValueCond     combat_value;
    PetPowerCond        pet_power;
    PetCond             pet;
    CardCond            card;
    EnhanceCond         enhance;
    EquipCond           equip;
	CounterCond         map_counter;
	CounterCond         player_counter;
    CatVisionCond       cat_vision;
    TalentCond          talent;

    void Pack(shared::net::ByteBuffer& buf)
    {
		buf << level        << gold         << cash             << score;
        buf << item         << gender       << roleclass        << zone;
        buf << premise_task << mutex_task   << instance         << global_counter;
        buf << segment      << reputation   << buff             << combat_value;
        buf << pet_power    << pet          << card             << enhance;
        buf << equip        << map_counter  << player_counter   << cat_vision;
        buf << talent;
    }

    void UnPack(shared::net::ByteBuffer& buf)
    {
		buf >> level        >> gold         >> cash             >> score;
        buf >> item         >> gender       >> roleclass        >> zone;
        buf >> premise_task >> mutex_task   >> instance         >> global_counter;
        buf >> segment      >> reputation   >> buff             >> combat_value;
        buf >> pet_power    >> pet          >>card              >> enhance;
        buf >> equip        >> map_counter  >> player_counter   >> cat_vision;
        buf >> talent;
    }
};

// 内联函数
inline bool TaskPremise::CheckDataValidity() const
{
	CHECK_VALIDITY(level)
	CHECK_VALIDITY(gold)
	CHECK_VALIDITY(cash)
	CHECK_VALIDITY(score)
	CHECK_VALIDITY(item)
	CHECK_VALIDITY(gender)
	CHECK_VALIDITY(roleclass)
	CHECK_VALIDITY(zone)
	CHECK_VALIDITY(premise_task)
	CHECK_VALIDITY(mutex_task)
	CHECK_VALIDITY(instance)
	CHECK_VALIDITY(global_counter)
	CHECK_VALIDITY(segment)
    CHECK_VALIDITY(reputation)
    CHECK_VALIDITY(buff)
    CHECK_VALIDITY(combat_value)
    CHECK_VALIDITY(pet_power)
    CHECK_VALIDITY(pet)
    CHECK_VALIDITY(card)
    CHECK_VALIDITY(enhance)
    CHECK_VALIDITY(equip)
	CHECK_VALIDITY(map_counter)
	CHECK_VALIDITY(player_counter)
	CHECK_VALIDITY(cat_vision)
	CHECK_VALIDITY(talent)
	return true;
}

} // namespace task

#endif // TASK_TASK_PREMISE_H_
