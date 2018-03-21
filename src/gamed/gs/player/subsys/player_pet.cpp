#include "player_pet.h"

#include "gs/player/subsys_if.h"
#include "gs/player/fixed_item_def.h"
#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/global/gmatrix_def.h"
#include "gs/template/data_templ/pet_templ.h"
#include "gs/template/data_templ/pet_item_templ.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/templ_manager.h"

#include "gamed/client_proto/C2G_proto.h"
#include "gamed/client_proto/G2C_proto.h"
#include "shared/security/randomgen.h"

namespace gamed
{

static int LocateRank(dataTempl::TemplID pet_id, int pet_blevel)
{
	const dataTempl::PetTempl* pTpl = s_pDataTempl->QueryDataTempl<dataTempl::PetTempl>(pet_id);
	if (!pTpl)
	{
		LOG_ERROR << "非法宠物模板ID: " << pet_id;
		return -1;
	}

	int rank = -1;
	for (size_t i = 0; i < pTpl->ranks.size(); ++ i)
	{
		if (pTpl->ranks[i].blevel_limit == 0)
		{
			break;
		}

		if (pet_blevel > pTpl->ranks[i].blevel_limit)
		{
			rank = i;
			continue;
		}
		else if (pet_blevel == pTpl->ranks[i].blevel_limit)
		{
			rank = i;
			break;
		}
		else if (pet_blevel < pTpl->ranks[i].blevel_limit)
		{
			break;
		}
	}
	return rank;
}

static int32_t GetCombatPetInvCap(int32_t max_pet_power)
{
	const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);
	const dataTempl::PetCombatInvConfig& tpl = pTpl->pet_combat_inv_config;

	int combat_pet_inv_cap = 0;
	for (size_t i = 1; i < tpl.combat_inv_cap_config.size(); ++ i)
	{
		if (tpl.combat_inv_cap_config[i] > max_pet_power)
			break;
		++ combat_pet_inv_cap;
	}

	return combat_pet_inv_cap;
}

/*******************************PlayerPet************************************/
/*******************************PlayerPet************************************/
/*******************************PlayerPet************************************/
/*******************************PlayerPet************************************/

void MakePetEntry(G2C::PetEntry& entry, const PlayerPet::Pet& pet)
{
	entry.pet_tid        = pet.id;
	entry.pet_exp        = pet.exp;
	entry.pet_level      = pet.level;
	entry.pet_blevel     = pet.blevel;
	entry.pet_combat_pos = pet.combat_pos;
	entry.pet_item_idx   = pet.item_idx;
}

void MakePetEntry(playerdef::PetEntry& entry, const PlayerPet::Pet& pet)
{
	entry.pet_id       = pet.id;
	entry.pet_exp      = pet.exp;
	entry.pet_level    = pet.level;
	entry.pet_blevel   = pet.blevel;
	entry.pet_item_idx = pet.item_idx;
}

void MakePetInfo(playerdef::PetInfo& info, const PlayerPet::Pet& pet)
{
	info.pet_id         = pet.id;
	info.pet_level      = pet.level;
	info.pet_blevel     = pet.blevel;
	info.pet_rank       = pet.rank;
	info.pet_item_idx   = pet.item_idx;
	info.pet_combat_pos = pet.combat_pos;
}

void MakePet(PlayerPet::Pet& pet, const playerdef::PetEntry& entry)
{
	pet.id       = entry.pet_id;
	pet.exp      = entry.pet_exp;
	pet.level    = entry.pet_level;
	pet.blevel   = entry.pet_blevel;
	pet.item_idx = entry.pet_item_idx;
	pet.rank     = LocateRank(entry.pet_id, entry.pet_blevel);
}

