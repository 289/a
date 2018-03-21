#ifndef COMMON_OBJDATAPOOL_PLAYER_ATTR_DECLAR_H_
#define COMMON_OBJDATAPOOL_PLAYER_ATTR_DECLAR_H_

#include <vector>
#include <string>

#include "common/obj_data/attr_type_def.h"
#include "common/obj_data/obj_attr_packet.h"
#include "common/obj_data/obj_packet_util.h"


namespace common {

/**
 * ATTENTION: 宏"OBJECTATTR_DEFINE"只能支持有限个数的参数（16个,具体见shared/net/packet/packet_util.h），
 *            超过数量请使用"MARSHAL_ATTR"和"UNMARSHAL_ATTR"宏
 */

enum PLAYER_ATTR
{
// 0
	USER_INFO = PLAYER_ATTR_TYPE_LOWER_LIMIT,
	ROLE_INFO,
	PLAYER_BASE_INFO,
	LOCATION_INFO,
	PLAYER_INVENTORY_DATA,

// 5
	PLAYER_PRELOAD_DATA,
	PLAYER_COOLDOWN_DATA,
	PLAYER_COMBAT_DATA,
	PLAYER_SKILL_DATA,
	PLAYER_PET_DATA,

// 10
	PLAYER_BUFF_DATA,
	PLAYER_TASK_DATA,
	PLAYER_OBJECT_BW_LIST,
	PLAYER_INSTANCE_DATA,
	PLAYER_MALL_DATA,

// 15
	PLAYER_MAIL_LIST,
	PLAYER_SPEC_MONEY,
	PLAYER_TALENT_DATA,
    PLAYER_AUCTION_DATA,
    PLAYER_FRIEND_DATA,

// 20
    PLAYER_REPUTATION_DATA,
    PLAYER_ACHIEVE_DATA,
    PLAYER_ENHANCE_DATA,
    PLAYER_PROP_REINFORCE_DATA,
    PLAYER_TITLE_DATA,

// 25
    PLAYER_STAR_DATA,
    PLAYER_BATTLEGROUND_DATA,
    PLAYER_COUNTER_DATA,
    PLAYER_GEVENT_DATA,
    PLAYER_PUNCH_CARD_DATA,

// 30
    PLAYER_PARTICIPATION,
    PLAYER_MOUNT_DATA,
    PLAYER_BOSS_CHALLENGE_DATA,
    PLAYER_ARENA_DATA,


// 1100
	MAX_ATTR_TYPE = PLAYER_ATTR_TYPE_UPPER_LIMIT
};

class PlayerRoleInfo : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerRoleInfo, ROLE_INFO);
public:
	int64_t roleid;
	std::string first_name;
	std::string mid_name;
	std::string last_name;
	int32_t created_time; // 角色创建时间,0表示首次创建/首次登陆
    int32_t last_login_time;
    int32_t last_logout_wid; // last logout world_id

	OBJECTATTR_DEFINE(roleid, first_name, mid_name, last_name, created_time, last_login_time, last_logout_wid);

	virtual void Release()
	{
		roleid          = 0;
		created_time    = 0;
        last_login_time = 0;
        last_logout_wid = 0;
		first_name.clear();
		mid_name.clear();
		last_name.clear();
	}
};

class PlayerBaseInfo : public ObjectAttrPacket 
{
	DECLARE_OBJECTATTR(PlayerBaseInfo, PLAYER_BASE_INFO);
public:
	uint8_t gender;         // 性别
	uint8_t cls;            // 职业
	uint8_t race;	        // 种族
	int32_t hp;             // 血量
	int32_t mp;             // 魔法值
	int32_t ep;             // 精力
	int32_t exp;            // 经验
	int32_t level;          // 等级
	int64_t money;          // 金钱
	int32_t cat_exp;        // 视觉经验
	int32_t cat_level;      // 视觉等级
    int32_t combat_value;   // 战斗力
    std::string ui_config;  // 客户端UI配置

	OBJECTATTR_DEFINE(gender, cls, race, hp, mp, ep, exp, level, money, cat_exp, cat_level, combat_value, ui_config);

