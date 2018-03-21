#ifndef SKILL_SKILL_WRAPPER_H_
#define SKILL_SKILL_WRAPPER_H_

#include "damage_msg.h"

namespace skill
{

class BuffWrapper;
class SkillTempl;
typedef std::map<SkillID, const SkillTempl*> SkillMap;

class SkillWrapper
{
	friend class BuffWrapper;
public:
	SkillWrapper(Player* player);

	// @brief 设置普攻，某些特殊状态只能释放普攻
	inline void set_default_skillid(SkillID id);
	inline SkillID default_skillid() const;

	// @brief 添加已学会的技能
	void AddSkill(SkillID id);

	// @brief 获取已领悟的技能模板
	const SkillTempl* GetSkill(SkillID id) const;

	// @brief 能否释放技能
	// 能否释放由玩家的状态（血，蓝，冷却，物品等）决定
	// @return SkillError
	int32_t CanCast(SkillID id) const;

	// @brief 获取技能目标
	// @param id in：希望释放的技能ID out：实际释放的技能ID（由玩家状态决定）
	// @param target out:技能攻击玩家列表
	// @return 技能施法的位置（PlayerPos&LineParam）
	int8_t GetTarget(SkillID& id, PlayerVec& target) const;

	// @brief 释放技能，计算技能伤害
	// @param id 技能ID，不会再检查能否释放，该ID应该使用GetTarget的返回值
	// @param target 技能的攻击目标，应使用GetTarget的返回值
	// @param dmg out：技能伤害数据
	// @param recast 是不是追击技能，追击技能不会导致消耗和冷却 
	void CastSkill(SkillID id, const PlayerVec& target, SkillDamage& dmg, bool recast);

	// @brief 扣除技能消耗
    void Consume(SkillID id);
private:
	void CastSkill(const SkillTempl* skill, const PlayerVec& target, SkillDamage& dmg);
	void CastPassiveSkill(SkillDamageVec& skilldmg_vec);
	void GetPassiveBuff(EffectTemplVec& buff_vec) const;
private:
	SkillID default_skill_;
	Player* player_;
	SkillMap active_skill_;
	SkillMap passive_skill_;
};

inline void SkillWrapper::set_default_skillid(SkillID id)
{
	default_skill_ = id;
}

inline SkillID SkillWrapper::default_skillid() const
{
	return default_skill_;
}

} // namespace skill

#endif // SKILL_SKILL_WRAPPER_H_
