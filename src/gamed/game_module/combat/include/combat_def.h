#ifndef __GAME_MODULE_COMBAT_DEF_H__
#define __GAME_MODULE_COMBAT_DEF_H__

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "lua.hpp"
#include "shared/base/callback_bind.h"
#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/proto_packet.h"

namespace combat
{

///
/// 时间相关(单位: ms)
///
#define COMBAT_TICK_PER_SEC             10     // 战斗系统每秒TICK数
#define MSEC_PER_TICK                   100	   // 战斗系统每个TICK有多少毫秒
#define MAX_BUFF_PLAY_TIME              1000   // 战斗BUFF播放时间(ms)，战斗对象在回合开始时由于BUFF掉血而死亡，此时将STATUS_ZOMBIE的时间设置为本值
#define MAX_PLAYER_DYING_TIME           3000   // 玩家濒死时间(ms)
#define COMBAT_OPEN_READY_TIME          1000   // 战斗开始准备时间(ms)
#define COMBAT_WAVE_READY_TIME          100    // 多波后续战斗准备时间(ms)
#define COMBAT_END_LATENCY_TIME         1500   // 战斗结束延迟时间(ms)
#define MONSTER_TRANSFORM_TIME          3000   // 怪物变身需要的时间(ms)

///
/// 战斗单位相关
///
#define MAX_COMBAT_UNIT_NUM             4                   // 最大战斗单位数
//#define MAX_PROP_NUM                    64                  // 最大属性个数
#define MAX_PROP_MASK                   0xFFFFFFFFFFFFFFFF  // 最大属性MASK
#define MAX_PET_INV_CAP                 3                   // 玩家战宠栏上限
#define MIN_COMBAT_POS                  0                   // 战斗站位的最小值
#define MAX_COMBAT_POS                  3                   // 战斗站位的最大值

///
/// 对象池容量
///
#define PVE_COMBAT_SIZE                 5000   // 最大PVE战场数
#define PVP_COMBAT_SIZE                 5000   // 最大PVP战场数
#define COMBAT_NPC_SIZE                 30000  // 最大战斗NPC个数
#define COMBAT_PLAYER_SIZE              8000   // 最大战斗玩家个数
#define WORLD_BOSS_SIZE                 1000   // 最大世界BOSS个数

///
/// 其它定义
///
#define MAX_SCENE_SCRIPT_COUNT          64     // 一个lua_State最多可以加载多少个场景策略脚本

/**
 * @brief 战斗结果
 */
enum COMBAT_RESULT
{
	RESULT_INVALID,
	RESULT_WIN, //赢
	RESULT_FAIL,//输
	RESULT_DRAW,//平局
};

/**
 * @class ItemEntry
 * @brief 简单物品
 */
struct ItemEntry
{
    ItemEntry()
        : item_id(0), item_count(0)
    {
    }

	int32_t item_id;
	int32_t item_count;
	NESTED_DEFINE(item_id, item_count);
};

/**
 * @class CombatAward
 * @brief 玩家战斗奖励
 */
struct CombatAward
{
    CombatAward()
    {
        Clear();
    }

	int64_t exp;
	int64_t money;
	std::vector<ItemEntry> items;
	std::vector<ItemEntry> lottery;
	void Clear()
	{
		exp = 0;
		money = 0;
		items.clear();
		lottery.clear();
	}
};

/**
 * @class MobKilled
 * @brief 击杀怪物信息
 */
struct MobKilled
{
    MobKilled()
        : mob_tid(0), mob_count(0)
    {
    }

	int32_t mob_tid;
	int32_t mob_count;
	NESTED_DEFINE(mob_tid, mob_count);
};

/**
 * @class CombatPVEResult
 * @brief PVE战斗结果
 */
struct CombatPVEResult
{
    CombatPVEResult()
        : result(RESULT_INVALID), monster_group_id(0), challengeid(0), taskid(0), hp(0),
        exp(0), money(0), pet_power(0)
    {
    }

	int8_t  result;
    int32_t monster_group_id;
    int32_t challengeid;
    int32_t taskid;
	int32_t hp;
	int32_t exp;
	int32_t money;
	int32_t pet_power;
	std::vector<ItemEntry> items;
	std::vector<ItemEntry> lottery;
	std::vector<MobKilled> mob_killed_vec;
	NESTED_DEFINE(result, monster_group_id, challengeid, taskid, hp, exp, money, pet_power, items, lottery, mob_killed_vec);
};

/**
 * @class CombatPVPResult
 * @brief PVP战斗结果
 */
struct CombatPVPResult
{
    CombatPVPResult()
        : result(RESULT_INVALID), combat_flag(0), combat_creator(0), hp(0), pet_power(0)
    {
    }

