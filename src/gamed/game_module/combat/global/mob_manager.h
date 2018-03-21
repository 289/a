#ifndef __GAME_MODULE_COMBAT_MOB_MANAGER_H__
#define __GAME_MODULE_COMBAT_MOB_MANAGER_H__


#include <set>
#include <map>
#include <vector>
#include <stdint.h>
#include "combat_types.h"
#include "combat_def.h"
#include "shared/base/singleton.h"

namespace dataTempl
{
class GolemTempl;
}

namespace combat
{

/*掉落表*/
struct DropTable
{
	struct Entry
	{
		TemplID item_id;        //物品ID
		int  item_count;        //物品个数
		int  drop_prob;         //原始的掉落概率
		int  probability;       //处理后掉落概率
		bool unique;            //唯一掉落
	};

	int drop_times; //掉落次数
	std::vector<Entry> drop_table; //掉落表

	bool GenerateDrop(std::vector<ItemEntry>& items_drop);
};

///
/// 怪物数据管理器
///
typedef std::vector<SkillID> SkillVec;
typedef std::vector<int32_t> PropsVec;
class MobManager : public shared::Singleton<MobManager>
{
public:
	struct SkillEntry
	{
		SkillID skill; //技能ID
		int32_t prob;  //触发概率
	};
	typedef std::vector<SkillEntry> SkillGroup;

private:
	struct Mob
	{
		TemplID    mob_id;			//怪物ID
		char       mob_lvl;         //怪物等级
		char       mob_fac;         //怪物阵营
		int32_t    drop_exp;        //掉落经验
		int32_t    drop_money;      //掉落金钱
		SkillID    normal_atk_id;   //普攻技能
		SkillGroup skill_group;     //技能组
		PropsVec   props;           //基本属性
		int32_t    comm_drop_prob;  //普通掉落生效概率
		DropTable* comm_drop_tbl;   //普通掉落表
		DropTable* spec_drop_tbl;   //特殊掉落表
        std::string model_src_path; //模型资源路径
	};

	typedef std::map<TemplID, Mob*>       MobMap;
	typedef std::map<TemplID, DropTable*> DropTblMap;
	MobMap mob_map_;
	DropTblMap drop_map_;

public:
	MobManager();
	~MobManager();

	static MobManager& GetInstance()
	{
		static MobManager instance;
		return instance;
	}

	void    Initialize();
	void    Release();

    bool    IsValidMob(TemplID mob_id) const;
	char    GetMobLevel(TemplID mob_id) const;
	int32_t GetMobFaction(TemplID mob_id) const;
	bool    GetMobSkillGroup(TemplID mob_id, SkillGroup& skills) const;
	bool    GetMobProps(TemplID mob_id, PropsVec& props) const;
	TemplID GetMobNormalAtkID(TemplID mob_id) const;
    std::string GetMobModel(TemplID mob_id) const;

	bool    GenerateDropItem(TemplID mob_id, std::vector<ItemEntry>& list);
	bool    GenerateLotteryItem(TemplID mob_id, std::vector<ItemEntry>& list);
	int32_t GenerateDropExp(TemplID mob_id);
	int32_t GenerateDropMoney(TemplID mob_id);
	DropTable* GetDropTable(TemplID drop_tbl_id);

private:
	void InitMobMap();
	void InitDropMap();
	Mob* GetMob(TemplID mob_id);
};

class GolemManager : public shared::Singleton<GolemManager>
{
private:
	/*struct Golem
	{
		TemplID  golem_id;   //魔偶ID
		SkillVec skills;     //魔偶基础技能
		PropsVec props;      //魔偶基础属性
		int32_t  power_gen;  //能量恢复速度
        std::string model;   //魔偶的模型路径
	};

	typedef std::map<TemplID, Golem*> GolemMap;*/
	typedef std::map<TemplID, const dataTempl::GolemTempl*> GolemMap;
	GolemMap golem_map_;

public:
	GolemManager();
	~GolemManager();

	static GolemManager& GetInstance()
	{
		static GolemManager instance;
		return instance;
	}

	void Initialize();
	void Release();

	bool IsGolemValid(TemplID golem_id) const;
	void GetGolemSkill(TemplID golem_id, SkillVec& skills) const;
	void GetGolemProps(TemplID golem_id, PropsVec& props) const;
	void GetGolemModel(TemplID golem_id, std::string& model) const;
};

class PetManager : public shared::Singleton<PetManager>
{
public:
	PetManager() {}
	~PetManager(){}

	static PetManager& GetInstance()
	{
		static PetManager instance;
		return instance;
	}
};

#define s_mob_man MobManager::GetInstance()
#define s_golem_man GolemManager::GetInstance()
#define s_pet_nan PetManager::GetInstance()

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_MOB_MANAGER_H__
