#ifndef GAMED_GS_GLOBAL_MSG_PLANE_H_
#define GAMED_GS_GLOBAL_MSG_PLANE_H_

///
/// 本文件已经在message.h里做了include，别的文件不需要单独引用
///

namespace gamed {

///
/// 地图MSG：接收方是WorldManager
///
enum
{
// 0
	GS_PLANE_MSG_NULL = GS_MSG_MAX,
	GS_PLANE_MSG_RULE_AREA_ENABLE,       // 地图内有规则区域激活
	GS_PLANE_MSG_RULE_AREA_DISABLE,      // 地图内有规则区域取消
	GS_PLANE_MSG_NPC_REBORN,             // npc重生消息
	GS_PLANE_MSG_AREA_OBJ_RELEASE,       // area释放，通知world把它从aoi中删除

// 5
	GS_PLANE_MSG_SWITCH_REQUEST,         // 向别的地图（非玩家所在地图）发送消息，请求开始传送，回SWITCH_REPLY
	GS_PLANE_MSG_SWITCH_REPLY,           // 传送请求被确认
	GS_PLANE_MSG_MATTER_REBORN,          // matter重生消息
	GS_PLANE_MSG_MAP_MONSTER_KILLED,     // 地图内怪物被玩家杀死
	GS_PLANE_MSG_MAP_ELEM_ACTIVE,        // 通知地图元素已经激活

// 10
    GS_PLANE_MSG_MAP_ELEM_DEACTIVE,      // 通知地图元素已经取消
	GS_PLANE_MSG_ENABLE_MAP_ELEM,        // 激活某个地图元素
	GS_PLANE_MSG_DISABLE_MAP_ELEM,       // 取消某个地图元素
	GS_PLANE_MSG_QUERY_NPC_ZONE_INFO,    // 查询npc区域信息
	GS_PLANE_MSG_MODIFY_MAP_COUNTER,     // 修改地图的计数器

// 15
	GS_PLANE_MSG_ACTIVE_MAP_CLOCK,       // 激活地图的计时器
	GS_PLANE_MSG_DEACTIVE_MAP_CLOCK,     // 取消地图的计时器
	GS_PLANE_MSG_QUERY_MAP_TEAM_INFO,    // 查询地图内组队信息
	GS_PLANE_MSG_PLAYER_QUIT_MAP,        // 玩家主动地图
	GS_PLANE_MSG_MAP_TEAM_CHANGE_POS,    // 玩家更换位置

// 20
	GS_PLANE_MSG_MAP_TEAM_CHANGE_LEADER, // 更换地图队伍的队长
	GS_PLANE_MSG_SYS_CLOSE_INS,          // 系统主动关闭副本，包括任务、脚本等系统可以直接关闭副本
	GS_PLANE_MSG_MAP_QUERY_PINFO_RE,     // 地图内查询玩家信息的回执
    GS_PLANE_MSG_MAP_DELIVER_TASK,       // 发放任务给地图里所有玩家
    GS_PLANE_MSG_MAP_PLAYER_GATHER_MINE, // 在地图里玩家采集矿成功会给副本发一个消息

// 25
    GS_PLANE_MSG_SYS_CLOSE_BG,           // 系统主动关闭战场，包括任务、脚本等系统可以直接关闭战场
    GS_PLANE_MSG_MT_APPLY_FOR_JOIN,      // 地图组队，玩家申请加入某个队伍
    GS_PLANE_MSG_MT_LEAVE_TEAM,          // 地图组队，玩家主动退出队伍
    GS_PLANE_MSG_MT_KICKOUT_MEMBER,      // 地图组队，队长踢人
    GS_PLANE_MSG_MAP_PROMPT_MESSAGE,     // 地图提示信息

// 30
    GS_PLANE_MSG_MAP_COUNTER_SUBSCRIBE,  // 地图计数器的订阅及退订
    GS_PLANE_MSG_SPOT_MAPELEM_TELEPORT,  // 脚本对某个定点地图元素做瞬移
    GS_PLANE_MSG_SPOT_MONSTER_MOVE,      // 脚本对摸个定点怪
    GS_PLANE_MSG_REACH_DESTINATION,      // 对象已经移动到指定点，脚本移动怪
    GS_PLANE_MSG_MAP_COUNTDOWN,          // 地图显示倒计时

// 35
    GS_PLANE_MSG_WORLD_BOSS_DEAD,        // 世界BOSS死亡通知地图
    GS_PLANE_MSG_LOCK_MAP_COUNTER,       // 锁定地图计数器
    GS_PLANE_MSG_UNLOCK_MAP_COUNTER,     // 解锁地图计数器
    GS_PLANE_MSG_SPOT_MONSTER_SPEED,     // 设置定点怪移动速度

