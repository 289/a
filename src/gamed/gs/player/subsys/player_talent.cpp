#include "player_talent.h"

#include "gs/player/subsys_if.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/talent_templ.h"
#include "gs/template/data_templ/talent_group_templ.h"

#include "gamed/client_proto/G2C_proto.h"

namespace gamed
{

static int GetMaxTalentLevel(int32_t talent_id)
{
	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<dataTempl::TalentTempl>(talent_id);
	ASSERT(pTpl);

	int level = 0;
	for (size_t i = 0; i < pTpl->lvlup_table.size(); ++ i)
	{
		const dataTempl::TalentTempl::LvlUpEntry& entry = pTpl->lvlup_table[i];
		if (i == 0 || entry.passive_skill_id > 0 || entry.ratio > 0)
			++ level;
		else
			break;
	}
	return level - 1;
}

static bool CheckTalentValidity(int32_t talent_group_id, int32_t talent_id)
{
	const dataTempl::TalentGroupTempl* pTpl = s_pDataTempl->QueryDataTempl<dataTempl::TalentGroupTempl>(talent_group_id);
	if (!pTpl)
		return false;

	for (size_t i = 0; i < pTpl->talent_list.size(); ++ i)
		if (talent_id == pTpl->talent_list[i])
			return true;

	return false;
}

/*****************************PlayerTalent******************************/
/*****************************PlayerTalent******************************/
/*****************************PlayerTalent******************************/
/*****************************PlayerTalent******************************/
PlayerTalent::PlayerTalent(Player& player): PlayerSubSystem(SUB_SYS_TYPE_TALENT, player)
{
	SAVE_LOAD_REGISTER(common::PlayerTalentData, PlayerTalent::SaveToDB, PlayerTalent::LoadFromDB);
}

PlayerTalent::~PlayerTalent()
{
	talent_group_vec_.clear();
}

void PlayerTalent::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::LevelUpTalent,  PlayerTalent::CMDHandler_LevelUpTalent);
}

bool PlayerTalent::LoadFromDB(const common::PlayerTalentData& data)
{
	for (size_t i = 0; i < data.talent_list.size(); ++ i)
	{
		const common::PlayerTalentData::talent_entry& entry = data.talent_list[i];

		if (entry.talent_id == 0)
		{
			if (IsTalentGroupExist(entry.talent_group_id))
			{
				ASSERT(false);
			}

			TalentGroup group;
			group.talent_group_id = entry.talent_group_id;
			talent_group_vec_.push_back(group);
		}
		else
		{
			TalentEntry talent;
			talent.talent_id = entry.talent_id;
			talent.level     = entry.talent_level;
			talent.tmp_level = 0;
			talent.max_level = GetMaxTalentLevel(entry.talent_id);

			TalentGroup* pGroup = NULL;
			if (!GetTalentGroup(entry.talent_group_id, pGroup))
			{
				TalentGroup group;
				group.talent_group_id = entry.talent_group_id;
				group.talent_list.push_back(talent);
				talent_group_vec_.push_back(group);
			}
			else
			{
				ASSERT(pGroup);
				pGroup->talent_list.push_back(talent);
			}

            ActivateTalent(entry.talent_group_id, entry.talent_id);
		}
	}
	return true;
}

bool PlayerTalent::SaveToDB(common::PlayerTalentData* pData)
{
	for (size_t i = 0; i < talent_group_vec_.size(); ++ i)
	{
		const TalentGroup& __group = talent_group_vec_[i];

		if (__group.talent_list.empty())
		{
			common::PlayerTalentData::talent_entry entry;

			entry.talent_group_id = __group.talent_group_id;
			entry.talent_id       = 0;
			entry.talent_level    = 0;
			pData->talent_list.push_back(entry);
		}
		else
		{
			for (size_t j = 0; j < __group.talent_list.size(); ++ j)
			{
				common::PlayerTalentData::talent_entry entry;

				entry.talent_group_id = __group.talent_group_id;
				entry.talent_id       = __group.talent_list[j].talent_id;
				entry.talent_level    = __group.talent_list[j].level;
				pData->talent_list.push_back(entry);
			}
		}
	}
	return true;
}