PlayerPet::PlayerPet(Player& player): PlayerSubSystem(SUB_SYS_TYPE_PET, player),
	pet_power_(0),
	pet_power_cap_(0),
	base_pet_power_gen_speed_(0),
    enh_pet_power_gen_speed_(0),
    base_pet_attack_cd_time_(0),
    enh_scale_pet_attack_cd_time_(0),
	combat_pet_count_(0),
	combat_pet_inv_size_(0)
{
	SAVE_LOAD_REGISTER(common::PlayerPetData, PlayerPet::SaveToDB, PlayerPet::LoadFromDB);
}

PlayerPet::~PlayerPet()
{
	pet_inv_.clear();

	pet_power_ = 0;
	pet_power_cap_ = 0;
	base_pet_power_gen_speed_ = 0;
    enh_pet_power_gen_speed_ = 0;
    base_pet_attack_cd_time_ = 0;
    enh_scale_pet_attack_cd_time_ = 0;
	combat_pet_count_ = 0;
	combat_pet_inv_size_ = 0;
}

void PlayerPet::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::SetCombatPet, PlayerPet::CMDHandler_SetCombatPet);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MoveCombatPet, PlayerPet::CMDHandler_MoveCombatPet);
	REGISTER_NORMAL_CMD_HANDLER(C2G::LevelUpPetBloodline, PlayerPet::CMDHandler_LevelUpBloodline);
	REGISTER_NORMAL_CMD_HANDLER(C2G::LevelUpPetPowerCap, PlayerPet::CMDHandler_LevelUpPetPowerCap);
	REGISTER_NORMAL_CMD_HANDLER(C2G::FullPetPower, PlayerPet::CMDHandler_FullPetPower);
}

void PlayerPet::RegisterMsgHandler()
{
}

bool PlayerPet::LoadFromDB(const common::PlayerPetData& data)
{
	///
	/// 宠物本体数据保存在宠物包裹中，和宠物战斗位置相关的保存在宠物子系统
	///

	pet_power_ = data.pet_power;
	pet_power_cap_ = data.pet_power_cap;

	if (data.combat_pet_data.size())
	{
		shared::net::ByteBuffer buffer;
		buffer.append(data.combat_pet_data.c_str(), data.combat_pet_data.size());

		std::vector<common::PlayerPetData::PetInfo> combat_pet_list;
		buffer >> combat_pet_list;

		for (size_t i = 0; i < combat_pet_list.size(); ++ i)
		{
			const common::PlayerPetData::PetInfo& info = combat_pet_list[i];

			PetVec::iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(info.pet_item_idx));
            if (it == pet_inv_.end() || it->id != info.pet_id)
            {
                continue;
            }
			//ASSERT(it != pet_inv_.end());
			//ASSERT(it->id == info.pet_id);

			it->identity = PET_TYPE_COMBAT;
			it->combat_pos = info.pet_combat_pos;

			++ combat_pet_count_;
		}
	}

	const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);

	const dataTempl::PetLvlUpPowerConfig& tpl = pTpl->pet_lvlup_power_config;
	base_pet_power_gen_speed_ = tpl.pet_power_gen_speed;
    base_pet_attack_cd_time_  = 5000; // 单位:毫秒

	combat_pet_inv_size_ = GetCombatPetInvCap(pet_power_cap_);
	ASSERT(combat_pet_inv_size_ >= combat_pet_count_);
	return true;
}

bool PlayerPet::SaveToDB(common::PlayerPetData* pData)
{
	pData->pet_power = pet_power_;
	pData->pet_power_cap = pet_power_cap_;

	if (combat_pet_count_ > 0)
	{
		std::vector<common::PlayerPetData::PetInfo> combat_pet_list;
		for (size_t i = 0; i < pet_inv_.size(); ++ i)
		{
			const Pet& pet = pet_inv_[i];
			if (pet.identity == PET_TYPE_COMBAT)
			{
				common::PlayerPetData::PetInfo info;
				info.pet_id         = pet.id;
				info.pet_item_idx   = pet.item_idx;
				info.pet_combat_pos = pet.combat_pos;

				combat_pet_list.push_back(info);
			}
		}

		if (combat_pet_list.size() > 0)
		{
			shared::net::ByteBuffer buffer;
			buffer << combat_pet_list;

			pData->combat_pet_data.append((const char*)(buffer.contents()), buffer.size());
		}
	}

	return true;
}

