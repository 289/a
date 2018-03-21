#ifndef __GAME_MODULE_COMBAT_NPC_H__
#define __GAME_MODULE_COMBAT_NPC_H__

#include "combat_unit.h"

namespace G2C
{
struct CombatMobInfo;
}; // namespace G2C

namespace combat
{

///
/// 战斗NPC(包含怪物、战宠和魔偶)
///

class NpcAI;
class NpcImp;
class CombatPlayer;
class CombatNpc : public CombatUnit
{
public:
	enum NpcType
	{
		TYPE_NPC_INVALID,
		TYPE_NPC_PET,       //宠物
		TYPE_NPC_GOLEM,     //魔偶
		TYPE_NPC_MOB,       //怪物
		TYPE_NPC_TEAMNPC,   //组队NPC
		TYPE_NPC_BOSS,      //世界BOSS
		TYPE_NPC_MAX,
	};

protected:
	TemplID npc_id_;        //NPC的ID
	char    npc_type_;      //NPC类型
	NpcAI*  npc_ai_;        //NPC的AI
	NpcImp* npc_imp_;       //NPC实体

public:
	CombatNpc();
	virtual ~CombatNpc();

	TemplID GetNpcID() const;
	void    SetNpcID(TemplID npc_id);
	char    GetNpcType() const;
	void    SetNpcType(char type);
	void    Clear();

	virtual bool IsPet() const;
	virtual bool IsGolem() const;
	virtual bool IsMob() const;
	virtual bool IsTeamNpc() const;
	virtual bool IsBoss() const;

	virtual void OnInit();
	virtual void OnDamage(int32_t dmg, UnitID attacker);
	virtual void OnDying();
	virtual void OnDeath();
	virtual void OnAttack();
	virtual void OnAttackEnd();
    virtual void OnTransformWaitEnd();
    virtual void OnEscapeWaitEnd();
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
	virtual void OnTeammateDead();
	virtual void OnHeartBeat();
    virtual void OnUpdateAttackState(int32_t action_time);

	virtual void DoHeal(int life, UnitID healer);
	virtual void DoDamage(int damage, UnitID attacker);
	virtual void IncProp(size_t index, int value);
	virtual void DecProp(size_t index, int value);
	virtual void IncPropScale(size_t index, int value);
	virtual void DecPropScale(size_t index, int value);

	virtual int  GetSkill() const;
	virtual int  GetProp(size_t index) const;
	virtual void GetEnemy(ObjIfVec& list) const;
	virtual void GetTeamMate(ObjIfVec& list) const;
	virtual void Trace() const;

	CombatUnit* GetOwner() const;
	void SetOwner(CombatUnit* owner);
	void OnActived();
	void OnDeActived();

	bool CanSummoned() const;
    bool CanTransform() const;
    bool CanEscape() const;

	///
	/// 怪物相关
	///
	void SetSkill(SkillID skill_id);
	void CastInstantSkill(SkillID skill_id);
    void TriggerTransform(TemplID new_mob_tid);//触发怪物变身
    void Transform(TemplID new_mob_tid); //怪物开始变身
    void TriggerEscape(int32_t result); // 触发怪物逃跑
    void Escape(); // 怪物逃跑

	///
	/// 为客户端准备数据
	///
	void SaveForClient(G2C::CombatMobInfo& info) const;

	///
	/// 宠物相关
	///
	void SetPetBLevel(int pet_blevel);
	void SetPetCombatPos(int pet_combat_pos);
	int  GetPetCombatPos() const;
	int32_t GetPowerConsume() const;

	///
	/// 魔偶相关
	///
	void RestorePower();
    void ConsumePower();
    int  GetDeActivePower() const;

