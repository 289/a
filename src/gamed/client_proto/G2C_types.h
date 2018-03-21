#ifndef GAMED_CLIENT_PROTO_G2C_TYPES_H_
#define GAMED_CLIENT_PROTO_G2C_TYPES_H_

#include "player_visible_state.h"
#include "types.h"

///
/// 几个协议共用的结构体都放在这个文件
///


namespace G2C {

typedef int32_t ObjectID;
typedef int32_t MapID;
typedef int32_t ElemID;


///
/// base
///
struct PlayerVisibleInfo
{
	// 可以看见的player信息（包括hostplayer和elseplayer都会收到）
    RoleID             roleid;
    uint32_t           equip_crc;
    uint8_t            dir;         // 朝向
    uint8_t            role_class;  // 玩家职业（战士、法师、枪械师）
    uint8_t            gender;      // 性别
    int16_t            level;
    int32_t            weapon_id;
    gamed::A2DVECTOR_PACK     pos;
    gamed::PlayerVisibleState visible_state;
    NESTED_DEFINE(roleid, equip_crc, dir, role_class, gender, level, weapon_id, pos, visible_state);
};

struct ObjectVisibleInfo
{
    ObjectID        obj_id;       // 对象在服务器的真实id
    int32_t         tid;          // 模板id，templ_id
    int32_t         eid;          // 地图元素id，elem_id
    uint8_t         dir;          // 朝向
    gamed::A2DVECTOR_PACK  pos;
    NESTED_DEFINE(obj_id, tid, eid, dir, pos);
};


///
/// 物品系统相关
///
struct ItemEntry
{
	int32_t item_id;
	int32_t item_count;
	NESTED_DEFINE(item_id, item_count);
};

struct ItemDetail
{
	int32_t type;
	int16_t index;
	int32_t count;
	int32_t proc_type;
	int32_t expire_date;
    std::string content;
	uint16_t content_crc;
	NESTED_DEFINE(type, index, count, proc_type, expire_date, content, content_crc);
};

struct ItemDetailVec
{
	std::vector<ItemDetail> item_list;
	NESTED_DEFINE(item_list);
};

enum UseItemErrorCode
{
    UIEC_INVALID = 0,     // 无效值
    UIEC_NO_TOOL,         // 使用失败,道具不足
    UIEC_RAND_TASK,       // 使用失败,随机任务失败
    UIEC_DELIVER_TASK,    // 使用失败,接任务失败
};

enum GainItemMode
{
    GIM_NORMAL = 0, // 普通方式
    GIM_INS_AWARD,  // 副本结束奖励的物品
    GIM_TASK,       // 任务系统发放的物品
    GIM_MAIL,       // 邮件系统发放的物品
    GIM_CARD,       // 卡牌系统发放的物品
    GIM_MAX
};


///
/// 战斗系统相关
///
enum CombatPartyType
{
    COMBAT_PARTY_INVALID,
    COMBAT_PARTY_ATTACKER,       //攻方
    COMBAT_PARTY_DEFENDER,       //守方
};

enum CombatPlayerStatus
{
	COMBAT_PLAYER_STATE_NORMAL,  //常态
	COMBAT_PLAYER_STATE_DEAD,    //死亡
	COMBAT_PLAYER_STATE_DYING,   //濒死
	COMBAT_PLAYER_STATE_REVIVE,  //复活
	COMBAT_PLAYER_STATE_ONLINE,  //上线
	COMBAT_PLAYER_STATE_OFFLINE, //下线
};

enum CombatMobType
{
    COMBAT_MOB_TYPE_INVALID,
    COMBAT_MOB_TYPE_PET,         //宠物
    COMBAT_MOB_TYPE_GOLEM,       //魔偶
    COMBAT_MOB_TYPE_MOB,         //怪物
    COMBAT_MOB_TYPE_TEAMNPC,     //组队NPC
    COMBAT_MOB_TYPE_BOSS,        //世界BOSS
};

struct CombatPetInfo
{
	int32_t pet_tid;
	int16_t pet_rank;
	int16_t pet_combat_pos;
	NESTED_DEFINE(pet_tid, pet_rank, pet_combat_pos);
};

struct CombatPlayerInfo
{
	int32_t unit_id;       //在战斗中的分配的ID
	int64_t roleid;        //角色ID
	std::string rolename;  //角色名
	int8_t state;          //状态
	int8_t gender;         //性别
	int8_t cls;            //职业
	int16_t level;         //等级
	int8_t pos;            //站位
	int32_t hp;            //血量
	int32_t mp;            //蓝
	int32_t max_hp;        //最大血量
	int32_t weapon;        //武器ID
	int8_t sneak_attacked; //是否被偷袭
    int8_t which_party;    //属于哪一方,1:攻方;2:守方.
	std::vector<CombatPetInfo> pet_list; //战宠列表
    std::vector<int32_t> skill_list;
	NESTED_DEFINE(unit_id, roleid, rolename, state, gender, cls, level, pos, hp, mp, max_hp, weapon, sneak_attacked, which_party, pet_list, skill_list);
};

struct CombatMobInfo
{
	int32_t unit_id; //战斗对象ID
	int32_t mob_tid; //怪物模板ID
	int32_t mob_hp;  //怪物血量
	int8_t  mob_pos; //怪物站位
	int8_t  sneak_attacked;//是否被偷袭
	NESTED_DEFINE(unit_id, mob_tid, mob_hp, mob_pos, sneak_attacked);
};

struct CombatGolemInfo
{
	int32_t unit_id;   //战斗对象ID
	int32_t golem_tid; //魔偶模板ID
	NESTED_DEFINE(unit_id, golem_tid);
};

struct CombatTeamNpcInfo
{
	int32_t unit_id; //战斗对象ID
	int32_t npc_tid; //NPC模板ID
	int8_t  npc_pos; //NPC站位
	int8_t  sneak_attacked;//是否被偷袭
	NESTED_DEFINE(unit_id, npc_tid, npc_pos, sneak_attacked);
};

struct CombatBuffInfo
{
    uint32_t buff_seq;
    int32_t buff_id;
    int32_t attacher;
    int32_t target;
    NESTED_DEFINE(buff_seq, buff_id, attacher, target);
};


///
/// 组队Npc相关
///
struct BuddyMemberInfo
{
    BuddyMemberInfo() : buddy_id(0), buddy_templ_id(0) { }
    int32_t buddy_id; // 0代表该位置为空，负数代表是玩家自己，正数代表是伙伴
    int32_t buddy_templ_id;
    NESTED_DEFINE(buddy_id, buddy_templ_id);
};


///
/// 属性相关
///
struct prop_entry
{
	enum ValueType
	{
		VT_POINT,
		VT_SCALE,
        VT_SHIELD=0xa,
	};

