#ifndef __GAME_MODULE_COMBAT__PROP_POLICY_H__
#define __GAME_MODULE_COMBAT__PROP_POLICY_H__

#include <vector>
#include "combat_param.h"

namespace combat
{

///
/// 战斗对象属性
///

typedef int32_t ExtendProp;

enum
{
	PROP_MASK_MAX_HP			= 0x0000000000000001,
	PROP_MASK_MAX_MP            = 0x0000000000000002,
	PROP_MASK_MAX_EP            = 0x0000000000000004,
	PROP_MASK_PHYSICAL_ATTACK   = 0x0000000000000008,
	PROP_MASK_MAGIC_ATTACK      = 0x0000000000000010,
	PROP_MASK_PHYSICAL_DEFENCE	= 0x0000000000000020,
	PROP_MASK_MAGIC_DEFENCE     = 0x0000000000000040,
	PROP_MASK_PHYSICAL_PIERCE   = 0x0000000000000080,
	PROP_MASK_MAGIC_PIERCE      = 0x0000000000000100,
	PROP_MASK_HIT               = 0x0000000000000200,
	PROP_MASK_DODGE             = 0x0000000000000400,
	PROP_MASK_CRIT_HIT          = 0x0000000000000800,
	PROP_MASK_ATB_TIME          = 0x0000000000001000,
	PROP_MASK_ATTACK_PRIORITY   = 0x0000000000002000,
	PROP_MASK_SPEED_GEN_POWER   = 0x0000000000004000,
	PROP_MASK_MOVE_SPEED        = 0x0000000000008000,

	///
	/// 敏感属性掩码,这部分属性变化可能引起其当前值变化
	///
	MUTABLE_EXTPROP_MASK = PROP_MASK_MAX_HP | PROP_MASK_MAX_MP | PROP_MASK_MAX_EP,
};

enum
{
	PROP_INDEX_MAX_HP,
	PROP_INDEX_MAX_MP,
	PROP_INDEX_MAX_EP,
	PROP_INDEX_PHYSICAL_ATTACK,
	PROP_INDEX_MAGIC_ATTACK,
	PROP_INDEX_PHYSICAL_DEFENCE,
	PROP_INDEX_MAGIC_DEFENCE,
	PROP_INDEX_PHYSICAL_PIERCE,
	PROP_INDEX_MAGIC_PIERCE,
	PROP_INDEX_HIT,
	PROP_INDEX_DODGE,
	PROP_INDEX_CRIT_HIT,
	PROP_INDEX_ATB_TIME,
	PROP_INDEX_ATTACK_PRIORITY,
	PROP_INDEX_POWER_GEN_SPEED,
	PROP_INDEX_MOVE_SPEED,
    PROP_INDEX_HIGHEST,             // 目前使用到的属性最大值
	PROP_INDEX_MAX = 64,