	///
	/// 同步BOSS血量
	///
	void SyncBossHP(int32_t new_hp);
};

class NpcImp
{
protected:
	typedef std::vector<SkillID> SkillVec;
	CombatNpc* parent_;
	CombatUnit* owner_;

public:
	NpcImp(CombatNpc* p):
		parent_(p), owner_(NULL)
	{}
	virtual ~NpcImp()
	{
		parent_ = NULL;
        owner_ = NULL;
	}

	virtual void OnInit()          {}
	virtual void OnDamage(int32_t dmg, UnitID attacker){}
	virtual void OnDying()         {}
	virtual void OnDeath()         {}
	virtual void OnAttack()        {}
	virtual void OnAttackEnd()     {}
    virtual void OnTransformWaitEnd() {}
    virtual void OnEscapeWaitEnd() {}
	virtual void OnRoundStart()    {}
	virtual void OnRoundEnd()      {}
	virtual void OnHeartBeat()     {}
	virtual void OnActived()       {}
	virtual void OnDeActived()     {}
    virtual void OnUpdateAttackState(int32_t action_time) {}
	virtual int  GetProp(size_t index) const            {return parent_->CombatUnit::GetProp(index);}
	virtual void DoHeal(int life, UnitID healer)        {parent_->CombatUnit::DoHeal(life, healer);}
	virtual void DoDamage(int damage, UnitID attacker)  {parent_->CombatUnit::DoDamage(damage, attacker);}
	virtual void IncProp(size_t index, int value)       {parent_->CombatUnit::IncProp(index, value);}
	virtual void DecProp(size_t index, int value)       {parent_->CombatUnit::DecProp(index, value);}
	virtual void IncPropScale(size_t index, int value)  {parent_->CombatUnit::IncPropScale(index, value);}
	virtual void DecPropScale(size_t index, int value)  {parent_->CombatUnit::DecPropScale(index, value);}
	virtual void SetOwner(CombatUnit* owner);
	virtual CombatUnit* GetOwner() const;
	virtual int  GetSkill() const                       {return 0;}
    virtual int  GetDeActivePower() const               {return 0;}
	virtual void GetEnemy(ObjIfVec& list) const         {}
	virtual void GetTeamMate(ObjIfVec& list) const      {}
	virtual void Trace() const                          {}


	///
	/// 为客户端准备数据
	///
	virtual void SaveForClient(G2C::CombatMobInfo& info) const {}

	///
	/// 怪物相关
	///
	virtual void SetSkill(SkillID skill_id) {}
	virtual void CastInstantSkill(SkillID skill_id) {}
    virtual void TriggerTransform(TemplID new_mob_tid) {}
    virtual void Transform(TemplID new_mob_tid) {}
    virtual void TriggerEscape(int32_t result) {}
    virtual void Escape() {}

	///
	/// 宠物相关
	///
	virtual void SetPetBLevel(int pet_blevel) {}
	virtual void SetPetCombatPos(int pet_combat_cos) {}
	virtual int  GetPetCombatPos() const { return -1; }
	virtual int32_t GetPowerConsume() const { return 0; }

	///
	/// 魔偶相关
	///
	virtual void RestorePower() {}

	///
	/// 同步BOSS血量
	///
	virtual void SyncBossHP(int32_t new_hp) {}
};

class AnimalImp : public NpcImp
{
public:
	AnimalImp(CombatNpc* p): NpcImp(p)
	{}
	virtual ~AnimalImp()
	{}

	virtual void DoHeal(int life, UnitID healer);
	virtual void DoDamage(int damage, UnitID attacker);
	virtual void IncProp(size_t index, int value);
	virtual void DecProp(size_t index, int value);
	virtual void IncPropScale(size_t index, int value);
	virtual void DecPropScale(size_t index, int value);
	virtual void GetEnemy(ObjIfVec& list) const;
	virtual void GetTeamMate(ObjIfVec& list) const;
};

class PetImp : public AnimalImp
{
protected:
	int16_t pet_blevel_;        // 宠物血脉等级
	int16_t pet_combat_pos_;    // 宠物在战宠栏的位置
	int32_t consume_on_attack_; // 攻击时消耗多少能量

public:
	PetImp(CombatNpc* p): AnimalImp(p),
		pet_blevel_(0),
		pet_combat_pos_(-1),
		consume_on_attack_(0)
	{}
	virtual ~PetImp()
	{}

