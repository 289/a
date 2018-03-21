#ifndef GAMED_GS_PLAYER_PROPERTY_H_
#define GAMED_GS_PLAYER_PROPERTY_H_

#include <stdint.h>
#include <vector>

namespace gamed
{

typedef int32_t ExtendProp;
typedef int64_t ExtendState;

enum
{
	PROP_INDEX_MAX_HP,             // 血量
	PROP_INDEX_MAX_MP,             // 技能消耗
	PROP_INDEX_MAX_EP,             // 猫类视觉能量
	PROP_INDEX_PHYSICAL_ATTACK,    // 物理攻击
	PROP_INDEX_MAGIC_ATTACK,       // 魔法攻击
	PROP_INDEX_PHYSICAL_DEFENCE,   // 物理防御
	PROP_INDEX_MAGIC_DEFENCE,      // 魔法防御
	PROP_INDEX_PHYSICAL_PIERCE,    // 物理穿透
	PROP_INDEX_MAGIC_PIERCE,       // 魔法穿透
	PROP_INDEX_HIT,                // 命中率
	PROP_INDEX_DODGE,              // 闪避
	PROP_INDEX_CRIT_HIT,           // 暴击率
	PROP_INDEX_ATTACK_SPEED,       // 攻击速度
	PROP_INDEX_ATTACK_PRIORITY,    // 先攻
	PROP_INDEX_POWER_GEN_SPEED,    // 某些技能能量的恢复速度
	PROP_INDEX_MOVE_SPEED,         // 移动速度

    PROP_INDEX_HIGHEST, // 检查时使用
	PROP_INDEX_MAX = 64,
};

/*
enum
{
	PROP_MASK_MAX_HP			= 0x0001,
	PROP_MASK_MAX_MP            = 0x0002,
	PROP_MASK_MAX_EP            = 0x0004,
	PROP_MASK_PHYSICAL_ATTACK   = 0x0008,
	PROP_MASK_MAGIC_ATTACK      = 0x0010,
	PROP_MASK_PHYSICAL_DEFENCE	= 0x0020,
	PROP_MASK_MAGIC_DEFENCE     = 0x0040,
	PROP_MASK_PHYSICAL_PIERCE   = 0x0080,
	PROP_MASK_MAGIC_PIERCE      = 0x0100,
	PROP_MASK_HIT               = 0x0200,
	PROP_MASK_DODGE             = 0x0400,
	PROP_MASK_CRIT_HIT          = 0x0800,
	PROP_MASK_ATB_TIME          = 0x1000,
	PROP_MASK_ATTACK_PRIORITY   = 0x2000,
	PROP_MASK_POWER_GEN_SPEED   = 0x4000,
	PROP_MASK_MOVE_SPEED        = 0x8000,
	PROP_MASK_ALL               = 0xFFFF,
};
*/


class Player;

/**
 * @class PropertyPolicy
 * @brief 玩家属性策略
 * @brief 包括玩家基础属性和扩展属性
 */
class PropertyPolicy
{
public:
	//计算玩家当前属性
	static void UpdatePlayerProp(Player* player);

	//计算玩家战斗中的属性
	static void CalPlayerCombatProp(Player* player, std::vector<int32_t>& props);

	//计算玩家HP恢复值
	static int32_t GetHPGen(Player* player);

private:
	static void UpdateExtendProp(Player* player);
	static void UpdateBasicProp(Player* player, const ExtendProp* old_props);
};

/**
 * @class PropRuler
 * @brief 玩家基础属性更新规则
 * @brief 规则0: 直接将最新的最大值赋值给当前值
 * @brief 规则1: 将最大值变化的差值加到当前值,当前值最小值为0;
 * @brief 规则2: 将最大值变化的差值加到当前值,当前值最小值为1;
 * @brief 规则3: 当前值不受最大值变化的影响,如果当前值大于最大值,则需更新当前值为最大值;
 * @brief 规则4: 最大值减小时,同规则3;最大值增大时,则当前值同步增加差值;
 */
class PropRuler
{
public:
	static int32_t UpdateProperty(char ruler, int32_t curval, int32_t old_maxval, int32_t new_maxval);

private:
	static int32_t UpdatePropByRule_0(int32_t curval, int32_t old_maxval, int32_t new_maxval);
	static int32_t UpdatePropByRule_1(int32_t curval, int32_t old_maxval, int32_t new_maxval);
	static int32_t UpdatePropByRule_2(int32_t curval, int32_t old_maxval, int32_t new_maxval);
	static int32_t UpdatePropByRule_3(int32_t curval, int32_t old_maxval, int32_t new_maxval);
	static int32_t UpdatePropByRule_4(int32_t curval, int32_t old_maxval, int32_t new_maxval);
};

}; // namespace gamed

#endif // GAMED_GS_PLAYER_PROPERTY_H_
