#include "player_skill.h"

#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"
#include "gs/global/gmatrix.h"
#include "gs/global/gmatrix_def.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/template/data_templ/player_class_templ.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/skill_item_templ.h"
#include "gs/template/data_templ/skill_tree_templ.h"
#include "gamed/client_proto/G2C_proto.h"


namespace gamed
{

static int16_t GetSkillTreeMaxLevel(int32_t sk_tree_id)
{
	const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(sk_tree_id);
	ASSERT(tpl);

	int16_t level = 0;
	for (size_t i = 0; i < tpl->lvlup_table.size(); ++ i)
		if (tpl->lvlup_table[i].skill_id > 0)
			++ level;
	return level;
}

static int32_t GetSkillTreeCallSkill(int32_t sk_tree_id, int level)
{
	const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(sk_tree_id);
	ASSERT(tpl);
	return tpl->lvlup_table[level].skill_id;
}

static bool CanUseExtendSkill(char cls)
{
	return cls == CLS_S_ATTACK_WARRIOR ||
		   cls == CLS_S_DEFENSE_WARRIOR ||
		   cls == CLS_S_DAMAGE_MAGE ||
		   cls == CLS_S_HEAL_MAGE ||
		   cls == CLS_S_SNIPE_ARCHER ||
		   cls == CLS_S_RAKE_ARCHER;
}

/*****************************PlayerSkill******************************/
/*****************************PlayerSkill******************************/
/*****************************PlayerSkill******************************/
/*****************************PlayerSkill******************************/

PlayerSkill::PlayerSkill(Player& player): PlayerSubSystem(SUB_SYS_TYPE_SKILL, player),
	base_skill_tree_vec_(INIT_SKILL_TREE_SIZE)/*,
	extend_skill_tree_vec_(INIT_SKILL_TREE_SIZE)*/
{
	SAVE_LOAD_REGISTER(common::PlayerSkillData, PlayerSkill::SaveToDB, PlayerSkill::LoadFromDB);
}

PlayerSkill::~PlayerSkill()
{
	base_skill_tree_vec_.clear();
	//extend_skill_tree_vec_.clear();
	skill_tree_pool_.clear();
}

void PlayerSkill::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::LearnSkill,  PlayerSkill::CMDHandler_LearnSkill);
	REGISTER_NORMAL_CMD_HANDLER(C2G::SwitchSkill, PlayerSkill::CMDHandler_SwitchSkill);
}

void PlayerSkill::RegisterMsgHandler()
{
}

void PlayerSkill::OnTransferCls()
{
	//加载职业初始技能
	LoadInitialClsSkillTree();

	//初始化可用技能组
	InitAvailiableSkillTree();

	//自动升级
	LevelUpAuto();
}

bool PlayerSkill::LoadFromDB(const common::PlayerSkillData& data)
{
	for (size_t i = 0; i < data.sk_tree_list.size(); ++ i)
	{
		const common::PlayerSkillData::SkillTree& entry = data.sk_tree_list[i];
		const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(entry.id);
		ASSERT(tpl);

		SkillTreeInfo info;
		info.id = entry.id;
		info.level = entry.level;
		info.active = entry.active;
		skill_tree_pool_.push_back(info);
	}

	//初始化可用技能组
	InitAvailiableSkillTree();

	//自动升级
	LevelUpAuto();

	return true;
}

bool PlayerSkill::SaveToDB(common::PlayerSkillData* pData)
{
	pData->sk_tree_list.clear();
	for (size_t i = 0; i < skill_tree_pool_.size(); ++ i)
	{
		SkillTreeInfo& info = skill_tree_pool_[i];

		common::PlayerSkillData::SkillTree entry;
		entry.id = info.id;
		entry.level = info.level;
		entry.active = info.active;

		pData->sk_tree_list.push_back(entry);
	}

	return true;
}

