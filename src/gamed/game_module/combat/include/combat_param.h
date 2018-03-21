#ifndef __GAME_MODULE_COMBAT_PARAM_H__
#define __GAME_MODULE_COMBAT_PARAM_H__

#include <stdint.h>
#include <vector>
#include <set>
#include <string>

namespace combat
{

/*职业属性恢复规则*/
struct cls_prop_sync_rule
{
	int role_cls;
	int hp_sync_rule;
	int mp_sync_rule;
	int ep_sync_rule;
	int power_gen_rule;
};

struct skill_tree
{
	int32_t skill_tree_id;
	int32_t skill_id;
	int8_t  level;
	int8_t  tmp_level;
	int8_t  max_level;
};

struct pet_entry
{
	int32_t pet_id;
	int16_t pet_rank;
    int16_t pet_level;
	int16_t pet_blevel;
	int16_t pet_item_idx;
	int16_t pet_combat_pos;
};

struct pet_data
{
    int32_t pet_power;
    int32_t pet_power_cap;
    int32_t pet_power_gen_speed;
    int32_t pet_attack_cd_time;

	std::vector<pet_entry> pet_vec;
};

struct player_data
{
    int64_t roleid;
    std::string name;
    int8_t  pos;
    int8_t  cls;
    int8_t  gender;
    int16_t level;
    int32_t master_id;
    int32_t weapon_id;
    int32_t normal_atk_id;
    int32_t hp;
    int32_t mp;
    int32_t ep;
    std::vector<int32_t> props;
    std::set<int32_t> addon_skills;
    //std::vector<int32_t> talent_skill_vec;
    //std::vector<int32_t> title_skill_vec;
    std::vector<skill_tree> skill_tree_vec;
    std::string cls_model_src;
    int32_t dying_time;

	pet_data petdata;
};

struct pve_combat_param
{
	int64_t creator;                 // 战斗创建者角色ID
	int64_t world_id;                // 战斗创建者所在的大世界ID
	int32_t combat_scene_id;         // 战斗场景ID
	int32_t mob_group_id;            // 怪物组ID
    int32_t task_id;                 // 战斗触发的任务ID
	int64_t world_monster_id;        // 怪物在大世界的对象ID
	int16_t init_player_count;       // 初始组队参战玩家个数
	int8_t  is_world_boss;           // 是否是世界BOSS
    int32_t challenge_id;            // 挑战组ID
	player_data playerdata;          // 创建战斗的玩家数据

	struct team_npc
	{
		int32_t id;
		int32_t pos;
	};
	std::vector<team_npc> team_npc_vec;
};

struct pvp_combat_param
{
    int64_t creator;                 // 战斗创建者的角色ID
    int64_t world_id;                // 战斗创建者所在的大世界ID
    int32_t combat_flag;             // 战斗类型标识
    int32_t combat_scene_id;         // 战斗场景ID
    player_data playerdata;          // 战斗创建者的玩家数据

	struct team_npc
	{
		int32_t id;
		int32_t pos;
	};
	std::vector<team_npc> team_npc_vec;
};

};

#endif // __GAME_MODULE_COMBAT_PARAM_H__
