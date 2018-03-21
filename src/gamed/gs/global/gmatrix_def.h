#ifndef GAMED_GS_GLOBAL_GMATRIX_DEF_H_
#define GAMED_GS_GLOBAL_GMATRIX_DEF_H_

#include "game_types.h"


namespace dataTempl {
	class PlayerClassTempl;
} // namespace dataTempl

namespace gamed {
namespace gmatrixdef {

	enum RpcCallTypes
	{
		RPCCALL_TYPE_PLAYER = 1,
	};

	struct WorldIDInfo
	{
		MapID      world_id;
		MapTag     world_tag;
	};

	struct ServerParamInfo
	{
		ServerParamInfo()
			: is_cross_realm_svr(false),
			  permit_debug_print(false),
              permit_debug_cmd(false),
              forbid_move_print(false)
		{ }

		bool is_cross_realm_svr;
		bool permit_debug_print;
        bool permit_debug_cmd;
        bool forbid_move_print;
	};

	/**
	 * @brief 玩家职业默认模板
	 *    1.用于填充刚创建的玩家角色的playerdata
	 */
	struct PlayerClassDefaultTempl
	{
		int32_t    cls_templ_id;
		int32_t    level;
		float      x;
		float      y;
		uint8_t    dir; 
		int32_t    world_id;
		int32_t    world_tag;
		const dataTempl::PlayerClassTempl* templ_ptr;
	};

	struct PlayerLvlUpConfigTable
	{
		int32_t lvlup_exp_tbl[255];
	};

	struct PlayerCatVisionConfig
	{
		int32_t ep_speed_use;
		int32_t ep_speed_gen;
		int32_t interval_gen;
		int32_t lvlup_exp_tbl[32];
		int16_t max_cat_vision_level;
	};

	struct PetPowerLvlUpConfig
	{
		struct PowerEntry
		{
			int32_t probability;
			int32_t power_inc_on_lvlup;
		};

		std::vector<PowerEntry> power_lvlup_tbl;
	};

    struct GameVersionInfo
    {
        std::string res_ver;  // 资源版本
    };

} // namespace gmatrixdef
} // namespace gamed

#endif // GAMED_GS_GLOBAL_GMATRIX_DEF_H_
