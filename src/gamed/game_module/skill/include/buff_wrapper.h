#ifndef SKILL_BUFF_WRAPPER_H_
#define SKILL_BUFF_WRAPPER_H_

#include "buff.h"

namespace skill
{

class InnerMsg;
class SkillWrapper;

class BuffWrapper
{
	friend class SkillWrapper;
public:
	BuffWrapper(Player* player);
	inline BuffVec& GetBuff();

	// @brief 玩家进入战场
	void CombatStart(SkillDamageVec& skilldmg_vec);

	// @brief 战斗结束
	void CombatEnd();

	// @brief 回合开始
	// @param buffdmg_vec 产生的Dot伤害
	void RoundStart(BuffDamageVec& buffdmg_vec);

	// @brief 回合结束
	// @param buffdmg_vec 产生的Dot伤害
	void RoundEnd(SkillDamageVec& skilldmg_vec, BuffDamageVec& buffdmg_vec);

	// @brief 释放技能
	// @param skill_dmg 
	//		  in: SkillWrapper.CastSkill的结果
	//		  out: 根据Buff修正后的技能伤害（比如护盾等）
	// @param buff_vec 技能攻击过程中产生的Dot伤害
	// @return 0 不追击，本回合结束；非0 追击技能ID
	int32_t CastSkill(SkillDamage& skill_dmg, BuffDamageVec& buffdmg_vec);

	// @brief 玩家濒死
	// @param buffdmg_vec 产生的Dot伤害
	void Dying(SkillDamageVec& skilldmg_ve, BuffDamageVec& buffdmg_vec);

	// @brief 玩家死亡
	// @param buffdmg_vec 产生的Dot伤害
	void Dead(BuffDamageVec& buffdmg_vec);

	// @brief 玩家复活
	void Revive(SkillDamageVec& skilldmg_vec);

	// 技能系统内部使用
	bool InLifeChain(InnerMsg& msg) const;
	void GetTarget(InnerMsg& msg);
	void ConsumeRevise(InnerMsg& msg);
	void CastRevise(InnerMsg& msg);
	void AttackedRevise(InnerMsg& msg);
	void AttachBuff(InnerMsg& msg);
	int8_t BeAttacked(InnerMsg& msg);
    int32_t GetChargedSkill() const;
    int32_t GetGolemId() const;

	// 调试使用
	void Show();
private:
	void ClearBuff();
	void ClearBuff(const EffectTempl* effect, Player* caster);
private:
	Player* player_;
	BuffVec buff_vec_;
};

inline BuffVec& BuffWrapper::GetBuff()
{
	return buff_vec_;
}

} // namespace skill

#endif // SKILL_BUFF_WRAPPER_H_