void PlayerSkill::CMDHandler_LearnSkill(const C2G::LearnSkill& cmd)
{
	if (cmd.skill_tree_id <= 0 ||
		cmd.skill_idx < 0 ||
		cmd.skill_idx >= (int)base_skill_tree_vec_.size())
		return;

	int32_t skill_idx  = cmd.skill_idx;
	int32_t sk_tree_id = cmd.skill_tree_id;

	SkillTreeData* sk_tree = NULL;
	//SkillTreeData* sk_tree_ex = NULL;
	if (base_skill_tree_vec_[skill_idx].id == sk_tree_id)
	{
		sk_tree = &(base_skill_tree_vec_[skill_idx]);
		//sk_tree_ex = &(extend_skill_tree_vec_[skill_idx]);
	}
	//else if (extend_skill_tree_vec_[skill_idx].id == sk_tree_id)
	//{
		//sk_tree = &(extend_skill_tree_vec_[skill_idx]);
		//sk_tree_ex = &(base_skill_tree_vec_[skill_idx]);
	//}

	if (!sk_tree/* || !sk_tree_ex*/)
		return;

	if (LevelUp(sk_tree, LVLUP_MODE_MANUAL))
	{
		sk_tree->is_active = true;
		UpdateSkillTree(sk_tree->id, sk_tree->level, sk_tree->is_active);
		player_.sender()->LearnSkill_Re(skill_idx);
		player_.sender()->UpdateSkillTree(sk_tree->id, skill_idx, sk_tree->level, sk_tree->tmp_level, sk_tree->is_active);

		/*if (sk_tree_ex->is_active)
		{
			//特殊情况。。。
			sk_tree_ex->is_active = false;
			UpdateSkillTree(sk_tree_ex->id, sk_tree_ex->level, sk_tree_ex->is_active);
			player_.sender()->UpdateSkillTree(sk_tree_ex->id, skill_idx, sk_tree_ex->level, sk_tree_ex->tmp_level, sk_tree_ex->is_active);
		}*/
	}
}

void PlayerSkill::CMDHandler_SwitchSkill(const C2G::SwitchSkill& cmd)
{
	if (!CanUseExtendSkill(player_.role_class()))
		return;

	int skill_idx = cmd.skill_idx;
	if (skill_idx < 0 || skill_idx >= (int)base_skill_tree_vec_.size())
		return;

	SkillTreeData& node1 = base_skill_tree_vec_[skill_idx];
	//SkillTreeData& node2 = extend_skill_tree_vec_[skill_idx];
	if (node1.level <= 0/* || node2.level <= 0*/)
		return;
	
	if (node1.is_active)
	{
		node1.is_active = false;
		//node2.is_active = true;
	}
	//else if (node2.is_active)
	//{
		//node2.is_active = false;
		//node1.is_active = true;
	//}
	
	player_.sender()->SwitchSkill_Re(skill_idx);
}

void PlayerSkill::LevelUpAuto()
{
	for (size_t i = 0; i < base_skill_tree_vec_.size(); ++ i)
	{
		SkillTreeData& node1 = base_skill_tree_vec_[i];
		//SkillTreeData& node2 = extend_skill_tree_vec_[i];

		if (node1.id > 0 && LevelUp(&node1, LVLUP_MODE_AUTO))
		{
			/*if (!node2.is_active) */node1.is_active = true;
			UpdateSkillTree(node1.id, node1.level, node1.is_active);
			player_.sender()->UpdateSkillTree(node1.id, i, node1.level, node1.tmp_level, node1.is_active);
		}
/*
		if (node2.id > 0 && LevelUp(&node2, LVLUP_MODE_AUTO))
		{
			if (!node1.is_active) node2.is_active = true;
			UpdateSkillTree(node2.id, node2.level, node2.is_active);
			player_.sender()->UpdateSkillTree(node2.id, i, node2.level, node2.tmp_level, node2.is_active);
		}
*/
	}
}

