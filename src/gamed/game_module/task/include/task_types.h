#ifndef TASK_TASK_TYPE_H_
#define TASK_TASK_TYPE_H_

#include <stdio.h>
#include <stdint.h>
#include <set>
#include <vector>
#include <map>
#include <string>
#include "shared/net/packet/packet_util.h"
#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/base_packet.h"

namespace gamed
{
class TaskInterface;
} // namespace gamed

namespace task
{

enum TaskError
{
	// 0
	ERR_TASK_SUCC,
	ERR_TASK_ID,
	ERR_TASK_WRONG_SUB,
	ERR_TASK_NOT_ROOT,
	ERR_TASK_SAME,
	// 5
	ERR_TASK_CANT_REDO,
	ERR_TASK_SCRIPT,
	ERR_TASK_LEVEL,
	ERR_TASK_GOLD,
	ERR_TASK_CASH,
	// 10
	ERR_TASK_SCORE,
	ERR_TASK_ITEM,
	ERR_TASK_GENDER,
	ERR_TASK_ROLECLASS,
	ERR_TASK_ZONE,
	// 15
	ERR_TASK_STATE,
	ERR_TASK_MONSTER,
	ERR_TASK_TIME,
	ERR_TASK_TASK,
	ERR_TASK_INSTANCE,
	// 20
	ERR_TASK_COUNTER,
	ERR_TASK_GUIDE,
	ERR_TASK_STORAGE,
    ERR_TASK_REPUTATION,
    ERR_TASK_BUFF,
    // 25
    ERR_TASK_COMBAT_VALUE,
    ERR_TASK_PET_POWER,
    ERR_TASK_PET,
    ERR_TASK_CARD,
    ERR_TASK_ENHANCE,
    // 30
    ERR_TASK_EQUIP,
    ERR_TASK_STAR,
    ERR_TASK_CAT_VISION,
    ERR_TASK_TALENT,
};

enum TaskFlag
{
	FLAG_GIVEUP = 1,
	FLAG_SEEK = 1 << 1,
	FLAG_DELIVER_TIPS = 1 << 2,
	FLAG_COMPLETE_TIPS = 1 << 3,
	FLAG_HIDE = 1 << 4,
	FLAG_AUTO = 1 << 5,
	FLAG_REDO_SUCC = 1 << 6,
	FLAG_REDO_FAIL = 1 << 7,
	FLAG_RECORD = 1 << 8,
	FLAG_SHARE = 1 << 9,
	FLAG_FAIL_PARENT_FAIL = 1 << 10,
	FLAG_SUCC_PARENT_SUCC = 1 << 11,
	FLAG_UI = 1 << 12,
	FLAG_CAT_VISION = 1 << 13,
	FLAG_CROSS_TRANS = 1 << 14,
	FLAG_CLOSE_NPC_AREA = 1 << 15,
	FLAG_PRIOR_TRACK = 1 << 16,
	FLAG_MAG_TASK = 1 << 17,
};

// 完成方式
enum FinishMode
{
	FINISH_DIRECT,
	FINISH_NPC,
};

enum SubTaskDelMode
{
	DELIVER_MANUAL,
	DELIVER_RANDOM,
	DELIVER_ORDER,
	DELIVER_ALL,
};

enum ActionType
{
	ACTION_NONE,
	ACTION_TALK_NPC,
	ACTION_KILL_MONSTER,
	ACTION_GATHER_MINE,
	ACTION_PATROL,
    ACTION_UI,
};

enum PathType
{
	PATH_DELIVER,
	PATH_COMPLETE,
	PATH_AWARD,
};

enum ApearType
{
	APPEAR_NORMAL,
	APPEAR_FALLOFF,
    APPEAR_FADEIN,
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

enum NameType
{
	NAME_FIRST,
	NAME_MID,
	NAME_LAST,
};

enum ScaleType
{
	SCALE_NORMAL,
	SCALE_SMALL,
	SCALE_BIG,
    SCALE_EXE = 0x80, // 是否执行变身操作
};

enum TagType
{
	TAG_NONE1,
	TAG_MAIN,		// 主线任务
	TAG_SUB,		// 支线任务
	TAG_FESTIVAL,	// 活动任务
	TAG_SCORE,	    // 学分任务
	TAG_INSTANCE,	// 副本任务
	TAG_REPUTATION,	// 声望任务
	TAG_EXP,	    // 经验任务
	TAG_MONEY,	    // 金币任务
	TAG_NONE2,
	TAG_GATHER,		// 收集任务
	TAG_MONSTER,	// 杀怪任务
	TAG_ERRAND,		// 对话任务
	TAG_CHALLENGE,	// 挑战任务
	TAG_LEVEL,		// 升级任务
	TAG_UNION,		// 多人任务
	TAG_VISIT,		// 查探任务
	TAG_GOAL,		// 目标任务
};

enum QualityType
{
	QUALITY_WHITE = 1,
	QUALITY_GREEN,
	QUALITY_BLUE,
	QUALITY_PURPLE,
	QUALITY_ORANGE,
	QUALITY_RED,
	QUALITY_GOLD,
	QUALITY_MAIN,
};

enum TalkType
{
	TALK_NONE = -1,
	TALK_DELIVER,
	TALK_UNQUALIFY,
	TALK_UNFINISH,
	TALK_FINISH,
};

enum DlgType
{
	DLG_NONE = -3,
	DLG_FIRSTNAME,
	DLG_CLS,
	DLG_MAX = DLG_CLS,
};

enum DlgPos
{
    DLG_POS_RIGHT,
    DLG_POS_BOTTOM,
};

enum StatusType
{
	STATUS_ERR,
	STATUS_NEW,
	STATUS_NEW_TIPS,
	STATUS_FINISH,
	STATUS_COMPLETE,
	STATUS_COMPLETE_TIPS,
	STATUS_MODIFY,
    STATUS_STORAGE,
};

enum ChatChannelType
{
	CHAT_CHANNEL_WORLD,
	CHAT_CHANNEL_TEAM,
	CHAT_CHANNEL_REGION,
};

enum DirType
{
	DIR_RIGHT,
	DIR_RIGHT_UP,
	DIR_UP,
	DIR_LEFT_UP,
	DIR_LEFT,
	DIR_LEFT_DOWN,
	DIR_DOWN,
	DIR_RIGHT_DOWN,
};

enum ReputationOp
{
    REPUTATION_OP_EQUAL_GREATER = -2,
    REPUTATION_OP_LESS,
    REPUTATION_OP_NONE,
	REPUTATION_OP_OPEN,
	REPUTATION_OP_ADD,
	REPUTATION_OP_DEC,
};

enum CounterType
{
    COUNTER_GLOBAL,
    COUNTER_MAP,
    COUNTER_PLAYER,
};

enum CounterOp
{
    // 比较操作
	COUNTER_OP_GREATER,
	COUNTER_OP_GREATER_EQUAL,
	COUNTER_OP_LESS,
	COUNTER_OP_LESS_EQUAL,
	COUNTER_OP_EQUAL,
    // 修改操作
    COUNTER_OP_INC,
    COUNTER_OP_DEC,
    COUNTER_OP_ASSIGN,
    COUNTER_OP_LOAD,
    COUNTER_OP_UNLOAD,
};

enum UIOp
{
    UI_OP_SHOW,
    UI_OP_HIDE,
};

enum BuffOp
{
    BUFF_OP_ADD,
    BUFF_OP_DEL,
};

enum FriendOp
{
    FRIEND_OP_ADD,
    FRIEND_OP_DEL,
    FRIEND_OP_ONLINE,
    FRIEND_OP_OFFLINE,
};

enum CameraMaskOp
{
	CAMERA_MASK_NONE,
	CAMERA_MASK_OPEN,
	CAMERA_MASK_CLOSE,
};

enum ValueOp
{
    VALUE_OP_LESS,
    VALUE_OP_LESS_EQUAL,
    VALUE_OP_EQUAL,
    VALUE_OP_GREATER_EQUAL,
    VALUE_OP_GREATER,
};

class TaskTempl;
class StorageTempl;
class TaskEntry;
typedef int32_t TaskID;
typedef int32_t StorageID;
typedef int32_t PackageID;
typedef int32_t MonsterID;
typedef int32_t BuffID;
typedef std::vector<TaskID> TaskIDVec;
typedef std::vector<const TaskTempl*> TaskVec;
typedef std::map<TaskID, const TaskTempl*> TaskMap;
typedef std::vector<const TaskEntry*> TaskEntryVec;
typedef std::map<int32_t, int32_t> ItemMap;
typedef std::vector<BuffID> BuffVec;

typedef gamed::TaskInterface Player;

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


extern bool __TASK_PRINT_FLAG;
inline void __SETPRTFLAG(bool flag)
{
	__TASK_PRINT_FLAG  = flag;
}

#ifdef __NO_STD_OUTPUT__
inline int __PRINTF(const char * fmt, ...)
{
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
#ifndef CLIENT_SIDE
	if (!__TASK_PRINT_FLAG)
		return 0;
#endif

	va_list ap;
	va_start(ap, fmt);
	int rst = vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
	return rst;
}
#endif

} // namespace task

#endif // TASK_TASK_TYPE_H_
