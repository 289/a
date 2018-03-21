#ifndef __GAME_MODULE_COMBAT_PET_MAN_H__
#define __GAME_MODULE_COMBAT_PET_MAN_H__

#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "combat_types.h"

namespace G2C
{
struct CombatPetInfo;
};

namespace combat
{

struct pet_data;

class Combat;
class CombatNpc;
class CombatPlayer;

/**
 * @class PetMan
 * @brief 战宠管理器
 */
class PetMan
{
protected:
private:
	struct PetInfo
	{
		TemplID pet_id;
		int16_t pet_rank;
		int16_t pet_level;
		int16_t pet_blevel;
		int16_t pet_item_idx;
		int16_t pet_combat_pos;
	};

	struct pet_finder
	{
		int pet_pos;
		pet_finder(int pos): pet_pos(pos) {}
		bool operator () (const PetInfo& pet) const
		{
			return pet.pet_combat_pos == pet_pos;
		}
	};

	typedef std::vector<PetInfo> CombatPetVec;

	CombatPlayer* player_;          // 宠物主人对象
	CombatPetVec  pet_vec_;         // 宠物对象列表
	CombatNpc*    pet_obj_;         // 宠物战斗对象
	Combat*       combat_;          // 位于哪个战场

	int32_t pet_power_;             // 宠物能量的当前值
	bool    pet_power_flag_;        // 标记宠物能量是否发生变化
	int32_t pet_power_cap_;         // 宠物能量上限
	int32_t pet_power_gen_speed_;   // 宠物能量恢复速度
    int32_t pet_attack_cd_time_;    // 宠物攻击的冷却时间

public:
	PetMan();
	virtual ~PetMan();

	bool Load(const pet_data& petdata);
	void Save(std::vector<G2C::CombatPetInfo>& pet_list) const;

	void SetPlayer(CombatPlayer* player);
	void SetCombat(Combat* combat);
	void UnRegisterATB();
	void HeartBeat();
	bool Attack(int pet_combat_pos);
	void GenPower();
	void ConsumePower(int32_t con);
	int32_t GetPower() const;
    int32_t GetAttackCDTime() const;
	UnitID GetAttackPetID() const;

	void Clear();
	void Trace() const;

private:
	bool IsPetExist(int pet_combat_pos) const;
	bool InitPetObj(int pet_combat_pos);
};

///
/// inline func
///
inline void PetMan::SetPlayer(CombatPlayer* player)
{
	player_ = player;
}

inline void PetMan::SetCombat(Combat* combat)
{
	combat_ = combat;
}

inline int32_t PetMan::GetPower() const
{
	return pet_power_;
}

inline int32_t PetMan::GetAttackCDTime() const
{
    return pet_attack_cd_time_;
}

};

#endif // __GAME_MODULE_COMBAT_PET_MAN_H__