	int8_t  type;
	int8_t  index;
	int32_t value;
	NESTED_DEFINE(type, index, value);
};

struct buff_entry
{
    int32_t  attacker;
    int32_t  target;
    int32_t  effect_id;
    uint32_t buff_seq;
    std::vector<prop_entry> props;
    std::vector<buff_entry> buff_feedback;
    NESTED_DEFINE(attacker, target, effect_id, buff_seq, props, buff_feedback);
};

struct effect_entry
{
    int32_t effect_id;
    uint32_t buff_seq; //BUFF序列号,为0表示无BUFF
    std::vector<uint32_t/*buff_seq*/> buff_list_del; //哪些BUFF被删除了
    int8_t status; //0:非暴击;1:暴击
    std::vector<prop_entry> props;
    std::vector<buff_entry> buff_feedback;
    NESTED_DEFINE(effect_id, status, props, buff_feedback);
};

struct target_entry
{
    int32_t target;
    //effect_list为空表示target成功闪避本次攻击
    std::vector<effect_entry> effect_list;
    NESTED_DEFINE(target, effect_list);
};

struct frame_entry
{
    std::vector<target_entry> target_list;
    std::vector<target_entry> redir_target_list;
    NESTED_DEFINE(target_list, redir_target_list);
};

struct skill_result
{
    int32_t skill_id;
    int8_t  attack_pos;
    std::vector<frame_entry> frame_list;
    NESTED_DEFINE(skill_id, attack_pos, frame_list);
};

///
/// 宠物相关
///
struct PetEntry
{
	int32_t pet_tid;         // 宠物模板ID
	int32_t pet_exp;         // 宠物经验
	int16_t pet_level;       // 宠物等级
	int16_t pet_blevel;      // 宠物血脉等级
	int16_t pet_combat_pos;  // 宠物在战斗栏的位置(-1表示不在战斗栏中)
	int16_t pet_item_idx;    // 宠物物品在宠物包裹中的位置

	NESTED_DEFINE(pet_tid, pet_exp, pet_level, pet_blevel, pet_combat_pos, pet_item_idx);
};


///
/// 邮件相关
///
struct Mail
{
	int64_t id;
	int32_t attr;
	int32_t time;
	int64_t sender;
	std::string sender_name;
	std::string title;
	std::string content;

	NESTED_DEFINE(id, attr, time, sender, sender_name, title, content);
};

struct MailAttach
{
	int64_t id;
	int32_t attach_cash;
	int32_t attach_score;
	ItemDetailVec attach_item;

	NESTED_DEFINE(id, attach_cash, attach_score, attach_item);
};


///
/// 地图组队相关
///
struct MapTeamMemberInfo
{
	MapTeamMemberInfo() 
	{
		Clear();
	}

	MapTeamMemberInfo(const MapTeamMemberInfo& rhs)
	{
		CopyFrom(rhs);
	}

	MapTeamMemberInfo& operator=(const MapTeamMemberInfo& rhs)
	{
		CopyFrom(rhs);
		return *this;
	}

	void CopyFrom(const MapTeamMemberInfo& rhs)
	{
		roleid    = rhs.roleid;
		first_name  = rhs.first_name;
		mid_name = rhs.mid_name;
		last_name = rhs.last_name;
		gender    = rhs.gender;
		roleclass = rhs.roleclass;
		online    = rhs.online;
	}

