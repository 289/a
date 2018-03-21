#ifndef __GAME_MODULE_COMBAT_UNIT_H__
#define __GAME_MODULE_COMBAT_UNIT_H__

#include <stdint.h>
#include "object.h"
#include "combat_types.h"
#include "prop_policy.h"
#include "unit_state.h"

#include "shared/net/packet/proto_packet.h"
#include "game_module/obj_if/obj_interface.h"
#include "game_module/skill/include/damage_msg.h"
#include "game_module/skill/include/buff_wrapper.h"
#include "game_module/skill/include/skill_wrapper.h"

namespace gamed
{
class ObjInterface;
}; // namespace gamed
typedef std::vector<gamed::ObjInterface*> ObjIfVec;

namespace skill
{
struct SkillDamage;
}; // namespace skill
typedef std::vector<skill::SkillDamage> SkillDamageVec;

namespace shared {
namespace net {
class ProtoPacket;
}; // namespace net
}; // namespace shared

namespace combat
{

struct MSG;
class Object;
class Combat;
class PropPolicy;
class CombatNpc;

/**
 * 战斗对象类型
 */
enum UnitType
{
	UT_INVALID,
	UT_PLAYER,
	UT_MOB,
	UT_PET,
	UT_GOLEM,
	UT_TEAM_NPC,
	UT_BOSS,
};

/**
 * @class CombatUnit
 * @brief 战斗对象基类(包含玩家、怪物、战宠、魔偶)
 */
class CombatUnit : public Object
{
protected:
	/**
	 * 战斗对象状态，彼此之间可以共存
	 */
	enum
	{
		STATE_DIZZY     = 0x0001,   // 眩晕
		STATE_SLEEP     = 0x0002,   // 昏睡
		STATE_SILENT    = 0x0004,   // 沉默
		STATE_CONFUSE   = 0x0008,   // 混乱
        STATE_CHARGING  = 0x0010,   // 正在蓄力
        STATE_CHARGED   = 0x0020,    // 蓄力完成
	};

	typedef std::vector<CombatNpc*> CombatNpcVec;
protected:
	BasicProp  basic_prop_;
	ExtendProp base_prop_[PROP_INDEX_HIGHEST];
	ExtendProp cur_prop_[PROP_INDEX_HIGHEST];
	ExtendProp enh_point_[PROP_INDEX_HIGHEST];
	ExtendProp enh_scale_[PROP_INDEX_HIGHEST];

	int8_t pos_;
    int8_t party_;                             //属于哪个派别
	short level_;                              //当前等级
	int64_t state_;                            //对象状态
    std::string model_;                        //模型资源
	UnitState unit_state_;                     //状态机状态
	skill::SkillWrapper* skill_;               //技能系统
	skill::BuffWrapper* buff_man_;             //战斗Buff
	skill::CooldownWrapper* skill_cd_;         //技能冷却
	gamed::ObjInterface* obj_if_;              //供技能使用
	int32_t refresh_extprop_mask_;             //标记哪些扩展属性被刷新
	bool    refresh_volatile_prop_;            //标记易变属性发生变化(hp/max-hp)
	bool    refresh_basic_prop_;               //标记基础属性发生变化(hp/mp/ep)
	bool    is_atb_owner_;                     //是否拥有ATB
    bool    is_transforming_;                  //处于变身中
    bool    is_escaping_;                      //处于逃跑中
	bool    attack_done_;                      //攻击完成标记
	UnitID  killer_;                           //被谁杀死
	SkillID default_skill_;                    //默认技能
	SkillID recast_skill_;                     //追击技能
	int32_t round_counter_;                    //回合计数
	CombatNpcVec   golem_list_;                // 处于休战状态的魔偶
	CombatNpc*     cur_golem_;                 // 处于出战状态的魔偶
    bool           golem_action_;              // 是否是魔偶行动
	bool    refresh_cur_golem_;                //标记当前魔偶属性发生变化，易变(mp)
	bool    refresh_other_golem_;              //标记其他魔偶属性发生变化(mp)
	Combat* combat_;                           //所在战场


protected:
	///
	/// 本回合技能攻击结果，回合结束时根据这个值来更新目标状态和同步客户端
	///
	std::vector<CombatUnit*>        target_list_;    // 本回合的攻击目标
	std::vector<skill::BuffDamage>  buff_damages_;   // 本回合BUFF产生的伤害
	std::vector<skill::SkillDamage> skill_damages_;  // 本回合技能产生的伤害

