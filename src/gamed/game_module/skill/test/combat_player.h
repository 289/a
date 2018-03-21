#ifndef GAMED_COMBAT_PLAYER_H_
#define GAMED_COMBAT_PLAYER_H_

#include <stdio.h>
#include "obj_interface.h"
#include "skill_wrapper.h"
#include "buff_wrapper.h"
#include "cooldown_wrapper.h"

namespace gamed
{

using namespace skill;

class CombatPlayer : public ObjInterface
{
public:
	CombatPlayer(int32_t id, int8_t pos, int8_t team)
		: id_(id), pos_(pos), team_(team)
	{
		prop_[PROP_INDEX_MAX_HP] = 600;
		prop_[PROP_INDEX_MAX_CON1] = 0;
		prop_[PROP_INDEX_MAX_CON2] = 0;
		prop_[PROP_INDEX_MAX_PHYSICAL_ATTACK] = 180;
		prop_[PROP_INDEX_MAX_MAGIC_ATTACK] = 0;
		prop_[PROP_INDEX_MAX_PHYSICAL_DEFENCE] = 85;
		prop_[PROP_INDEX_MAX_MAGIC_DEFENCE] = 45;
		prop_[PROP_INDEX_MAX_PHYSICAL_PIERCE] = 0;
		prop_[PROP_INDEX_MAX_MAGIC_PIERCE] = 0;
		prop_[PROP_INDEX_MAX_HIT] = 955;
		prop_[PROP_INDEX_MAX_DODGE] = 5;
		prop_[PROP_INDEX_MAX_CRIT_HIT] = 55;
		prop_[PROP_INDEX_MAX_ATTACK_SPEED] = 3000;
		prop_[PROP_INDEX_MAX_ATTACK_PRIORITY] = 0;
		prop_[PROP_INDEX_MAX_MANA_RESTORE] = 0;
		prop_[PROP_INDEX_MAX_MOVE_SPEED] = 0;
		prop_[PROP_INDEX_HP] = 600;
		prop_[PROP_INDEX_CON1] = 1000;
		prop_[PROP_INDEX_CON2] = 1000;

		memcpy(base_prop_, prop_, sizeof(prop_));

		skill_ = new SkillWrapper(this);
		buff_ = new BuffWrapper(this);
		cooldown_ = new CooldownWrapper(this);

		dizzy_ = false;
		sleep_ = false;
		silent_ = false;
		confuse_ = false;
		charging_ = false;
		charge_ = false;
        stone_ = false;
        seal_ = false;
	}

	CombatPlayer(const CombatPlayer& rhs)
		: id_(rhs.id_), pos_(rhs.pos_), team_(rhs.team_)
	{
		for (size_t i = 0; i < 19; ++i)
		{
			prop_[i] = rhs.prop_[i];
		}

		skill_ = new SkillWrapper(this);
		buff_ = new BuffWrapper(this);
		cooldown_ = new CooldownWrapper(this);
	}

	// 基本信息相关接口
	virtual int32_t GetId() const
	{
		return id_;
	}
	virtual int32_t GetRoleClass() const
	{
		return 1;
	}
	virtual int8_t  GetLevel() const
	{
		return 1;
	}
	virtual int64_t GetCurTime() const
	{
		return 0;
	}
	virtual bool IsInCombat() const
	{
		return true;
	}

	virtual bool IsDead() const
	{
		return GetProperty(PROP_INDEX_HP) <= 0;
	}
	virtual bool IsDying() const
	{
		return IsDead();
	}
	virtual bool IsDizzy() const
	{
		return dizzy_;
	}
	virtual bool IsSilent() const
	{
		return silent_;
	}
	virtual bool IsStone() const
	{
		return stone_;
	}
	virtual void SetDizzy(bool dizzy=false)
	{
		dizzy_ = dizzy;
	}
	virtual void SetSilent(bool silent=false)
	{
		silent_ = silent;
	}
	virtual void SetSeal(bool seal=false)
	{
		seal_ = seal;
	}
	virtual void SetStone(bool stone=false)
	{
		stone_ = stone;
	}
    // TODO
    virtual bool IsSleep() const {return false;}
    virtual bool IsConfuse() const {return false;}
    virtual bool IsCharging() const {return false;}
    virtual void SetSleep(bool sleep=false) {}
    virtual void SetConfuse(bool sleep=false) {}
    virtual void SetCharging(bool sleep=false) {}

