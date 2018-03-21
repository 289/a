#ifndef GAMED_GS_PLAYER_PLAYER_DEF_H_
#define GAMED_GS_PLAYER_PLAYER_DEF_H_

#include "shared/net/packet/bit_set.h"
#include "gs/global/math_types.h"
#include "gs/global/game_types.h"
#include "client_proto/player_visible_state.h"


namespace gamed {
namespace playerdef {

	const int MAX_PLAYER_LEVEL           = 255;
	const int MAX_CAT_VISION_LEVEL       = 32;
	const int INVALID_CLS_TEMPLATE       = 0;
	const int INIT_INVENTORY_SIZE        = 175;
	const int MAX_HIDE_INV_SIZE          = 100;
	const int MAX_TASK_INV_SIZE          = 100;
	const int MAX_PET_INV_SIZE           = 128;
	const int MAX_CARD_INV_SIZE          = 100;
	const int MAX_MOUNT_INV_SIZE         = 100;
	const int MAX_SKILL_TREE_LEVEL       = 50;
	const int INIT_SKILL_TREE_SIZE       = 14;
	const int MAX_TIME_WAIT_MOB_RESPONSE = 3;
	const int64_t MAX_PLAYER_MONEY       = 0x7fffffffffffffff;
    const int32_t MAX_PLAYER_CASH        = 1000000000;  // 最大元宝数10亿
    const int32_t MAX_PLAYER_SCORE       = 1000000000;  // 最大学分数10亿

	const int INVINCIBLE_BUFF_TIME       = 30;
    const int MAX_UI_NUM                 = 16;
    const int HP_GEN_INTERVAL            = 2;

    const int SEC_PER_DAY                = 23*3600+59*60+59;

	///
	/// 玩家扩展状态,作为玩家状态的一个辅助状态.
	/// 主要提供给大世界BUFF使用
	///
	enum EXTEND_STATE
	{
		ES_IMMUNE_ACTIVE_MONSTER = 0x0001, // 免疫主动怪攻击
		ES_IMMUNE_LANDMINE       = 0x0002, // 免疫暗雷攻击
		ES_IMMUNE_TEAM_COMBAT    = 0x0004, // 免疫组队战斗
	};

	///
	/// 玩家职业定义
	///    （1）以下枚举的定义需要与客户端、编辑器、/config/player_class_template.lua里的定义一致
	///    （2） "J" stand for junior, "S" stand for senior 
	///    （3）TODO:以下Class是通过role_class存盘的，因此顺序不能变，新增的职业只能添加在末尾
	///
	enum PlayerClass
	{
		// 新手
		CLS_NEWBIE = 0,        // 初心者（新手）
		// 战士
		CLS_PRIMARY_WARRIOR,   // 初级战士
		CLS_J_ATTACK_WARRIOR,  // 中阶攻击战士
		CLS_S_ATTACK_WARRIOR,  // 高阶攻击战士
		CLS_J_DEFENSE_WARRIOR, // 中阶防御战士
		CLS_S_DEFENSE_WARRIOR, // 高阶防御战士
		// 法师
		CLS_PRIMARY_MAGE,      // 初级法师
		CLS_J_DAMAGE_MAGE,     // 中阶输出型法师
		CLS_S_DAMAGE_MAGE,     // 高阶输出型法师
		CLS_J_HEAL_MAGE,       // 中阶治疗型法师
		CLS_S_HEAL_MAGE,       // 高阶治疗型法师
		// 射手
		CLS_PRIMARY_ARCHER,    // 初级射手
		CLS_J_SNIPE_ARCHER,    // 中阶狙击型射手（单攻） 
		CLS_S_SNIPE_ARCHER,    // 高阶狙击型射手
		CLS_J_RAKE_ARCHER,     // 中阶扫射型射手（群攻，暂定）
		CLS_S_RAKE_ARCHER,     // 高阶扫射型射手

		// end
		CLS_MAXCLS_LABEL
	};

	enum SaveDataOpType
	{
		SDOP_LOGOUT    = 0,
		SDOP_HEARTBEAT = 1,
	};

	enum LogoutType
	{
		LT_DISCONNECT_ON_LINK = 0,
		LT_DB_SAVE_ERROR      = 1,
		LT_LINK_DISCONNECT    = 2,
		LT_MASTER_DISCONNECT  = 3,
		LT_GS_STOP            = 4,
		LT_CHANGE_MAP         = 5,
		LT_FATAL_ERROR        = 6,
	};

	// 要与task里的NameType定义一致
	enum NameType
	{
		NT_FIRST_NAME,
		NT_MIDDLE_NAME,
		NT_LAST_NAME,
        NT_MAX
	};
	