	struct AttackResult
	{
		std::vector<UnitID> targets;
		skill::SkillDamage skill_damage;
	};

	std::vector<AttackResult> attack_result_vec_;

public:
	CombatUnit(char type);
	virtual ~CombatUnit();

	friend class Combat;
	friend class UnitState;
	friend class PropPolicy;
	friend class CombatObjInterface;

	virtual void MessageHandler(const MSG& msg);
	virtual void HeartBeat();
	virtual void Trace() const;

	//test
	void dump() const;
    std::string model() { return model_; }

public:
	void Initialize();
	void Die();
	void Dying();
	bool RoundStart();
	void RoundEnd();
	bool Attack();
	void AttackEnd();
	void AttackedEnd();
    void TransformWaitEnd();
    void TransformingEnd();
    void EscapeWaitEnd();
    bool GolemCast();
	void CombatStart();
	void OnSneakedEnd();
	void OnAttacked(int attacked_time);
	void OnCombatEnd();
	void BroadCastCMD(shared::net::ProtoPacket& packet, UnitID ex_id=0) const;
	void Clear();
    void OnSendCurGolemProp() const;
    void OnSendOtherGolemProp() const;


public:
	///
	/// virtual func
	///
	virtual bool IsPet() const            { return false; }
	virtual bool IsGolem() const          { return false; }
	virtual bool IsMob() const            { return false; }
	virtual bool IsTeamNpc() const        { return false; }
	virtual bool IsBoss() const           { return false; }
	virtual char GetCls() const           { return -1; }
	virtual int  GetSkill() const         { return 0; }
	virtual void GetEnemy(ObjIfVec& list) const;
	virtual void GetTeamMate(ObjIfVec& list) const;
	virtual void GetGolem(ObjIfVec& list) const;
	virtual int  GetEnemiesAlive() const;
	virtual int  GetTeammatesAlive() const;
	virtual void OnInit()  {}
	virtual void OnCombatStart() {}
	virtual void OnAttack() {}
    virtual void OnUpdateAttackState(int32_t action_time);
	virtual void OnAttackEnd() {}
    virtual void OnAttackedEnd() {}
    virtual void OnTransformWaitEnd() {}
    virtual void OnEscapeWaitEnd() {}
	virtual void OnDamage(int32_t damage, UnitID attacker) {}
	virtual void OnDying() {}
	virtual void OnDeath() {}
	virtual void OnRoundStart() {}
	virtual void OnRoundEnd()   {}
	virtual bool StartAttack();
	virtual void DoHeal(int life, UnitID healer);
	virtual void DoDamage(int damage, UnitID attacker);
	virtual int32_t GetProp(size_t index) const;
	virtual int32_t GetBaseProp(size_t index) const;
	virtual void IncProp(size_t index, int value);
	virtual void DecProp(size_t index, int value);
	virtual void IncPropScale(size_t index, int value);
	virtual void DecPropScale(size_t index, int value);
	virtual void OnHeartBeat() {}
	virtual void SendPacket(const shared::net::ProtoPacket&) const {}
	virtual void OnMessageHandler(const MSG& msg) {}
    virtual void ResetState() {}

public:
	///
	/// 战斗单位相关
	///
	void SetPos(int pos);
	int  GetPos() const;
	void SetLevel(int16_t level);
	int  GetLevel() const;
    void SetParty(int party);
    int  GetParty() const;
	int  GetRoundCounter() const;
	void SetATBOwner();
    void SetModel(const std::string& model);
    std::string GetModel() const;
	bool CanAction() const;
	bool CanAttacked() const;
	bool IsAlive() const;
	bool IsATBOwner() const;

	bool IsDizzy() const;
	bool IsSleep() const;
	bool IsSilent() const;
	bool IsConfuse() const;
	bool IsCharging() const;
	bool IsCharged() const;

	void SetDizzy(bool bdizzy);
	void SetSleep(bool bsleep);
	void SetSilent(bool bsilent);
	void SetConfuse(bool bconfuse);
	void SetCharging(bool bcharging);
	void SetCharged(bool bcharged);

	bool IsPlayer() const { return xid_.IsPlayer(); }
	bool IsNPC() const    { return xid_.IsNPC(); }

	///
	/// ATB相关
	///
	void RegisterATB();
	void RegisterInstantATB();
	void ResetATB();
	void UnRegisterATB();

	///
	/// 技能相关
	///
	void AddSkill(SkillID skillid);
	void SetDefaultSkill(SkillID skillid);
	void SetRecastSkill(SkillID skillid);
	void ClrRecastSkill();
    void Consume(SkillID skillid);
	SkillID GetRecastSkill() const;
	SkillID GetDefaultSkill() const;
	const SkillDamageVec& GetSkillDamages() const;