void PlayerPet::PlayerGetPetData()
{
	G2C::PetData packet;
	packet.pet_power = pet_power_;
	packet.pet_power_cap = pet_power_cap_;

	for (size_t i = 0; i < pet_inv_.size(); ++ i)
	{
		Pet& __pet = pet_inv_[i];

		G2C::PetEntry entry;
		MakePetEntry(entry, __pet);
		packet.pet_list.push_back(entry);
	}

	SendCmd(packet);
}

bool PlayerPet::GainPet(int32_t pet_item_id, int pet_item_idx)
{
	///
	/// 调用本接口表示玩家游戏中真正获得宠物
	/// 所以需要通知客户度
	///

	const dataTempl::PetItemTempl* pTpl = s_pDataTempl->QueryDataTempl<dataTempl::PetItemTempl>(pet_item_id);
	if (!pTpl)
		return false;

	Pet pet;
	pet.id         = pTpl->pet_id;
	pet.exp        = 0;
	pet.level      = pTpl->pet_level;
	pet.blevel     = pTpl->pet_blevel;
	pet.identity   = PET_TYPE_NORMAL;
	pet.combat_pos = -1;
	pet.item_idx   = pet_item_idx;
	pet.rank       = LocateRank(pTpl->pet_id, pTpl->pet_blevel);

	pet_inv_.push_back(pet);

	G2C::GainPet packet;
	MakePetEntry(packet.pet, pet);
	SendCmd(packet);

	return true;
}

bool PlayerPet::RegisterPet(const playerdef::PetEntry& pet)
{
	///
	/// 调用本接口并不是指玩家获得宠物,
	/// 而是玩家上线时使用宠物物品来初始化宠物子系统,
	/// 所以这里不需要通知客户端
	///

	int pet_item_idx = pet.pet_item_idx;
	if (IsPetExist(pet_item_idx))
	{
		LOG_ERROR << "玩家 " << player_.role_id() << " 添加宠物失败，宠物物品IDX：" << pet_item_idx; 
		ASSERT(false);
		return false;
	}

	Pet __pet;
	MakePet(__pet, pet);
	pet_inv_.push_back(__pet);
	return true;
}

bool PlayerPet::UnRegisterPet(int pet_item_idx)
{
	PetVec::iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx));
	if (it == pet_inv_.end())
	{
		LOG_ERROR << "玩家 " << player_.role_id() << " 删除宠物失败，宠物物品IDX：" << pet_item_idx;
		ASSERT(false);
		return false;
	}

	if (it->identity == PET_TYPE_COMBAT)
	{
		-- combat_pet_count_;
		ASSERT(combat_pet_count_ >= 0);
	}

	pet_inv_.erase(it);

	G2C::LostPet packet;
	packet.pet_item_inv_idx = pet_item_idx;
	SendCmd(packet);
	return true;
}

bool PlayerPet::QueryPetInfo(int pet_item_idx, playerdef::PetEntry& pet) const
{
	PetVec::const_iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx));
	if (it == pet_inv_.end())
		return false;

	MakePetEntry(pet, *it);
	return true;
}

void PlayerPet::QueryCombatPetInfo(std::vector<playerdef::PetInfo>& list) const
{
	for (size_t i = 0; i < pet_inv_.size(); ++ i)
	{
		const Pet& pet = pet_inv_[i];
		if (pet.identity != PET_TYPE_COMBAT)
			continue;

		playerdef::PetInfo  info;
		MakePetInfo(info, pet);
		list.push_back(info);
	}
}