	int8_t  result;
    int32_t combat_flag;
    int64_t combat_creator;
	int32_t hp;
	int32_t pet_power;
	std::vector<int64_t/*roleid*/> player_killed_vec;
	NESTED_DEFINE(result, combat_flag, combat_creator, hp, pet_power, player_killed_vec);
};

/**
 * @class MobInfo
 * @brief 怪物数据
 */
struct MobInfo
{
    MobInfo()
        : id(0), pos(0), lvl(0)
    {
    }

	int32_t id;
	int8_t  pos;
	int8_t  lvl;
};
typedef MobInfo NpcInfo;

/**
 * @brief 世界BOSS的状态，每场战斗结束时同步
 */
enum WBStatusType
{
    WBST_BOSS_WIN = 1,  // 单场boss赢
    WBST_BOSS_LOSE,     // 单场boss输
    WBST_BOSS_DRAW,     // 单场boss平局
    WBST_BOSS_ALIVE,    // boss活着且现在没有战斗
    WBST_BOSS_DEAD,     // boss死亡
};

/**
 * @class WorldBossCombatStatus
 * @brief 世界BOSS战斗结果
 */
struct WorldBossCombatStatus
{
    struct DamageEntry
    {
        DamageEntry()
            : roleid(0), damage(0)
        {
        }

        int64_t roleid;
        int64_t damage;
        NESTED_DEFINE(roleid, damage);
    };

    int32_t status; // 对应WBStatusType枚举
	std::vector<DamageEntry> dmg_list;
	NESTED_DEFINE(status, dmg_list);
};


typedef shared::bind::Callback<void (int64_t, int32_t, const shared::net::ProtoPacket&)> CombatSenderCallBack;
typedef shared::bind::Callback<void (int64_t, int32_t, int32_t, const CombatPVEResult&, int32_t)> CombatPVEResultCallBack;
typedef shared::bind::Callback<void (int64_t, int32_t, int32_t, const CombatPVPResult&, int32_t)> CombatPVPResultCallBack;
typedef shared::bind::Callback<void (int64_t, int32_t)> CombatStartCallBack;
typedef shared::bind::Callback<void (int64_t, int64_t, int32_t, bool, const std::vector<int64_t>, const std::vector<MobKilled>&)> CombatPVEEndCallBack;
typedef shared::bind::Callback<void (int64_t, int64_t, int32_t, bool)> CombatPVPEndCallBack;
typedef shared::bind::Callback<void (int32_t, const WorldBossCombatStatus&)> WorldBossStatusCallBack;


/**
 * @class CombatAwardConfig
 * @brief 战斗奖励相关配置
 * @brief 组队战斗获得的奖励需要用模板中的校正系数计算
 */
class CombatAwardConfig
{
public:
	typedef std::vector<float> FactorVec;

	static FactorVec exp_factor_vec;
	static FactorVec money_factor_vec;

public:
	static void  InitTeamAwardConfig();
	static float GetTeamAwardExpFactor(int n/*组队人数*/);
	static float GetTeamAwardMoneyFactor(int n/*组队人数*/);
};

///
/// global function
///
int32_t GetSkillByLevel(int32_t sk_tree_id, int32_t level); // 根据技能树等级获得相应的调用技能
int32_t GetCombatSceneEventID(int32_t combat_scene_id);     // 获取战斗场景的脚本id
void    StackDump(const char* func, lua_State* state);
void    Normalization(int32_t* prob, size_t size);


extern bool __COMBAT_PRINT_FLAG;
inline void __SETPRTFLAG(bool flag)
{
	__COMBAT_PRINT_FLAG  = flag;
}

#ifdef __NO_STD_OUTPUT__
inline int __PRINTF(const char * fmt, ...)
{
	return 0;
}
#else
#include <stdarg.h>
inline int __PRINTF(const char * fmt, ...)
#ifdef __GNUC__	
		__attribute__ ((format (printf, 1, 2)))
#endif
;

inline int __PRINTF(const char * fmt, ...)
{
	if (__builtin_expect(!__COMBAT_PRINT_FLAG, 1)) 
		return 0;

	va_list ap;
	va_start(ap, fmt);
	int rst = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
	return rst;
}
#endif

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_DEF_H__