	struct ElsePlayerInfo
	{
		XID     who;
		int32_t link_id;
		int32_t sid_in_link;
	};

	typedef ElsePlayerInfo SenderInfo;
	

    //
    // 冷却时间单位为毫秒
    //
    #define COOL_TIME_TIDY_INV 30*1000

    /*需要与G2C里的PlayerCoolDownIndex枚举一致*/
    /*添加index需要在player_cooldown.cpp里添加静态断言*/
    enum CoolDownIndex
    {
        PCD_INDEX_INVALID = 0,
        PCD_INDEX_TIDY_INV,
        PCD_INDEX_MAX,
    };

    struct CoolDownInfo
    {
        int16_t cd_type;
        int32_t cd_index;
		int32_t cd_time;
	};


    //
	// 复活坐标（全局坐标）
    //
	struct ResurrectCoord
	{
		ResurrectCoord()
			: map_id(0),
			  coord(0.f, 0.f)
		{ }

		inline bool HasResurrectPoint() const { return map_id != 0; }

		MapID     map_id;   // 为0时表示该区域没有复活点
		A2DVECTOR coord;
	};


    //
	// 最终汇总后的区域规则，player身上只有一份
    //
	struct AreaRulesInfo
	{
		AreaRulesInfo()
			: is_allow_pk(false)
		{ }

		inline void Reset() 
		{
			is_allow_pk = false;
			resurrect_coord.map_id  = 0; 
			resurrect_coord.coord.x = 0;
			resurrect_coord.coord.y = 0;
			battle_scene_list.clear();
		}

		inline bool HasBattleScene() const { return !battle_scene_list.empty(); }

		bool                  is_allow_pk;
		ResurrectCoord        resurrect_coord;
		std::vector<int32_t>  battle_scene_list;
	};

	struct ServiceProviderInfo
	{
		XID       xid;
		A2DVECTOR pos;
	};

	/*宠物数据*/
	struct PetEntry
	{
		int32_t pet_id;
		int32_t pet_exp;
		int16_t pet_level;
		int16_t pet_blevel;
		int16_t pet_item_idx;
	};

	/*宠物信息*/
	struct PetInfo
	{
		int32_t pet_id;
		int16_t pet_rank;
        int16_t pet_level;
		int16_t pet_blevel;
		int16_t pet_item_idx;
		int16_t pet_combat_pos;
	};

	/*技能树信息*/
	struct SkillTreeInfo
	{
		int32_t skill_tree_id;
		int32_t skill_id;
		int8_t  level;
		int8_t  tmp_level;
		int8_t  max_level;
	};

	struct BuddyMemberInfo
	{
		int32_t tpl_id;
		int32_t pos_index;
	};

	struct ObjectBWList
	{
		void clear()
		{
			black_list.clear();
			white_list.clear();
		}

		std::set<int32_t> black_list;
		std::set<int32_t> white_list;
	};

	struct SpecSavePos
	{
		SpecSavePos() 
			: world_id(0) 
		{ }

		bool has_spec_pos() const
		{ 
			return world_id > 0; 
		}

		void clear()
		{
			world_id = 0;
			pos.x    = -4096;
			pos.y    = -4096;
		}

		int32_t   world_id;
		A2DVECTOR pos;
	};

	struct DramaSavePos
	{
		DramaSavePos() 
			: world_id(0) 
		{ }

		bool has_drama_pos() const
		{ 
			return world_id > 0; 
		}

		void clear()
		{
			world_id = 0;
			pos.x    = -4096;
			pos.y    = -4096;
		}

		int32_t   world_id;
		A2DVECTOR pos;
	};
	
	struct limit_sale_goods
	{
		int32_t goods_id;
		int32_t goods_remains;
	};

	struct goods_detail
	{
		int32_t goods_id;      //商品ID
		int32_t goods_remains; //剩余商品个数
		int32_t goods_limit_buy;//限购个数
	};

	struct goods_refresh_time
	{
		int32_t goods_id;
		time_t timestamp;
	};

	struct MailAttach
	{
        MailAttach()
            : id(0), count(0), flag(0), type(0), prob(0), valid_time(0)
        {
        }

		int32_t id;
		int32_t count;
		int8_t flag;
		int8_t type;
		int32_t prob;
		int32_t valid_time;

		NESTED_DEFINE(id, count, flag, type, prob, valid_time);
	};

	struct SysMail
	{
        SysMail()
            : attach_score(0)
        {
        }

		int32_t attach_score;
		std::string sender;
		std::string title;
		std::string content;
		std::vector<MailAttach> attach_list;

		NESTED_DEFINE(attach_score, sender, title, content, attach_list);
	};