int32_t PlayerPet::GetCombatPetNum(int32_t level, int32_t blevel, int32_t rank) const
{
    int32_t num = 0;
	for (size_t i = 0; i < pet_inv_.size(); ++ i)
	{
		const Pet& pet = pet_inv_[i];
		if (pet.identity == PET_TYPE_COMBAT && pet.level >= level && pet.blevel >= blevel && pet.rank >= rank)
        {
            ++num;
        }
	}
    return num;
}

void PlayerPet::QueryPetPowerInfo(int32_t& power, int32_t& power_cap, int32_t& power_gen_speed) const
{
	power = pet_power_;
	power_cap = pet_power_cap_;
	power_gen_speed = base_pet_power_gen_speed_ + enh_pet_power_gen_speed_;
}

int32_t PlayerPet::QueryPetAttackCDTime() const
{
    return base_pet_attack_cd_time_ * (1.0f + (double)(enh_scale_pet_attack_cd_time_)/10000.0f);
}

void PlayerPet::GainPower(int32_t power)
{
	int32_t tmp = pet_power_ + power;
	if (tmp <= pet_power_cap_)
		pet_power_ = tmp;
	else
		pet_power_ = pet_power_cap_;

	G2C::PetPower packet;
	packet.power = pet_power_;
	SendCmd(packet);
}

void PlayerPet::SetPower(int value)
{
	if (value < 0 || value > pet_power_cap_)
		return;

	pet_power_ = value;

	G2C::PetPower packet;
	packet.power = pet_power_;
	SendCmd(packet);
}

void PlayerPet::IncPowerGenSpeed(int value)
{
    enh_pet_power_gen_speed_ += value;
}

void PlayerPet::DecPowerGenSpeed(int value)
{
    enh_pet_power_gen_speed_ -= value;
}

void PlayerPet::IncAttackCDTime(int32_t time)
{
    enh_scale_pet_attack_cd_time_ += time;
}

void PlayerPet::DecAttackCDTime(int32_t time)
{
    enh_scale_pet_attack_cd_time_ -= time;
    if (enh_scale_pet_attack_cd_time_ < -10000)
    {
        enh_scale_pet_attack_cd_time_ = -10000;
    }
}

void PlayerPet::CMDHandler_SetCombatPet(const C2G::SetCombatPet& cmd)
{
	int opt = cmd.op_type;
	int pet_item_idx = cmd.pet_item_inv_idx;
	int pet_combat_pos = cmd.pet_combat_inv_pos;
	if (pet_item_idx < 0)
		return;

	if (opt == C2G::SetCombatPet::OP_PUSH)
	{
		//出战
		if (pet_combat_pos < 0 || pet_combat_pos >= combat_pet_inv_size_)
			return;

		if (!HasEmptyCombatSlot())
			return;

		PetVec::iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx));
		if (it == pet_inv_.end())
			return;

		PetVec::iterator it2 = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder2(pet_combat_pos));
		if (it2 != pet_inv_.end())
		{
			if (it2->item_idx == pet_item_idx)
				return;

			-- combat_pet_count_;

			it2->combat_pos = -1;
			it2->identity = PET_TYPE_NORMAL;
		}

		if (it->identity != PET_TYPE_COMBAT)
		{
			++ combat_pet_count_;
		}

		it->combat_pos = pet_combat_pos;
		it->identity   = PET_TYPE_COMBAT;

		G2C::SetCombatPet_Re packet;
		packet.result  = 1;
		packet.op_type = opt;
		packet.pet_item_inv_idx   = pet_item_idx;
		packet.pet_combat_inv_pos = pet_combat_pos;
		SendCmd(packet);
	}
	else if (opt == C2G::SetCombatPet::OP_POP)
	{
		//休战
		PetVec::iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx));
		if (it == pet_inv_.end() || it->identity == PET_TYPE_NORMAL)
			return;

		it->combat_pos = -1;
		it->identity   = PET_TYPE_NORMAL;

		-- combat_pet_count_;

		G2C::SetCombatPet_Re packet;
		packet.result = 1;
		packet.op_type = opt;
		packet.pet_item_inv_idx = pet_item_idx;
		packet.pet_combat_inv_pos = -1;
		SendCmd(packet);
	}
}