	GS_PLANE_MSG_MAX
};


///
/// parameter struct
///
struct plane_msg_npc_reborn
{
	int32_t elem_id;
	int32_t templ_id;
};

struct plane_msg_switch_request
{
	RoleID    player_roleid;
	A2DVECTOR dest_pos;
};

struct plane_msg_switch_reply
{
	bool      is_success; 
	RoleID    player_roleid;
	A2DVECTOR dest_pos;
};

struct plane_msg_matter_reborn
{
	int32_t elem_id;
	int32_t templ_id;
};

struct plane_msg_query_npc_zone_info
{
	int32_t elem_id;
	int32_t link_id;
	int32_t sid_in_link;
};

// Mmc -- Modify map counter
enum MicOpType
{
	MMC_INCREASE = 1,
	MMC_DECREASE,
	MMC_ASSIGNMENT,
};

struct plane_msg_modify_map_counter
{
	int32_t op_type; // 对应上面的MmcOpType
	int32_t index;
	int32_t value;
};

struct plane_msg_active_map_clock
{
	int32_t index;
	int32_t times;
	int32_t seconds;
};

struct plane_msg_map_team_change_pos
{
	int32_t src_index;
	int32_t des_index;
};

struct plane_msg_sys_close_ins
{
	enum SysType
	{
		ST_TASK,
		ST_SCRIPT,
	};

	enum InsResult
	{
		PLAYER_FAILURE,
		PLAYER_VICTORY,
	};

	int32_t sys_type;   // 对应枚举SysType
	int32_t ins_result; // 对应枚举InsResult
};

struct plane_msg_map_query_pinfo_re
{
	int32_t combat_value;
};

struct plane_msg_sys_close_bg
{
	enum SysType
	{
		ST_TASK,
		ST_SCRIPT,
	};

	enum BGResult
	{
		FACTION_A_VICTORY = 1,
		FACTION_B_VICTORY,
		FACTION_C_VICTORY,
		FACTION_D_VICTORY,
	};

	int32_t sys_type;   // 对应枚举SysType
	int32_t bg_result;  // 对应枚举BGResult
};

struct plane_msg_map_prompt_message
{
    int32_t index;    // 第几句话
    int32_t delay;    // 延时几秒播
    int32_t duration; // 持续多长时间
};

struct plane_msg_map_counter_subscribe
{
    int32_t index;    // 计数器index，-1表示玩家离开地图
    bool    is_subscribe; // true表示订阅，false表示退订
};

struct plane_msg_spot_mapelem_teleport
{
    int32_t elem_id;  // 定点地图元素id
    float   pos_x;    // 瞬移目标点的x坐标
    float   pos_y;    // 瞬移目标点的y坐标
    int8_t  dir;      // 瞬移到目标点后的朝向
};

struct plane_msg_spot_monster_move
{
    int32_t elem_id;  // 定点地图元素id
    float   pos_x;    // 移动目标点的x坐标
    float   pos_y;    // 移动目标点的y坐标
    float   speed;    // 米每秒
};

struct plane_msg_spot_monster_speed
{
    int32_t elem_id;  // 定点地图元素id
    float   speed;    // 米每秒
};

struct plane_msg_reach_destination
{
    int32_t elem_id;  // 怪物的地图元素
    float   pos_x;    // 到达地点的x坐标
    float   pos_y;    // 到达地点的y坐标
};

struct plane_msg_map_countdown
{
    int32_t countdown; // 倒计时数
    int32_t screen_x;  // 在屏幕上显示位置的x坐标
    int32_t screen_y;  // 在屏幕上显示位置的y坐标
};

struct plane_msg_lock_map_counter
{
    int32_t index; // 地图计数器的索引
    int32_t value; // 锁定的值
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_MSG_PLANE_H_