void PlayerSkill::LevelUpTrigger(int32_t sk_tree_id, int lvl, int mode)
{
	if (sk_tree_id <= 0)
		return;

	int skill_idx = -1;
	SkillTreeData* sk_tree = FindSkillTree(sk_tree_id, skill_idx);
	if (!sk_tree)
		return;

	if (sk_tree->level >= sk_tree->max_level)
		return;

	bool level_up = false;
	switch (mode)
	{
		case LVLUP_MODE_TASK:
		{
			//任务导致的升级影响的是技能树的基础等级, 永久生效.
			//任务导致的升级不需要检查升级条件.
			const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(sk_tree->id);
			ASSERT(tpl);
			if (tpl->lvlup_table[sk_tree->level].lvlup_cond.cls_lvl_limit > player_.level())
				return;

			int __level = sk_tree->level;
			sk_tree->level += lvl;
			if (sk_tree->level > sk_tree->max_level)
				sk_tree->level = sk_tree->max_level;

			if (__level != sk_tree->level)
				level_up = true;
		}
		break;
		case LVLUP_MODE_EQUIP:
		case LVLUP_MODE_BUFF:
        case LVLUP_MODE_ENHANCE:
        case LVLUP_MODE_CARD:
        case LVLUP_MODE_TALENT:
		{
			//技能未激活则不会生效
			if (sk_tree->level == 0)
				break;

			//升级只影响临时等级
			sk_tree->tmp_level += lvl;
            if (sk_tree->tmp_level < 0)
            {
                sk_tree->tmp_level = 0;
            }
			level_up = true;
		}
		break;
		default:
			ASSERT(false);
		break;
	};

	if (level_up)
	{
		player_.sender()->UpdateSkillTree(sk_tree->id, skill_idx, sk_tree->level, sk_tree->tmp_level, sk_tree->is_active);
        GLog::log("玩家 %ld 的技能%d升级到%d级, 临时等级为%d", player_.role_id(), sk_tree->id, sk_tree->level, sk_tree->tmp_level);
	}
}

void PlayerSkill::LevelUpAllSkill(int lvl, int mode)
{
    int32_t i = 0;
    SkillTreeVec::iterator it = base_skill_tree_vec_.begin();
    for (; it != base_skill_tree_vec_.end(); ++it, ++i)
    {
        LevelUpTrigger(it->id, lvl, mode);
    }
}

void PlayerSkill::InitAvailiableSkillTree()
{
	//根据职业模板可用技能组和技能池来初始化可用技能组
	for (size_t i = 0; i < base_skill_tree_vec_.size(); ++ i)
	{
		base_skill_tree_vec_[i].Clear();
		//extend_skill_tree_vec_[i].Clear();
	}

	//获取职业模板
	gmatrixdef::PlayerClassDefaultTempl cls_default = Gmatrix::GetPlayerClsTemplCfg(player_.role_class());
	const dataTempl::PlayerClassTempl* pTpl = cls_default.templ_ptr;

	//加载基础可用技能
	for (size_t i = 0; i < pTpl->skill_trees_base.size(); ++ i)
	{
		SkillTreeData& node = base_skill_tree_vec_[i];

		int32_t sk_tree_id = pTpl->skill_trees_base[i];
		if (sk_tree_id <= 0)
			break;

		SkillTreePool __pool = skill_tree_pool_;
		SkillTreePool::iterator it = std::find_if(__pool.begin(), __pool.end(), SkFinder<SkillTreeInfo>(sk_tree_id));
		if (it != __pool.end())
		{
			//已激活技能
			node.id = it->id;
			node.level = it->level;
			node.max_level = GetSkillTreeMaxLevel(node.id);
			node.is_active = it->active;
		}
		else
		{
			//未激活技能
			node.id = sk_tree_id;
			node.max_level = GetSkillTreeMaxLevel(node.id);
		}
	}
/*
	if (CanUseExtendSkill(player_.role_class()))
	{
		//加载扩展可用技能
		for (size_t i = 0; i < pTpl->skill_trees_extend.size(); ++ i)
		{
			SkillTreeData& node = extend_skill_tree_vec_[i];
			int32_t sk_tree_id = pTpl->skill_trees_extend[i];
			if (sk_tree_id <= 0)
				break;

			SkillTreePool __pool = skill_tree_pool_;
			SkillTreePool::iterator it = std::find_if(__pool.begin(), __pool.end(), SkFinder<SkillTreeInfo>(sk_tree_id));
			if (it != __pool.end())
			{
				//已激活技能
				node.id = it->id;
				node.level = it->level;
				node.max_level = GetSkillTreeMaxLevel(node.id);
				node.is_active = it->active;
			}
			else
			{
				//未激活技能
				node.id = sk_tree_id;
				node.max_level = GetSkillTreeMaxLevel(node.id);
			}
		}
	}
*/
}

