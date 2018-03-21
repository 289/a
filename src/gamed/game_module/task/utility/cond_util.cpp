#include "cond_util.h"
#include "task_interface.h"
#include "task_templ.h"
#include "task_data.h"
#include "entry_util.h"
#include "util.h"

namespace task
{

static bool CompareValue(int32_t value, int32_t needed, int8_t op)
{
    if (op == VALUE_OP_LESS && value >= needed)
    {
        return false;
    }
    else if (op == VALUE_OP_LESS_EQUAL && value > needed)
    {
        return false;
    }
    else if (op == VALUE_OP_EQUAL && value != needed)
    {
        return false;
    }
    else if (op == VALUE_OP_GREATER_EQUAL && value < needed)
    {
        return false;
    }
    else if (op == VALUE_OP_GREATER && value <= needed)
    {
        return false;
    }
    return true;
}

bool CondUtil::Check(Player* player, const LevelCond& cond)
{
	bool qualified = true;
	int32_t level = player->GetLevel();
	if (cond.min_level && level < cond.min_level)
	{
		qualified = false;
	}
	if (cond.max_level && level > cond.max_level)
	{
		qualified = false;
	}
	return qualified;
}

bool CondUtil::Check(Player* player, const GoldCond& cond)
{
	return player->GetGoldNum() >= cond.gold;
}

bool CondUtil::Check(Player* player, const CashCond& cond)
{
	return player->GetCash() >= cond.cash;
}

bool CondUtil::Check(Player* player, const ScoreCond& cond)
{
	return player->GetScore() >= cond.score;
}

bool CondUtil::Check(Player* player, const ItemCond& cond, bool succ)
{
	const ItemInfoVec& item_list = cond.item_list;
	ItemInfoVec::const_iterator it = item_list.begin();
    for (; it != item_list.end(); ++it)
    {
        if (cond.all_match && player->GetItemCount(it->id) < it->count)
        {
            return false;
        }
        else if (!cond.all_match && player->GetItemCount(it->id) >= it->count)
        {
            return true;
        }
    }
    return item_list.empty() ? succ : cond.all_match;
}

bool CondUtil::Check(Player* player, const GenderCond& cond)
{
	return cond.gender == GENDER_NONE || player->GetGender() == cond.gender;
}

bool CondUtil::Check(Player* player, const RoleClassCond& cond)
{
	if (cond.class_list.size() == 0)
	{
		return true;
	}

	RoleClassCond::RoleClassVec::const_iterator it = cond.class_list.begin();
	for (; it != cond.class_list.end(); ++it)
	{
		if (*it == player->GetRoleClass())
		{
			return true;
		}
	}
	return false;
}

bool CondUtil::Check(Player* player, const InstanceCond& cond)
{
	if (cond.instance_list.size() == 0)
	{
		return true;
	}

    int32_t count = 0;
	InstanceInfoVec::const_iterator it = cond.instance_list.begin();
	for (; it != cond.instance_list.end(); ++it)
	{
        count = player->GetInsFinishCount(it->id);
		if (cond.all_match && count < it->value)
		{
			return false;
		}
        else if (!cond.all_match && count >= it->value)
        {
            return true;
        }
	}
	return cond.all_match;
}

static bool MatchCounter(int32_t own, int32_t cmp, int8_t op)
{
	switch (op)
	{
	case COUNTER_OP_GREATER:
		return own > cmp;
	case COUNTER_OP_GREATER_EQUAL:
		return own >= cmp;
	case COUNTER_OP_LESS:
		return own < cmp;
	case COUNTER_OP_LESS_EQUAL:
		return own <= cmp;
	case COUNTER_OP_EQUAL:
		return own == cmp;
	default:
		return false;
	}
}

bool CondUtil::Check(Player* player, const CounterCond& cond, bool succ)
{
	int32_t value = 0;
	CounterVec::const_iterator it = cond.counter_list.begin();
	for (; it != cond.counter_list.end(); ++it)
	{
        if (it->type == COUNTER_GLOBAL)
        {
            if (!player->GetGlobalCounter(it->id, value))
            {
                return false;
            }
        }
        else if (it->type == COUNTER_MAP)
        {
            if (!player->GetMapCounter(it->id, it->world_id, value))
            {
                return false;
            }
        }
        else if (it->type == COUNTER_PLAYER)
        {
            if (!player->GetPlayerCounter(it->id, value))
            {
                return false;
            }
        }

		if (cond.all_match && !MatchCounter(value, it->value, it->op))
		{
			return false;
		}
		if (!cond.all_match && MatchCounter(value, it->value, it->op))
		{
			return true;
		}
	}
    return cond.counter_list.empty() ? succ : cond.all_match;
}

bool CondUtil::Check(Player* player, const ZoneCond& cond, bool succ)
{
	const ZoneOpVec& zone_list = cond.zone_list;
	if (zone_list.size() == 0)
	{
		return succ;
	}

	double x = 0, y = 0;
	int32_t world_id = player->GetPos(x, y);
	ZoneOpVec::const_iterator it = zone_list.begin();
	for (; it != zone_list.end(); ++it)
	{
        //if (it->Match(world_id, (float)x, (float)y))
        if (Util::MatchZoneOp(it->zone, world_id, (float)x, (float)y, it->op))
        {
            return true;
        }
	}
	return false;
}

static bool Exist(TaskData* task, const TaskInfo& info, bool cmp_count = false)
{
	EntryMap::iterator it = task->task_list.find(info.id);
	if (it == task->task_list.end())
	{
		return false;
	}
    if (!cmp_count)
    {
        return true;
    }
    int32_t* data = (int32_t*)(&(it->second.data[0]));
    return data[0] >= info.value;
}

static bool CheckPremiseTask(Player* player, const TaskCond& cond)
{
	size_t match_count = 0;
	FinishTask* finish = player->GetFinishTask();
	FinishTimeTask* finish_time = player->GetFinishTimeTask();
	const TaskInfoVec& task_list = cond.task_list;
	TaskInfoVec::const_iterator it = task_list.begin();
	for (; it != task_list.end(); ++it)
	{
		const TaskInfo& info = *it;
		if (Exist(finish, info) || Exist(finish_time, info, true))
		{
			++match_count;
		}
	}
	return match_count == task_list.size();
}

static bool CheckMutexTask(Player* player, const TaskCond& cond)
{
	ActiveTask* active = player->GetActiveTask();
	FinishTask* finish = player->GetFinishTask();
	FinishTimeTask* finish_time = player->GetFinishTimeTask();
	const TaskInfoVec& task_list = cond.task_list;
	TaskInfoVec::const_iterator it = task_list.begin();
	for (; it != task_list.end(); ++it)
	{
		const TaskInfo& info = *it;
		if (Exist(active, info) || Exist(finish, info) || Exist(finish_time, info, true))
		{
			return true;
		}
	}
	return false;
}

bool CondUtil::Check(Player* player, const TaskCond& cond, bool mutex)
{
	return mutex ? CheckMutexTask(player, cond) : CheckPremiseTask(player, cond);
}

bool CondUtil::Check(const TaskEntry* entry)
{
	// 检查任务脚本执行完成标记是否设置
    return entry->IsFinishGoal(); 
}

// 返回还剩余多少时间
int32_t CondUtil::GetRemainTime(int32_t now, const TaskEntry* entry, const TimeCond& cond, bool succ)
{
	if (!succ && cond.time == 0)
	{
		// 直接返回now让其不满足
		return now;
	}

	if (cond.record_offline)
	{
		return cond.time - (now - entry->deliver_time);
	}
	int32_t offset = succ ? 0 : EntryUtil::CalcFailOffset(entry->templ);
	int32_t* data = (int32_t*)(&(entry->data[offset]));
	return cond.time - (data[0] + now - data[1]);
}

bool CondUtil::Check(Player* player, const TaskEntry* entry, const TimeCond& cond, bool succ)
{
	int32_t remain = GetRemainTime(player->GetCurTime(), entry, cond, succ);
	return remain <= 0;
}

bool CondUtil::Check(Player* player, const TaskEntry* entry, const MonsterCond& cond)
{
	int32_t count = 0;
	int32_t* data = (int32_t*)(&(entry->data[0]));
	const MonsterKilledVec& monster_list = cond.monster_list;
	for (size_t i = 0; i < monster_list.size(); ++i)
	{
		const MonsterKilled& monster = monster_list[i];
		count = monster.drop_item_id == 0 ? data[i] : player->GetItemCount(monster.drop_item_id);
		if (count < monster.monster_count)
		{
			return false;
		}
	}
	return true;
}

bool CondUtil::Check(Player* player, const TimeSegmentCond& cond)
{
	return cond.segment.IsMatch(player->GetCurTime());
}

bool CondUtil::Check(Player* player, const ReputationCond& cond)
{
    ReputationVec::const_iterator rit = cond.reputation_list.begin();
    for (; rit != cond.reputation_list.end(); ++rit)
    {
        if (rit->op < REPUTATION_OP_EQUAL_GREATER || rit->op > REPUTATION_OP_LESS)
        {
            return false;
        }
        int32_t rep = player->GetReputation(rit->id);
        if (rep == -1) // 没开启
        {
            return false;
        }
        if (rit->op == REPUTATION_OP_EQUAL_GREATER && rep < rit->value)
        {
            return false;
        }
        if (rit->op == REPUTATION_OP_LESS && rep >= rit->value)
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const BuffCond& cond)
{
    BuffVec::const_iterator bit = cond.buff_list.begin();
    for (; bit != cond.buff_list.end(); ++bit)
    {
        if (!player->IsWorldBuffExist(*bit))
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const CombatValueCond& cond)
{
    int32_t combat_value = player->GetCombatValue();
    CombatValueVec::const_iterator vit = cond.value_list.begin();
    for (; vit != cond.value_list.end(); ++vit)
    {
        if (!CompareValue(combat_value, vit->value, vit->op))
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const PetPowerCond& cond)
{
    return player->GetPetPower() >= cond.value;
}

bool CondUtil::Check(Player* player, const PetCond& cond)
{
    PetVec::const_iterator it = cond.pet_list.begin();
    for (; it != cond.pet_list.end(); ++it)
    {
        if (player->GetPetNum(it->level, it->blevel, it->rank) < it->needed_value)
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const CardCond& cond)
{
    CardVec::const_iterator it = cond.card_list.begin();
    for (; it != cond.card_list.end(); ++it)
    {
        if (player->GetCardNum(it->level, it->rank) < it->needed_value)
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const EnhanceCond& cond)
{
    EnhanceVec::const_iterator it = cond.enhance_list.begin();
    for (; it != cond.enhance_list.end(); ++it)
    {
        if (player->GetEnhanceNum(it->level, it->rank) < it->needed_value)
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const EquipCond& cond)
{
    EquipVec::const_iterator it = cond.equip_list.begin();
    for (; it != cond.equip_list.end(); ++it)
    {
        if (player->GetEquipNum(it->level, it->rank) < it->needed_value)
        {
            return false;
        }
    }
    return true;
}

bool CondUtil::Check(Player* player, const StarCond& cond)
{
    return player->GetFullActivateStarNum() >= cond.full_activate_num;
}

bool CondUtil::Check(Player* player, const CatVisionCond& cond)
{
	bool qualified = true;
	int32_t level = player->GetCatVisionLevel();
	if (cond.min_level && level < cond.min_level)
	{
		qualified = false;
	}
	if (cond.max_level && level > cond.max_level)
	{
		qualified = false;
	}
	return qualified;
}

bool CondUtil::Check(Player* player, const TalentCond& cond)
{
    TalentVec::const_iterator rit = cond.talent_list.begin();
    for (; rit != cond.talent_list.end(); ++rit)
    {
        int32_t value = player->GetTalentLevel(rit->talent_gid, rit->talent_id);
        if (!CompareValue(value, rit->value, rit->op))
        {
            return false;
        }
    }
    return true;
}

int32_t CondUtil::Check(Player* player, const TaskPremise& premise)
{
	if (!Check(player, premise.level))
	{
		return ERR_TASK_LEVEL;
	}
	if (!Check(player, premise.gold))
	{
		return ERR_TASK_GOLD;
	}
	if (!Check(player, premise.cash))
	{
		return ERR_TASK_CASH;
	}
	if (!Check(player, premise.score))
	{
		return ERR_TASK_SCORE;
	}
	if (!Check(player, premise.item, true))
	{
		return ERR_TASK_ITEM;
	}
	if (!Check(player, premise.gender))
	{
		return ERR_TASK_GENDER;
	}
	if (!Check(player, premise.roleclass))
	{
		return ERR_TASK_ROLECLASS;
	}
	if (!Check(player, premise.zone, true))
	{
		return ERR_TASK_ZONE;
	}
	if (!Check(player, premise.premise_task, false))
	{
		return ERR_TASK_TASK;
	}
	if (Check(player, premise.mutex_task, true))
	{
		return ERR_TASK_TASK;
	}
	if (!Check(player, premise.instance))
	{
		return ERR_TASK_INSTANCE;
	}
	if (!Check(player, premise.global_counter, true))
	{
		return ERR_TASK_COUNTER;
	}
	if (!Check(player, premise.segment))
	{
		return ERR_TASK_TIME;
	}
    if (!Check(player, premise.reputation))
    {
        return ERR_TASK_REPUTATION;
    }
    if (!Check(player, premise.buff))
    {
        return ERR_TASK_BUFF;
    }
    if (!Check(player, premise.combat_value))
    {
        return ERR_TASK_COMBAT_VALUE;
    }
    if (!Check(player, premise.pet_power))
    {
        return ERR_TASK_PET_POWER;
    }
    if (!Check(player, premise.pet))
    {
        return ERR_TASK_PET;
    }
    if (!Check(player, premise.card))
    {
        return ERR_TASK_CARD;
    }
    if (!Check(player, premise.enhance))
    {
        return ERR_TASK_ENHANCE;
    }
    if (!Check(player, premise.equip))
    {
        return ERR_TASK_EQUIP;
    }
	if (!Check(player, premise.map_counter, true))
	{
		return ERR_TASK_COUNTER;
	}
	if (!Check(player, premise.player_counter, true))
	{
		return ERR_TASK_COUNTER;
	}
	if (!Check(player, premise.cat_vision))
	{
		return ERR_TASK_CAT_VISION;
	}
	if (!Check(player, premise.talent))
	{
		return ERR_TASK_TALENT;
	}

	return ERR_TASK_SUCC;
}

void CondUtil::TakeAway(Player* player, const TaskPremise& premise)
{
	if (premise.gold.take_away)
	{
		player->TakeAwayGold(premise.gold.gold);
	}
	if (premise.cash.take_away)
	{
		player->TakeAwayCash(premise.cash.cash);			
	}
	if (premise.score.take_away)
	{
		player->TakeAwayScore(premise.score.score);
	}
	if (premise.item.take_away)
	{
		const ItemInfoVec& item_list = premise.item.item_list;
		ItemInfoVec::const_iterator it = item_list.begin();
		for (; it != item_list.end(); ++it)
		{
			player->TakeAwayItem(it->id, it->count);
		}
	}
}

void CondUtil::TakeAway(Player* player, const TaskSucc& succ)
{
	if (succ.goal == GOAL_GOLD && succ.gold.take_away)
	{
		player->TakeAwayGold(succ.gold.gold);
	}
	else if (succ.goal == GOAL_CASH && succ.cash.take_away)
	{
		player->TakeAwayCash(succ.cash.cash);
	}
	else if (succ.goal == GOAL_SCORE && succ.score.take_away)
	{
		player->TakeAwayScore(succ.score.score);
	}
	else if (succ.goal == GOAL_COLLECT_ITEM && succ.item.take_away)
	{
		const ItemInfoVec& item_list = succ.item.item_list;
		ItemInfoVec::const_iterator it = item_list.begin();
		for (; it != item_list.end(); ++it)
		{
			player->TakeAwayItem(it->id, it->count);
		}
	}
}

int32_t CondUtil::Check(Player* player, const TaskEntry* entry, const TaskSucc& succ)
{
    if (!entry->IsScriptEnd())
    {
        return ERR_TASK_SCRIPT;
    }

	switch (succ.goal)
	{
	case GOAL_NO:
		return ERR_TASK_SUCC;
	case GOAL_LEVEL:
		return Check(player, succ.level) ? ERR_TASK_SUCC : ERR_TASK_LEVEL;
	case GOAL_GOLD:
		return Check(player, succ.gold) ? ERR_TASK_SUCC : ERR_TASK_GOLD;
	case GOAL_CASH:
		return Check(player, succ.cash) ? ERR_TASK_SUCC : ERR_TASK_CASH;
	case GOAL_SCORE:
		return Check(player, succ.score) ? ERR_TASK_SUCC : ERR_TASK_SCORE;
	case GOAL_COLLECT_ITEM:
		return Check(player, succ.item, true) ? ERR_TASK_SUCC : ERR_TASK_ITEM;
	case GOAL_KILL_MONSTER:
		return Check(player, entry, succ.monster) ? ERR_TASK_SUCC : ERR_TASK_MONSTER;
	case GOAL_WAIT_TIME:
		return Check(player, entry, succ.time, true) ? ERR_TASK_SUCC : ERR_TASK_TIME;
	case GOAL_ZONE:
		return Check(player, succ.zone, true) ? ERR_TASK_SUCC : ERR_TASK_ZONE;
	case GOAL_INSTANCE:
		return Check(player, succ.instance) ? ERR_TASK_SUCC : ERR_TASK_INSTANCE;
	case GOAL_GLOBAL_COUNTER:
		return Check(player, succ.global_counter, true) ? ERR_TASK_SUCC : ERR_TASK_COUNTER;
    case GOAL_REPUTATION:
        return Check(player, succ.reputation) ? ERR_TASK_SUCC : ERR_TASK_REPUTATION;
    case GOAL_BUFF:
        return Check(player, succ.buff) ? ERR_TASK_SUCC : ERR_TASK_BUFF;
    case GOAL_COMBAT_VALUE:
        return Check(player, succ.combat_value) ? ERR_TASK_SUCC : ERR_TASK_COMBAT_VALUE;
    case GOAL_PET_POWER:
        return Check(player, succ.pet_power) ? ERR_TASK_SUCC : ERR_TASK_PET_POWER;
    case GOAL_PET:
        return Check(player, succ.pet) ? ERR_TASK_SUCC : ERR_TASK_PET;
    case GOAL_CARD:
        return Check(player, succ.card) ? ERR_TASK_SUCC : ERR_TASK_CARD;
    case GOAL_ENHANCE:
        return Check(player, succ.enhance) ? ERR_TASK_SUCC : ERR_TASK_ENHANCE;
    case GOAL_EQUIP:
        return Check(player, succ.equip) ? ERR_TASK_SUCC : ERR_TASK_EQUIP;
	case GOAL_MAP_COUNTER:
		return Check(player, succ.map_counter, true) ? ERR_TASK_SUCC : ERR_TASK_COUNTER;
	case GOAL_PLAYER_COUNTER:
		return Check(player, succ.player_counter, true) ? ERR_TASK_SUCC : ERR_TASK_COUNTER;
    case GOAL_TASK:
    case GOAL_MINIGAME:
    case GOAL_UI:
    case GOAL_COMBAT_FAIL:
        return Check(entry) ? ERR_TASK_SUCC : ERR_TASK_TASK;
    case GOAL_STAR:
        return Check(player, succ.star) ? ERR_TASK_SUCC : ERR_TASK_STAR;
    case GOAL_TALENT:
        return Check(player, succ.talent) ? ERR_TASK_SUCC : ERR_TASK_TALENT;
    case GOAL_TIME_SEGMENT:
        return Check(player, succ.time_segment) ? ERR_TASK_SUCC : ERR_TASK_TIME;
	default:
		return ERR_TASK_TASK;
	}
}

int32_t CondUtil::Check(Player* player, const TaskEntry* entry, const TaskFail& fail)
{
	if (Check(player, fail.zone, false))
	{
		return ERR_TASK_SUCC;
	}
	if (Check(player, entry, fail.time, false))
	{
		return ERR_TASK_SUCC;
	}
	if (Check(player, fail.global_counter, false))
	{
		return ERR_TASK_SUCC;
	}
	if (Check(player, fail.map_counter, false))
	{
		return ERR_TASK_SUCC;
	}
	if (Check(player, fail.player_counter, false))
	{
		return ERR_TASK_SUCC;
	}
    if (Check(player, fail.item, false))
    {
        return ERR_TASK_SUCC;
    }
	return ERR_TASK_TASK;
}

} // namespace task
