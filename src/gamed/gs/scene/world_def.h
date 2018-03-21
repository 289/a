#ifndef GAMED_GS_SCENE_WORLD_STRUCT_H_
#define GAMED_GS_SCENE_WORLD_STRUCT_H_

#include "gs/global/math_types.h"
#include "gs/global/game_types.h"

#include "grid.h"


namespace gamed {
namespace world {

	struct GridInitData
	{
		int     row;
		int     column;
		float   step;
		float   xstart;
		float   ystart;
		rect    local_rt;
	};

	struct VisibleInitData
	{
		float min_visible_range;
		float max_visible_range;
	};

	struct WorldInitData
	{
		MapID         world_id;
		MapTag        world_tag;

		GridInitData    grid;
		VisibleInitData visible;
	};

	struct off_node_t
	{
		off_node_t(Grid& grid, int offset_x, int offset_y)
			: x_off(offset_x),
			  y_off(offset_y)
		{
			idx_off = (offset_y * grid.reg_column()) + offset_x;
		}

		bool operator==(const off_node_t& rhs) const
		{
			return rhs.idx_off == idx_off;
		}

		int idx_off;
		int x_off;
		int y_off;
	};

	struct player_base_info
	{
		enum
		{
			IMMUNE_MASK_INVALID        = 0x00000000,
			IMMUNE_MASK_ACTIVE_MONSTER = 0x00000001, // 免疫主动怪
		};

		bool immune_active_monster() const
		{
			return immune_mask & IMMUNE_MASK_ACTIVE_MONSTER;
		}

		XID        xid;
        int32_t    master_id;
		int32_t    link_id;
		int32_t    sid_in_link;
		A2DVECTOR  pos;
		int16_t    gender;
		int16_t	   cls; 
		int8_t     dir;
		int32_t    level;
		int32_t    faction;
		bool       can_combat;
		int32_t    immune_mask;  // 对应上面的mask
        int32_t    combat_value; // 玩家战斗力
	};

    // 主要保存一些静态的变长信息，通过MSG同步的
    struct player_extra_info
    {
        std::string first_name;
        std::string middle_name;
        std::string last_name;
    };

	struct worldobj_base_info
	{
		int32_t    tid;  // templ id
		int32_t    eid;  // elem id
		A2DVECTOR  pos;
		int8_t     dir;
		bool       can_combat;
	};

	struct instance_info
	{
		instance_info() 
			: ins_serial_num(0),
			  ins_templ_id(0),
			  world_id(0),
			  world_tag(0),
			  ins_create_time(0)
		{ }

		inline bool operator==(const instance_info& rhs) const
		{
			if (ins_serial_num == rhs.ins_serial_num &&
				ins_templ_id == rhs.ins_templ_id &&
				world_id == rhs.world_id &&
				world_tag == rhs.world_tag &&
				ins_create_time == rhs.ins_create_time)
			{
				return true;
			}
			return false;
		}

		inline bool operator!=(const instance_info& rhs) const
		{
			return !(*this == rhs);
		}

		inline bool has_info() const
		{
			if (ins_serial_num == 0)
				return false;
			return true;
		}

		inline void clear()
		{
			ins_serial_num  = 0;
			ins_templ_id    = 0;
			world_id        = 0;
			world_tag       = 0;
			ins_create_time = 0;
		}

		int64_t ins_serial_num;
		int32_t ins_templ_id;
		int32_t world_id;
		int32_t world_tag;
		int64_t ins_create_time;
	};

    enum BGEnterType
    {
        BGET_ENTER_WORLD,
        BGET_CHANGE_MAP,
    };

    struct battleground_info
	{
		battleground_info() 
			: bg_serial_num(0),
			  bg_templ_id(0),
			  world_id(0),
			  world_tag(0),
			  bg_create_time(0)
		{ }

		inline bool operator==(const battleground_info& rhs) const
		{
			if (bg_serial_num == rhs.bg_serial_num &&
				bg_templ_id == rhs.bg_templ_id &&
				world_id == rhs.world_id &&
				world_tag == rhs.world_tag &&
				bg_create_time == rhs.bg_create_time)
			{
				return true;
			}
			return false;
		}

		inline bool operator!=(const battleground_info& rhs) const
		{
			return !(*this == rhs);
		}

		inline bool has_info() const
		{
			if (bg_serial_num == 0)
				return false;
			return true;
		}

		inline void clear()
		{
			bg_serial_num  = 0;
			bg_templ_id    = 0;
			world_id       = 0;
			world_tag      = 0;
			bg_create_time = 0;
		}

		int64_t bg_serial_num;
		int32_t bg_templ_id;
		int32_t world_id;
		int32_t world_tag;
		int64_t bg_create_time;
	};

    struct player_link_info
    {
        RoleID  role_id;
        int32_t link_id;
        int32_t sid_in_link;
    };

} // namespace world
} // namespace gamed

#endif // GAMED_GS_SCENE_WORLD_STRUCT_H_
