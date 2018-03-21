#ifndef __GAME_MODULE_COMBAT_PLAYER_CMD_H__
#define __GAME_MODULE_COMBAT_PLAYER_CMD_H__

namespace combat
{

enum COMBAT_CMD
{
	COMBAT_CMD_INVALID,

	// 1
	COMBAT_CMD_SELECT_SKILL,
	COMBAT_CMD_TERMINATE_COMBAT,
    COMBAT_CMD_PET_ATTACK,
    COMBAT_CMD_PLAYER_ONLINE,
    COMBAT_CMD_PLAYER_OFFLINE,

    // 5

	COMBAT_CMD_MAX,
};

struct PlayerCMD
{
	int32_t cmd_no;
	int64_t roleid;
    std::vector<int32_t> params;
};

///
/// 1
///
/*struct SelectSkill : public PlayerCMD
{
	int32_t skillid;
    bool player_select;
};

struct TerminateCombat : public PlayerCMD
{
};

struct SwitchPet : public PlayerCMD
{
	int target_pet_item_idx;
};

struct PetAttack : public PlayerCMD
{
    int pet_combat_pos;
};

struct PlayerOnline : public PlayerCMD
{
};

struct PlayerOffline : public PlayerCMD
{
};*/

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_PLAYER_CMD_H__