void PlayerPet::CMDHandler_MoveCombatPet(const C2G::MoveCombatPet& cmd)
{
	int src_combat_pos  = cmd.src_combat_inv_pos;
	int dest_combat_pos = cmd.dest_combat_inv_pos;

	if (src_combat_pos < 0 ||
		dest_combat_pos < 0 ||
		src_combat_pos == dest_combat_pos ||
		src_combat_pos >= combat_pet_inv_size_ ||
		dest_combat_pos >= combat_pet_inv_size_)
		return;

	PetVec::iterator it_src = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder2(src_combat_pos));
	if (it_src == pet_inv_.end())
		return;

	PetVec::iterator it_dest = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder2(dest_combat_pos));
	if (it_dest != pet_inv_.end())
	{
		it_dest->combat_pos = src_combat_pos;
	}

	it_src->combat_pos = dest_combat_pos;

	G2C::MoveCombatPet_Re packet;
	packet.result = 1;
	packet.src_combat_inv_pos = src_combat_pos;
	packet.dest_combat_inv_pos = dest_combat_pos;
	SendCmd(packet);
}

void PlayerPet::CMDHandler_LevelUpBloodline(const C2G::LevelUpPetBloodline& cmd)
{
	int pet_item_idx = cmd.pet_item_inv_idx;
	int inc_prob_item_id = cmd.inc_prob_item_id;
	int inc_prob_item_count = cmd.inc_prob_item_count;

	if (pet_item_idx < 0 ||
		inc_prob_item_id < 0 ||
		inc_prob_item_count < 0)
		return;

	///
	/// 检查升级条件
	///

	if (!IsPetExist(pet_item_idx))
		return;

	PetVec::iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx));
	if (it->blevel >= MAX_PET_BLEVEL)
		return; //已满级

	const dataTempl::GlobalConfigTempl *pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);

	const dataTempl::PetLvlUpBloodConfig& tpl = pTpl->pet_lvlup_blood_config;
	if (it->blevel >= (int)(tpl.lvlup_array.size()))
		return; //已满级

	//检查提升升级概率的道具
	if (!player_.CheckItem(inc_prob_item_id, inc_prob_item_count))
		return;

	//检查升级金钱
	const dataTempl::PetLvlUpBloodConfig::LvlupNode& node = tpl.lvlup_array[it->blevel];
	if (player_.GetMoney() < node.lvlup_money)
		return;

	//检查升级道具
	if (!player_.CheckItem(node.lvlup_item_id, node.lvlup_item_count))
		return;

	//检查提升概率的道具
	if (inc_prob_item_id > 0 && inc_prob_item_id != node.inc_prob_item_id)
		return;

	if (inc_prob_item_count > 0 && (inc_prob_item_count % node.inc_prob_item_count) != 0)
		return;

	///
	/// 开始升级
	///

	//扣钱,升级道具,提升概率道具
	if (node.lvlup_money > 0)
		player_.SpendMoney(node.lvlup_money);
	if (node.lvlup_item_id > 0)
		player_.TakeOutItem(node.lvlup_item_id, node.lvlup_item_count);
	if (inc_prob_item_count > 0)
		player_.TakeOutItem(node.inc_prob_item_id, inc_prob_item_count);

	int result = 0;
	int32_t inc_prob = 100 * (inc_prob_item_count / node.inc_prob_item_count);
	if (shared::net::RandomGen::RandUniform(1, 10000) < (node.lvlup_prob + inc_prob))
	{
		result = 1;
		++ it->blevel;

		//更新宠物位阶
		it->rank = LocateRank(it->id, it->blevel);

		//更新宠物包裹
		player_.UpdatePetItem(it->item_idx);

		LOG_INFO << "玩家 " << player_.role_id() << " 的宠物(" << it->id << ")血脉升级成功，当前血脉等级 " << it->blevel << " 级";
	}
	else
	{
		LOG_INFO << "玩家 " << player_.role_id() << " 的宠物(" << it->id << ")血脉升级失败";
	}

	G2C::LevelUpBloodline_Re packet;
	packet.result = result;
	packet.blevel = it->blevel;
	packet.pet_item_inv_idx = pet_item_idx;
	SendCmd(packet);
}

