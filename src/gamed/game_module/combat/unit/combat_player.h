#ifndef __GAME_MODULE_COMBAT_PLAYER_H__
#define __GAME_MODULE_COMBAT_PLAYER_H__

#include <set>
#include <map>
#include <vector>
#include <stdint.h>
#include "combat_unit.h"
#include "combat_def.h"
#include "combat_param.h"
#include "player_cmd.h"
#include "pet_man.h"
#include "cool_down.h"

#include "shared/base/callback_bind.h"


namespace G2C
{
struct CombatPlayerInfo;
}; //namespace G2C

namespace combat
{

///
/// 战斗玩家
///

class CombatNpc;
class CombatPlayer : public CombatUnit
{
public:
	struct RoleInfo
	{
		RoleID roleid;
		std::string rolename;
		char cls;
		char gender;
		short level;
		TemplID weapon_id;
	};

	struct SkillTree
	{
		TemplID skill_tree_id; //技能树模板ID
		SkillID skill_id;      //调用技能
		int8_t  level;         //基础等级
		int8_t  tmp_level;     //临时等级
		int8_t  max_level;     //升级上限
	};

private:
	typedef std::set<SkillID>  SkillSet;
	typedef std::vector<SkillTree>  SkillTreeVec;

private:
	RoleInfo       role_;             // 角色信息
	int32_t        master_id_;        // 位于哪个MASTER
	SkillID        cur_skill_;        // 当前选择的技能
	PetMan         pet_man_;          // 宠物管理器
    SkillSet       addon_skills_;   // 被动技能
	SkillTreeVec   skill_tree_vec_;   // 技能树列表
	CombatAward    combat_award_;     // 战斗奖励
	CoolDownMan    cooldown_man_;     // 冷却管理器
	bool           attack_done_;      // 攻击是否完成
	bool           offline_;          // 玩家是否离线
	bool           gen_mp_flag_;      // 能量恢复标记(由技能系统控制)
    int32_t        dying_time_;       // 玩家濒死时间

private:
    struct mob_killed
    {
        UnitID  unit_id;
        TemplID mob_tid;
    };

    struct player_killed
    {
        UnitID unit_id;
        RoleID roleid;
    };

    std::vector<mob_killed> mob_list_killed_; // 击杀怪物列表
    std::vector<player_killed> player_list_killed_; // 击杀玩家列表

public:
	CombatPlayer();
	virtual ~CombatPlayer();

	friend class CombatObjInterface;

	void OnPlayerJoin();

	bool CanSelectSkill() const;
	bool CanCastPetSkill() const;

	virtual char GetCls() const   { return role_.cls; }
	virtual int  GetLevel() const { return role_.level; }
    virtual int  GetSkill() const;
	virtual void OnInit();
	virtual void OnAttack();
    virtual void OnAttackEnd();
	virtual void OnDamage(int32_t dmg, UnitID attacker);
	virtual void OnDying();
	virtual void OnDeath();
	virtual void OnRoundStart();
	virtual void OnRoundEnd();
	virtual void OnHeartBeat();
	//virtual bool StartAttack();
    virtual void OnUpdateAttackState(int32_t action_time);
	virtual void DoDamage(int damage, UnitID attacker);
	virtual void HandleMSG(const MSG& msg);
	virtual void SendPacket(const shared::net::ProtoPacket&) const;
    virtual void ResetState();
	virtual void Trace() const;

	bool    IsOffline() const              { return offline_; }
	char    GetGender() const              { return role_.gender; }
	RoleID  GetRoleID() const              { return role_.roleid; }
	int32_t GetMasterServerID() const      { return master_id_; }
	void    SetMasterServerID(int32_t id)  { master_id_ = id; }
	void    SetRoleInfo(int64_t roleid, const std::string& rolename, char cls, char gender, short level, TemplID weapon_id);
	bool    Load(const player_data& playerdata);
    bool    LoadPetData(const player_data& playerdata);
	void    SaveForClient(G2C::CombatPlayerInfo& info) const;
	void    SetGenPowerFlag();
	PetMan* GetPetMan();
	int32_t GetPetPower() const;
	void    Clear();

	///
	/// CMD-Handler
	///
	bool HandleCMD(const PlayerCMD& cmd);
	bool CMDHandler_SelectSkill(const PlayerCMD& cmd);
	bool CMDHandler_PetAttack(const PlayerCMD& cmd);
	bool CMDHandler_TerminateCombat(const PlayerCMD& cmd);
    bool CMDHandler_PlayerOnline(const PlayerCMD& cmd);
    bool CMDHandler_PlayerOffline(const PlayerCMD& cmd);

	///
	/// 复活相关
	///
	void Revive();
	void Resurrect(int32_t scale);

	///
	/// 宠物相关
	///
	void GenPetPower();
	UnitID GetAttackPetID() const;

	///
	/// 技能相关
	///
	bool IsSkillExist(SkillID skill_id) const;
	bool IncSkillLevel(TemplID sk_tree_id, int32_t lvl);
	bool DecSkillLevel(TemplID sk_tree_id, int32_t lvl);

	///
	/// 战斗奖励相关
	///
	const CombatAward& GetAward() const;
	void GainCombatExp(int exp);
	void GainCombatMoney(int money);
	void GainCombatItem(int item_id, int item_count);
	void GainLotteryItem(int item_id, int item_count);
	void GainCombatAward(const std::vector<ItemEntry>& items, int32_t money, int32_t exp);

	///
	/// 击杀怪物统计
	///
	void KillMob(UnitID unit_id, TemplID mob_tid);
    void KillPlayer(UnitID unit_id, RoleID roleid);
	void GetMobsKilled(std::vector<MobKilled>& mob_killed_vec) const;
    std::vector<RoleID> GetPlayersKilled() const;

	///
	/// 冷却相关
	///
	bool TestCoolDown(int cooldown_id) const;
	bool SetCoolDown(int cooldown_id, int interval);

private:
	char GetStatus() const;//同步数据给客户端时调用
	int  CalDamageValue(const skill::SkillDamage& damage) const;
};


///
/// inline func
///
inline bool CombatPlayer::CanSelectSkill() const
{
	//return unit_state_.OptionPolicy(OPT_SELECT_SKILL);
    return true;
}

inline bool CombatPlayer::CanCastPetSkill() const
{
	return unit_state_.OptionPolicy(OPT_PET_ATTACK);
}

inline PetMan* CombatPlayer::GetPetMan()
{
	return &pet_man_;
}

inline int32_t CombatPlayer::GetPetPower() const
{
	return pet_man_.GetPower();
}

}; // namepspace combat

#endif // __GAME_MODULE_COMBAT_PLAYER_H__