	void Clear()
	{
		roleid    = 0;
		first_name = "";
		mid_name = "";
		last_name = "";
		gender    = 0;
		roleclass = 0;
		online    = false;
	}

	int64_t roleid;
	std::string first_name;
	std::string mid_name;
	std::string last_name;
	int8_t gender;
	int8_t roleclass;
	bool   online;
	NESTED_DEFINE(roleid, first_name, mid_name, last_name, gender, roleclass, online);
};


///
/// 拍卖相关
///
enum AuctionCurrencyType
{
    ACT_SCORE, // 学分
    ACT_CASH,  // 元宝
};

struct AuctionItemDetail
{
    int64_t auction_id;
    int8_t  currency_type; // 0表示学分，1表示元宝. 应上面的AuctionCurrencyType
    int32_t buyout_price;
    int32_t bid_price;
    int32_t item_id;
    std::string item_name;
    std::string item_info;
    NESTED_DEFINE(auction_id, currency_type, buyout_price, bid_price, item_id, item_name, item_info);
};


///
/// 好友相关
///
struct FriendInfo
{
    enum FriendType
    {
        FRIEND_NPC = 0x01,
        FRIEND_ENEMY = 0x02,
        FRIEND_BLACKLIST = 0x04,
        FRIEND_NPC_OFFLINE = 0x08,
    };

    int64_t roleid;
    int8_t  flag;

    FriendInfo() 
        : roleid(0), flag(0)
    {
    }

    FriendInfo(int64_t id, int8_t f)
        : roleid(id), flag(f)
    {
    }
    NESTED_DEFINE(roleid, flag);
};
typedef std::vector<FriendInfo> FriendList;


///
/// 初阶属性强化相关
///
enum PrimaryPropRFIndex
{
    PPRFI_HP = 0, // 生命值
    PPRFI_P_ATK,  // 物理攻击
    PPRFI_M_ATK,  // 魔法攻击
    PPRFI_P_DEF,  // 物理防御
    PPRFI_M_DEF,  // 魔法防御
    PPRFI_P_PIE,  // 物理穿透
    PPRFI_M_PIE,  // 魔法穿透
    PPRFI_MAX
};

struct PPRFBriefInfo
{
    int32_t last_recover_time;  // 上次恢复能量的时间点，绝对时间
    int32_t cur_energy;         // 当前的能量值
    NESTED_DEFINE(last_recover_time, cur_energy);
};


///
/// 冷却相关
///
enum PlayerCoolDownIndex
{
    PCD_INDEX_INVALID = 0,
    PCD_INDEX_TIDY_INV, // 整理包裹冷却
    PCD_INDEX_MAX,
};

struct CDEntry
{
    int32_t index;	  //冷却组id，物品id，或系统冷却INDEX，对应PlayerCoolDownIndex枚举
    int32_t interval; //冷却时间(ms)
    NESTED_DEFINE(index, interval);
};


///
/// 卡牌相关
///
struct CardEntry
{
    int32_t id;         // 卡牌模板ID
    int32_t exp;        // 卡牌经验
    int16_t star_id;    // 镶嵌的星辰模板ID
    int16_t item_idx;   // 卡牌物品在卡牌包裹中的位置

    NESTED_DEFINE(id, exp, star_id, item_idx);
};


///
/// 地图相关
///
struct MapPromptMessage
{
    int32_t templ_id;  // 战场或副本的模板id
    int32_t index;     // 对应第几句提示语
    int32_t delay;     // 延迟几秒后播提示，0表示立即播放
    int32_t duration;  // 提示持续多长时间，最小1秒，服务器保证
    NESTED_DEFINE(templ_id, index, delay, duration);
};


///
/// 决斗相关
///
struct DuelMemberInfo
{
    RoleID  role_id;
    int32_t combat_value;
    NESTED_DEFINE(role_id, combat_value);
};


///
/// 副本相关
///

// 副本记录玩家信息
struct InsRecordPInfo
{
    RoleID role_id;
    std::string first_name;
    std::string mid_name;
    std::string last_name;
    int32_t level;
    int8_t  cls;
    int32_t combat_value;
    NESTED_DEFINE(role_id, first_name, mid_name, last_name, level, cls, combat_value);
};

///
/// 活动相关
///
struct GeventInfo
{
    int32_t gevent_gid;     // 活动组ID
    int32_t last_join_time; // 上次参加的时间
    int32_t num;            // 目前参加的次数
    NESTED_DEFINE(gevent_gid, last_join_time, num);
};

///
/// 世界BOSS相关
///
struct WBDamageRecord
{
    int64_t damage;
    int64_t roleid;
    std::string first_name;
    std::string mid_name;
    std::string last_name;
    NESTED_DEFINE(damage, roleid, first_name, mid_name, last_name);
};

struct WBDamageList
{
    int32_t monster_tid; 
    int32_t ranking;  // 该玩家自己的排名
    int64_t damage;   // 该玩家造成的伤害
    std::vector<WBDamageRecord> records; // 具体的排行数据
    NESTED_DEFINE(monster_tid, ranking, damage, records);
};

} // namespace G2C

#endif // GAMED_CLIENT_PROTO_G2C_TYPES_H_
