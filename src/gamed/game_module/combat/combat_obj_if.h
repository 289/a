#ifndef __GAME_MODULE_COMBAT_OBJ_IF_H__
#define __GAME_MODULE_COMBAT_OBJ_IF_H__

#include "obj_if/obj_interface.h"
#include "combat_types.h"

namespace combat
{

struct XID;
class CombatUnit;
class CombatObjInterface : public gamed::ObjInterface
{
	CombatUnit* obj;

public:
	explicit CombatObjInterface(CombatUnit* unit);
	virtual ~CombatObjInterface();
	CombatUnit* GetParent() const;

	// 基本信息
	virtual UnitID  GetId() const;
	virtual int8_t  GetLevel() const;
	virtual RoleID  GetRoleID() const;
	virtual int32_t GetRoleClass() const;
	virtual int64_t GetCurTime() const;
	virtual int8_t  GetStatus() const;
	virtual int8_t  GetBuffStatus() const;
	virtual bool    IsInCombat() const;
    virtual gamed::ObjInterface* GetOwner();

	// 玩家状态
	virtual bool IsDead() const;
	virtual bool IsDying() const;
	virtual bool IsDizzy() const;
	virtual bool IsSilent() const;
	virtual bool IsStone() const;
    virtual bool IsCharging() const;
	virtual bool IsCharged() const;
	virtual void SetDizzy(bool dizzy=false);
	virtual void SetSilent(bool silent=false);
	virtual void SetSeal(bool seal=false);
	virtual void SetStone(bool stone=false);
	virtual void SetCharging(bool charging=false);
	virtual void SetCharged(bool charged=false);

	virtual bool IsSleep() const;
	virtual bool IsConfuse() const;
	virtual void SetSleep(bool sleep=false);
	virtual void SetConfuse(bool confuse=false);

	// 属性相关
	virtual int32_t GetHP();
	virtual int32_t GetMP();
	virtual int32_t GetEP();
	virtual int32_t GetProperty(int32_t index) const;
	virtual int32_t GetMaxProp(int32_t index) const;
	virtual void    DoDamage(int32_t hp, UnitID attacker);
	virtual void    DoHeal(int32_t hp, UnitID healer);
	virtual void    ConsumeProp(int32_t index, int32_t value);
	virtual void    IncPropPoint(int32_t index, int32_t value);
	virtual void    DecPropPoint(int32_t index, int32_t value);
	virtual void    IncPropScale(int32_t index, int32_t value);
	virtual void    DecPropScale(int32_t index, int32_t value);
	virtual void    SetGenPowerFlag();

	virtual void    Revive(int32_t scale);

	// 物品相关
	virtual int32_t GetItemCount(TemplID item_id) const;
	virtual void    TakeOutItem(TemplID item_id, int32_t count);
	virtual bool    CheckItem(TemplID item_id, int32_t count); 
	virtual bool    CheckItem(int32_t index, TemplID item_id, int32_t count);

	// 战场相关
	virtual int8_t  GetPos() const;
	virtual bool    CanAction() const;
	virtual bool    CanAttacked() const;
	virtual void    GetTeamMate(std::vector<ObjInterface*>& players) const;
	virtual void    GetEnemy(std::vector<ObjInterface*>& players) const;
	virtual void    GetGolem(std::vector<ObjInterface*>& players) const;
	virtual void    IncSkillLevel(TemplID sk_tree_id, int32_t lvl);
	virtual void    DecSkillLevel(TemplID sk_tree_id, int32_t lvl);

	// BUFF相关
	virtual void AttachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher);
	virtual void DetachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher);

	// 技能相关
	virtual skill::SkillWrapper* GetSkill() const;
	virtual skill::BuffWrapper* GetBuff() const;
	virtual skill::CooldownWrapper* GetCooldown() const;

	// 魔偶相关(限玩家)
	virtual bool PlayerSummonGolem(TemplID golem_id);
	virtual bool PlayerPowerGolem(int32_t power);
	virtual int32_t GetGolemProp(int32_t golem_id, int8_t index);
    virtual int32_t GetGolemDeActivePower(int32_t golem_id);

	// test
	virtual void Trace() const;
};

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_OBJ_IF_H__
