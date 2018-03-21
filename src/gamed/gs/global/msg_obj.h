#ifndef GAMED_GS_GLOBAL_MSG_OBJ_H_
#define GAMED_GS_GLOBAL_MSG_OBJ_H_

///
/// 本文件已经在message.h里做了include，别的文件不需要单独引用
///

namespace gamed {

///
/// message type define
/// 普通MSG：接收方是Object
///
enum 
{
// 0
	GS_MSG_NULL = kInvalidMsgType,     // 空消息
	GS_MSG_OBJ_HEARTBEAT,              // Obj的心跳协议
	GS_MSG_OBJ_ENTER_VIEW,             // object进入视野
	GS_MSG_OBJ_LEAVE_VIEW,             // object离开视野
	GS_MSG_OBJ_SESSION_REPEAT,         // 表示session要继续执行

// 5 
    GS_MSG_OBJ_SESSION_END,            // 对象的session完成
	GS_MSG_ENTER_RULES_AREA,           // 通知object，它已进入带规则区域
	GS_MSG_LEAVE_RULES_AREA,           // 通知object，它已离开带规则区域
	GS_MSG_PLAYER_TRIGGER_COMBAT,      // 玩家发起战斗，需要等待应答后才能进入战斗
	GS_MSG_PLAYER_TRIGGER_COMBAT_RE,   // 玩家触发战斗的回应，不管成功还是失败都必须回应

// 10
	GS_MSG_COMBAT_SEND_CMD,            // 战斗发送CMD
	GS_MSG_COMBAT_PVE_RESULT,          // PVE战斗结果,发送给player
	GS_MSG_COMBAT_START,               // 战斗开始
	GS_MSG_COMBAT_PVE_END,             // PVE战斗结束
	GS_MSG_OBJ_TRIGGER_COMBAT,         // 除了玩家外，其他对象发起战斗，接收方应该是player

// 15
	GS_MSG_OBJ_TRIGGER_COMBAT_RE,      // 其他对象发起战斗的回应
	GS_MSG_HATE_YOU,                   // 被对方hate，相当于已经被对方盯上
	GS_MSG_SERVICE_HELLO,              // say hello to Npc, Npc是ServiceNpc
	GS_MSG_SERVICE_GREETING,           // ServiceNpc回应player的Hello，这里可能已对Hello做出相应AI
	GS_MSG_SERVICE_REQUEST,            // 发送服务请求，接收方可能是player或者npc，player收到说明是UI服务

// 20
    GS_MSG_SERVICE_DATA,               // 服务请求的回应
	GS_MSG_ERROR_MESSAGE,              // 内部传递错误消息，多半是object发给player，然后发送给client
	GS_MSG_JOIN_TEAM,                  // 有成员加入队伍
	GS_MSG_LEAVE_TEAM,                 // 有成员离开队伍
	GS_MSG_CHANGE_TEAM_LEADER,         // 队伍更换队长

// 25
	GS_MSG_CHANGE_TEAM_POS,            // 队伍战斗站位变化
	GS_MSG_TEAM_INFO,                  // 队伍创建
	GS_MSG_QUERY_TEAM_MEMBER,          // 队伍成员查询该玩家的具体信息
	GS_MSG_COMPANION_ENTER_COMBAT,     // 队友或者同阵营的玩家进入战斗，检查如果满足条件也进入这场战斗
	GS_MSG_PLAYER_REGION_TRANSPORT,    // 通知player开始传送

// 30
	GS_MSG_PLAYER_SWITCH_ERROR,        // 通知player在本gs内地图传送失败
	GS_MSG_GATHER_REQUEST,             // 采集请求，发给matter
	GS_MSG_GATHER_REPLY,               // 采集回复，发给player
	GS_MSG_GATHER_CANCEL,              // 撤消采集，发给matter
	GS_MSG_GATHER_COMPLETE,            // player采集完成，发给matter

// 35
	GS_MSG_GATHER_RESULT,              // 采集完成，结果发给player
	GS_MSG_MINE_HAS_BEEN_ROB,          // 矿被别人抢走，发给player
	GS_MSG_ADD_FILTER,                 // 添加Buff
	GS_MSG_DEL_FILTER,                 // 删除Buff
	GS_MSG_SYS_TRIGGER_COMBAT,         // gs内部触发的战斗

// 40
	GS_MSG_QUERY_EXTPROP,              // 请求其它玩家的扩展属性
	GS_MSG_DRAMA_GATHER,               // 剧情采集
	GS_MSG_DRAMA_GATHER_MG_RESULT,     // 剧情采集，小游戏(minigame)开启方式的采集结果
	GS_MSG_INS_TRANSFER_PREPARE,       // 传送进入副本，准备阶段
	GS_MSG_INS_TRANSFER_START,         // 可以开始传送进副本, 使用msg_player_region_transport做参数

// 45
	GS_MSG_QUERY_EQUIPCRC,             // 请求其它玩家的装备CRC
	GS_MSG_QUERY_EQUIPMENT,            // 请求其它玩家的装备信息
	GS_MSG_GET_STATIC_ROLE_INFO,       // 请求指定玩家的静态角色信息
	GS_MSG_ENTER_INS_REPLY,            // master回复副本进入消息
	GS_MSG_WORLD_CLOSING,              // 地图关闭，所有的object都能接受到

// 50
    GS_MSG_RELATE_MAP_ELEM_CLOSED,     // 发给Object告知关联的地图元素已经被关闭
	GS_MSG_SERVICE_QUERY_CONTENT,      // 发给服务NPC，获取服务内容
	GS_MSG_SERVICE_ERROR,              // NPC服务发生错误
	GS_MSG_SHOP_SHOPPING_FAILED,       // 商店购物失败
	GS_MSG_WORLD_DELIVER_TASK,         // 地图向玩家发放任务

// 55
	GS_MSG_GET_ATTACH_REPLY,		   // 获取邮件附件
	GS_MSG_SEND_MAIL_REPLY,			   // 发送邮件回复
	GS_MSG_DELETE_MAIL_REPLY,		   // 发送邮件回复
	GS_MSG_ANNOUNCE_NEW_MAIL,		   // 有新邮件
	GS_MSG_QUERY_TEAM_COMBAT,          // 队友查询战斗id的消息，用于共享血量战斗

// 60
	GS_MSG_QUERY_TEAM_COMBAT_RE,       // 队友查询战斗id的回执，参数是战斗id，id是0表示查询失败
	GS_MSG_COMBAT_WORLD_BOSS_END,      // 世界BOSS战斗结束
	GS_MSG_TEAMMEMBER_LOCALINS_REPLY,  // 本服组队副本队员给队长的回应
	GS_MSG_QUIT_TEAM_LOCAL_INS,        // 队员退出副本挑战界面，只有队长能收到
	GS_MSG_TEAM_LOCAL_INS_INVITE,      // 队长向队员发组队本服副本邀请

// 65
	GS_MSG_PLAYER_QUIT_INS,            // 玩家主动退出副本，发给自己
	GS_MSG_MAP_TEAM_INFO,              // 地图队伍信息，创建时同步发给player
	GS_MSG_MAP_TEAM_JOIN,              // 新玩家加入地图队伍
	GS_MSG_MAP_TEAM_LEAVE,             // 玩家离开地图队伍
	GS_MSG_MAP_TEAM_CHANGE_POS,        // 地图队伍玩家交换位置

// 70
	GS_MSG_MAP_TEAM_CHANGE_LEADER,     // 地图队伍换队长
	GS_MSG_MAP_TEAM_QUERY_MEMBER,      // 地图队伍查询队友信息
	GS_MSG_MAP_TEAM_STATUS_CHANGE,     // 地图队伍队员状态发生变化，上线或下线
	GS_MSG_INS_FINISH_RESULT,          // 副本结束结果
	GS_MSG_MAP_QUERY_PLAYER_INFO,      // 地图主动查询玩家信息

// 75
    GS_MSG_TEAM_CROSS_INS_INVITE,      // 队长发起跨服副本邀请
    GS_MSG_TEAM_CROSS_INS_INVITE_RE,   // 回应队长的跨服副本邀请
	GS_MSG_COMBAT_PVP_RESULT,          // PVP战斗结果,发送给player
	GS_MSG_COMBAT_PVP_END,             // PVP战斗结束
    GS_MSG_DUEL_REQUEST,               // 发起决斗请求

// 80
    GS_MSG_DUEL_REQUEST_RE,            // 决斗请求的回应
    GS_MSG_TEAMMATE_DUEL_REQUEST,      // 由队员发起的决斗请求
    GS_MSG_TEAMMATE_DUEL_REQUEST_RE,   // 回应队员，队长是否同意他发起决斗
    GS_MSG_DUEL_PREPARE,               // 决斗准备，开始倒计时
    GS_MSG_START_JOIN_DUEL_COMBAT,     // 战斗已经创建好，邀请对方加入战斗

// 85
    GS_MSG_ENHANCE_FAILED,             // 附魔失败
	GS_MSG_BG_TRANSFER_PREPARE,        // 进入战场，准备阶段
	GS_MSG_ENTER_BG_REPLY,             // master回复战场进入消息
	GS_MSG_BG_TRANSFER_START,          // 可以开始传送进战场, 使用msg_player_region_transport做参数
    GS_MSG_NOTIFY_LANDMINE_INFO,       // 进入暗雷区域后，给玩家同步暗雷信息

// 90
    GS_MSG_PLAYER_QUIT_BG,             // 玩家主动退出战场
    GS_MSG_BG_FINISH_RESULT,           // 战场结束
    GS_MSG_MAP_TEAM_JOIN_TEAM,         // 一方玩家发起申请或邀请
    GS_MSG_GLOBAL_COUNTER_CHANGE,      // 全局计数器改变，通知玩家
    GS_MSG_MAP_COUNTER_CHANGE,         // 地图计数器改变，通知玩家

// 95
    GS_MSG_P_TRIGGER_COMBAT_SUCCESS,   // 玩家发起的PVE战斗成功创建，通知保留模型的怪
    GS_MSG_OBJECT_TELEPORT,            // 对象瞬移
    GS_MSG_MONSTER_MOVE,               // 定点怪脚本控制移动
    GS_MSG_RE_PUNCH_CARD_HELP,         // 帮助好友补签每日签到
    GS_MSG_RE_PUNCH_CARD_HELP_REPLY,   // 好友补签的回复

// 100
    GS_MSG_RE_PUNCH_CARD_HELP_ERROR,   // 好友帮忙补签但改好友已经不在线或已经有别人帮他补签
    GS_MSG_WB_COLLECT_PLAYER_INFO,     // world-boss收集参加战斗的玩家信息
    GS_MSG_MAP_TEAM_TIDY_POS,          // 整理地图组队的队员位置
    GS_MSG_WORLD_BOSS_DEAD,            // 世界BOSS死亡
    GS_MSG_MONSTER_SPEED,              // 设置怪的移动速度

// 105


