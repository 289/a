#ifndef SKILL_DATATEMPL_SKILL_TEMPL_H_
#define SKILL_DATATEMPL_SKILL_TEMPL_H_

#include "base_skill_templ.h"
#include "hit_rate.h"
#include "cooldown_info.h"
#include "recast.h"
#include "skill_target.h"
#include "skill_range.h"
#include "camera_move.h"
#include "condition.h"
#include "attack_frame.h"

namespace skill
{

class SkillTempl : public BaseTempl
{
	DECLARE_SKILLSYS_TEMPL(SkillTempl, TEMPL_TYPE_SKILL);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;

    int32_t summon_golemid() const;
    inline bool use_portrait() const;
    inline int32_t cooldown_time() const;
    inline int8_t skill_target_type() const;
public:
	// 基本信息
	int8_t type;                    // 技能类型
	int8_t magician;                // 施法者类型
	int8_t team;                    // 目标阵营
	int8_t scope;                   // 技能生效范围
	int8_t action;                  // 技能动作类型
    int8_t flag;                    // 技能设置

	HitRate hit;					// 技能命中
	CooldownInfo cooldown;			// 技能冷却
	ReCast recast;					// 技能追击 

	SkillTarget target;				// 技能动作目标
	SkillRange range;				// 效果目标

    CameraMoveVec camera;           // 镜头移动

	CondVec premise;				// 释放条件
	CondVec consume;				// 技能消耗

	int8_t trigger_mode;			// 多段打击触发方式
	AttackFrameVec attack_frame;	// 攻击帧

	SkillGroupSet skill_group;      // 技能分组

    int8_t cast_type;               // 原地动作/原地光效
	std::string cast_path;          // 原地动作/光效路径

	int32_t move_frame;             // 攻击位移帧数
	std::string attack_path;        // 攻击动作
	std::string attack_gfx_path;    // 攻击光效

	int32_t atb_stop_time;          // ATB暂停时间
};

inline bool SkillTempl::use_portrait() const
{
    return flag & SKILL_FLAG_PORTRAIT;
}

inline int32_t SkillTempl::cooldown_time() const
{
    return cooldown.time;
}

inline int8_t SkillTempl::skill_target_type() const
{
    if (team == TEAM_MATE)
    {
        return range.type == RANGE_SINGLE ? SKILL_TARGET_MATE_SINGLE : SKILL_TARGET_MATE_MULTI;
    }
    else
    {
        return range.type == RANGE_SINGLE ? SKILL_TARGET_ENEMY_SINGLE : SKILL_TARGET_ENEMY_MULTI;
    }
}

} // namespace skill

#endif // SKILL_DATATEMPL_SKILL_TEMPL_H_