void PlayerPet::CMDHandler_LevelUpPetPowerCap(const C2G::LevelUpPetPowerCap& cmd)
{
	const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);
	const dataTempl::PetLvlUpPowerConfig& tpl = pTpl->pet_lvlup_power_config;

	///
	/// 检查能量升级条件
	///
	int times = cmd.lvlup_times;
	if (times <= 0)
		return;

	if (pet_power_cap_ >= tpl.max_pet_power_up_limit)
		return;

    int32_t item_spend = 0;
    int32_t cash_spend = 0;
    int32_t item_lvlup_times = 0;

	if (player_.CountItem(tpl.item_need_on_lvlup) < times * tpl.item_count_on_lvlup)
	{
		//道具不够,用元宝补充        
        item_lvlup_times = player_.CountItem(tpl.item_need_on_lvlup) / tpl.item_count_on_lvlup;
		item_spend = item_lvlup_times * tpl.item_count_on_lvlup;
	}
    else
    {
        item_lvlup_times = times;
    }
    item_spend = item_lvlup_times * tpl.item_count_on_lvlup;;
	cash_spend = (times - item_lvlup_times) * tpl.cash_need_on_lvlup;

	if (player_.GetCash() < cash_spend)
	{
		//道具和元宝都不够,非法请求
		return;
	}

	///
	/// 提升能量上限
	///
	int32_t __times = 0;
	std::vector<int32_t> offset_list;
	for (int i = 1; i <= times; ++ i)
	{
		//计算能量上限的提升值
		int32_t power_inc_on_lvlup = 0;
		int32_t rand_num = shared::net::RandomGen::RandUniform(1, 10000);
		const gmatrixdef::PetPowerLvlUpConfig& cfg = Gmatrix::GetPetPowerLvlUpConfig();
		for (size_t i = 0; i < cfg.power_lvlup_tbl.size(); ++ i)
		{
			if (rand_num <= cfg.power_lvlup_tbl[i].probability)
			{
				power_inc_on_lvlup = cfg.power_lvlup_tbl[i].power_inc_on_lvlup;
				break;
			}
		}

		//提升能量上限
		int32_t offset = power_inc_on_lvlup;
		int32_t tmp    = pet_power_cap_ + power_inc_on_lvlup;
		if (tmp >= tpl.max_pet_power_up_limit)
		{
			offset = tpl.max_pet_power_up_limit - pet_power_cap_;
		}

		++ __times;

		pet_power_cap_ += offset;

		ASSERT(pet_power_cap_ <= tpl.max_pet_power_up_limit);

		offset_list.push_back(offset);

		if (pet_power_cap_ >= tpl.max_pet_power_up_limit)
			break;
	}

	///
	/// 重新计算升级消耗
	///
	if (__times < times)
	{
		if (__times * tpl.item_count_on_lvlup <= item_spend)
		{
            item_lvlup_times = __times;
		}
        item_spend = item_lvlup_times * tpl.item_count_on_lvlup;;
        cash_spend = (__times - item_lvlup_times) * tpl.cash_need_on_lvlup;
    }

	///
	/// 扣除升级消耗
	///
	if (item_spend > 0)
		player_.TakeOutItem(tpl.item_need_on_lvlup, item_spend);
	if (cash_spend > 0)
		player_.UseCash(cash_spend);

	//刷新宠物出战栏大小
	combat_pet_inv_size_ = GetCombatPetInvCap(pet_power_cap_);
	ASSERT(combat_pet_inv_size_ >= combat_pet_count_);

	LOG_INFO << "玩家 " << player_.role_id() << " 的宠物能量上限升级，宠物能量上限提升到 " << pet_power_cap_ << ", 战宠栏大小：" << combat_pet_inv_size_;

	G2C::LevelUpPetPowerCap_Re packet;
	packet.result = 1;
	packet.list = offset_list;
	packet.pet_power_cap = pet_power_cap_;
	SendCmd(packet);
}

