#ifndef SKILL_SKILL_INFO_H_
#define SKILL_SKILL_INFO_H_

#include "damage_msg.h"

namespace skill
{

// 初始化技能系统（技能数据以及动作数据），在使用技能之前调用
bool InitSkillData(const char* file);
bool InitActionData(const char* file);

// 战斗相关接口
int32_t GetATBStopTime(SkillID id);
void GetPauseTime(const std::string& model, const SkillDamageVec& dmg, PauseTime& pause);

} // namespace skill

#endif // SKILL_SKILL_INFO_H_