	virtual void Release() 
	{
		gender = 0;
		cls = 0;
		race = 0;
		hp = 0;
		mp = 0;
		ep = 0;
		exp = 0;
		level = 0;
		money = 0;
		cat_exp = 0;
		cat_level = 0;
        combat_value = 0;
    }
};

class LocationInfo : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(LocationInfo, LOCATION_INFO);
public:
	float x;            // x坐标
	float y;            // y坐标
	uint8_t dir;        // 朝向
	int32_t world_id;   // map_id
	int32_t world_tag;  // map_tag

	OBJECTATTR_DEFINE(x, y, dir, world_id, world_tag);

	virtual void Release()
	{
		x               = 0.f;
        y               = 0.f;
		dir             = 0;
        world_id        = 0;
        world_tag       = 0;
	}
};

class PlayerInventoryData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerInventoryData, PLAYER_INVENTORY_DATA);
public:
	struct ItemData
	{
		int32_t type;
		int16_t index;
		int32_t price;
		int32_t count;
		int32_t pile_limit;
		int32_t proc_type;
		int32_t expire_date;
		std::string content;

		NESTED_DEFINE(type, index, price, count, pile_limit, proc_type, expire_date, content);
		ItemData() :
			type(-1),
			index(0),
			price(0),
			count(0),
			pile_limit(0),
			proc_type(0),
			expire_date(0)
			{ }

		ItemData(const ItemData& rhs) :
			type(rhs.type),
			index(rhs.index),
			price(rhs.price),
			count(rhs.count),
			pile_limit(rhs.pile_limit),
			proc_type(rhs.proc_type),
			expire_date(rhs.expire_date),
			content(rhs.content)
			{ }

		void Clear()
		{
			type = -1;
			index = 0;
			price = 0;
			count = 0;
			pile_limit = 0;
			proc_type= 0;
			expire_date = 0;
			content.clear();
		}
	};

	int16_t inventory_cap;
	std::vector<ItemData> inventory;
	std::vector<ItemData> equipment;
	std::vector<ItemData> hide_inv;
	std::vector<ItemData> task_inv;
	std::vector<ItemData> pet_inv;
	std::vector<ItemData> card_inv;
	std::vector<ItemData> mount_inv;

	OBJECTATTR_DEFINE(inventory_cap, inventory, equipment, hide_inv, task_inv, pet_inv, card_inv, mount_inv);

	virtual void Release()
	{
		inventory_cap = 0;

		for (size_t i = 0; i < inventory.size(); ++ i)
			inventory[i].Clear();

		for (size_t i = 0; i < equipment.size(); ++ i)
			equipment[i].Clear();

		for (size_t i = 0; i < hide_inv.size(); ++ i)
			hide_inv[i].Clear();

		for (size_t i = 0; i < task_inv.size(); ++ i)
			task_inv[i].Clear();

		for (size_t i = 0; i < pet_inv.size(); ++ i)
			pet_inv[i].Clear();

		for (size_t i = 0; i < card_inv.size(); ++ i)
			card_inv[i].Clear();

		for (size_t i = 0; i < mount_inv.size(); ++ i)
			mount_inv[i].Clear();
	}
};

class PlayerPreloadData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerPreloadData, PLAYER_PRELOAD_DATA);
public:
	OBJECTATTR_DEFINE();

	virtual void Release() { }
};

class PlayerCoolDownData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerCoolDownData, PLAYER_COOLDOWN_DATA);
public:
    std::string cd_list; // 格式对应scalable/player/cooldown_data.proto
	OBJECTATTR_DEFINE(cd_list);

	virtual void Release()
	{
		cd_list.clear();
	}
};