    ///
    /// GM所采用的消息
    ///
// 0
	GS_MSG_GM_GETPOS = 800,            // 取得指定玩家的坐标

	GS_MSG_MAX = 5000
};


///
/// parameter struct
///  （1）复杂的消息参数可以以pack的形式定义在msg_pack_def.h里，该类消息最好不是很频繁的消息，
///       因为marshal，unmarshal本身有一定开销，虽然开销不大。
///
struct msg_obj_enter_view
{
	XID     object;
	int64_t param;
};

struct msg_obj_leave_view
{
	XID object;
};

struct msg_combat_result
{
	int32_t hp;
	int16_t result;
	int32_t exp_gain;
	int32_t money_gain;
	int16_t item_count;
	struct award_item
	{
		int32_t item_id;
		int32_t item_count;
	} items[];

	int32_t mob_count;
	struct mob_killed
	{
		int32_t mob_id;
		int32_t mob_count;
	} mob_killed_list[];
};

struct msg_combat_send_cmd
{
	const void* packet;
};

enum MSG_MonsterType
{
	MSG_MT_NORMAL = 0, // 普通怪
	MSG_MT_WORLD_BOSS, // 世界boss
};

// 怪物触发战斗时会带上这个参数发给player 
struct msg_obj_trigger_combat
{
	msg_obj_trigger_combat() 
		: monster_type(MSG_MT_NORMAL),
		  monster_group_id(0),
		  battle_scene_id(0),
		  require_combatend_notify(false),
		  landmine_interval(0)
	{ }

