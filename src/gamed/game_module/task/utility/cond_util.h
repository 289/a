#ifndef TASK_COND_UTIL_H_
#define TASK_COND_UTIL_H_

#include "task_types.h"

namespace task
{

class LevelCond;
class GoldCond;
class CashCond;
class ScoreCond;
class ItemCond;
class GenderCond;
class RoleClassCond;
class ZoneCond;
class TimeCond;
class MonsterCond;
class TaskCond;
class InstanceCond;
class CounterCond;
class TimeSegmentCond;
class ReputationCond;
class BuffCond;
class CombatValueCond;
class PetPowerCond;
class PetCond;
class CardCond;
class EnhanceCond;
class EquipCond;
class StarCond;
class CatVisionCond;
class TalentCond;
class TaskPremise;
class TaskSucc;
class TaskFail;
class TaskEntry;

// 条件检查的辅助函数
class CondUtil
{
public:
	static int32_t Check(Player* player, const TaskPremise& premise);
	static int32_t Check(Player* player, const TaskEntry* entry, const TaskSucc& succ);
	static int32_t Check(Player* player, const TaskEntry* entry, const TaskFail& fail);
	static void TakeAway(Player* player, const TaskPremise& premise);
	static void TakeAway(Player* player, const TaskSucc& succ);
	static int32_t GetRemainTime(int32_t now, const TaskEntry* entry, const TimeCond& cond, bool succ);
private:
	static bool Check(Player* player, const LevelCond& cond);
	static bool Check(Player* player, const GoldCond& cond);
	static bool Check(Player* player, const CashCond& cond);
	static bool Check(Player* player, const ScoreCond& score);
	static bool Check(Player* player, const ItemCond& cond, bool succ);
	static bool Check(Player* player, const GenderCond& cond);
	static bool Check(Player* player, const RoleClassCond& cond);
	static bool Check(Player* player, const InstanceCond& cond);
	static bool Check(Player* player, const CounterCond& cond, bool succ);
	static bool Check(Player* player, const ZoneCond& cond, bool succ);
	static bool Check(Player* player, const TaskCond& cond, bool mutex);
	static bool Check(Player* player, const TimeSegmentCond& cond);
	static bool Check(Player* player, const ReputationCond& cond);
	static bool Check(Player* player, const BuffCond& cond);
    static bool Check(Player* player, const CombatValueCond& cond);
    static bool Check(Player* player, const PetPowerCond& cond);
    static bool Check(Player* player, const PetCond& cond);
    static bool Check(Player* player, const CardCond& cond);
    static bool Check(Player* player, const EnhanceCond& cond);
    static bool Check(Player* player, const EquipCond& cond);
    static bool Check(Player* player, const StarCond& cond);
	static bool Check(Player* player, const CatVisionCond& cond);
	static bool Check(Player* player, const TalentCond& cond);
	static bool Check(const TaskEntry* entry);
	static bool Check(Player* player, const TaskEntry* entry, const TimeCond& cond, bool succ);
	static bool Check(Player* player, const TaskEntry* entry, const MonsterCond& cond);
};

} // namespace task

#endif // TASK_COND_CHECKER_H_