class PlayerCombatData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerCombatData, PLAYER_COMBAT_DATA);
public:
	int32_t unit_id;
	int32_t combat_id;
    int8_t  combat_type;
	int8_t  combat_result;
	int32_t combat_remain_hp;
    int32_t combat_pet_power;
	int64_t combat_award_exp;
	int64_t combat_award_money;
    int64_t combat_pvp_type;
	std::string combat_mob_killed_list;
	std::string combat_award_items_drop;
	std::string combat_award_items_lottery;
    std::string combat_player_killed_list;

	OBJECTATTR_DEFINE(unit_id,
            combat_id,
            combat_type,
            combat_result,
            combat_award_exp,
            combat_award_money,
            combat_remain_hp,
            combat_pet_power,
            combat_pvp_type,
            combat_mob_killed_list,
            combat_award_items_drop,
            combat_award_items_lottery,
            combat_player_killed_list);

	virtual void Release()
	{
		unit_id            = 0;
		combat_id          = 0;
        combat_type        = 0;
		combat_result      = 0;
		combat_remain_hp   = 0;
        combat_pet_power   = 0;
		combat_award_exp   = 0;
		combat_award_money = 0;
        combat_pvp_type    = 0;
		combat_mob_killed_list.clear();
		combat_award_items_drop.clear();
		combat_award_items_lottery.clear();
        combat_player_killed_list.clear();
	}
};

class PlayerSkillData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerSkillData, PLAYER_SKILL_DATA);
public:
	struct SkillTree
	{
		int32_t id;
		int8_t  level;
		int8_t  index;
		int8_t  location;
		int8_t  active;
		NESTED_DEFINE(id, level, index, location, active);
	};
	std::vector<SkillTree> sk_tree_list;

	OBJECTATTR_DEFINE(sk_tree_list);

	virtual void Release()
	{
		sk_tree_list.clear();
	}
};

class PlayerPetData: public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerPetData, PLAYER_PET_DATA);
public:
	struct PetInfo
	{
		int32_t pet_id;
		int16_t pet_combat_pos;
		int16_t pet_item_idx;

		NESTED_DEFINE(pet_id, pet_combat_pos, pet_item_idx);
	};

	int32_t pet_power;           // 宠物能量值
	int32_t pet_power_cap;       // 宠物能量上限
	std::string combat_pet_data; // 战宠信息

	OBJECTATTR_DEFINE(pet_power, pet_power_cap, combat_pet_data);

	virtual void Release()
	{
		pet_power = 0;
		pet_power_cap = 0;
		combat_pet_data.clear();
	}
};

struct FilterInfo
{
    FilterInfo() 
        : filter_type(0),
          timeout(0),
          effect_id(0)
    { }

    int32_t filter_type;
    int32_t timeout;
    int32_t effect_id;
    std::string detail; // 每种类型的存盘filter有自己的格式，格式对应scalable/player/filter_data.proto
    NESTED_DEFINE(filter_type, timeout, effect_id, detail);
};

class PlayerBuffData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerBuffData , PLAYER_BUFF_DATA);
public:
    std::vector<FilterInfo> buff_list;
	OBJECTATTR_DEFINE(buff_list);

	virtual void Release()
	{
		buff_list.clear();
	}
};

class PlayerTaskData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerTaskData , PLAYER_TASK_DATA);
public:
	std::string active_task;
	std::string finish_task;
	std::string finish_time_task;
	std::string task_diary;
	std::string task_storage;

	OBJECTATTR_DEFINE(active_task, finish_task, finish_time_task, task_diary, task_storage);

	virtual void Release()
	{
		active_task.clear();
		finish_task.clear();
		finish_time_task.clear();
		task_diary.clear();
		task_storage.clear();
	}
};

class PlayerObjectBWList : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerObjectBWList, PLAYER_OBJECT_BW_LIST);
public:
	std::string black_list; // 矿或npc的模板id
	std::string white_list;

	OBJECTATTR_DEFINE(black_list, white_list);

	virtual void Release()
	{
		black_list.clear();
		white_list.clear();
	}
};

