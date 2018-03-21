#ifndef GAMED_GS_PLAYER_SKILL_IF_H_
#define GAMED_GS_PLAYER_SKILL_IF_H_

#include "game_module/obj_if/obj_interface.h"


namespace gamed {

class Player;

class PlayerSkillIf : public ObjInterface
{
public:
	PlayerSkillIf(Player* player);
	virtual ~PlayerSkillIf();

    // 基本信息相关接口
	virtual int32_t GetId() const        { return 0; }
	virtual int32_t GetRoleClass() const { return 0; } // 返回职业模板ID
	virtual int8_t  GetLevel() const     { return 0; }
	virtual int64_t GetCurTime() const   { return 0; }
	virtual bool IsInCombat() const      { return false; }
    virtual ObjInterface* GetOwner()     { return this; }

	//玩家状态
	virtual bool IsDead() const          { return false; }
	virtual bool IsDying() const         { return false; }
	virtual bool IsDizzy() const         { return false; }
	virtual bool IsSilent() const        { return false; }
	virtual bool IsStone() const         { return false; }
	virtual bool IsCharging() const      { return false; }
	virtual bool IsCharged() const       { return false; }
	virtual void SetDizzy(bool dizzy=false)       { }
	virtual void SetSilent(bool silent=false)     { }
	virtual void SetSeal(bool seal=false)         { }
	virtual void SetStone(bool stone=false)       { }
	virtual void SetCharging(bool charging=false) { }
	virtual void SetCharged(bool charging=false)  { }

	// TODO:暂时没有相应的效果逻辑
	virtual bool IsSleep() const         { return false; }
	virtual bool IsConfuse() const       { return false; }
	virtual void SetSleep(bool sleep=false)       { }
	virtual void SetConfuse(bool confuse=false)   { }

	//属性相关接口
	virtual void DoDamage(int32_t hp, int32_t attacker)     { } //掉血
	virtual void DoHeal(int32_t hp, int32_t healer=0)       { } //治疗
	virtual int32_t GetProperty(int32_t index) const;
	virtual void IncPropPoint(int32_t index, int32_t value) { }
	virtual void DecPropPoint(int32_t index, int32_t value) { }
	virtual void IncPropScale(int32_t index, int32_t value) { }
	virtual void DecPropScale(int32_t index, int32_t value) { }
	virtual void SetGenPowerFlag()     { } // 恢复能量值
	virtual void Revive(int32_t scale) { }

	//物品相关接口
	virtual int32_t GetItemCount(int32_t item_id) const     { return 0; }  
	virtual void TakeOutItem(int32_t item_id, int32_t count){ }

	//战场相关
	virtual int8_t GetPos() const      { return 0; }     // 返回自己在队伍中的站位
	virtual bool CanAction() const 	   { return false; } // 是否能行动
	virtual bool CanAttacked() const   { return false; } // 是否能被攻击
	virtual void GetTeamMate(std::vector<ObjInterface*>& players) const { }
	virtual void GetEnemy(std::vector<ObjInterface*>& players) const    { }
	virtual void GetGolem(std::vector<ObjInterface*>& players) const    { }
	virtual void IncSkillLevel(int32_t sk_tree_id, int32_t lvl)         { } //提升技能临时等级
	virtual void DecSkillLevel(int32_t sk_tree_id, int32_t lvl)         { } //降低技能临时等级

	//BUFF相关
	virtual void AttachBuffer(uint32_t buff_seq/*BUFF序列号*/, int32_t buff_id, int32_t attacher) { }
	virtual void DetachBuffer(uint32_t buff_seq/*BUFF序列号*/, int32_t buff_id, int32_t attacher) { }

	//技能相关接口
	virtual skill::SkillWrapper* GetSkill() const       { return NULL; }
	virtual skill::BuffWrapper* GetBuff() const         { return NULL; }
	virtual skill::CooldownWrapper* GetCooldown() const { return NULL; }

	//魔偶相关(限玩家)
	virtual bool PlayerSummonGolem(int32_t golem_id)    { return false; } // 玩家获得魔偶
	virtual bool PlayerPowerGolem(int32_t power)        { return false; } // 玩家给魔偶加能量
	virtual int32_t GetGolemProp(int32_t golem_id, int8_t index) { return 0; }
    virtual int32_t GetGolemDeActivePower(int32_t golem_id) { return 0; }

	//test
	virtual void Trace() const { }


private:
	Player* pplayer_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_SKILL_IF_H_