void PlayerTalent::CMDHandler_LevelUpTalent(const C2G::LevelUpTalent& cmd)
{
	int32_t talent_group_id = cmd.talent_group_id;
	int32_t talent_id = cmd.talent_id;

	if (talent_group_id <= 0 || talent_id <= 0)
		return;

	const dataTempl::TalentGroupTempl* pGroupTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentGroupTempl>(talent_group_id);
	if (!pGroupTpl)
		return;

	const dataTempl::TalentTempl* pTalentTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentTempl>(talent_id);
	if (!pTalentTpl)
		return;

	if (!CheckTalentValidity(talent_group_id, talent_id))
		return;

	if (!IsTalentGroupExist(talent_group_id))
		return;

	if (!ManualLevelUp(talent_group_id, talent_id))
		return;

	G2C::LevelUpTalent_Re packet;
	packet.result = 1;
	packet.talent_group_id = talent_group_id;
	packet.talent_id = talent_id;
	SendCmd(packet);
}

bool PlayerTalent::OpenTalentGroup(int32_t talent_group_id)
{
	if (IsTalentGroupExist(talent_group_id))
		return false;

	if (!s_pDataTempl->QueryDataTempl<dataTempl::TalentGroupTempl>(talent_group_id))
		return false;

	TalentGroup __group;
	__group.talent_group_id = talent_group_id;
	__group.talent_list.clear();
	talent_group_vec_.push_back(__group);

	G2C::OpenTalentGroup packet;
	packet.talent_group_id = talent_group_id;
	SendCmd(packet);

	LOG_INFO << "玩家 " << player_.role_id() << " 开启天赋组, talent_group_id: " << talent_group_id;

	return true;
}

bool PlayerTalent::OpenTalent(int32_t talent_group_id, int32_t talent_id)
{
	TalentGroupVec::const_iterator it_group = std::find_if(talent_group_vec_.begin(), talent_group_vec_.end(), talent_group_finder(talent_group_id));
	if (it_group == talent_group_vec_.end())
        return false;

	ASSERT(CheckTalentValidity(talent_group_id, talent_id));

	TalentEntry talent;
	talent.talent_id = talent_id;
	talent.level     = 0;
	talent.tmp_level = 0;
	talent.max_level = GetMaxTalentLevel(talent_id);

	it_group->talent_list.push_back(talent);

    return true;
}

bool PlayerTalent::CloseTalentGroup(int32_t talent_group_id)
{
	if (!IsTalentGroupExist(talent_group_id))
		return false;

	TalentGroupVec::iterator it = std::find_if(talent_group_vec_.begin(), talent_group_vec_.end(), talent_group_finder(talent_group_id));
	if (it == talent_group_vec_.end())
        return false;

	talent_group_vec_.erase(it);

	LOG_INFO << "玩家 " << player_.role_id() << " 关闭天赋组, talent_group_id: " << talent_group_id;

	return true;
}

bool PlayerTalent::ManualLevelUp(int32_t talent_group_id, int32_t talent_id)
{
	ASSERT(IsTalentGroupExist(talent_group_id));
	ASSERT(CheckTalentValidity(talent_group_id, talent_id));

	int level = 0;
	TalentEntry* pTalent = NULL;
	if (GetTalent(talent_group_id, talent_id, pTalent))
	{
		//天赋已开启
		ASSERT(pTalent);
		if (pTalent->level >= pTalent->max_level)
			return false; //满级

		level = pTalent->level;
	}

	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentTempl>(talent_id);
	if (!pTpl)
		return false;

	const dataTempl::TalentTempl::LvlUpEntry& lvlup_entry = pTpl->lvlup_table[level];

	//检查升级条件
	if (lvlup_entry.lvlup_mode != dataTempl::TalentTempl::LVLUP_MODE_MANUAL)
		return false;

	if (!player_.CheckMoney(lvlup_entry.money_need_on_lvlup))
		return false;

	if (!player_.CheckCash(lvlup_entry.cash_need_on_lvlup))
		return false;

	for (size_t i = 0; i < lvlup_entry.items_need_on_lvlup.size(); ++ i)
	{
		if (lvlup_entry.items_need_on_lvlup[i].item_id <= 0 || lvlup_entry.items_need_on_lvlup[i].item_count <= 0)
			continue;

		if (!player_.CheckItem(lvlup_entry.items_need_on_lvlup[i].item_id, lvlup_entry.items_need_on_lvlup[i].item_count))
			return false;
	}

	//扣除升级消耗
	player_.SpendMoney(lvlup_entry.money_need_on_lvlup);
	player_.SpendScore(lvlup_entry.score_need_on_lvlup);
	player_.UseCash(lvlup_entry.cash_need_on_lvlup);
	for (size_t i = 0; i < lvlup_entry.items_need_on_lvlup.size(); ++ i)
	{
		if (lvlup_entry.items_need_on_lvlup[i].item_id <= 0 || lvlup_entry.items_need_on_lvlup[i].item_count <= 0)
            continue;
		player_.TakeOutItem(lvlup_entry.items_need_on_lvlup[i].item_id, lvlup_entry.items_need_on_lvlup[i].item_count);
	}

	if (!pTalent)
	{
		//天赋未开启
		OpenTalent(talent_group_id, talent_id);
		GetTalent(talent_group_id, talent_id, pTalent);
	}

	//先让天赋生效
	DeActivateTalent(talent_group_id, talent_id);

	++ pTalent->level;

	//再让天赋生效
	ActivateTalent(talent_group_id, talent_id);

	NotifyClient(talent_group_id, talent_id, pTalent->level, pTalent->tmp_level);

	LOG_INFO << "玩家 " << player_.role_id() << "手动升级天赋成功, "
		     << ", talent_group_id: "  << talent_group_id
			 << ", talent_id: "        << talent_id
			 << ", talent_level: "     << pTalent->level
			 << ", talent_tmp_level: " << pTalent->tmp_level;

	return true;
}

