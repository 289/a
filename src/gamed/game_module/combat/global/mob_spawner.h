#ifndef __GAME_MODULE_COMBAT_MOB_SPAWNER_H__
#define __GAME_MODULE_COMBAT_MOB_SPAWNER_H__


#include <set>
#include <map>
#include <vector>
#include <stdint.h>
#include "combat_types.h"
#include "combat_def.h"
#include "shared/base/singleton.h"

namespace combat
{

enum MOB_GROUP_TYPE
{
	MOB_GROUP_TYPE_RANDOM,
	MOB_GROUP_TYPE_FIXED,
};

enum COMBAT_POS
{
	COMBAT_POS_RANDOM,
	COMBAT_POS1,
	COMBAT_POS2,
	COMBAT_POS3,
	COMBAT_POS4,
};

///
/// 群组怪生成器
///
class MobSpawner : public shared::Singleton<MobSpawner>
{
private:
	struct MobGroup
	{
		struct Mob
		{
			TemplID id;		  //怪物ID
			short   gen_prob; //出生概率
			int8_t  pos;      //出生位置
			bool    unique;	  //唯一性
		};
		typedef std::vector<Mob> MobVec;
	
		TemplID group_id;                       // 怪物组ID
		TemplID next_group_id;                  // 连续战斗的怪物组ID
		int  combat_common_hint_id;             // 连续战斗时的常态提示语的索引ID
		int  combat_start_hint_id;              // 连续战斗时的出场提示语的索引ID
		char group_type;                        // 怪物组类型
		bool team_revise;                       // 组队修正
		int  probability[MAX_COMBAT_UNIT_NUM];  // 出现n只怪物的概率
		MobVec candidates;					    // 候选怪物列表
		int  sneak_attack_probability;          // 怪物组偷袭玩家的概率
		int  sneak_attacked_probability;        // 怪物组被玩家偷袭的概率
	};
	
private:
	typedef std::map<TemplID, MobGroup> MobGroupMap;
	MobGroupMap map_;

public:
	MobSpawner();
	~MobSpawner();

	static MobSpawner& GetInstance()
	{
		static MobSpawner instance;
		return instance;
	}

	void Initialize();
	void Release();
	bool GenerateMob(int mob_group_id, std::vector<MobInfo>& mob_list, int min_mob_count);

	int32_t GetNextMobGroupID(TemplID mob_group_id) const;
	int32_t GetCombatCommonHintID(TemplID mob_group_id) const;
	int32_t GetCombatStartHintID(TemplID mob_group_id) const;
	bool TestSneakAttack(TemplID mob_group_id) const;
	bool TestSneakAttacked(TemplID mob_group_id) const;

private:
	bool GenerateRandomMob(const MobGroup& grop, std::vector<MobInfo>& mob_list, int min_mob_count);
	bool GenerateFixedMob(const MobGroup& grop, std::vector<MobInfo>& mob_list);
};

#define s_mob_spawner MobSpawner::GetInstance()

};

#endif // __GAME_MODULE_COMBAT_MOB_SPAWNER_H__