void PlayerSkill::LoadInitialClsSkillTree()
{
	//获取职业模板
	gmatrixdef::PlayerClassDefaultTempl cls_default = Gmatrix::GetPlayerClsTemplCfg(player_.role_class());
	const dataTempl::PlayerClassTempl* pTpl = cls_default.templ_ptr;

	//加载初始技能组
	for (size_t i = 0; i < pTpl->initial_skill_trees.size(); ++ i)
	{
		int32_t sk_tree_id = pTpl->initial_skill_trees[i];
		for (size_t j = 0; j < pTpl->skill_trees_base.size(); ++ j)
		{
			if (pTpl->skill_trees_base[j] == sk_tree_id)
			{
				SkillTreePool& __pool = skill_tree_pool_;
				SkillTreePool::iterator it = std::find_if(__pool.begin(), __pool.end(), SkFinder<SkillTreeInfo>(sk_tree_id));
				if (it == __pool.end())
				{
					SkillTreeInfo info;
					info.id = sk_tree_id;
					info.level = 1;
					info.active = true;
					skill_tree_pool_.push_back(info);
				}
				break;
			}
		}

		if (CanUseExtendSkill(player_.role_class()))
		{
			for (size_t j = 0; j < pTpl->skill_trees_extend.size(); ++ j)
			{
				if (pTpl->skill_trees_extend[j] == sk_tree_id)
				{
					SkillTreePool& __pool = skill_tree_pool_;
					SkillTreePool::iterator it = std::find_if(__pool.begin(), __pool.end(), SkFinder<SkillTreeInfo>(sk_tree_id));
					if (it == __pool.end())
					{
						SkillTreeInfo info;
						info.id = sk_tree_id;
						info.level = 1;
						info.active = true;
						skill_tree_pool_.push_back(info);
					}
					break;
				}
			}
		}
	}
}

bool PlayerSkill::LevelUp(SkillTreeData* sk_tree, int mode)
{
	if (!sk_tree || mode < 0)
		return false;

	//获取技能树模板
	const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(sk_tree->id);
	ASSERT(tpl);

	bool rst = false;
	for (;;)
	{
		//检查升级条件
		if(sk_tree->level >= sk_tree->max_level)
			return rst;

		int32_t mask = GetLvlUpMask(sk_tree->id, sk_tree->level);
		if (!(mask & (1 << (mode-1))))
			return rst;

		//获取升级条件
		const dataTempl::SkillTreeTempl::LvlUpEntry::LvlUpCond& cond = tpl->lvlup_table[sk_tree->level].lvlup_cond;

		if (player_.GetLevel() < cond.cls_lvl_limit)
			return rst;

		if (!player_.CheckMoney(cond.money_need))
			return rst;

		if (!player_.CheckCash(cond.cash_need))
			return rst;

		if (!player_.CheckItem(cond.item1_need, cond.item1_count))
			return rst;

		if (!player_.CheckItem(cond.item2_need, cond.item2_count))
			return rst;

		//检查通过
		//扣除升级消耗
		player_.SpendMoney(cond.money_need);
		player_.UseCash(cond.cash_need);
		player_.TakeOutItem(cond.item1_need, cond.item1_count);
		player_.TakeOutItem(cond.item2_need, cond.item2_count);

		//提升等级
		sk_tree->level ++;

		rst = true;

        GLog::log("玩家 %ld 的技能%d升级到%d级, 临时等级为%d", player_.role_id(), sk_tree->id, sk_tree->level, sk_tree->tmp_level);

        if (mode == LVLUP_MODE_MANUAL)
        {
            break;
        }
	}

	return true;
}