	gamed::ObjInterface* GetObjIf();
	skill::SkillWrapper* GetSkillWrapper();
	skill::BuffWrapper* GetBuffWrapper();
	skill::CooldownWrapper* GetCDWrapper();

	void AttachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher);
	void DetachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher);

	///
	/// 状态相关
	///
	bool IsDead() const;
	bool IsDying() const;
	bool IsZombie() const;
	bool IsZombieDying() const;
	bool IsNormal() const;
	bool IsAction() const;
	bool IsAttacked() const;
	bool IsSneaked() const;
    bool IsTransformWait() const;
    bool IsTransforming() const;
    bool IsPlayerAction() const;
    bool IsGolemAction() const;
	bool IsWaitGolem() const;
	void UpdateState(Event event, int timeout=-1);
	void SetStateTimeout(int timeout);
	int  GetStateTimeout() const;

	///
	/// 战场相关
	///
	Combat* GetCombat() const;
	void SetCombat(Combat* cbt);

	///
	/// 战斗对象之间的关系
	///
	bool IsEnemy(UnitID unit_id) const;
	bool IsTeammate(UnitID unit_id) const;

	///
	/// 喊话
	///
	void Speak(int id_talk, int msec);

    ///
    /// 变身相关
    ///
    void SetTransforming();
    void ClrTransforming();
    bool TestTransforming() const;

    ///
    /// 逃跑相关
    ///
    void SetEscaping();
    bool TestEscaping() const;

	void SetAttackDone();
	void ClrAttackDone();
	bool TestAttackDone() const;
	void SetSneaked();

	///
	/// 魔偶相关
	///
	int  GetGolem(TemplID golem_id);
	bool SummonGolem(TemplID golem_id);
	bool PowerGolem(int32_t power);
	int32_t GetGolemProp(TemplID golem_id, size_t index);
    int32_t GetGolemDeActivePower(TemplID golem_id);
    UnitID GetCurGolemID() const;

public:
	///
	/// 属性相关
	///
	int  GetHP() const;
	int  GetMP() const;
	int  GetEP() const;
	int  GetMaxHP() const;
	int  GetMaxMP() const;
	int  GetMaxEP() const;
	int  GetATBTime() const;
	int  GetAttackPriority() const;
	int  GetMaxProp(size_t index) const;
    void SyncHP(int hp);
	void SetHP(int hp);
	void SetMP(int mp);
	void SetEP(int ep);
	void SetATBTime(int atb_time);

	void SetBasicProp(int32_t hp, int32_t mp, int32_t ep);
	void SetBaseProp(const std::vector<int32_t>& props);
	void RefreshExtendProp();

	void SetRefreshVolatileProp();
	void ClrRefreshVolatileProp();
	bool TestRefreshVolatileProp() const;

	void SetRefreshBasicProp();
	void ClrRefreshBasicProp();
	bool TestRefreshBasicProp() const;

	void SetRefreshExtPropMask(size_t index);
	void ClrRefreshExtPropMask();
	int32_t GetRefreshExtPropMask() const;

	void SetRefreshCurGolem();
	void ClrRefreshCurGolem();
	bool TestRefreshCurGolem() const;

	void SetRefreshOtherGolem();
	void ClrRefreshOtherGolem();
	bool TestRefreshOtherGolem() const;

	UnitID GetKiller() const;
	void SetKiller(UnitID killer);

	void SendBuffResult(const skill::BuffDamageVec& damages) const;//广播协议

protected:
	void IncMP(size_t index, int value);
	void DecMP(size_t index, int value);
	void IncEP(size_t index, int value);
	void DecEP(size_t index, int value);
	void IncPropPoint(size_t index, int value);
	void DecPropPoint(size_t index, int value);

	bool CheckSneaked();
	bool DoAttack(SkillID skillid, bool recast=false);
	void SaveBuffMessage(const skill::BuffDamageVec& damages);

	void UpdateAndSendProp();//更新并同步属性
	void SendBasicProp() const;//发给自己
	void SendExtendProp() const;//发给自己
	void SendVolatileProp() const;//广播协议
    void SendCurGolemProp() const;//广播协议
    void SendOtherGolemProp() const;//发给自己
};

#include "combat_unit.inl"

}; // namepspace combat

#endif // __GAME_MODULE_COMBAT_UNIT_H__