// instance data
class PlayerInstanceData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerInstanceData, PLAYER_INSTANCE_DATA);
public:
	class Detail : public shared::copyable
	{
	public:
		int64_t ins_serial_num;
		int32_t ins_templ_id;
		int32_t world_id;
		int32_t world_tag;
		int64_t ins_create_time;
		int32_t is_consumed_cond;
		float   ins_pos_x;
		float   ins_pos_y;

		NESTED_DEFINE(ins_serial_num, ins_templ_id, world_id, world_tag, ins_create_time, is_consumed_cond, ins_pos_x, ins_pos_y);

		void release()
		{
			ins_serial_num   = 0;
			ins_templ_id     = 0;
			world_id         = 0;
			world_tag        = 0;
			ins_create_time  = 0;
			is_consumed_cond = 0;
			ins_pos_x        = 0.0f;
			ins_pos_y        = 0.0f;
		}
	};

	Detail cur_ins;
	std::vector<Detail> history_ins;

	OBJECTATTR_DEFINE(cur_ins, history_ins);

	virtual void Release()
	{
		cur_ins.release();
		history_ins.clear();
	}
};

class PlayerMallData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerMallData, PLAYER_MALL_DATA);
public:
	struct OrderEntry
	{
		int32_t goods_id;    //购买的商品ID
		int32_t goods_count; //购买的商品个数
		int32_t timestamp;   //购买时间
		NESTED_DEFINE(goods_id, goods_count, timestamp);
	};

	int32_t mall_cash;                  // 玩家可用点数
	int32_t mall_cash_used;             // 玩家已使用点数
	std::vector<OrderEntry> order_list; // 商城购买记录

	OBJECTATTR_DEFINE(mall_cash, mall_cash_used, order_list);

	virtual void Release()
	{
		order_list.clear();
	}
};

class PlayerMailList : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerMailList, PLAYER_MAIL_LIST);
public:
	struct Mail
	{
		int8_t op;				// 邮件操作
		int64_t id;				// 邮件标识ID
		int32_t attr;			// 邮件属性
		int32_t time;			// 邮件发送的时间
		int64_t sender;			// 发送者RoleID
		std::string name;		// 发送者名字
		std::string title;		// 邮件标题
		std::string content;	// 邮件内容

		NESTED_DEFINE(op, id, attr, time, sender, name, title, content);
	};

	std::vector<Mail> mail_list;

	OBJECTATTR_DEFINE(mail_list);

	virtual void Release()
	{
		mail_list.clear();
	}
};

class PlayerSpecMoney : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerSpecMoney, PLAYER_SPEC_MONEY);
public:
	int64_t score_total;
	int64_t score_used;

	OBJECTATTR_DEFINE(score_total, score_used);

	virtual void Release()
	{
		score_total = 0;
		score_used = 0;
	}
};

class PlayerTalentData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerTalentData, PLAYER_TALENT_DATA);
public:
	struct talent_entry
	{
		int32_t talent_group_id;
		int32_t talent_id;
		int16_t talent_level;
		NESTED_DEFINE(talent_group_id, talent_id, talent_level);
	};

	std::vector<talent_entry> talent_list;

	OBJECTATTR_DEFINE(talent_list);

	virtual void Release()
	{
		talent_list.clear();
	}
};

class PlayerAuctionData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerAuctionData, PLAYER_AUCTION_DATA);
public:
    enum
    {
        PLAYER_AUCTION = 0,
        PLAYER_BID     = 1,
    };

    struct auction_entry
    {
        int8_t auction_type; // 0是玩家的拍卖，1是玩家的竞拍
        int64_t auction_id;
        int32_t bid_price;
        NESTED_DEFINE(auction_type, auction_id, bid_price);
    };

    std::vector<auction_entry> auction_list;

    OBJECTATTR_DEFINE(auction_list);

    virtual void Release()
    {
        auction_list.clear();
    }
};

class PlayerFriendData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerFriendData, PLAYER_FRIEND_DATA);
public:
    struct friend_entry
    {
        int64_t roleid;
        int8_t type;
        NESTED_DEFINE(roleid, type);
    };

    std::vector<friend_entry> friend_list;

    OBJECTATTR_DEFINE(friend_list);

    virtual void Release()
    {
        friend_list.clear();
    }
};

class PlayerReputationData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerReputationData, PLAYER_REPUTATION_DATA);
public:
    struct reputation_entry
    {
        int32_t reputation_id;
        int32_t reputation_point;
        NESTED_DEFINE(reputation_id, reputation_point)
    };

    std::vector<reputation_entry> reputation_list;

    OBJECTATTR_DEFINE(reputation_list);

    virtual void Release()
    {
        reputation_list.clear();
    }
};

