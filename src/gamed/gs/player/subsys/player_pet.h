#ifndef GAMED_GS_SUBSYS_PLAYER_PET_H_ 
#define GAMED_GS_SUBSYS_PLAYER_PET_H_ 

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"

namespace G2C
{
	struct PetEntry;
};

namespace gamed {

#define MAX_PET_LEVEL  255   // 最大宠物普通等级
#define MAX_PET_BLEVEL 255   // 最大宠物血脉等级
#define MAX_PET_RANK   10    // 最大宠物品阶

class PlayerPet : public PlayerSubSystem
{
protected:
private:
	/*宠物类型*/
	enum PetType
	{
		PET_TYPE_NORMAL,
		PET_TYPE_COMBAT,
	};

	/*宠物本体*/
	struct Pet
	{
		int32_t id;             // 宠物ID
		int32_t exp;            // 宠物经验
		int16_t level;          // 宠物等级
		int16_t blevel;         // 血脉等级
		int16_t rank;           // 宠物位阶
		int16_t identity;       // 宠物身份(0:非战宠;1:战宠)
		int16_t combat_pos;     // 宠物在战斗栏的位置
		int16_t item_idx;       // 对应的宠物物品在宠物包裹中的位置

		Pet(): id(0), exp(0), level(0), blevel(0), rank(-1), identity(0), combat_pos(-1), item_idx(-1) {}
	};
	typedef std::vector<Pet> PetVec;

	struct pet_finder
	{
		int pet_item_idx;
		pet_finder(int index): pet_item_idx(index) {}
		bool operator() (const Pet& pet) const
		{
			return pet.item_idx == pet_item_idx;
		}
	};

	struct pet_finder2
	{
		int pet_combat_pos;
		pet_finder2(int index): pet_combat_pos(index) {}
		bool operator() (const Pet& pet) const
		{
			return pet.combat_pos == pet_combat_pos;
		}
	};

	///
	/// 存盘数据
	///
	PetVec  pet_inv_;         // 宠物栏列表
	int32_t pet_power_;       // 宠物当前能量值
	int32_t pet_power_cap_;   // 宠物能量上限值

	///
	/// 运行时数据,不存盘
	///
	int32_t base_pet_power_gen_speed_;       // 宠物能量恢复速度的基础值
    int32_t enh_pet_power_gen_speed_;        // 宠物能量恢复速度的强化值
    int32_t base_pet_attack_cd_time_;        // 宠物攻击CD的基础值(ms)
    int32_t enh_scale_pet_attack_cd_time_;   // 宠物攻击CD的强化值(ms)
	int16_t combat_pet_count_;               // 战宠个数
	int16_t combat_pet_inv_size_;            // 战宠栏容量

public:
	PlayerPet(Player& player);
	virtual ~PlayerPet();

	friend void MakePetEntry(G2C::PetEntry& entry, const Pet& pet);
	friend void MakePetEntry(playerdef::PetEntry& entry, const Pet& pet);
	friend void MakePetInfo(playerdef::PetInfo& info, const Pet& pet);
	friend void MakePet(Pet& pet, const playerdef::PetEntry& entry);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	bool SaveToDB(common::PlayerPetData* pData);
	bool LoadFromDB(const common::PlayerPetData& data);

	void PlayerGetPetData();

	bool GainPet(int32_t pet_item_id, int pet_item_idx);
	void GainExp(int32_t exp);

	bool RegisterPet(const playerdef::PetEntry& pet);
	bool UnRegisterPet(int pet_item_idx);
	bool QueryPetInfo(int pet_item_idx, playerdef::PetEntry& pet) const;
	void QueryCombatPetInfo(std::vector<playerdef::PetInfo>& pet_list) const;
	void QueryPetPowerInfo(int32_t& power, int32_t& power_cap, int32_t& power_gen_speed) const;
    int32_t QueryPetAttackCDTime() const;
    int32_t GetCombatPetNum(int32_t level, int32_t blevel, int32_t rank) const;

	void SetPower(int value);
	void GainPower(int value);

	void IncPowerGenSpeed(int32_t value);
	void DecPowerGenSpeed(int32_t value);
    void IncAttackCDTime(int32_t time);
    void DecAttackCDTime(int32_t time);

protected:
	void CMDHandler_SetCombatPet(const C2G::SetCombatPet&);
	void CMDHandler_MoveCombatPet(const C2G::MoveCombatPet&);
	void CMDHandler_LevelUpBloodline(const C2G::LevelUpPetBloodline&);
	void CMDHandler_LevelUpPetPowerCap(const C2G::LevelUpPetPowerCap&);
	void CMDHandler_FullPetPower(const C2G::FullPetPower&);

private:
	bool LevelUp(int pet_item_idx);
	bool IsPetExist(int pet_item_idx) const;
	bool HasEmptyCombatSlot() const;
	void OnPetPowerLevelUp();
};

};

#endif
