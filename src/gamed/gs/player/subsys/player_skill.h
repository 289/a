#ifndef GAMED_GS_SUBSYS_PLAYER_SKILL_H_
#define GAMED_GS_SUBSYS_PLAYER_SKILL_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"

namespace gamed
{

///
/// 技能子系统
///

class PlayerSubSystem;
class PlayerSkill : public PlayerSubSystem
{
public:
	enum LOCATION
	{
		LOCATION_INVALID,
		LOCATION_POOL,
		LOCATION_BASE,
		LOCATION_EXTEND,
		LOCATION_MAX,
	};

private:
	enum LvlUpModeMask
	{
		LVLUP_MASK_AUTO     = 0x0001,
		LVLUP_MASK_MANUAL   = 0x0002,
		LVLUP_MASK_TRIGGER  = 0x0004,
	};

	/*技能树信息*/
	struct SkillTreeInfo
	{
		int32_t id;
		int8_t level;
		int8_t active;
	};

	/*技能树数据*/
	struct SkillTreeData
	{
		int32_t id;       //技能树ID
		int8_t level;     //基础等级
		int8_t tmp_level; //临时等级
		int8_t max_level; //最大等级
		bool   is_active; //是否选中

		SkillTreeData():
			id(0),
			level(0),
			tmp_level(0),
			max_level(0),
			is_active(false)
		{}

		void Clear()
		{
			id = 0;
			level = 0;
			tmp_level = 0;
			max_level = 0;
			is_active = false;
		}
	};

	template<class T>
	struct SkFinder
	{
		int32_t sk_tree_id;
		SkFinder(int32_t id): sk_tree_id(id) {}
		bool operator () (const T& v) const
		{
			return sk_tree_id == v.id;
		}
	};

private:
	typedef std::vector<SkillTreeInfo> SkillTreePool;
	typedef std::vector<SkillTreeData> SkillTreeVec;
	typedef std::vector<int32_t/*skill*/> PassiveSkillVec;

	SkillTreeVec    base_skill_tree_vec_;    // 基础可用技能组,初级角色也可以使用
    // 扩展第一个版本没有
	//SkillTreeVec    extend_skill_tree_vec_;  // 扩展可用技能组,角色二转后才可以使用
	SkillTreePool   skill_tree_pool_;        // 角色技能数据库,角色当前拥有的所有技能
	PassiveSkillVec passive_skill_vec_;      // 玩家被动技能列表

public:
	PlayerSkill(Player& player);
	virtual ~PlayerSkill();

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();
	virtual void OnTransferCls();

	bool SaveToDB(common::PlayerSkillData* pData);
	bool LoadFromDB(const common::PlayerSkillData& data);

	void CMDHandler_LearnSkill(const C2G::LearnSkill& cmd);
	void CMDHandler_SwitchSkill(const C2G::SwitchSkill& cmd);

	void PlayerGetSkillData() const;
	void QueryAvailableSkillTree(std::vector<playerdef::SkillTreeInfo>& sk_tree_info_list) const;

	void LevelUpAuto(); //系统自动升级，对所有技能树进行升级尝试，升级条件生效
	void LevelUpTrigger(int32_t sk_tree_id, int lvl, int lvlup_mode); //外界触发的升级，升级条件不生效
	void LevelUpAllSkill(int lvl, int lvlup_mode); //外界触发的升级，升级所有技能组

private:
	SkillTreeData* FindSkillTree(int32_t sk_tree_id, int& skill_idx);
	void    InitAvailiableSkillTree();
	void    LoadInitialClsSkillTree();
	bool    LevelUp(SkillTreeData* sk_tree, int mode);
	void    UpdateSkillTree(int32_t sk_tree_id, int level, bool active);
	int32_t GetLvlUpMask(int32_t sk_tree_id, int level) const;
};

};

#endif // GAMED_GS_SUBSYS_PLAYER_SKILL_H_
