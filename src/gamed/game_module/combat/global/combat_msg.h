#ifndef __GAME_MODULE_COMBAT_MESSAGE_H__
#define __GAME_MODULE_COMBAT_MESSAGE_H__

#include <stdint.h>
#include "combat_types.h"


namespace combat
{

struct MSG
{
	int message;
	XID target;
	XID source;
	int64_t param;
	int content_len;
	const void* content;
};

void DupeMessage(MSG& dest_msg, const MSG& msg);
void FreeMessage(MSG& msg);
void BuildMessage(MSG& msg, int message, const XID& target, const XID& source, int64_t param, const void* buf, int len);


enum
{
    // 0
	COMBAT_MSG_INVALID,

	// 1
	COMBAT_MSG_BOSS_DAMAGED,                 // BOSS受击掉血, 通知世界BOSS
	COMBAT_MSG_BOSS_HEALED,                  // BOSS治疗加血, 通知世界BOSS
	COMBAT_MSG_BOSS_COMBAT_END,              // BOSS战斗结束, 通知世界BOSS
	COMBAT_MSG_SOMEONE_DEAD,                 // 通知战场有战斗对象死亡

	// 5
	COMBAT_MSG_COMBAT_END,                   // 通知战场战斗结束
    COMBAT_MSG_COMBAT_CONTINUE,              // 接波战斗继续战斗
    COMBAT_MSG_RESUME_SCENE_SCRIPT,          // 恢复场景脚本运行
    COMBAT_MSG_TRIGGER_TRANSFORM,            // 触发怪物变身
    COMBAT_MSG_START_TRANSFORM,              // 怪物开始变身

    // 10
    COMBAT_MSG_TRIGGER_ESCAPE,              // 触发怪物逃跑
    COMBAT_MSG_START_ESCAPE,                // 怪物开始逃跑
};

struct combat_msg_boss_damaged
{
	CombatID combat_id;
	int32_t  boss_pos;
	UnitID   attacker;
	int32_t  damage;
};

struct combat_msg_boss_healed
{
	CombatID combat_id;
	int32_t  boss_pos;
	int32_t  life;
};

} // namespace combat

#endif // __GAME_MODULE_COMBAT_MESSAGE_H__