	virtual void OnInit();
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
	virtual void OnAttack();
	virtual void OnAttackEnd();
	virtual void Trace() const;

	/*宠物独有的函数*/
	virtual void SetPetBLevel(int pet_blevel);
	virtual void SetPetCombatPos(int pet_combat_pos);
	virtual int  GetPetCombatPos() const;
	virtual int32_t GetPowerConsume() const;
};

class GolemImp : public AnimalImp
{
protected:
	SkillVec skill_group_;  //魔偶技能组
	size_t cur_skill_idx_;  //当前技能下标
    bool appear_;           //是否是刚被召唤出来，刚出来放的第一次技能不消耗能量
    int8_t deactive_power_; //被收回时的能量值

public:
	GolemImp(CombatNpc* p) : AnimalImp(p),
		cur_skill_idx_(0), appear_(false), deactive_power_(0)
	{}
	virtual ~GolemImp()
	{}

	virtual void OnInit();
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
    virtual void OnUpdateAttackState(int32_t action_time);
	virtual void OnActived();
	virtual void OnDeActived();
	virtual int  GetSkill() const;
	virtual void IncProp(size_t index, int value);
	virtual void DecProp(size_t index, int value);
	virtual int  GetProp(size_t index) const;
    virtual int  GetDeActivePower() const;
	virtual void Trace() const;
	virtual void OnAttackEnd();

	/*魔偶独有的函数*/
	virtual void RestorePower();
};

class MobImp : public NpcImp
{
protected:
	struct SkillEntry
	{
		SkillID skill_id;
		int32_t cast_probability;
	};
	typedef std::vector<SkillEntry> SkillGroup;
	SkillGroup skill_group_;       // 怪物技能组
	SkillID    ai_skill_;          // 怪物策略技能
	int32_t    ai_skill_rounder_;  // 怪物策略技能生效回合
    TemplID    new_mob_tid_;       // 将要变成的怪物ID

public:
	MobImp(CombatNpc* p) :
        NpcImp(p),
        ai_skill_(0),
        ai_skill_rounder_(0),
        new_mob_tid_(0)
	{}
	virtual ~MobImp()
	{
		ai_skill_ = 0;
		ai_skill_rounder_ = 0;
        new_mob_tid_ = 0;

		skill_group_.clear();
	}

	virtual void OnInit();
	virtual void OnDamage(int32_t dmg, UnitID attacker);
	virtual void OnDying();
	virtual void OnDeath();
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
    virtual void OnTransformWaitEnd();
    virtual void OnEscapeWaitEnd();

	virtual SkillID GetSkill() const;
	virtual void SetSkill(SkillID skill_id);
	virtual void CastInstantSkill(SkillID skill_id);
    virtual void TriggerTransform(TemplID new_mob_tid);
    virtual void Transform(TemplID new_mob_tid);
    virtual void TriggerEscape(int32_t result);
    virtual void Escape();
	virtual void GetEnemy(ObjIfVec& list) const;
	virtual void GetTeamMate(ObjIfVec& list) const;
	virtual void SaveForClient(G2C::CombatMobInfo& info) const;
	virtual void Trace() const;
};

class BossImp : public MobImp
{
private:

public:
	BossImp(CombatNpc* p): MobImp(p)
	{}
	virtual ~BossImp()
	{}

	virtual void OnInit();
	virtual void OnDamage(int32_t dmg, UnitID attacker);
	virtual void OnHeal(int32_t life);
	virtual void OnDeath();
	virtual void Trace() const;
	virtual void SyncBossHP(int32_t new_hp);
};

};

#endif // __GAME_MODULE_COMBAT_NPC_H__