bool PlayerTalent::SystemLevelUp(int32_t talent_group_id, int32_t talent_id)
{
	if (!IsTalentGroupExist(talent_group_id))
		return false;

	if (!CheckTalentValidity(talent_group_id, talent_id))
		return false;

	int level = 0;
	TalentEntry* pTalent = NULL;
	if (GetTalent(talent_group_id, talent_id, pTalent))
	{
		//天赋已经开启
		ASSERT(pTalent);
		if (pTalent->level >= pTalent->max_level)
			return false; //满级

		level = pTalent->level;
	}

	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentTempl>(talent_id);
	if (!pTpl)
		return false;

	const dataTempl::TalentTempl::LvlUpEntry& lvlup_entry = pTpl->lvlup_table[level];
	if (lvlup_entry.lvlup_mode != dataTempl::TalentTempl::LVLUP_MODE_SYSTEM)
		return false;

	if (!pTalent)
	{
		//天赋未开启
		OpenTalent(talent_group_id, talent_id);
		GetTalent(talent_group_id, talent_id, pTalent);
	}

	DeActivateTalent(talent_group_id, talent_id);

	++ pTalent->level;

	ActivateTalent(talent_group_id, talent_id);

	NotifyClient(talent_group_id, talent_id, pTalent->level, pTalent->tmp_level);

	LOG_INFO << "玩家 " << player_.role_id() << "的天赋系统升级, "
		     << ", talent_group_id: "  << talent_group_id
			 << ", talent_id: "        << talent_id
			 << ", talent_level: "     << pTalent->level
			 << ", talent_tmp_level: " << pTalent->tmp_level;

	return true;
}

bool PlayerTalent::IncTempLevel(int32_t talent_group_id, int32_t talent_id)
{
	TalentEntry* pTalent = NULL;
	if (!GetTalent(talent_group_id, talent_id, pTalent))
		return false;

	ASSERT(pTalent);
	if (pTalent->level >= pTalent->max_level)
		return false; //满级

	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentTempl>(talent_id);
	if (!pTpl)
		return false;

	DeActivateTalent(talent_group_id, talent_id);

	++ pTalent->tmp_level;

	ActivateTalent(talent_group_id, talent_id);

	NotifyClient(talent_group_id, talent_id, pTalent->level, pTalent->tmp_level);

	LOG_INFO << "玩家 " << player_.role_id() << "的天赋临时等级被提升, "
		     << ", talent_group_id: "  << talent_group_id
			 << ", talent_id: "        << talent_id
			 << ", talent_level: "     << pTalent->level
			 << ", talent_tmp_level: " << pTalent->tmp_level;

	return true;
}

bool PlayerTalent::DecTempLevel(int32_t talent_group_id, int32_t talent_id)
{
	TalentEntry* pTalent = NULL;
	if (!GetTalent(talent_group_id, talent_id, pTalent))
		return false;

	ASSERT(pTalent);
	if (pTalent->tmp_level <= 0)
		return false;

	DeActivateTalent(talent_group_id, talent_id);

	-- pTalent->tmp_level;

	ActivateTalent(talent_group_id, talent_id);

	NotifyClient(talent_group_id, talent_id, pTalent->level, pTalent->tmp_level);

	LOG_INFO << "玩家 " << player_.role_id() << "的天赋临时等级被降低, "
		     << ", talent_group_id: "  << talent_group_id
			 << ", talent_id: "        << talent_id
			 << ", talent_level: "     << pTalent->level
			 << ", talent_tmp_level: " << pTalent->tmp_level;

	return true;
}

