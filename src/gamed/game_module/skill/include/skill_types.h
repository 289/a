#ifndef SKILL_SKILL_TYPES_H_
#define SKILL_SKILL_TYPES_H_

#include <stdio.h>
#include <stdint.h>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "shared/net/packet/base_packet.h"
#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bytebuffer.h"

namespace gamed
{
class ObjInterface;
} // namespace gamed

namespace skill
{

enum SkillError
{
	ERR_SUCCESS,
	ERR_COOLDOWN,
	ERR_ITEM,
	ERR_PROP,
    ERR_GOLEM,
    ERR_STATUS,
};

enum PropIndex
{
	PROP_INDEX_INVALID = -1,
    // 0
	PROP_INDEX_MAX_HP,						// 血量
	PROP_INDEX_MAX_CON1,					// 技能消耗1
	PROP_INDEX_MAX_CON2,					// 技能消耗2
	PROP_INDEX_MAX_PHYSICAL_ATTACK,			// 物理攻击
	PROP_INDEX_MAX_MAGIC_ATTACK,			// 第三类防御
    // 5
	PROP_INDEX_MAX_PHYSICAL_DEFENCE,		// 物理防御
	PROP_INDEX_MAX_MAGIC_DEFENCE,			// 魔法防御
	PROP_INDEX_MAX_PHYSICAL_PIERCE,			// 物理穿透
	PROP_INDEX_MAX_MAGIC_PIERCE,			// 魔法穿透
	PROP_INDEX_MAX_HIT,						// 命中率
    // 10
	PROP_INDEX_MAX_DODGE,					// 闪避
	PROP_INDEX_MAX_CRIT_HIT,				// 暴击率
	PROP_INDEX_MAX_ATTACK_SPEED,			// 攻击速度
	PROP_INDEX_MAX_ATTACK_PRIORITY,			// 先攻
	PROP_INDEX_MAX_MANA_RESTORE,			// 能量恢复速度
    // 15
	PROP_INDEX_MAX_MOVE_SPEED,              // 大世界移动速度
    PROP_INDEX_HIGHEST,
    PROP_INDEX_MAX = 64,
	// 只有如下几个属性值有当前值，其余属性的当前值等于最大值
	PROP_INDEX_HP = PROP_INDEX_MAX,
	PROP_INDEX_CON1,
	PROP_INDEX_CON2,
};

enum RoleClass
{
	CLS_NONE = -1,
	// 新手
	CLS_NEWBIE = 0,        // 初心者（新手）
	// 战士
	CLS_PRIMARY_WARRIOR,   // 初级战士
	CLS_J_ATTACK_WARRIOR,  // 中阶攻击战士
	CLS_S_ATTACK_WARRIOR,  // 高阶攻击战士
	CLS_J_DEFENSE_WARRIOR, // 中阶防御战士
	CLS_S_DEFENSE_WARRIOR, // 高阶防御战士
	// 法师
	CLS_PRIMARY_MAGE,      // 初级法师
	CLS_J_DAMAGE_MAGE,     // 中阶输出型法师
	CLS_S_DAMAGE_MAGE,     // 高阶输出型法师
	CLS_J_HEAL_MAGE,       // 中阶治疗型法师
	CLS_S_HEAL_MAGE,       // 高阶治疗型法师
	// 射手
	CLS_PRIMARY_ARCHER,    // 初级射手
	CLS_J_SNIPE_ARCHER,    // 中阶狙击型射手（单攻） 
	CLS_S_SNIPE_ARCHER,    // 高阶狙击型射手
	CLS_J_RAKE_ARCHER,     // 中阶扫射型射手（群攻，暂定）
	CLS_S_RAKE_ARCHER,     // 高阶扫射型射手

