#ifndef GAMED_OBJ_INTERFACE_H_
#define GAMED_OBJ_INTERFACE_H_

#include <stdint.h>
#include <vector>

namespace skill
{
class SkillWrapper;
class BuffWrapper;
class CooldownWrapper;
} // namespace skill

namespace gamed
{

///
/// 战斗对象封装类,提供给技能、任务等系统使用
///
class ObjInterface
{
public:
	ObjInterface()
	{
	}
	virtual ~ObjInterface()
	{
	}

	// 基本信息相关接口
	virtual int32_t GetId() const = 0;
	virtual int32_t GetRoleClass() const = 0; // 返回职业模板ID
	virtual int8_t  GetLevel() const = 0;
	virtual int64_t GetCurTime() const = 0;
	virtual bool IsInCombat() const = 0;
    virtual ObjInterface* GetOwner() = 0;

	//玩家状态
	virtual bool IsDead() const = 0;
	virtual bool IsDying() const = 0;
	virtual bool IsDizzy() const = 0;
	virtual bool IsSilent() const = 0;
	virtual bool IsStone() const = 0;
	virtual bool IsCharging() const = 0;
	virtual bool IsCharged() const = 0;
	virtual void SetDizzy(bool dizzy=false) = 0;
	virtual void SetSilent(bool silent=false) = 0;
	virtual void SetSeal(bool seal=false) = 0;
	virtual void SetStone(bool stone=false) = 0;
	virtual void SetCharging(bool charging=false) = 0;
	virtual void SetCharged(bool charged=false) = 0;

	// TODO:暂时没有相应的效果逻辑
	virtual bool IsSleep() const = 0;
	virtual bool IsConfuse() const = 0;
	virtual void SetSleep(bool sleep=false) = 0;
	virtual void SetConfuse(bool confuse=false) = 0;

	//属性相关接口
	virtual void DoDamage(int32_t hp, int32_t attacker) = 0; //掉血
	virtual void DoHeal(int32_t hp, int32_t healer=0) = 0;   //治疗
	virtual int32_t GetProperty(int32_t index) const = 0;
	virtual void IncPropPoint(int32_t index, int32_t value) = 0;
	virtual void DecPropPoint(int32_t index, int32_t value) = 0;
	virtual void IncPropScale(int32_t index, int32_t value) = 0;
	virtual void DecPropScale(int32_t index, int32_t value) = 0;
	virtual void SetGenPowerFlag() = 0; // 恢复能量值
	virtual void Revive(int32_t scale) = 0;

	//物品相关接口
	virtual int32_t GetItemCount(int32_t item_id) const = 0;
	virtual void TakeOutItem(int32_t item_id, int32_t count) = 0;

	//战场相关
	virtual int8_t GetPos() const = 0;		// 返回自己在队伍中的站位
	virtual bool CanAction() const = 0;		// 是否能行动
	virtual bool CanAttacked() const = 0;	// 是否能被攻击
	virtual void GetTeamMate(std::vector<ObjInterface*>& players) const = 0;
	virtual void GetEnemy(std::vector<ObjInterface*>& players) const = 0;
	virtual void GetGolem(std::vector<ObjInterface*>& players) const = 0;
	virtual void IncSkillLevel(int32_t sk_tree_id, int32_t lvl) = 0; //提升技能临时等级
	virtual void DecSkillLevel(int32_t sk_tree_id, int32_t lvl) = 0; //降低技能临时等级

	//BUFF相关
	virtual void AttachBuffer(uint32_t buff_seq/*BUFF序列号*/, int32_t buff_id, int32_t attacher) = 0;
	virtual void DetachBuffer(uint32_t buff_seq/*BUFF序列号*/, int32_t buff_id, int32_t attacher) = 0;

	//技能相关接口
	virtual skill::SkillWrapper* GetSkill() const = 0;
	virtual skill::BuffWrapper* GetBuff() const = 0;
	virtual skill::CooldownWrapper* GetCooldown() const = 0;

	//魔偶相关(限玩家)
	virtual bool PlayerSummonGolem(int32_t golem_id) = 0; // 玩家获得魔偶
	virtual bool PlayerPowerGolem(int32_t power) = 0;   // 玩家给魔偶加能量
	virtual int32_t GetGolemProp(int32_t golem_id, int8_t index) = 0;//获取魔偶的属性
    virtual int32_t GetGolemDeActivePower(int32_t golem_id) = 0;//获取魔偶被收回时的能量值

	//test
	virtual void Trace() const {}
};

}; // namespace gamed

#endif // GAMED_OBJ_INTERFACE_H_