class PlayerAchieveData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerAchieveData , PLAYER_ACHIEVE_DATA);
public:
	std::string finish_achieve;
    std::string achieve_data;

	OBJECTATTR_DEFINE(finish_achieve, achieve_data);

	virtual void Release()
	{
		finish_achieve.clear();
		achieve_data.clear();
	}
};

class PlayerEnhanceData : public ObjectAttrPacket
{
	DECLARE_OBJECTATTR(PlayerEnhanceData , PLAYER_ENHANCE_DATA);
public:
    struct enhance_entry
    {
        int32_t enhance_id;
        int8_t slot_status;
        int8_t open_mode;
        NESTED_DEFINE(enhance_id, slot_status, open_mode)
    };

    std::vector<enhance_entry> enhance_list;

	OBJECTATTR_DEFINE(enhance_list);

	virtual void Release()
	{
        enhance_list.clear();
	}
};

class PlayerPropRFData : public ObjectAttrPacket
{
    // 属性强化
    DECLARE_OBJECTATTR(PlayerPropRFData, PLAYER_PROP_REINFORCE_DATA);
public:
    std::string primary_rf_data; // 格式对应scalable/player/prop_reinforce.proto

    OBJECTATTR_DEFINE(primary_rf_data);

    virtual void Release()
    {
        primary_rf_data.clear();
    }
};

class PlayerTitleData : public ObjectAttrPacket
{
    // 称号
    DECLARE_OBJECTATTR(PlayerTitleData, PLAYER_TITLE_DATA);
public:
    int32_t cur_title;      // 当前称号
    std::string title_data; // 角色称号列表

    OBJECTATTR_DEFINE(cur_title, title_data);

    virtual void Release()
    {
        cur_title = 0;
        title_data.clear();
    }
};

class PlayerStarData : public ObjectAttrPacket
{
    // 星盘
    DECLARE_OBJECTATTR(PlayerStarData, PLAYER_STAR_DATA);
public:
    struct star_entry
    {
        int32_t star_id;
        int8_t spark_index;
        int8_t spark_locked;
        NESTED_DEFINE(star_id, spark_index, spark_locked)
    };

    std::vector<star_entry> star_list;

    OBJECTATTR_DEFINE(star_list);

    virtual void Release()
    {
        star_list.clear();
    }
};

class BattleGroundData : public ObjectAttrPacket
{
    // 玩家的战场数据
    DECLARE_OBJECTATTR(BattleGroundData, PLAYER_BATTLEGROUND_DATA);
public:
    struct CurBGInfo
	{
		int64_t bg_serial_num;
		int32_t bg_templ_id;
		int32_t world_id;
		int32_t world_tag;
		int64_t bg_create_time;
		int32_t is_consumed_cond;
		float   bg_pos_x;
		float   bg_pos_y;
		NESTED_DEFINE(bg_serial_num, bg_templ_id, world_id, world_tag, bg_create_time, is_consumed_cond, bg_pos_x, bg_pos_y);

		void release()
		{
			bg_serial_num    = 0;
			bg_templ_id      = 0;
			world_id         = 0;
			world_tag        = 0;
			bg_create_time   = 0;
			is_consumed_cond = 0;
			bg_pos_x         = 0.0f;
			bg_pos_y         = 0.0f;
		}
	};

    struct HistoryData
    {
        HistoryData() { release(); }

        int32_t bg_templ_id;          // 战场模板id
        int64_t checkin_per_day;      // 每日签入存盘的时间 
        int32_t entertimes_per_day;   // 从每日签入存盘时间到现在该副本的进入次数
        NESTED_DEFINE(bg_templ_id, checkin_per_day, entertimes_per_day);

        void release()
        {
            bg_templ_id        = 0;
            checkin_per_day    = 0;
            entertimes_per_day = 0;
        }
    };

    CurBGInfo cur_bg;
    std::vector<HistoryData> history_bg;
    OBJECTATTR_DEFINE(cur_bg, history_bg);