	int8_t  monster_type;     // 怪物类型，与MSG_MonsterType枚举相对应
	int32_t monster_group_id;
	int32_t battle_scene_id;
	bool    require_combatend_notify;
	int32_t landmine_interval; // 暗雷区域的遇敌间隔
};

struct msg_obj_trigger_combat_re
{
	bool    is_success;
    int32_t landmine_interval;
};

// 玩家触发战斗的回应
struct msg_player_trigger_combat_re
{
	bool    is_success;
	bool    require_combatend_notify;
	int8_t  monster_type;     // 怪物类型，与MSG_MonsterType枚举相对应
	int32_t monster_group_id;
	int32_t battle_scene_id;
};

struct msg_join_team
{
	RoleID new_memberid;
	int32_t pos;
};

struct msg_change_team_pos
{
	int32_t src_index;
	int32_t des_index;
};

struct msg_team_info
{
	int32_t teamid;
	RoleID  leaderid;
	RoleID  pos1;
	RoleID  pos2;
	RoleID  pos3;
	RoleID  pos4;
};

struct msg_query_team_member
{
	RoleID  query_roleid;
	int32_t link_id;
	int32_t client_sid_in_link;
};

struct msg_player_region_transport
{
	msg_player_region_transport()
		: elem_id(0),
		  ins_templ_id(0),
          bg_templ_id(0),
          is_resurrect(false),
		  source_world_id(-1),
		  target_world_id(-1)
	{ }