void PlayerSkill::PlayerGetSkillData() const
{
	G2C::SkillData packet;
	for (size_t i = 0; i < base_skill_tree_vec_.size(); ++ i)
	{
		const SkillTreeData& node = base_skill_tree_vec_[i];
		if (node.level <= 0)
			continue;

		G2C::SkillData::SkillTree entry;
		entry.skill_tree_id = node.id;
		entry.skill_idx = i;
		entry.cur_level = node.level;
		entry.tmp_level = node.tmp_level;
		entry.is_active = node.is_active ? 1 : 0;

		packet.base_skill_tree_list.push_back(entry);
	}
/*
	for (size_t i = 0; i < extend_skill_tree_vec_.size(); ++ i)
	{
		const SkillTreeData& node = extend_skill_tree_vec_[i];
		if (node.level <= 0)
			continue;

		G2C::SkillData::SkillTree entry;
		entry.skill_tree_id = node.id;
		entry.skill_idx = i;
		entry.cur_level = node.level;
		entry.tmp_level = node.tmp_level;
		entry.is_active = node.is_active ? 1 : 0;

		packet.extend_skill_tree_list.push_back(entry);
	}
*/
	player_.sender()->SendCmd(packet);
}

void PlayerSkill::QueryAvailableSkillTree(std::vector<playerdef::SkillTreeInfo>& sk_tree_info_list) const
{
	for (size_t i = 0; i < base_skill_tree_vec_.size(); ++ i)
	{
		const SkillTreeData& node1 = base_skill_tree_vec_[i];
		//const SkillTreeData& node2 = extend_skill_tree_vec_[i];
		if (node1.level <= 0/* && node2.level <= 0*/)
			continue;

		const SkillTreeData* sk_tree = NULL;
		if (node1.is_active)
			sk_tree = &node1;
		//else
			//sk_tree = &node2;

		playerdef::SkillTreeInfo info;
		info.skill_tree_id = sk_tree->id;
		info.level         = sk_tree->level;
		info.tmp_level     = sk_tree->tmp_level;
		info.max_level     = sk_tree->max_level;

		//获取技能树对应的技能ID
		int level = sk_tree->level + sk_tree->tmp_level;
		if (level > sk_tree->max_level)
			level = sk_tree->max_level;
		info.skill_id = GetSkillTreeCallSkill(sk_tree->id, level);
		ASSERT(info.skill_id > 0);

		sk_tree_info_list.push_back(info);
	}
}

PlayerSkill::SkillTreeData* PlayerSkill::FindSkillTree(int32_t sk_tree_id, int& skill_idx)
{
	SkillTreeVec::iterator it = std::find_if(base_skill_tree_vec_.begin(), base_skill_tree_vec_.end(), SkFinder<SkillTreeData>(sk_tree_id));
	if (it != base_skill_tree_vec_.end())
	{
		skill_idx = it - base_skill_tree_vec_.begin();
		return &(*it);
	}

	/*it = std::find_if(extend_skill_tree_vec_.begin(), extend_skill_tree_vec_.end(), SkFinder<SkillTreeData>(sk_tree_id));
	if (it != extend_skill_tree_vec_.end())
	{
		skill_idx = it - base_skill_tree_vec_.begin();
		return &(*it);
	}*/

	skill_idx = -1;
	return NULL;
}

void PlayerSkill::UpdateSkillTree(int32_t sk_tree_id, int level, bool active)
{
	SkillTreePool& __list = skill_tree_pool_;
	SkillTreePool::iterator it = std::find_if(__list.begin(), __list.end(), SkFinder<SkillTreeInfo>(sk_tree_id));
	if (it == __list.end())
	{
		SkillTreeInfo info;
		info.id = sk_tree_id;
		info.level = level;
		info.active = active;
		__list.push_back(info);
	}
	else
	{
		it->level = level;
		it->active = active;
	}
}

int32_t PlayerSkill::GetLvlUpMask(int32_t sk_tree_id, int level) const
{
	const dataTempl::SkillTreeTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::SkillTreeTempl>(sk_tree_id);
	ASSERT(tpl);

	int16_t lvlup_mode = tpl->lvlup_table[level].lvlup_cond.lvlup_mode;
	int16_t lvlup_mask = 0;
	switch (lvlup_mode) 
	{
		case LVLUP_MODE_AUTO:
			lvlup_mask = LVLUP_MASK_AUTO;
		break;
		case LVLUP_MODE_MANUAL:
			lvlup_mask = LVLUP_MASK_MANUAL;
		break;
		case LVLUP_MODE_TRIGGER:
			lvlup_mask = LVLUP_MASK_TRIGGER;
		break;
		default:
			ASSERT(false);
		break;
	};

	return lvlup_mask;
}

};