void PlayerPet::CMDHandler_FullPetPower(const C2G::FullPetPower& cmd)
{
	if (pet_power_ >= pet_power_cap_)
		return;

	const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);
	const dataTempl::PetLvlUpPowerConfig& tpl = pTpl->pet_lvlup_power_config;

	if (!player_.CheckCash(tpl.cash_need_full_power))
		return;

	pet_power_ = pet_power_cap_;
	player_.UseCash(tpl.cash_need_full_power);

	G2C::FullPetPower_Re packet;
	packet.result = 1;
	packet.pet_power = pet_power_;
	SendCmd(packet);
}

void PlayerPet::GainExp(int32_t exp)
{
	if (exp <= 0)
		return;

	for (size_t i = 0; i < pet_inv_.size(); ++ i)
	{
		Pet& pet = pet_inv_[i];
		if (pet.identity != PET_TYPE_COMBAT)
			continue;

		pet.exp += exp;

		LOG_INFO << "玩家 " << player_.role_id() << " 的宠物(" << pet.id << ")获得经验：" << exp << ", 当前经验：" << pet.exp;

		if (LevelUp(pet.item_idx))
		{
			G2C::PetLevelUp packet;
			packet.pet_item_idx = pet.item_idx;
			packet.pet_level    = pet.level;
			packet.pet_exp      = pet.exp;
			SendCmd(packet);
		}
		else
		{
			G2C::PetGainExp packet;
			packet.pet_item_idx = pet.item_idx;
			packet.pet_exp      = pet.exp;
			SendCmd(packet);
		}

		//更新宠物包裹
		player_.UpdatePetItem(pet.item_idx);
	}
}

bool PlayerPet::LevelUp(int pet_item_idx)
{
	if (pet_item_idx < 0 || !IsPetExist(pet_item_idx))
	{
		ASSERT(false);
		return false;
	}

	const dataTempl::GlobalConfigTempl *pTpl = s_pDataTempl->QueryGlobalConfigTempl();
	ASSERT(pTpl);
	const dataTempl::PetLvlUpExpConfig& tpl = pTpl->pet_lvlup_exp_config;

	PetVec::iterator it = std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx));
	ASSERT(it != pet_inv_.end());

	bool is_level_up = false;
	for (;;)
	{
		if (it->level >= (int)(tpl.exp_array.size()))
			break; //满级

		int32_t need_exp = tpl.exp_array[it->level];
		if (need_exp == 0)
			break; //无效

		if (need_exp > it->exp)
			break; //经验不够

		is_level_up = true;
		it->exp -= need_exp;
		it->level += 1;

		LOG_INFO << "玩家 " << player_.role_id() << " 的宠物(" << it->id << ")升级到 " << it->level << " 级";

		if (it->level >= MAX_PET_LEVEL)
		{
			it->exp = 0;
			break;
		}
	}

	return is_level_up;
}

bool PlayerPet::IsPetExist(int pet_item_idx) const
{
	return std::find_if(pet_inv_.begin(), pet_inv_.end(), pet_finder(pet_item_idx)) != pet_inv_.end();
}

bool PlayerPet::HasEmptyCombatSlot() const
{
	return combat_pet_count_ < combat_pet_inv_size_;
}

};