int32_t PlayerTalent::GetTalentLevel(int32_t talent_group_id, int32_t talent_id) const
{
	TalentEntry* pTalent = NULL;
	if (!GetTalent(talent_group_id, talent_id, pTalent))
		return 0;
    int32_t level = pTalent->level + pTalent->tmp_level;
    return level >= pTalent->max_level ? pTalent->max_level : level;
}

void PlayerTalent::ActivateTalent(int32_t talent_group_id, int32_t talent_id) const
{
	TalentEntry* pTalent = NULL;
	if (!GetTalent(talent_group_id, talent_id, pTalent))
		return;

	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentTempl>(talent_id);
	ASSERT(pTpl);

	int level = pTalent->level + pTalent->tmp_level;
	if (level > pTalent->max_level)
	{
		level = pTalent->max_level;
	}

	const dataTempl::TalentTempl::LvlUpEntry& entry = pTpl->lvlup_table[level];

    //对玩家属性的影响
	int ratio = entry.ratio;
	for (size_t idx = 0; idx < pTpl->inc_prop_base.size(); ++ idx)
	{
		int32_t __prop = pTpl->inc_prop_base[idx] * (ratio / 10000);
		if (__prop > 0)
		{
			player_.IncPropPoint(idx, __prop);
		}
	}

    //修正瞄类视觉恢复速度
    player_.value_prop().IncCatVisionGenSpeed(entry.adjust_value_cat_vision_gen_speed);
    //修正瞄类视觉恢复间隔
    player_.value_prop().DecCatVisionGenInterval(entry.adjust_value_cat_vision_gen_interval);
    //修正战斗掉落经验
    player_.value_prop().IncAwardExpAdjustFactor(entry.adjust_scale_combat_award_exp);
    //修正战斗掉落金钱
    player_.value_prop().IncAwardMoneyAdjustFactor(entry.adjust_scale_combat_award_money);
    //修正宠物能量恢复速度
    player_.IncPetPowerGenSpeed(entry.adjust_value_pet_power_gen_speed);
    //修正宠物攻击的冷却时间
    player_.DecPetAttackCDTime(entry.adjust_scale_pet_skill_cd_time);
    if (entry.adjust_skill_tree_id != 0 && entry.adjust_value_skill_temp_level > 0)
    {
        player_.LevelUpSkill(entry.adjust_skill_tree_id, entry.adjust_value_skill_temp_level, LVLUP_MODE_TALENT);
    }
}

void PlayerTalent::DeActivateTalent(int32_t talent_group_id, int32_t talent_id) const
{
	TalentEntry* pTalent = NULL;
	if (!GetTalent(talent_group_id, talent_id, pTalent))
		return;

	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<const dataTempl::TalentTempl>(talent_id);
	ASSERT(pTpl);

	int level = pTalent->level + pTalent->tmp_level;
	if (level > pTalent->max_level)
	{
		level = pTalent->max_level;
	}

	const dataTempl::TalentTempl::LvlUpEntry& entry = pTpl->lvlup_table[level];

    //对玩家属性的影响
	int ratio = entry.ratio;
	for (size_t idx = 0; idx < pTpl->inc_prop_base.size(); ++ idx)
	{
		int32_t __prop = pTpl->inc_prop_base[idx] * (ratio / 10000);
		if (__prop > 0)
		{
			player_.DecPropPoint(idx, __prop);
		}
	}

    //修正瞄类视觉恢复速度
    player_.value_prop().DecCatVisionGenSpeed(entry.adjust_value_cat_vision_gen_speed);
    //修正瞄类视觉恢复间隔
    player_.value_prop().IncCatVisionGenInterval(entry.adjust_value_cat_vision_gen_interval);
    //修正战斗掉落经验
    player_.value_prop().DecAwardExpAdjustFactor(entry.adjust_scale_combat_award_exp);
    //修正战斗掉落金钱
    player_.value_prop().DecAwardMoneyAdjustFactor(entry.adjust_scale_combat_award_money);
    //修正宠物能量恢复速度
    player_.DecPetPowerGenSpeed(entry.adjust_value_pet_power_gen_speed);
    //修正宠物攻击的冷却时间
    player_.IncPetAttackCDTime(entry.adjust_scale_pet_skill_cd_time);
    // 修正技能临时等级
    if (entry.adjust_skill_tree_id != 0 && entry.adjust_value_skill_temp_level > 0)
    {
        player_.LevelUpSkill(entry.adjust_skill_tree_id, -1 * entry.adjust_value_skill_temp_level, LVLUP_MODE_TALENT);
    }
}

