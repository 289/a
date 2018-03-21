#include "pet_man.h"
#include "combat.h"
#include "combat_npc.h"
#include "combat_player.h"
#include "combat_man.h"
#include "combat_def.h"

#include "client_proto/G2C_proto.h"

namespace combat
{

/*********************************PetMan******************************/
/*********************************PetMan******************************/
/*********************************PetMan******************************/
/*********************************PetMan******************************/
PetMan::PetMan():
	player_(NULL),
	pet_obj_(NULL),
	combat_(NULL),
	pet_power_(0),
	pet_power_flag_(false),
	pet_power_cap_(0),
	pet_power_gen_speed_(0),
    pet_attack_cd_time_(0)
{
}

PetMan::~PetMan()
{
}

bool PetMan::Load(const pet_data& data)
{
	//加载宠物能量数据
	pet_power_ = data.pet_power;
	pet_power_cap_ = data.pet_power_cap;
	pet_power_gen_speed_ = data.pet_power_gen_speed;
    pet_attack_cd_time_ = data.pet_attack_cd_time;

	//加载宠物本身数据
	pet_vec_.resize(data.pet_vec.size());
	for (size_t i = 0; i < data.pet_vec.size(); ++ i)
	{
		const pet_entry& pet = data.pet_vec[i];

		pet_vec_[i].pet_id = pet.pet_id;
		pet_vec_[i].pet_rank = pet.pet_rank;
		pet_vec_[i].pet_level = pet.pet_level;
		pet_vec_[i].pet_blevel = pet.pet_blevel;
		pet_vec_[i].pet_item_idx = pet.pet_item_idx;
		pet_vec_[i].pet_combat_pos = pet.pet_combat_pos;
	}

	if (pet_vec_.size() > 0)
	{
		//分配宠物战斗对象
		CombatNpc* npc = s_pCombatMan->AllocNPC();
		if (!npc)
			return false;

		pet_obj_ = npc;

		if (!InitPetObj(pet_vec_.begin()->pet_combat_pos))
		{
			return false;
		}

		s_pCombatMan->InsertNPCToMan(npc);
		npc->Unlock();

		//加入战场以供查询
		combat_->AddCombatPet(pet_obj_);
	}

	return true;
}

void PetMan::Save(std::vector<G2C::CombatPetInfo>& pet_list) const
{
	pet_list.resize(pet_vec_.size());
	for (size_t i = 0; i < pet_vec_.size(); ++ i)
	{
		const PetInfo& pet = pet_vec_[i];

		pet_list[i].pet_tid = pet.pet_id;
		pet_list[i].pet_rank = pet.pet_rank;
		pet_list[i].pet_combat_pos = pet.pet_combat_pos;
	}
}

void PetMan::HeartBeat()
{
	if (pet_power_flag_)
	{
		G2C::CombatPetPower packet;
		packet.pet_power = pet_power_;
		player_->SendPacket(packet);

		pet_power_flag_ = false;
	}

	if (pet_obj_)
	{
		pet_obj_->HeartBeat();
	}
}

bool PetMan::Attack(int pet_combat_pos)
{
	if (!IsPetExist(pet_combat_pos))
		return false;

	if (!player_->TestCoolDown(COOL_DOWN_INDEX_PET_ATTACK))
		return false;

	if (pet_obj_->GetPetCombatPos() != pet_combat_pos)
	{
		//切换了新宠物来施放技能
		pet_obj_->Clear();
		if (!InitPetObj(pet_combat_pos))
			return false;
	}

	if (pet_power_ < pet_obj_->GetPowerConsume())
		return false;

	combat_->RegisterInstantATB(pet_obj_);
	return true;
}

void PetMan::UnRegisterATB()
{
	if (pet_obj_)
	{
		combat_->UnRegisterATB(pet_obj_->GetID());
	}
}

bool PetMan::IsPetExist(int pet_pos) const
{
	CombatPetVec::const_iterator it = std::find_if(pet_vec_.begin(), pet_vec_.end(), pet_finder(pet_pos));
	return it != pet_vec_.end();
}

bool PetMan::InitPetObj(int pet_combat_pos)
{
	if (!IsPetExist(pet_combat_pos))
		return false;

	const PetInfo& __pet = *(std::find_if(pet_vec_.begin(), pet_vec_.end(), pet_finder(pet_combat_pos)));

	pet_obj_->SetNpcID(__pet.pet_id);
	pet_obj_->SetNpcType(CombatNpc::TYPE_NPC_PET);
	pet_obj_->Initialize();
    pet_obj_->SetLevel(__pet.pet_level);
	pet_obj_->SetPetBLevel(__pet.pet_blevel);
	pet_obj_->SetPetCombatPos(pet_combat_pos);
	pet_obj_->SetCombat(combat_);
	pet_obj_->SetOwner(player_);
    pet_obj_->SetParty(player_->GetParty());

	return true;
}

void PetMan::GenPower()
{
	int32_t __power = pet_power_;
	int32_t tmp = pet_power_ + pet_power_gen_speed_;
	if (tmp > pet_power_cap_)
		pet_power_ = pet_power_cap_;
	else
		pet_power_ = tmp;

	if (pet_power_ != __power)
	{
		pet_power_flag_ = true;
	}
}

void PetMan::ConsumePower(int32_t con)
{
	int32_t __power = pet_power_;
	int32_t tmp = pet_power_ - con;
	if (tmp >= 0)
		pet_power_ = tmp;
	else
		pet_power_ = 0;

	if (pet_power_ != __power)
	{
		pet_power_flag_ = true;
	}
}

UnitID PetMan::GetAttackPetID() const
{
	if (pet_obj_)
	{
		return pet_obj_->GetID();
	}
	return 0;
}

void PetMan::Clear()
{
	if (pet_obj_)
	{
		pet_obj_->Clear();

		combat_->RmvCombatPet(pet_obj_);

		//归还对象池
		shared::MutexLockGuard keeper(pet_obj_->GetMutex());
		s_pCombatMan->RemoveNPCFromMan(pet_obj_);
		s_pCombatMan->FreeNPC(pet_obj_);
	}

	player_ = NULL;
	combat_ = NULL;
	pet_power_ = 0;
	pet_power_flag_ = false;
	pet_power_cap_ = 0;
	pet_power_gen_speed_ = 0;
    pet_attack_cd_time_ = 0;
	pet_obj_ = NULL;

	pet_vec_.clear();
}

void PetMan::Trace() const
{
	__PRINTF("宠物信息：");
	__PRINTF("宠物能量值：%d", pet_power_);
	__PRINTF("宠物能量恢复速度：%d", pet_power_gen_speed_);
	__PRINTF("宠物攻击的冷却时间：%d", pet_attack_cd_time_);
	__PRINTF("宠物列表：");

	for (size_t i = 0; i < pet_vec_.size(); ++ i)
	{
		const PetInfo& pet = pet_vec_[i];

		__PRINTF("\t\tpet_id(%d), pet_rank(%d), pet_blevel(%d), pet_item_idx(%d), pet_combat_pos(%d)",
				pet.pet_id,
				pet.pet_rank,
				pet.pet_blevel,
				pet.pet_item_idx,
				pet.pet_combat_pos);
	}

	if (pet_obj_)
	{
		pet_obj_->Trace();
	}
}

};