	// end
	CLS_MAXCLS_LABEL
};

enum SkillType
{
	SKILL_ACTIVE,
	SKILL_PASSIVE,
};

enum Magician
{
	MAGICIAN_PLAYER,
	MAGICIAN_PET,
	MAGICIAN_MONSTER,
	MAGICIAN_EQUIP,
	MAGICIAN_SCENE,
};

enum TargetTeam
{
	TEAM_MATE,
	TEAM_ENEMY,
};

enum Scope
{
	SCOPE_SCENE = 0x01,
	SCOPE_COMBAT = 0x02,
	SCOPE_GLOBAL = 0x03,
};

enum ActionType
{
	ACTION_MELEE,
	ACTION_RANGE,
};

enum CastType
{
    CAST_ACTION,
    CAST_GFX,
};

enum HitType
{
	HIT_REVISE,
	HIT_ASSIGN,
};

enum ReCastType
{
	RECAST_NONE = 0x00,
	RECAST_KILL = 0x01,
	RECAST_HIT = 0x02,
	RECAST_DODGE = 0x04,
	RECAST_CRIT = 0x08,
};

enum TargetType
{
    // 0
	TARGET_SELF,
	TARGET_APPOINT,
	TARGET_RANDOM,
	TARGET_SEQUENCE,
	TARGET_HP,
    // 5
	TARGET_LINE,
	TARGET_CLASS,
	TARGET_POSITION,
	TARGET_DEAD,
	TARGET_TYPE_MAX,
};

// PlayerPos&LineParam表示玩家可能的施法位置
enum PlayerPos
{
	POS_INVALID = -1,
	POS_1,
	POS_2,
	POS_3,
	POS_4,
	MAX_PLAYER_NUM,
	POS_CENTER = 4,
};

enum HPParam
{
	HP_VALUE_MIN,
	HP_VALUE_MAX,
	HP_SCALE_MIN,
	HP_SCALE_MAX,
};

enum LineParam
{
	LINE_FRONT = 5,			// 贯穿前排
	LINE_TWO_FRONT,			// 贯穿两线前排
	LINE_VMID,				// 贯穿纵向中线
	LINE_TWO_BACK,			// 贯穿两线后排
	LINE_BACK,				// 贯穿后排
	LINE_UP,				// 贯穿上排
	LINE_TWO_UP,			// 贯穿两线上排
	LINE_HMID,				// 贯穿横向中线
	LINE_TWO_DOWN,			// 贯穿两线下排
	LINE_DOWN,				// 贯穿下排
	LINE_RAND_VERTICAL,		// 随机纵向贯穿
	LINE_RAND_HORIZONTAL,	// 随机横向贯穿
	LINE_RAND_VTWO,			// 随机纵向两线贯穿
	LINE_RAND_HTWO,			// 随机横向两项贯穿
};

enum PosParam
{
	FRONT_ROW,	// 前排：1，2，3号位
	BACK_ROW,	// 后排：2，3，4号位
};

enum RangeType
{
	RANGE_SINGLE,
	RANGE_RANGE,
	RANGE_ALL,
	RANGE_CHAIN,
	RANGE_LINE,
	RANGE_BULLET_LINE,
};

enum TriggerMode
{
    TRIGGER_NONE,
	TRIGGER_ACTION,
	TRIGGER_GFX,
};

enum CondType
{
	COND_PROP,
	COND_ITEM,
};

enum LogicOp
{
	OP_LESS,
	OP_EQUAL,
	OP_GREATER,
	OP_GREATER_EQUAL,
};

enum EffectType
{
	EFFECT_INSTANT,
	EFFECT_BUFF,
};

enum RedirectType
{
    // 0
	REDIR_NO,
	REDIR_CASTER,
	REDIR_RANDOM_MATE,
	REDIR_ALL_MATE,
	REDIR_RANDOM_ENEMY,
    // 5
	REDIR_ALL_ENEMY,
	REDIR_TRIGGER,
};

enum BuffType
{
	BUFF_NORMAL,
	BUFF_POINT,
};

enum UpdateTime
{
	UPDATE_ROUNDSTART,
	UPDATE_BEATTACKED,
	UPDATE_ROUNDEND,
};

enum LogicType
{
    // 0
	LOGIC_BASIC_ATTACK,
	LOGIC_CHANGE_PROP,
	LOGIC_CONTROL,
	LOGIC_TAUNT,
	LOGIC_NORMAL_SHIELD,
    // 5
	LOGIC_REBOUND_SHIELD,
	LOGIC_SUMMON_GOLEM,
	LOGIC_POWER_FLAG,
	LOGIC_POWER_GOLEM,
	LOGIC_RESET_CD,
    // 10
	LOGIC_PURIFY,
	LOGIC_LIFE_CHAIN,
	LOGIC_REVIVE,
	LOGIC_REVISE,
	LOGIC_TRIGGER_EFFECT,
    // 15
    LOGIC_CHARGE,
    LOGIC_INVINCIBLE,
    LOGIC_TRANSFORM,
	LOGIC_MAX,
};

enum AttackType
{
	ATTACK_PHYSICAL = 0x01,
	ATTACK_MAGIC    = 0x02,
    ATTACK_THIRD    = 0x04,
	ATTACK_VALUE    = 0x07,
};

enum ShieldType
{
	SHIELD_PHYSICAL = 0x01,
	SHIELD_MAGIC    = 0x02,
	SHIELD_THIRD    = 0x04,
	SHIELD_VALUE    = 0x07,
};

enum ChangeType
{
	CHANGE_POINT = 0x08,
	CHANGE_SCALE = 0x0a,
};

enum ControlType
{
	CONTROL_DIZZY,
	CONTROL_SILENCE,
	CONTROL_SEAL,
	CONTROL_STONE,
};

enum TriggerType
{
	TRIGGER_ALL,
	TRIGGER_RANDOM,
};

enum DamageType
{
	DAMAGE_POINT,
	DAMAGE_SCALE,
	DAMAGE_SHIELD = 0x10,
};

enum ReviseType
{
	REVISE_ATTACK,
	REVISE_HEAL,
	REVISE_CRIT,
	REVISE_CONSUME,
	REVISE_DAMAGE,
};

enum EffectStatus
{
	EFFECT_NORMAL = 0x00,
	EFFECT_CRIT = 0x01,
	EFFECT_REDIR = 0x02,
};

enum BulletType
{
    BULLET_GFX = 0x00,
    BULLET_ACTION = 0x01,
    BULLET_TRIGGER_ACTION = 0x02,
};

enum CameraMoveType
{
    CAMERA_MOVE_CAST,
    CAMERA_MOVE_ACTION,
    CAMERA_MOVE_GFX,
    CAMERA_MOVE_BULLET,
    CAMERA_MOVE_TARGET,
    CAMERA_MOVE_ATTACKED,
};

enum SkillFlag
{
    SKILL_FLAG_PORTRAIT = 1, // 使用攻击立绘
};

enum SkillTargetType
{
    SKILL_TARGET_MATE_SINGLE,
    SKILL_TARGET_MATE_MULTI,
    SKILL_TARGET_ENEMY_SINGLE,
    SKILL_TARGET_ENEMY_MULTI,
};

typedef int32_t EffectID;
typedef int32_t SkillID;
typedef int32_t CooldownGID;
typedef int32_t MutexGID;
typedef int32_t SkillGID;
typedef int32_t EffectGID;

typedef gamed::ObjInterface Player;
typedef std::vector<int32_t> ParamVec;
typedef std::vector<Player*> PlayerVec;
typedef std::vector<EffectID> EffectVec;
typedef std::set<EffectID> EffectIDSet;
typedef std::set<SkillGID> SkillGroupSet;
typedef std::set<EffectGID> EffectGroupSet;
typedef std::map<RoleID, int32_t> PauseTime;
class EffectTempl;
typedef std::vector<const EffectTempl*> EffectTemplVec;

#define CHECK_INRANGE(value, min, max) \
	if (value < min || value > max) \
	{ \
		return false; \
	}

#define CHECK_VALIDITY(value) \
	if (!value.CheckDataValidity()) \
	{ \
		return false; \
	}

#define CHECK_VEC_VALIDITY(value) \
	for (size_t i = 0; i < value.size(); ++i) \
	{ \
		if (!value[i].CheckDataValidity()) \
		{ \
			return false; \
		} \
	}


extern bool __SKILL_PRINT_FLAG;

inline void __SETPRTFLAG(bool flag)
{
	__SKILL_PRINT_FLAG  = flag;
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
	if (!__SKILL_PRINT_FLAG)
		return 0;

	va_list ap;
	va_start(ap, fmt);
	int rst = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
	return rst;
}
#endif

} // namespace skill

#endif // SKILL_SKILL_TYPES_H_