	struct ItemEntry
	{
		int32_t item_id;
		int32_t item_count;
	};

	struct LandmineRecord
	{
		LandmineRecord() : count(0), reject_flag(0) { }
		int32_t   count;
		A2DVECTOR old_pos;
		int32_t   reject_flag;
	};

	struct PassivityFlag
	{
		PassivityFlag() { clear(); }

		void clear()
		{
			flag      = 0;
			timeout   = 0;
			is_reject = false;
		}

		bool is_passive() const
		{
			return flag == 1;
		}

		void set_passive(int32_t time)
		{
			if (is_reject)
				return;

			if (is_passive())
			{
				timeout += time;
			}
			else
			{
				flag    = 1;
				timeout = time;
			}
		}

		void set_reject()
		{
			is_reject = true;
			clear();
		}

		void clr_reject()
		{
			is_reject = false;
			clear();
		}
		
		// 有被动标记时玩家不能进入战斗、不能进行传送
		int32_t flag;      // 0表示不在被动状态，1表示处在被动状态
		int32_t timeout;
		bool    is_reject; // 拒绝设置被动标记（踢人的时候会设置）
	};

	struct InsInfoCond
	{
		enum EnterType 
		{
			CREATE = 0,
			ENTER,
			REENTER,
		};

		int64_t serial_num;
		int64_t create_time;
		int32_t ins_tid;
		int8_t  enter_type; // 对应EnterType枚举
	};

    struct BGInfoCond
	{
		enum EnterType 
		{
			ENTER = 0,
			REENTER, // 断线后重新登陆
		};

		int64_t serial_num;
		int64_t create_time;
		int32_t bg_tid;
		int8_t  enter_type; // 对应EnterType枚举
	};

    struct AuctionItemData
    {
        int32_t currency_type; 
        int32_t buyout_price; 
        int32_t bid_price;
        int32_t item_id;
        std::string item_name; 
        std::string item_info;
    };

    ///
    /// PVP战斗
    ///
    struct StartPVPResult
    {
        int32_t combat_id;
    };

    enum CombatPVPType
    {
        PVP_TYPE_NONE = 0,
        PVP_TYPE_DUEL, // 决斗pvp
    };

    // 战斗结束后需要处理的数据
    struct PVPEndExtraData
    {
        PVPEndExtraData() : remain_hp(0) { }
        int32_t remain_hp;
    };

    struct PVPEndInfo
    {
        RoleID  creator;
        int32_t pvp_type;
    };

    enum LvlUpMode
    {
        LVLUP_MODE_INVALID,
        LVLUP_MODE_AUTO,    //系统自动升级(存盘)
        LVLUP_MODE_MANUAL,  //玩家手动升级(存盘)
        LVLUP_MODE_TRIGGER, //外界触发导致升级(存盘视具体情况)
        LVLUP_MODE_TASK,    //任务导致升级(存盘)
        LVLUP_MODE_EQUIP,   //装备导致升级(不存盘)
        LVLUP_MODE_BUFF,    //buff导致升级(不存盘)
        LVLUP_MODE_ENHANCE, // 附魔导致升级（不存盘）
        LVLUP_MODE_CARD,    // 卡牌导致升级（不存盘）
        LVLUP_MODE_TALENT,  // 天赋导致升级（不存盘）
        LVLUP_MODE_MAX,
    };

    ///
    /// 卡牌相关
    ///
    struct CardEntry
    {
        CardEntry()
            : id(0), exp(0), star_id(0), item_idx(0), card_templ(NULL)
        {
        }

        int32_t id;                 // 卡牌模板ID
        int32_t exp;                // 卡牌经验
        int32_t star_id;            // 镶嵌的星辰模板ID
        int16_t item_idx;           // 卡牌物品在卡牌包裹中的位置
        const void*   card_templ;   // 对应的模板指针

        NESTED_DEFINE(id, exp, star_id, item_idx);
    };


    ///
    /// 物品相关
    ///

    // 需要与G2C里的枚举一致，在player_inventory.cpp里做静态断言
    // 物品获得的方式，客户端需要知道是什么方式获得的物品
    enum GainItemMode
    {
        GIM_NORMAL = 0,  // 普通方式获得物品（默认）
        GIM_INS_AWARD,   // 副本结束奖励的物品
        GIM_TASK,        // 任务系统发放的物品
        GIM_MAIL,        // 邮件系统发放的物品
        GIM_CARD,        // 卡牌系统发放的物品
        GIM_MAX
    };

} // namespace playerdef
} // namespace gamed

#endif // GAMED_GS_PLAYER_PLAYER_DEF_H_