	int32_t   elem_id;      // 可以不填，只有传送区域发起的传送需要填
	int32_t   ins_templ_id; // 可以不填，不进副本可以不填
    int32_t   bg_templ_id;  // 可以不填，不进战场可以不填
    bool      is_resurrect; // 可以不填，是否是复活传送
	int32_t   source_world_id;
	int32_t   target_world_id;
	A2DVECTOR target_pos;
};

struct msg_combat_end
{
	static const size_t kMaxCombatPlayers = 8;

	msg_combat_end() 
		: is_win(false),
		  count(0)
	{ 
		memset(&players, 0, sizeof(players)); 
	}

	void push_back(RoleID id) 
	{
		assert(count <= kMaxCombatPlayers);
		players[count++] = id;
	}

	RoleID at(size_t index) const
	{
		assert(index < kMaxCombatPlayers);
		return players[index];
	}

	size_t size() const
	{
		return count;
	}

	void set_combat_win(bool win)
	{
		is_win = win;
	}

	bool combat_is_win() const
	{
		return is_win;
	}

private:
	bool   is_win;
	size_t count;
	RoleID players[kMaxCombatPlayers];
};

struct msg_companion_enter_combat
{
    int32_t combat_id;
    int32_t world_boss_id;
};

struct msg_gather_request
{
	int32_t gather_seq_no;
	int32_t gather_timeout; // 单位:s
};

struct msg_gather_reply
{
	int32_t gather_seq_no;
	int32_t templ_id;
};

struct msg_gather_result
{
	int32_t gather_seq_no;
	int32_t templ_id;
};

struct msg_sys_trigger_combat
{
    msg_sys_trigger_combat() 
        : challenge_id(0), task_id(0)
    {
    }