    virtual void Release()
    {
        cur_bg.release();
        history_bg.clear();
    }
};

class PlayerCounterData : public ObjectAttrPacket
{
    // 玩家计数器
    DECLARE_OBJECTATTR(PlayerCounterData, PLAYER_COUNTER_DATA);
public:
    std::string info; // 计数器信息数据，对应scalable/player/counter_data.proto
    OBJECTATTR_DEFINE(info);

    virtual void Release()
    {
        info.clear();
    }
};

class PlayerGeventData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerGeventData, PLAYER_GEVENT_DATA);
public:
    struct gevent_entry
    {
        int32_t gevent_gid;
        int32_t last_join_time;
        int32_t num;
        NESTED_DEFINE(gevent_gid, last_join_time, num);
    };

    std::vector<gevent_entry> gevent_list;
    OBJECTATTR_DEFINE(gevent_list);

    virtual void Release()
    {
        gevent_list.clear();
    }
};

class PlayerPunchCardData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerPunchCardData, PLAYER_PUNCH_CARD_DATA);
public:
    int32_t last_punch_time;       // 上次签到时间
    int32_t last_refresh_time;     // 上次检查刷新的时间
    int32_t re_punch_count;        // 自己帮好友补签的次数
    int32_t history_punch;         // 所有签到次数
    int32_t history_award_got;     // 历史奖励领取到哪一档，记录的是当前档的累计次数
    int32_t monthly_punch_mask;    // 每月已经签到哪几天，是一个mask，标记[1,31]
    std::string monthly_award_got; // 当月已经领取的奖励，记录的是累计次数
                                   // 对应scalable/player/punch_card.proto

    OBJECTATTR_DEFINE(last_punch_time, last_refresh_time, re_punch_count, history_punch, history_award_got, monthly_punch_mask, monthly_award_got);

    virtual void Release()
    {
        last_punch_time    = 0;
        last_refresh_time  = 0;
        re_punch_count     = 1;
        history_punch      = 0;
        history_award_got  = 0;
        monthly_punch_mask = 0;
        monthly_award_got.clear();
    }
};

class PlayerParticipationData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerParticipationData, PLAYER_PARTICIPATION);
public:
    int32_t participation;
    std::string award_info;

    OBJECTATTR_DEFINE(participation, award_info);

    virtual void Release()
    {
        participation = 0;
        award_info.clear();
    }
};

class PlayerMountData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerMountData, PLAYER_MOUNT_DATA);
public:
    int32_t mount_index;                        // 当前骑乘的坐骑索引
    std::vector<int32_t> mount_equip_level;     // 骑具等级列表

    OBJECTATTR_DEFINE(mount_index, mount_equip_level);

    virtual void Release()
    {
        mount_index = -1;
        mount_equip_level.clear();
    }
};

class PlayerBossChallengeData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerBossChallengeData, PLAYER_BOSS_CHALLENGE_DATA);
public:
    std::string challenge_data;

    OBJECTATTR_DEFINE(challenge_data);

    virtual void Release()
    {
        challenge_data.clear();
    }
};

class PlayerArenaData : public ObjectAttrPacket
{
    DECLARE_OBJECTATTR(PlayerArenaData, PLAYER_ARENA_DATA);
public:
    int32_t win;            // 胜场数
    int32_t lose;           // 失败数
    int32_t win_streak;     // 连胜数
    int32_t today_score;    // 今日积分
    int32_t week_score;     // 本周积分
    int32_t total_score;    // 总积分(决定排位)
    int32_t free_ticket;    // 今日剩余的免费挑战次数
    int32_t buy_ticket;     // 付费购买的挑战次数

    OBJECTATTR_DEFINE(win, lose, win_streak, today_score, week_score, total_score, free_ticket, buy_ticket);

    virtual void Release()
    {
        win = 0;
        lose = 0;
        win_streak = 0;
        today_score = 0;
        week_score = 0;
        total_score = 0;
        free_ticket = 0;
        buy_ticket = 0;
    }
};

} // namespace common

#endif // COMMON_OBJDATAPOOL_PLAYER_ATTR_DECLAR_H_
