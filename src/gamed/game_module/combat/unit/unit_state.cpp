#include "unit_state.h"
#include "combat_unit.h"
#include "combat_def.h"
#include "combat.h"

namespace combat
{

/***************************State***************************/
/***************************State***************************/
/***************************State***************************/
/***************************State***************************/
void State::SetTimeOut(int time)
{
	if (time < 0)
	{
		return;
	}

    //translate to heartbeat-tick
	timeout = time / MSEC_PER_TICK;

    //make up for the precision loss of time
    if ((time % MSEC_PER_TICK) > 20)
    {
        ++ timeout;
    }
}

int State::HeartBeat(CombatUnit* unit)
{
	if (timeout == -1)
	{
        //won't timeout forever
		return -1;
	}

	if (-- timeout <= 0)
	{
        timeout = 0;
		//state timeout
		OnTime(unit);
        return 0;
	}

    //hasn't timeout until now
    return timeout;
}

void State::OnTime(CombatUnit* unit)
{
	switch (status)
	{
		case STATUS_ACTION:
        case STATUS_GOLEM_ACTION:
		{
			unit->RoundEnd();
		}
		break;
		case STATUS_ATTACKED:
        case STATUS_WG_ATTACKED:
		{
			unit->AttackedEnd();
		}
		break;
		case STATUS_ZOMBIE_DYING:
		{
			assert(unit->IsPlayer());
			unit->Dying();
		}
        break;
		case STATUS_DYING:
        {
			assert(unit->IsPlayer());
			unit->Die();
        }
		break;
		case STATUS_ZOMBIE:
		{
			assert(unit->IsPlayer() || unit->IsMob() || unit->IsTeamNpc() || unit->IsBoss());
			unit->Die();
		}
		break;
		case STATUS_SNEAKED:
		{
			assert(false);
			//unit->OnSneakedEnd();
		};
		break;
        case STATUS_TRANSFORM_WAIT:
        {
            assert(unit->IsMob());
            unit->TransformWaitEnd();
        }
        break;
        case STATUS_TRANSFORMING:
        {
            assert(unit->IsMob());
            unit->TransformingEnd();
        }
        case STATUS_ESCAPE_WAIT:
        {
            assert(unit->IsMob());
            unit->EscapeWaitEnd();
        }
        break;
        //case STATUS_PLAYER_ACTION:
        //{
            //assert(unit->IsPlayer());
            //unit->WaitGolemCast();
        //}
        //break;
        //case STATUS_WAIT_GOLEM:
        //{
            //assert(unit->IsPlayer());
            //unit->GolemCast();
        //}
        //break;
		default:
		{
			assert(false && "此状态不应该超时");
		}
		break;
	};
}

/***************************UnitState*******************************/
/***************************UnitState*******************************/
/***************************UnitState*******************************/
/***************************UnitState*******************************/
struct StateShiftNode
{
	int cur;
	int event;
	int next;
	const char* accept_unit_set;
};

/**
 * 战斗对象的状态机
 * 状态机状态转移表
 */
static StateShiftNode state_shift_table[] = 
{
	/*cur-state*/             /*event*/               /*next-state*/            /*accept-unit-set*/
	{STATUS_NULL,             EVENT_INIT,             STATUS_NORMAL,            "all,"},                       // 战斗对象在对象池中被创建时使用
	{STATUS_NORMAL,           EVENT_ACTION,           STATUS_ACTION,            "player, mob, boss, pet,"},    // 正常的攻击状态转换
	{STATUS_NORMAL,           EVENT_ATTACKED,         STATUS_ATTACKED,          "player, mob, boss,"},         // 正常的受击状态转换
	{STATUS_ATTACKED,         EVENT_ATTACKED,         STATUS_ATTACKED,          "player, mob, boss,"},         // 战斗对象被多次攻击时的状态转换
	{STATUS_SNEAKED,          EVENT_ATTACKED,         STATUS_ATTACKED,          "player, mob, boss,"},         // 战斗对象处于偷袭状态时被攻击
	{STATUS_ACTION,           EVENT_TIMEOUT,          STATUS_NORMAL,            "player, mob, boss, pet,"},    // 攻击动作结束后的状态恢复
	{STATUS_ATTACKED,         EVENT_TIMEOUT,          STATUS_NORMAL,            "player, mob, boss,"},         // 受击动作结束后的状态恢复
	{STATUS_NORMAL,           EVENT_DYING,            STATUS_ZOMBIE_DYING,      "player,"},                    // 玩家血量小于0，并且战斗还有队友存在时的状态转换
	{STATUS_ATTACKED,         EVENT_DYING,            STATUS_ZOMBIE_DYING,      "player,"},                    // 玩家被多次攻击导致的血量小于0，并且还有活的队友存在
	{STATUS_ZOMBIE_DYING,     EVENT_ATTACKED,         STATUS_ZOMBIE_DYING,      "player,"},                    // 玩家濒死后受到攻击，此时实际上玩家还未正式进入濒死状态
	{STATUS_ZOMBIE_DYING,     EVENT_TIMEOUT,          STATUS_DYING,             "player,"},                    // 玩家受击结束，正式进入濒死状态
	{STATUS_ZOMBIE_DYING,     EVENT_REVIVE,           STATUS_NORMAL,            "player,"},                    // 玩家从濒死状态1复活
	{STATUS_DYING,            EVENT_ATTACKED,         STATUS_DYING,             "player,"},                    // 玩家濒死后受到攻击
	{STATUS_DYING,            EVENT_TIMEOUT,          STATUS_DEAD,              "player,"},                    // 玩家濒死状态2超时，进入死亡状态
	{STATUS_DYING,            EVENT_REVIVE,           STATUS_NORMAL,            "player,"},                    // 玩家从濒死状态2复活
	{STATUS_DEAD,             EVENT_REVIVE,           STATUS_NORMAL,            "player,"},                    // 玩家从死亡状态复活
	{STATUS_NORMAL,           EVENT_ZOMBIE,           STATUS_ZOMBIE,            "player, mob, boss,"},         // 正常状态死亡
	{STATUS_ACTION,           EVENT_ZOMBIE,           STATUS_ZOMBIE,            "player, mob, boss,"},         // 攻击时死亡
	{STATUS_ATTACKED,         EVENT_ZOMBIE,           STATUS_ZOMBIE,            "player, mob, boss,"},         // 受击时候死亡
	{STATUS_SNEAKED,          EVENT_ZOMBIE,           STATUS_ZOMBIE,            "player, mob, boss,"},         // 被偷袭时死亡
	{STATUS_ZOMBIE,           EVENT_TIMEOUT,          STATUS_DEAD,              "player, mob, boss,"},         // 战斗对象正常死亡
	{STATUS_NORMAL,           EVENT_SNEAKED,          STATUS_SNEAKED,           "player, mob, boss,"},         // 战斗对象进入被偷袭状态
	{STATUS_SNEAKED,          EVENT_TIMEOUT,          STATUS_NORMAL,            "player, mob, boss,"},         // 战斗对象被偷袭状态超时

	{STATUS_NORMAL,           EVENT_PLAYER_ACTION,    STATUS_PLAYER_ACTION,     "player,"},                    // 先于魔偶出手并等待魔偶出手完毕
	{STATUS_PLAYER_ACTION,    EVENT_WAIT_GOLEM,       STATUS_WAIT_GOLEM,        "player,"},                    // 玩家召唤魔偶结束，等待魔偶出手
	{STATUS_WAIT_GOLEM,       EVENT_GOLEM_ACTION,     STATUS_GOLEM_ACTION,      "player,"},                    // 魔偶出手，攻击结束
	{STATUS_GOLEM_ACTION,     EVENT_TIMEOUT,          STATUS_ACTION,            "player,"},                    // 魔偶出手，攻击结束
    {STATUS_WAIT_GOLEM,       EVENT_ATTACKED,         STATUS_WG_ATTACKED,       "player,"},                    // 在等待魔偶出手时被攻击
	{STATUS_WG_ATTACKED,      EVENT_ATTACKED,         STATUS_WG_ATTACKED,       "player,"},                    // 等待魔偶战斗对象被多次攻击时的状态转换
	{STATUS_WG_ATTACKED,      EVENT_DYING,            STATUS_ZOMBIE_DYING,      "player,"},                    // 玩家血量小于0，并且战斗还有队友存在时的状态转换
	{STATUS_WG_ATTACKED,      EVENT_ZOMBIE,           STATUS_ZOMBIE,            "player,"},                    // 受击时候死亡
	{STATUS_WG_ATTACKED,      EVENT_TIMEOUT,          STATUS_WAIT_GOLEM,        "player,"},                    // 受击时候死亡
    {STATUS_WAIT_GOLEM,       EVENT_ZOMBIE,           STATUS_ZOMBIE,            "player,"},                    // 在等待魔偶出手时死亡

    //怪物变身相关的状态转换
    {STATUS_ACTION,           EVENT_TRANSFORM,        STATUS_TRANSFORM_WAIT,    "mob,"},                       // 怪物变身准备
    {STATUS_ATTACKED,         EVENT_TRANSFORM,        STATUS_TRANSFORM_WAIT,    "mob,"},                       // 怪物变身准备
    {STATUS_NORMAL,           EVENT_TRANSFORM,        STATUS_TRANSFORM_WAIT,    "mob,"},                       // 怪物变身准备
    {STATUS_TRANSFORM_WAIT,   EVENT_TIMEOUT,          STATUS_DEAD,              "mob,"},                       // 怪物变身等待超时
    {STATUS_NORMAL,           EVENT_TRANSFORMING,     STATUS_TRANSFORMING,      "mob,"},                       // 怪物开始变身
    {STATUS_TRANSFORMING,     EVENT_TIMEOUT,          STATUS_NORMAL,            "mob,"},                       // 怪物变身完成

    //魔偶状态转换相关，暂时未生效
	{STATUS_NORMAL,           EVENT_ACTION,           STATUS_NORMAL,            "golem,"},                       // 正常的攻击状态转换
	{STATUS_SLEEP,            EVENT_ACTIVED,          STATUS_NORMAL,            "golem,"},
	{STATUS_NORMAL,           EVENT_DEACTIVED,        STATUS_SLEEP,             "golem,"},
	{STATUS_ACTION,           EVENT_DEACTIVED,        STATUS_SLEEP,             "golem,"},
	{STATUS_ZOMBIE,           EVENT_DEACTIVED,        STATUS_ZOMBIE,            "golem,"},
	{STATUS_NORMAL,           EVENT_POWER_EMPTY,      STATUS_ZOMBIE,            "golem,"},
	{STATUS_ACTION,           EVENT_POWER_EMPTY,      STATUS_ZOMBIE,            "golem,"},
	{STATUS_SLEEP,            EVENT_POWER_FULL,       STATUS_SLEEP,             "golem,"},
	{STATUS_ZOMBIE,           EVENT_POWER_FULL,       STATUS_SLEEP,             "golem,"},

    //怪物逃跑相关的状态转换
    {STATUS_ACTION,           EVENT_ESCAPE,           STATUS_ESCAPE_WAIT,       "mob,"},                       // 怪物逃跑准备
    {STATUS_ATTACKED,         EVENT_ESCAPE,           STATUS_ESCAPE_WAIT,       "mob,"},                       // 怪物逃跑准备
    {STATUS_NORMAL,           EVENT_ESCAPE,           STATUS_ESCAPE_WAIT,       "mob,"},                       // 怪物逃跑准备
    {STATUS_ESCAPE_WAIT,      EVENT_TIMEOUT,          STATUS_DEAD,              "mob,"},                       // 怪物逃跑等待超时

    //释放
	{STATUS_NORMAL,           EVENT_RELEASE,          STATUS_NULL,              "all,"},                       // 释放战斗对象给对象池
	{STATUS_ACTION,           EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_ATTACKED,         EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_ZOMBIE_DYING,     EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_DYING,            EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_DEAD,             EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_SLEEP,            EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_ZOMBIE,           EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_SNEAKED,          EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_TRANSFORM_WAIT,   EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_TRANSFORMING,     EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_PLAYER_ACTION,    EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_WAIT_GOLEM,       EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_GOLEM_ACTION,     EVENT_RELEASE,          STATUS_NULL,              "all,"},
	{STATUS_ESCAPE_WAIT,      EVENT_RELEASE,          STATUS_NULL,              "all,"},
};

static int null[]             = { };
static int normal[]           = { OPT_SELECT_SKILL, OPT_ACTION, OPT_ATTACKED, OPT_PET_ATTACK, OPT_TRANSFORM, OPT_ESCAPE };
static int action[]           = { OPT_SELECT_SKILL, OPT_PET_ATTACK, OPT_TRANSFORM, OPT_ESCAPE };
static int attacked[]         = { OPT_SELECT_SKILL, OPT_ATTACKED, OPT_PET_ATTACK, OPT_TRANSFORM, OPT_ESCAPE };
static int zombie_dying[]     = { OPT_ATTACKED };
static int dying[]            = { OPT_ATTACKED };
static int dead[]             = { };
static int sleep[]            = { OPT_SUMMONED };
static int zombie[]           = { };
static int sneaked[]          = { OPT_SELECT_SKILL, OPT_ATTACKED, OPT_PET_ATTACK};
static int transform_wait[]   = { };
static int transforming[]     = { };
static int escape_wait[]      = { };
static int player_action[]    = { OPT_SELECT_SKILL };
static int wait_golem[]       = { OPT_SELECT_SKILL, OPT_ATTACKED };
static int golem_attacked[]   = { OPT_SELECT_SKILL, OPT_ATTACKED };
static int golem_action[]     = { OPT_SELECT_SKILL };

UnitState::StateShiftTable UnitState::state_table;
UnitState::StateVec UnitState::state_pool;
UnitState::StateShiftRecordMap UnitState::state_shift_record_map;
shared::MutexLock UnitState::record_map_lock;

static void TransUnitList(const char* str, std::vector<UnitType>& list)
{
	if (!str) return;

	std::string buf(str);
	if (buf.find("player") != std::string::npos)
		list.push_back(UT_PLAYER);
	if (buf.find("mob") != std::string::npos)
		list.push_back(UT_MOB);
	if (buf.find("pet") != std::string::npos)
		list.push_back(UT_PET);
	if (buf.find("golem") != std::string::npos)
		list.push_back(UT_GOLEM);
	if (buf.find("boss") != std::string::npos)
		list.push_back(UT_BOSS);
	if (buf.find("all") != std::string::npos)
	{
		list.push_back(UT_PLAYER);
		list.push_back(UT_MOB);
		list.push_back(UT_PET);
		list.push_back(UT_GOLEM);
		list.push_back(UT_TEAM_NPC);
		list.push_back(UT_BOSS);
	}

	assert(!list.empty());
}

#define MAKE_KEY(cur, next, event) ((cur << 16) | (next << 8) | event)

void UnitState::InitStateTable()
{
	/*初始化状态池*/
	state_pool.resize(STATUS_MAX);
	state_pool[STATUS_NULL]           = State(STATUS_NULL,            null,           sizeof(null)/sizeof(int));
	state_pool[STATUS_NORMAL]         = State(STATUS_NORMAL,          normal,         sizeof(normal)/sizeof(int));
	state_pool[STATUS_ACTION]         = State(STATUS_ACTION,          action,         sizeof(action)/sizeof(int));
	state_pool[STATUS_ATTACKED]       = State(STATUS_ATTACKED,        attacked,       sizeof(attacked)/sizeof(int));
	state_pool[STATUS_ZOMBIE_DYING]   = State(STATUS_ZOMBIE_DYING,    zombie_dying,   sizeof(zombie_dying)/sizeof(int));
	state_pool[STATUS_DYING]          = State(STATUS_DYING,           dying,          sizeof(dying)/sizeof(int));
	state_pool[STATUS_DEAD]           = State(STATUS_DEAD,            dead,           sizeof(dead)/sizeof(int));
	state_pool[STATUS_SLEEP]          = State(STATUS_SLEEP,           sleep,          sizeof(sleep)/sizeof(int));
	state_pool[STATUS_ZOMBIE]         = State(STATUS_ZOMBIE,          zombie,         sizeof(zombie)/sizeof(int));
	state_pool[STATUS_SNEAKED]        = State(STATUS_SNEAKED,         sneaked,        sizeof(sneaked)/sizeof(int));
	state_pool[STATUS_TRANSFORM_WAIT] = State(STATUS_TRANSFORM_WAIT,  transform_wait, sizeof(transform_wait)/sizeof(int));
	state_pool[STATUS_TRANSFORMING]   = State(STATUS_TRANSFORMING,    transforming,   sizeof(transforming)/sizeof(int));
	state_pool[STATUS_ESCAPE_WAIT]    = State(STATUS_ESCAPE_WAIT,     escape_wait,    sizeof(escape_wait)/sizeof(int));
	state_pool[STATUS_PLAYER_ACTION]  = State(STATUS_PLAYER_ACTION,   player_action,  sizeof(player_action)/sizeof(int));
	state_pool[STATUS_WAIT_GOLEM]     = State(STATUS_WAIT_GOLEM,      wait_golem,     sizeof(wait_golem)/sizeof(int));
	state_pool[STATUS_WG_ATTACKED]    = State(STATUS_WG_ATTACKED,     golem_attacked, sizeof(golem_attacked)/sizeof(int));
	state_pool[STATUS_GOLEM_ACTION]   = State(STATUS_GOLEM_ACTION,    golem_action,   sizeof(golem_action)/sizeof(int));


	/*初始化状态转换表*/
	for (size_t i = 0; i < sizeof(state_shift_table) / sizeof(StateShiftNode); ++ i)
	{
		StateShiftNode& entry = state_shift_table[i];
		Event event = static_cast<Event>(entry.event);
		Status cur_status = static_cast<Status>(entry.cur);
		Status next_status = static_cast<Status>(entry.next);

		std::vector<UnitType> list;
		TransUnitList(entry.accept_unit_set, list);
		for (size_t i = 0; i < list.size(); ++ i)
		{
			UnitType ut = list[i];
			state_table[ut][cur_status][event] = next_status;
		}
	}

	/*初始化状态转移记录表*/
	StateShiftTable::iterator it = state_table.begin();
	for (; it != state_table.end(); ++ it)
	{
		UnitType unit_type = static_cast<UnitType>(it->first);
		ShiftMap& map = state_shift_record_map[unit_type];

		StateShiftMap& map1 = it->second;
		for (StateShiftMap::iterator it1 = map1.begin(); it1 != map1.end(); ++ it1)
		{
			Status cur = it1->first;
			Event2StatusMap& map2 = it1->second;
			for (Event2StatusMap::iterator it2 = map2.begin(); it2 != map2.end(); ++ it2)
			{
				Status next = it2->second;
				Event  event = it2->first;

				ShiftRecord record;
				record.cur = cur;
				record.next = next;
				record.event = event;
				record.times = 0;

				int key = MAKE_KEY(cur, next, event);
				assert(map.find(key) == map.end());
				map[key] = record;
			}
		}
	}
}

UnitState::UnitState():
	unit_type(UT_INVALID)
{
	state = state_pool[STATUS_NULL];
}

UnitState::~UnitState()
{
}

void UnitState::Init(int utype)
{
	unit_type = utype;
	Update(EVENT_INIT);
}

bool UnitState::Update(Event event, int timeout, CombatUnit* unit)
{
	Event2StatusMap& event2status_map = state_table[unit_type][state.status];
	Event2StatusMap::iterator it = event2status_map.find(event);
	assert(it != event2status_map.end());

    int32_t old_status = state.status;
    int32_t old_timeout = state.GetTimeOut();

	//test
	RecordStateShift(unit_type, state.status, state_pool[it->second].status, event);

	state = state_pool[it->second];
	state.SetTimeOut(timeout);

    if (unit != NULL && state.status == STATUS_WG_ATTACKED)
    {    
	__PRINTF("UnitState::Update unit_id=%d unit_type=%d old_status=%d old_timeout=%d event=%d new_status=%d new_timeout=%d", unit->GetID(), unit_type, old_status, old_timeout, event, state.status, state.GetTimeOut());
    }
	return true;
}

void UnitState::HeartBeat(CombatUnit* unit)
{
	int rst = state.HeartBeat(unit);
    if (!rst)
    {
        //state timeout
        if (IsZombieDying())
        {
            Update(EVENT_TIMEOUT, MAX_PLAYER_DYING_TIME);
        }
        else if (IsGolemAttacked())
        {
            Update(EVENT_TIMEOUT, 100);
        }
        else if (!IsPlayerAction() && !IsWaitGolem() && !IsGolemAction())
        {
            Update(EVENT_TIMEOUT);
        }
    }
    else if (rst < 0)
    {
        //won't timeout forever
    }
    else if (rst > 0)
    {
        //hasn't timeout until now
    }
}

bool UnitState::OptionPolicy(int opt) const
{
	return state.OptionPolicy(opt);
}

void UnitState::Clear()
{
	Update(EVENT_RELEASE);
	unit_type = UT_INVALID;
}

void UnitState::RecordStateShift(int unit_type, Status cur, Status next, Event event)
{
	record_map_lock.lock();

	ShiftMap& record_map = state_shift_record_map[unit_type];

	int key = MAKE_KEY(cur, next, event);
	assert(record_map.find(key) != record_map.end());
	record_map[key].times ++;

	record_map_lock.unlock();
}

void UnitState::Trace()
{

#define DUMP_STATE_SHIFT_RECORD(unit_type) \
	{ \
		switch (unit_type) \
		{ \
		case UT_PLAYER: \
			__PRINTF("玩家状态转移记录："); \
		break; \
		case UT_MOB: \
			__PRINTF("怪物状态转移记录："); \
		break; \
		case UT_PET: \
			__PRINTF("战宠状态转移记录："); \
		break; \
		case UT_GOLEM: \
			__PRINTF("魔偶状态转移记录："); \
		break; \
		case UT_BOSS: \
			  __PRINTF("世界BOSS状态转移表："); \
		break; \
		break; \
		default: \
			assert(false); \
		break; \
		} \
		__PRINTF("当前状态\t目标状态\t触发事件\t触发次数"); \
		ShiftMap& map = state_shift_record_map[unit_type]; \
		for (ShiftMap::iterator it = map.begin(); it != map.end(); ++ it) \
		{ \
			const char* cur_status_name = status_name_vec[it->second.cur]; \
			const char* next_status_name = status_name_vec[it->second.next]; \
			const char* event_name = event_name_vec[it->second.event]; \
			__PRINTF("%-16s%-16s%-16s%-16d", cur_status_name, next_status_name, event_name, it->second.times); \
		} \
	}

	static const char* status_name_vec[] =  {
											"null",
											"normal",
											"action",
											"attacked",
											"zombie_dying",
											"dead",
											"sleep",
											"zombie",
											"sneaked"
											};

	static const char* event_name_vec[] =   {
											"init",
											"action",
											"attacked",
											"zombie_dying",
											"dead",
											"zombie",
											"release",
											"actived",
											"deactived",
											"power_full",
											"power_empty",
											"sneak_attacked",
											"zombie2",
											"revive",
											"timeout"
											};

	record_map_lock.lock();

	DUMP_STATE_SHIFT_RECORD(UT_PLAYER);
	DUMP_STATE_SHIFT_RECORD(UT_MOB);
	DUMP_STATE_SHIFT_RECORD(UT_PET);
	DUMP_STATE_SHIFT_RECORD(UT_GOLEM);
	DUMP_STATE_SHIFT_RECORD(UT_TEAM_NPC);
	DUMP_STATE_SHIFT_RECORD(UT_BOSS);

	record_map_lock.unlock();

#undef DUMP_STATE_SHIFT_RECORD
}

};