	PROP_INDEX_HP = PROP_INDEX_MAX,
	PROP_INDEX_MP,
	PROP_INDEX_EP,
};

struct BasicProp
{
	int32_t hp;
	int32_t mp;
	int32_t ep;
};

struct extend_prop
{
	int max_hp;             //最大血量
	int max_con1;           //最大消耗1
	int max_con2;           //最大消耗2
	int physical_attack;    //物理攻击
	int magic_attack;       //法术攻击
	int physical_defence;   //物理防御
	int magic_defence;      //法术防御
	int physical_pierce;    //物理穿透
	int magic_pierce;       //法术穿透
	int hit_rate;           //命中率
	int dodge_rate;         //闪避率
	int crit_hit_rate;      //暴击率
	int attack_speed;       //攻击速度
	int attack_priority;    //先攻值
	int golem_power;        //魔偶能量
	int reserved2;
};

/*属性同步规则*/
enum PROP_SYNC_RULE
{
	PROP_SYNC_RULE_0,
	PROP_SYNC_RULE_1,
	PROP_SYNC_RULE_2,
	PROP_SYNC_RULE_3,
	PROP_SYNC_RULE_4,
};

/*能量恢复规则*/
enum GEN_POWER_RULE
{
	GEN_POWER_RULE_1=1,
	GEN_POWER_RULE_2,
	GEN_POWER_RULE_3,
	GEN_POWER_RULE_4,
	GEN_POWER_RULE_5,
};

enum CALL_TIME
{
	CT_ATTACK,
	CT_DAMAGED,
	CT_ROUND_END,
};

class CombatUnit;
class PropPolicy
{
public:
	static void UpdateProperty(CombatUnit* obj);
	static void UpdatePower(CombatUnit* obj, int call_time, int32_t damage=0);

private:
	static void UpdateExtendProp(CombatUnit* obj);
	static void UpdateBasicProp(CombatUnit* obj, const ExtendProp* old_props);
	static void UpdateMobBasicProp(CombatUnit* obj);
	static void UpdatePlayerBasicProp(CombatUnit* obj, const ExtendProp* old_props);
};

/**
 * @class PlayerPropRuler
 * @brief 角色属性同步和恢复规则
 */
class PlayerPropRuler
{
public:
	typedef int PropSyncRule;
	typedef int PropGenRule;
	typedef struct
	{
		PropSyncRule sync_rule[3];
		PropGenRule power_gen_rule;
	} PropRule;
	typedef std::vector<PropRule> PropRuleVec;

	static PropRuleVec prop_rule_vec;

	static void InitClsPropSyncRule(const std::vector<cls_prop_sync_rule>& list);
	static int GetPlayerPropSyncRule(int cls, int prop_index);
	static int GetPlayerPowerGenRule(int cls);
	static int32_t UpdatePlayerProp(int ruler, int32_t curval, int32_t old_maxval, int32_t new_maxval);

	/**
	 * @brief 角色属性同步规则
	 * @brief 规则0: 直接将最新的最大值赋值给当前值
	 * @brief 规则1: 将最大值变化的差值加到当前值,当前值最小值为0;
	 * @brief 规则2: 将最大值变化的差值加到当前值,当前值最小值为1;
	 * @brief 规则3: 当前值不受最大值变化的影响,如果当前值大于最大值,则需更新当前值为最大值;
	 * @brief 规则4: 最大值减小时,同规则3;最大值增大时,则当前值同步增加差值;
	 * @brief 属性规则只适应于玩家的属性0-2,怪物属性规则统一为规则3;
	 */
	static int32_t UpdatePropByRule_1(int32_t curval, int32_t old_maxval, int32_t new_maxval);	
	static int32_t UpdatePropByRule_2(int32_t curval, int32_t old_maxval, int32_t new_maxval);	
	static int32_t UpdatePropByRule_3(int32_t curval, int32_t old_maxval, int32_t new_maxval);	
	static int32_t UpdatePropByRule_4(int32_t curval, int32_t old_maxval, int32_t new_maxval);	
	static int32_t UpdatePropByRule_5(int32_t curval, int32_t old_maxval, int32_t new_maxval);	

	/**
	 * @brief 角色能量恢复规则
	 * @brief 规则1：角色默认恢复规则，不恢复能量
	 * @brief 规则2：攻击时恢复
	 * @brief 规则3：被攻击时恢复
	 * @brief 规则4：回合结束时恢复
	 * @brief 规则5：施放技能时恢复
	 */
	static int32_t GenPowerByRule_1();
	static int32_t GenPowerByRule_2(int32_t dmg, int32_t max_hp, int32_t speed_gen_power);
	static int32_t GenPowerByRule_3(int32_t dmg, int32_t max_hp, int32_t speed_gen_power);
	static int32_t GenPowerByRule_4(int32_t speed_gen_power);
	static int32_t GenPowerByRule_5(int32_t speed_gen_power);
};

}; // namespace combat

#endif // __GAME_MODULE_COMBAT__PROP_POLICY_H__