	//属性相关接口
	virtual void DoDamage(int32_t hp, int32_t attacker)
	{
		bool before = prop_[PROP_INDEX_HP] > 0;
		prop_[PROP_INDEX_HP] -= hp;		
		bool after = prop_[PROP_INDEX_HP] > 0;
		if (before && !after)
		{
			BuffDamageVec buffdmg_vec;
			buff_->Dying(buffdmg_vec);
			buff_->Dead(buffdmg_vec);
			__PRINTF("Player id=%d Dead buffdmg_vec.size=%d", id_, (int32_t)buffdmg_vec.size());
		}
	}
	virtual void DoHeal(int32_t hp, int32_t healer)
	{
		prop_[PROP_INDEX_HP] += hp;
		if (prop_[PROP_INDEX_HP] > prop_[PROP_INDEX_MAX_HP])
		{
			prop_[PROP_INDEX_HP] = prop_[PROP_INDEX_MAX_HP];
		}
	}
	virtual int32_t GetProperty(int32_t index) const
	{
		if (index >= 1000)
		{
			return base_prop_[index - 1000];
		}
		else
		{
			return prop_[index];
		}
	}
	virtual void IncPropPoint(int32_t index, int32_t value)
	{
		prop_[index] += value;
	}
	virtual void DecPropPoint(int32_t index, int32_t value)
	{
		prop_[index] -= value;
		if (prop_[index] < 0)
		{
			prop_[index] = 0;
		}
	}
	virtual void IncPropScale(int32_t index, int32_t value)
	{
	}
	virtual void DecPropScale(int32_t index, int32_t value)
	{
	}
	virtual void SetGenPowerFlag()
	{
	}
	virtual void Revive(int32_t scale)
	{
		prop_[PROP_INDEX_HP] = prop_[PROP_INDEX_MAX_HP] * scale / 10000.0;
	}

	//物品相关接口
	virtual int32_t GetItemCount(int32_t item_id) const
	{
		return 0;
	}
	virtual void TakeOutItem(int32_t item_id, int32_t count)
	{
	}

	// 战场相关
	virtual int8_t GetPos() const
	{
		return pos_;
	}
	virtual bool CanAction() const
	{
		return true;
	}
	virtual bool CanAttacked() const
	{
		return true;
	}
	virtual void GetTeamMate(std::vector<ObjInterface*>& players) const;
	virtual void GetEnemy(std::vector<ObjInterface*>& players) const;
	virtual void IncSkillLevel(int32_t sk_tree_id, int32_t lvl)
	{
	}
	virtual void DecSkillLevel(int32_t sk_tree_id, int32_t lvl)
	{
	}

	// BUFF相关
	virtual void AttachBuffer(uint32_t buff_seq, int32_t buff_id, int32_t attacher)
	{
	}
	virtual void DetachBuffer(uint32_t buff_seq, int32_t buff_id, int32_t attacher)
	{
	}
	//void Show()
	//{
		//buff_->Show();
	//}

	// 技能相关接口
	virtual SkillWrapper* GetSkill() const
	{
		return skill_;
	}
	virtual BuffWrapper* GetBuff() const
	{
		return buff_;
	}
	virtual CooldownWrapper* GetCooldown() const
	{
		return cooldown_;
	}

	//魔偶相关(限玩家)
	virtual bool PlayerSummonGolem(int32_t golem_id)
	{
		return true;
	}
	virtual bool PlayerPowerGolem(int32_t power)
	{
		return true;
	}
	virtual int32_t GetGolemPower()
	{
		return 0;
	}
private:
	int32_t id_;
	int32_t pos_;
	int32_t team_;
	int32_t prop_[19];
	int32_t base_prop_[19];
	bool dizzy_;
	bool sleep_;
	bool silent_;
	bool confuse_;
	bool charging_;
	bool charge_;
	bool stone_;
	bool seal_;
	SkillWrapper* skill_;
	BuffWrapper* buff_;
	CooldownWrapper* cooldown_;
};

} // namespace gamed

#endif // GAMED_COMBAT_PLAYER_H_