bool PlayerTalent::IsTalentGroupExist(int32_t talent_group_id) const
{
	TalentGroupVec::const_iterator it_group = std::find_if(talent_group_vec_.begin(), talent_group_vec_.end(), talent_group_finder(talent_group_id));
	return it_group != talent_group_vec_.end();
}

bool PlayerTalent::IsTalentExist(int32_t talent_group_id, int32_t talent_id) const
{
	TalentGroupVec::const_iterator it_group = std::find_if(talent_group_vec_.begin(), talent_group_vec_.end(), talent_group_finder(talent_group_id));
	if (it_group == talent_group_vec_.end())
		return false;

	const TalentVec& talents = it_group->talent_list;
	TalentVec::const_iterator it_talent = std::find_if(talents.begin(), talents.end(), talent_finder(talent_id));
	return it_talent != talents.end();
}

bool PlayerTalent::GetTalent(int32_t talent_group_id, int32_t talent_id, TalentEntry*& pTalent) const
{
	TalentGroupVec::const_iterator it_group = std::find_if(talent_group_vec_.begin(), talent_group_vec_.end(), talent_group_finder(talent_group_id));
	if (it_group == talent_group_vec_.end())
		return false;

	TalentVec& talents = it_group->talent_list;
	TalentVec::iterator it_talent = std::find_if(talents.begin(), talents.end(), talent_finder(talent_id));
	if (it_talent == talents.end())
		return false;

	pTalent = &(*it_talent);
	return true;
}

bool PlayerTalent::GetTalentGroup(int32_t talent_group_id, TalentGroup*& pGroup)
{
	TalentGroupVec::iterator it_group = std::find_if(talent_group_vec_.begin(), talent_group_vec_.end(), talent_group_finder(talent_group_id));
	if (it_group == talent_group_vec_.end())
		return false;

	pGroup = &(*it_group);
	return true;
}

int32_t PlayerTalent::GetTalentSkill(int32_t talent_group_id, int32_t talent_id) const
{
	TalentEntry* pTalent = NULL;
	if (!GetTalent(talent_group_id, talent_id, pTalent))
		return 0;

	int level = pTalent->level + pTalent->tmp_level;
	if (level > pTalent->max_level)
	{
		level = pTalent->max_level;
	}

	const dataTempl::TalentTempl* pTpl = s_pDataTempl->QueryDataTempl<dataTempl::TalentTempl>(talent_id);
	ASSERT(pTpl);

	return pTpl->lvlup_table[level].passive_skill_id;
}

void PlayerTalent::PlayerGetTalentData() const
{
	G2C::TalentData packet;

	TalentGroupVec::const_iterator it = talent_group_vec_.begin();
	for (; it != talent_group_vec_.end(); ++ it)
	{
		G2C::TalentData::TalentGroup group;
		group.talent_group_id = it->talent_group_id;

		group.talent_list.resize(it->talent_list.size());
		for (size_t i = 0; i < it->talent_list.size(); ++ i)
		{
			group.talent_list[i].talent_id = it->talent_list[i].talent_id;
			group.talent_list[i].level     = it->talent_list[i].level;
			group.talent_list[i].tmp_level = it->talent_list[i].tmp_level;
		}
		packet.talent_group_list.push_back(group);
	}

	SendCmd(packet);
}

void PlayerTalent::QueryTalentSkill(std::set<int32_t>& skills) const
{
	for (size_t i = 0; i < talent_group_vec_.size(); ++ i)
	{
		const TalentGroup& __group = talent_group_vec_[i];
		for (size_t j = 0; j < __group.talent_list.size(); ++ j)
		{
			int32_t skill_id = GetTalentSkill(__group.talent_group_id, __group.talent_list[j].talent_id);
            if (skill_id != 0)
            {
			    skills.insert(skill_id);
            }
		}
	}
}

void PlayerTalent::NotifyClient(int32_t talent_group_id, int32_t talent_id, int level, int tmp_level) const
{
	G2C::TalentDetail packet;
	packet.talent_group_id = talent_group_id;
	packet.talent_id = talent_id;
	packet.level = level;
	packet.tmp_level = tmp_level;
	SendCmd(packet);
}

};
