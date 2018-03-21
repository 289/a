#ifndef TASK_TASK_SUCC_H_
#define TASK_TASK_SUCC_H_

#include "basic_cond.h"

namespace task
{

// 任务目标，任务需要完成的内容
enum TaskGoal
{
    // 0
	GOAL_NO,
	GOAL_LEVEL,
	GOAL_GOLD,
	GOAL_CASH,
	GOAL_SCORE,
    // 5
	GOAL_COLLECT_ITEM,
	GOAL_KILL_MONSTER,
	GOAL_WAIT_TIME,
	GOAL_ZONE,
	GOAL_TASK,
    // 10
	GOAL_INSTANCE,
	GOAL_GLOBAL_COUNTER,
	GOAL_MINIGAME,
	GOAL_UI,
    GOAL_REPUTATION,
    // 15
    GOAL_BUFF,
    GOAL_COMBAT_VALUE,
    GOAL_PET_POWER,
    GOAL_PET,
    GOAL_CARD,
    // 20
    GOAL_ENHANCE,
    GOAL_EQUIP,
	GOAL_MAP_COUNTER,
    GOAL_COMBAT_FAIL,
    GOAL_PLAYER_COUNTER,
    // 25
    GOAL_STAR,
    GOAL_TALENT,
    GOAL_TIME_SEGMENT,
	GOAL_MAX = GOAL_TIME_SEGMENT,
};

class TaskSucc
{
public:
	TaskSucc()
		: goal(GOAL_NO), dlg_type(DLG_NONE)
	{
	}

	inline bool CheckDataValidity() const;

	int8_t              goal;
	int32_t             dlg_type;
	LevelCond           level;
	GoldCond            gold;
	CashCond            cash;
	ScoreCond           score;
	ItemCond            item;
	TimeCond            time;
	ZoneCond            zone;
	MonsterCond         monster;
	InstanceCond        instance;
	CounterCond         global_counter;
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
    StarCond            star;
    TalentCond          talent;
    TimeSegmentCond     time_segment;

    void Pack(shared::net::ByteBuffer& buf)
    {
		buf << goal         << dlg_type         << level        << gold;
        buf << cash         << score            << item         << time;
        buf << zone         << monster          << instance     << global_counter;
        buf << reputation   << buff             << combat_value << pet_power;
        buf << pet          << card             << enhance      << equip;
        buf << map_counter  << player_counter   << star         << talent;
        buf << time_segment;
    }

    void UnPack(shared::net::ByteBuffer& buf)
    {
		buf >> goal         >> dlg_type         >> level        >> gold;
        buf >> cash         >> score            >> item         >> time;
        buf >> zone         >> monster          >> instance     >> global_counter;
        buf >> reputation   >> buff             >> combat_value >> pet_power;
        buf >> pet          >> card             >> enhance      >> equip;
        buf >> map_counter  >> player_counter   >> star         >> talent;
        buf >> time_segment;
    }
};

inline bool TaskSucc::CheckDataValidity() const
{
	CHECK_INRANGE(goal, GOAL_NO, GOAL_MAX)
	CHECK_VALIDITY(level)
	CHECK_VALIDITY(gold)
	CHECK_VALIDITY(cash)
	CHECK_VALIDITY(score)
	CHECK_VALIDITY(item)
	CHECK_VALIDITY(time)
	CHECK_VALIDITY(zone)
	CHECK_VALIDITY(monster)
	CHECK_VALIDITY(instance)
	CHECK_VALIDITY(global_counter)
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
	CHECK_VALIDITY(star)
	CHECK_VALIDITY(talent)
	CHECK_VALIDITY(time_segment)
    return true;
}

} // namespace task

#endif // TASK_TASK_SUCC_H_