    int32_t challenge_id;
    int32_t task_id;
	int32_t monster_group_id;
	int32_t battle_scene_id;
};

struct msg_query_extprop
{
	int64_t requester;
	int32_t link_id;
	int32_t sid_in_link;
};

struct msg_query_equipcrc
{
	int64_t requester;
	int32_t link_id;
	int32_t sid_in_link;
};

struct msg_query_equipment
{
	int64_t requester;
	int32_t link_id;
	int32_t sid_in_link;
};

struct msg_drama_erase_result
{
	int32_t gather_seq_no;
	bool is_success;
};

struct msg_get_static_role_info
{
	int64_t requester;
	int32_t link_id;
	int32_t sid_in_link;
};

struct msg_enter_ins_reply
{
	int8_t  enter_type;
	int8_t  err_code;
	int64_t ins_serial_num;
	int32_t ins_templ_id;
	int32_t world_id;
	int32_t world_tag;
	int64_t ins_create_time;
	int32_t src_world_id;
	float   src_pos_x;
	float   src_pos_y;
	int64_t leader_id;
	int32_t self_team_pos;
};

struct msg_service_query_content
{
	int32_t link_sid;
	int32_t client_sid;
};

struct msg_shop_shopping_failed
{
	int32_t err_code;
	int32_t goods_id;
	int32_t goods_count;
	int32_t goods_remains;
};

struct msg_send_mail_reply
{
	int64_t mailid;
};

struct msg_delete_mail_reply
{
	int64_t mailid;
};

struct msg_query_team_combat_re
{
	int64_t monster_obj_id;
	int32_t combat_id;
};

struct msg_quit_team_local_ins
{
	int64_t member_roleid;
	int32_t ins_group_id;
};

struct msg_ins_transfer_prepare
{
	int32_t ins_templ_id;
	int32_t request_type;
};

struct msg_team_local_ins_invite
{
	int64_t leader_roleid;
	int32_t ins_group_id;
};

struct msg_map_team_change_pos
{
	int32_t src_index;
	int32_t des_index;
};

struct msg_map_team_query_member
{
	int32_t linkid;
	int32_t sid_in_link;
};

struct msg_map_team_status_change
{
	int64_t roleid;
	bool online;
};

enum InsFinishResult
{
	ICR_PLAYER_FAIL = 0,
	ICR_PLAYER_WIN,
};

struct msg_ins_finish_result
{
	msg_ins_finish_result()
		: clear_time(std::numeric_limits<int32_t>::max()),
		  ins_result(ICR_PLAYER_FAIL),
		  is_svr_record(false),
          svr_clear_time(-1)
	{ }

	int32_t clear_time;
	int32_t ins_result;     // 对应InsFinishResult枚举
	bool    is_svr_record;  // 是否打破服务器纪录
    int32_t svr_clear_time; // 上次破服务器记录的时间
};

struct msg_team_cross_ins_invite
{
    int64_t leader_roleid;
    int32_t ins_group_id;
    int32_t ins_templ_id;
};

struct msg_duel_request
{
    int64_t requester; // 发起者的roleid 
    bool  is_team_req; // 对方是不是组队发起决斗
    float requester_pos_x;
    float requester_pos_y;
};

enum DuelRequestReply
{
    DRR_SUCCESS = 0, // 决斗成功
    DRR_NOT_AGREE,   // 不同意
    DRR_LEADER_NOT_AROUND, // 队长不在附近
    DRR_TOO_FAR,     // 距离太远
};

struct msg_duel_request_re
{
    int8_t  duel_result; // 对应上面的DuelRequestReply枚举
};

struct msg_teammate_duel_request_re
{
    bool    agreement;
    int64_t duel_roleid;
};

struct msg_enhance_failed
{
	int32_t err_code;
};

struct msg_bg_transfer_prepare
{
	int32_t bg_templ_id;
	int32_t request_type;
};

struct msg_notify_landmine_info
{
    bool    is_leave_area;
    int32_t elem_id;
    int32_t encounter_timer;
};

struct msg_bg_finish_result
{
	int32_t clear_time;
};

struct msg_global_counter_change
{
    int32_t index;
    int32_t value;
};

struct msg_map_counter_change
{
    int32_t index;
    int32_t value;
};

struct msg_object_teleport
{
    int32_t elem_id;
    float   pos_x;
    float   pos_y;
    int8_t  dir;
};

struct msg_monster_move
{
    int32_t elem_id;
    float   pos_x;
    float   pos_y;
    float   speed;
};

struct msg_monster_speed
{
    int32_t elem_id;
    float   speed;
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_MSG_OBJ_H_
