#ifndef GAMED_CLIENT_PROTO_G2C_PROTO_H_
#define GAMED_CLIENT_PROTO_G2C_PROTO_H_

#include "shared/base/copyable.h"
#include "shared/net/packet/packet.h"
#include "player_visible_state.h"
#include "G2C_error.h"
#include "G2C_types.h"
#include "types.h"


namespace G2C {

using namespace shared::net;
using namespace gamed;

///
/// cmd type
///
enum
{
// 0
    ERROR_MESSAGE = G2C_CMD_LOWER_LIMIT,// gs发的错误消息
	PLAYER_VOLATILE_INFO,               // 玩家易变信息
	PLAYER_VISIBLE_INFO_NOTIFY,         // 玩家可见信息
	ENTER_WORLD_RE,                     // 登录进入世界回应
	PLAYER_START_MOVE,                  // 别的玩家开始移动

// 5
	PLAYER_MOVE,                        // 别的玩家移动
	PLAYER_STOP_MOVE,                   // elseplayer停止移动
	PLAYER_VISIBLE_INFO_CHANGE,         // player可见信息变化
	PLAYER_ENTER_VIEW,                  // else player 进入视野
	PLAYER_LEAVE_VIEW,                  // else player 离开视野

// 10
	OBJECT_VISIBLE_INFO_NOTIFY,         // object可见信息
	OBJECT_VISIBLE_INFO_LIST,           // object可见信息列表
	PULL_PLAYER_BACK_TO_VALID_POS,      // hostplayer被拉到某个位置
	ENTER_EVENT_AREA,                   // 进入事件区域
	NOTIFY_PLAYER_MOVE_PROP,            // 通知玩家的移动属性

// 15
	SELF_ITEM_DETAIL,					// 物品详细数据
	SELF_ITEM_INFO_LIST,                // 物品简要信息列表
	SELF_ITEM_DETAIL_LIST,              // 物品详细数据列表
	DROP_INV_ITEM,                      // 销毁包裹物品
	MOVE_INV_ITEM,                      // 移动包裹物品

// 20
	SELL_INV_ITEM,                      // 出售包裹物品
	EXCHANGE_INV_ITEM,                  // 交换包裹物品
	EQUIP_ITEM,                         // 装备物品
	UNDO_EQUIP,                         // 卸载装备
	USE_ITEM,                           // 使用物品

// 25
	REFINE_EQUIP_RE,                    // 精炼装备
    OBJECT_ENTER_VIEW,                  // object进入视野
	OBJECT_LEAVE_VIEW,                  // object离开视野
	COOL_DOWN_DATA,                     // 冷却数据
	SET_COOL_DOWN,                      // CD被设置

// 30
	BROADCAST_PLAYER_PULLBACK,          // 广播玩家被拉到某个位置
	COMBAT_PLAYER_JOIN_FAIL,            // 玩家加入结果
	COMBAT_SELECT_SKILL_RE,             // 选择技能结果
	COMBAT_PVE_START,                   // PVE战斗开始
	COMBAT_PVP_START,                   // PVP战斗开始

// 35
	COMBAT_PVE_END,                     // PVE战斗结束
	COMBAT_PLAYER_BASE_PROP,            // 同步玩家基础属性(非广播)
	COMBAT_SKILL_RESULT,                // 技能攻击结果
	COMBAT_BUFF_RESULT,                 // 回合BUFF结果
	COMBAT_AWARD,                       // 战斗奖励

// 40
	COMBAT_MOB_DEAD,                    // 战场怪物死亡
	COMBAT_PLAYER_STATE,                // 战场玩家状态
	COMBAT_UNIT_VOLATILE_PROP,          // 同步战斗单位的易变属性(广播)
	COMBAT_PLAYER_EXTPROP,              // 同步玩家ExtProp(非广播)
	COMBAT_PVE_CONTINUE,                // 玩家回归PVE战场继续战斗

// 45
	COMBAT_PVP_CONTINUE,                // 玩家回归PVP战场继续战斗
	PLAYER_BASE_INFO,                   // 玩家基本信息
	PLAYER_EXTEND_PROP,                 // 玩家其它属性
	PLAYER_GAIN_EXP,                    // 玩家获得经验
	PLAYER_LEVEL_UP,                    // 玩家升级

// 50
	ELSE_PLAYER_ENTER_COMBAT,           // 玩家进入战斗
	ELSE_PLAYER_LEAVE_COMBAT,           // 玩家脱离战斗
	OBJECT_MOVE,                        // 除玩家外的对象移动
	OBJECT_STOP_MOVE,                   // 除玩家外的对象停止移动
	SKILL_DATA,                         // 玩家技能数据

// 55
	COMBAT_BUFF_DATA,                   // 战斗BUFF数据
	NPC_GREETING,                       // 和Npc Greeting打招呼
	QUERY_TEAM_MEMBER_RE,               // 客户端像master查询队友信息，gs给客户端回
	GAIN_PET,                           // 玩家获得宠物
	LOST_PET,                           // 玩家失去宠物

// 60
	PET_GAIN_EXP,                       // 宠物经验变化
	PET_LEVEL_UP,                       // 宠物升级
	SET_COMBAT_PET_RE,                  // 设置战斗宠物
	LEVELUP_BLOODLINE_RE,               // 升级宠物血脉结果
	LEVELUP_PET_POWER_CAP_RE,           // 升级宠物能量结果

// 65
	COMBAT_ROUND_END,                   // 战斗回合结束
	COMBAT_PLAYER_JOIN,                 // 其它玩家加入战斗
	GET_OWN_MONEY,                      // 玩家上线获得金钱数据
	GAIN_MONEY,                         // 玩家获得金钱
	SPEND_MONEY,                        // 玩家消耗金钱

// 70
	NOTIFY_SELF_POS,                    // 更新host player的位置，客户端收到后把玩家设置到这个位置
	COMBAT_PLAYER_GAIN_GOLEM,           // 战斗玩家获得魔偶
	COMBAT_PLAYER_SWITCH_GOLEM,         // 战斗玩家切换魔偶
	UPDATE_SKILL_TREE,                  // 更新技能树数据
	TASK_NOTIFY_CLIENT,                 // 任务系统给客户端发消息

// 75
	ENTER_RULE_AREA,                    // 进入规则区域
	LEAVE_RULE_AREA,                    // 离开规则区域
	TRANSFER_PREPARE,                   // 通知客户端准备传送
	PLAYER_DEAD,                        // 玩家死亡
	PLAYER_RESURRECT,                   // 玩家复活

// 80
	JOIN_TEAM_REQ,                      // 别的玩家发过来的组队申请
	BUDDY_JOIN_TEAM,                    // 伙伴进入队伍
	BUDDY_LEAVE_TEAM,                   // 伙伴离开队伍
	BUDDY_TEAM_INFO,                    // 伙伴队伍创建，队伍信息
	CHANGE_BUDDY_TEAM_POS_RE,           // 伙伴调整位置回复，成功时才发该协议

// 85
	GATHER_START,                       // 采集开始
	SELF_GATHER_START,                  // 发给hostplayer自己的采集开始
	GATHER_STOP,                        // 采集结束
	TEAMMATE_ENTER_COMBAT_FAIL,         // 组队玩家进入战斗失败
	UPDATE_VISIBLE_BUFF,                // 更新玩家身上带光效的BUFF列表

// 90
	PLAYER_GAIN_ITEM,                   // 玩家获得物品
	CAT_VISION_GAIN_EXP,                // 获得瞄类视觉经验
	CAT_VISION_LEVEL_UP,                // 瞄类视觉升级
	OPEN_CAT_VISION_RE,                 // 开启瞄类视觉回复
	CLOSE_CAT_VISION_RE,                // 关闭瞄类视觉回复

// 95
	CLOSE_CAT_VISION,                   // 关闭瞄类视觉
	LEARN_SKILL_RE,                     // 学习技能的结果
	SWITCH_SKILL_RE,                    // 切换技能的结果
	ELSE_PLAYER_EXTPROP,                // 其它玩家的16个属性
	COMBAT_UNIT_SPEAK,                  // 战斗对象喊话

// 100
	TASK_DATA,							// 玩家任务数据
	COMBAT_SUMMON_MOB,                  // 战斗中召唤怪物、NPC等
	COMBAT_MULTI_MOB_SPEAK,             // 战斗中所有怪物一起喊话
	TRANSFER_CLS,                       // 玩家转职(废弃)
	DRAMA_GATHER_START,                 // 剧情矿采集开始，只发自己，不广播

// 105
	DRAMA_GATHER_STOP,                  // 剧情矿采集结束，只发自己，不广播
	OBJECT_BW_LIST,                     // 通知客户端object的黑白名单
	OBJECT_BW_LIST_CHANGE,              // 通知object黑白名单变化
	ELSE_PLAYER_EQUIPCRC,               // 其它玩家的装备CRC
	ELSE_PLAYER_EQUIPMENT,              // 其它玩家的装备信息

// 110
	CHANGE_NAME_RE,                     // 改名字的回复，包括任务改名等
	BROADCAST_CHANGE_NAME,              // 广播给周围玩家，玩家改名
	GET_STATIC_ROLE_INFO_RE,            // 指定玩家的角色静态信息
	SELF_STATIC_ROLE_INFO,              // hostplayer自己的静态角色信息
	MAP_KICKOUT_COUNTDOWN,              // 副本等特殊地图踢人倒计时

// 115
    GET_INSTANCE_DATA_RE,               // 玩家进入副本
	CHAT_MSG,                           // 聊天消息
	MOVE_COMBAT_PET_RE,                 // 移动战宠结果
	COMBAT_UNIT_TURN_FRONT,             // 战斗对象从背对战场转为正对战场
	QUERY_NPC_ZONE_INFO_RE,             // 把npc区域信息发给客户端

// 120
	MALL_DATA,                          // 商城数据
	MALL_GOODS_DETAIL,                  // 商城内特定商品的详细信息
	MALL_SHOPPING_RE,                   // 商城购物结果
	GET_OWN_CASH,                       // 玩家有多少元宝
	NPC_SERVICE_CONTENT,                // NPC服务的内容

// 125
	SERVER_TIME,                        // 服务器的当前时间
	SHOP_SHOPPING_RE,                   // 商店购物结果
	QUERY_ITEM_INFO_RE,					// 查询物品信息
	ANNOUNCE_NEW_MAIL,					// 通知玩家有新邮件
	GET_MAIL_LIST_RE,					// 返回邮件列表

// 130
	GET_MAIL_ATTACH_RE,					// 获取邮件附件回复
	DELETE_MAIL_RE,						// 删除邮件回复
	TAKEOFF_MAIL_ATTACH_RE,				// 取走邮件附件回复
	SEND_MAIL_RE,						// 发送邮件回复
	TEAMMEMBER_LOCAL_INS_REPLY,         // 队友对队长邀请的回应，只会发给队长

// 135
    TEAMMEMBER_QUIT_LOCAL_INS,          // 队友退出本服组队挑战
	TEAMMEMBER_AGREE_LOCAL_INS,         // 队友同意参加副本，只会发给队员
	GAIN_SCORE,                         // 玩家获得学分
	SPEND_SCORE,                        // 玩家消耗学分
	TEAM_LOCAL_INS_INVITE,              // 本服组队副本邀请，发给队员

// 140
	MAP_TEAM_INFO,                      // 副本队伍信息
	MAP_TEAM_QUERY_MEMBER_RE,           // 查询副本队伍里的成员信息
	MAP_TEAM_JOIN,                      // 新玩家加入副本队伍
	MAP_TEAM_LEAVE,                     // 玩家退出副本队伍（主动退出，即退出副本）
	MAP_TEAM_STATUS_CHANGE,             // 副本队伍成员状态变化（在线、掉线）

// 145
	MAP_TEAM_CHANGE_LEADER_RE,          // 副本队伍，队长变化
	MAP_TEAM_CHANGE_POS_RE,             // 有玩家调换位置
	TRANSFER_GENDER,                    // 变性（废弃）
	COMBAT_LEARN_SKILL,                 // 战斗中得到一个技能
	PLAYER_ENTER_INS_MAP,               // 玩家进入副本地图，主动通知客户端

// 150
	INSTANCE_END,                       // 副本结束
	PLAYER_SCORE,						// 玩家有多少学分
	SELF_ITEM_DETAIL_LIST_ALL,          // 玩家的所有包裹数据，上线时发送给客户端
	PET_POWER,                          // 宠物能量更新
	PET_DATA,                           // 宠物数据--上线发送

// 155
	COMBAT_PET_ATTACK_RE,               // 战斗中让宠物攻击的结果
	TALENT_DATA,                        // 玩家天赋数据(上线发送)
	TALENT_DETAIL,                      // 单个天赋的详细数据
	LEVELUP_TALENT_RE,                  // 升级天赋的结果
	OPEN_TALENT_GROUP,                  // 开启天赋组

// 160
	COMBAT_PET_POWER,                   // 战斗中同步宠物能量
	FULL_PET_POWER_RE,                  // 一键加满宠物能量的结果
	LEVELUP_CARD_RE,                    // 卡牌升级
    AUCTION_ITEM_RE,                    // 拍卖物品回应，是否挂单成功
    AUCTION_CANCEL_RE,                  // 取消拍卖物品回应，是否取消成功

// 165
    AUCTION_BUYOUT_RE,                  // 一口价买物品的回应
    AUCTION_BID_RE,                     // 竞价买物品的回应
    AUCTION_REVOKE,                     // 对应的拍卖id已经撤销
    PLAYER_LEAVE_INS_MAP,               // 通知客户端已经离开副本，可以清除副本的界面及数据
    COMBAT_PLAYER_SELECT_SKILL,         // 玩家技能变更，每回合广播给战场中的其它玩家。

// 170
    FRIEND_DATA,                        // 好友数据
    ADD_FRIEND_RE,                      // 添加好友回复
    DELETE_FRIEND_RE,                   // 删除好友回复
    TEAM_LEADER_CONVENE,                // 队长召集队友，发给队友
    COMBAT_MOB_TRANSFORM,               // 战斗过程中怪物变身

// 175
    TEAM_CROSS_INS_INVITE,              // 跨服副本的队长邀请
    TEAM_CROSS_INS_INVITE_REPLY,        // 跨服副本队长邀请的回应，发给队长
    TEAMMEMBER_AGREE_CROSS_INS,         // 队友同意参加跨服副本，只会发给队员
    TEAM_CROSS_INS_REQUEST_COMPLETE,    // 跨服副本申请已经发送给报名服务器
	COMBAT_PVP_END,                     // PVP战斗结束

// 180
    TITLE_DATA,                         // 玩家称号数据
    GAIN_TITLE,                         // 玩家获得称号
    SWITCH_TITLE_RE,                    // 玩家切换称号的响应
    BROADCAST_PLAYER_TITLE,             // 玩家称号变更，广播给周边玩家
    DUEL_REQUEST,                       // 别的玩家发来决斗请求

// 185
    DUEL_REQUEST_RE,                    // 发起决斗请求的回应
    TEAMMATE_DUEL_REQUEST,              // 本队队员要求向别的玩家发起决斗
    TEAMMATE_DUEL_REQUEST_RE,           // 本队队长给队员的回应
    DUEL_PREPARE,                       // 决斗准备开始
    REPUTATION_DATA,                    // 玩家声望数据

// 190
    OPEN_REPUTATION,                    // 玩家开启新的声望
    MODIFY_REPUTATION,                  // 玩家声望变化
    UI_DATA,                            // 客户端UI配置数据
    PLAYER_PRIMARY_PROP_RF,             // 上线时同步初阶属性强化数据
    QUERY_PRIMARY_PROP_RF_RE,           // 打开界面查询信息的回复

// 195
    PRIMARY_PROP_REINFORCE_RE,          // 一键强化初阶属性的回复
    BUY_PRIMARY_PROP_RF_ENERGY_RE,      // 购买初阶属性强化能量的回应
    ACHIEVE_DATA,                       // 成就数据
    ACHIEVE_NOTIFY_CLIENT,              // 成就系统给客户端发消息
    LOGIN_DATA,                         // 登录时间数据

// 200
    ENHANCE_DATA,                       // 附魔数据
    OPEN_ENHANCE_SLOT_RE,               // 开启附魔位回复
    PROTECT_ENHANCE_SLOT_RE,            // 保护附魔位
    UNPROTECT_ENHANCE_SLOT_RE,          // 解除附魔位保护
    ENHANCE_RE,                         // 附魔结果

// 205
    STAR_DATA,                          // 星盘数据
    OPEN_STAR_RE,                       // 开启星盘
    ACTIVATE_SPARK_RE,                  // 激活星火
    USE_ITEM_ERROR,                     // 使用物品失败的错误码
    PLAYER_ENTER_BG_MAP,                // 玩家进入战场地图

// 210
    PLAYER_LEAVE_BG_MAP,                // 玩家离开战场地图
    GET_BATTLEGROUND_DATA_RE,           // 获取战场信息回应
    EQUIP_CARD_RE,                      // 镶嵌卡牌回复
    GAIN_CARD,                          // 获得卡牌（废弃）
    LOST_CARD,                          // 失去卡牌（废弃）

// 215
    MAP_TEAM_JOIN_TEAM_REQUEST,         // 别的玩家发过来的加入队伍请求或邀请
    INS_PROMPT_MESSAGE,                 // 副本地图的提示信息，用于副本里策划写的脚本
    BG_PROMPT_MESSAGE,                  // 战场地图的提示信息，用于战场里策划写的脚本
    GLOBAL_COUNTER_CHANGE,              // 全局计数器改变
    MAP_COUNTER_CHANGE,                 // 地图计数器改变

// 220
    COMBAT_BUFF_SYNC,                   // 玩家上线后同步战场中的Buff信息
    UPDATE_TASK_STORAGE_RE,             // 付费刷新库任务回复
    COMBAT_MOB_ESCAPE,                  // 战斗过程中怪物逃跑
    PLAYER_COUNTER_CHANGE,              // 玩家计数器变化
    PLAYER_COUNTER_LIST,                // 玩家计算器列表，getalldata时候发

// 225
    NOTIFY_OBJECT_POS,                  // 设置对象的位置
    NPC_FRIEND_STATUS_CHANGE,           // 好友状态改变
    SHOW_MAP_COUNTDOWN,                 // 显示地图倒计时(副本、战场里的脚本发)
    COMBAT_SCENE_SHAKE,                 // 战斗场景震动
    COMBAT_UNIT_CUR_GOLEM_PROP,         // 战斗当前魔偶属性变化

// 230
    COMBAT_UNIT_OTHER_GOLEM_PROP,       // 战斗休息魔偶属性变化
    PUNCH_CARD_DATA,                    // 玩家的签到信息
    PUNCH_CARD_RESULT,                  // 签到结果
    QUERY_INSTANCE_RECORD_RE,           // 查询副本记录的回复
    QUERY_PUNCH_CARD_DATA_RE,           // 查询补签数据的回复

// 235
    GAIN_PUNCH_CARD_AWARD_RE,           // 领取签到奖励的回复
    GEVENT_DATA,                        // 活动数据
    JOIN_GEVENT_RE,                     // 参加活动回复
    PARTICIPATION_DATA,                 // 活跃度数据
    PARTICIPATION_CHANGE,               // 活跃度变化

// 240
    OBJECT_HATE_YOU,                    // 有object已经watch该player，即将发动攻击等ai动作
    RE_PUNCH_CARD_HELP,                 // 有人请求帮忙补签
    MOUNT_DATA,                         // 坐骑数据
    MOUNT_MOUNT_RE,                     // 骑乘坐骑回复
    MOUNT_EXCHANGE_RE,                  // 骑乘坐骑回复

// 245
    MOUNT_EQUIP_LEVELUP_RE,             // 骑具升级回复
    GET_PARTICIPATION_AWARD_RE,         // 领取获取度奖励回复
    OPEN_MOUNT_EQUIP,                   // 开启骑具
    GAIN_MOUNT,                         // 获得坐骑
    LOST_MOUNT,                         // 失去坐骑

// 250
    COMBAT_SKILL_SYNC,                  // 战斗中技能状态的同步
    GEVENT_DATA_CHANGE,                 // 活动数据变化
    ITEM_DISASSEMBLE_RE,                // 物品分解的回复
    GAME_SERVER_VERSION,                // 服务器版本：资源版本等
    RE_PUNCH_CARD_HELP_RE,              // 帮别人补签的回复

// 255
    BOSS_CHALLENGE_DATA,                // 界面BOSS挑战数据
    GET_BOSS_CHALLENGE_AWARD_RE,        // 领取BOSS奖励回复
    GET_CLEAR_CHALLENGE_AWARD_RE,       // 领取通关奖励回复
    MAP_TEAM_TIDY_POS,                  // 地图组队站位调整
    BOSS_CHALLENGE_RE,                  // BOSS挑战成功

// 260
    QUERY_WORLD_BOSS_RECORD_RE,         // 查询世界BOSS的伤害排行
    WB_COMBAT_END_RECORD,               // 世界BOSS战斗结束时广播排行信息
    COMBAT_WAIT_SELECT_SKILL,           // 战斗中等待客户端选择技能
    COMBAT_WAIT_SELECT_PET,             // 战斗中等待客户端选择宠物
    GAIN_CASH,                          // 获得元宝

// 265
    SPEND_CASH,                         // 使用元宝
    ARENA_OD_BUY_TICKET_RE,             // 离线竞技场，购买挑战次数回复
    

// max: 7900
	MAX_CMD_NUM = G2C_CMD_UPPER_LIMIT
};


/**
 * @brief: proto_packet define
 */
///
///---- cmd: 0
///
class ErrorMessage : public ProtoPacket
{
	DECLARE_PROTOPACKET(ErrorMessage, ERROR_MESSAGE);
public:
	uint16_t error_num;

	PACKET_DEFINE(error_num);
};

class PlayerVolatileInfo : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerVolatileInfo, PLAYER_VOLATILE_INFO);
public:
	int32_t hp;
	int32_t mp;
	int32_t ep;
	PACKET_DEFINE(hp, mp, ep);
};

class PlayerVisibleInfoNotify : public ProtoPacket
{
	// 可以看见的player信息（包括hostplayer和elseplayer都会收到）
    DECLARE_PROTOPACKET(PlayerVisibleInfoNotify, PLAYER_VISIBLE_INFO_NOTIFY);
public:
    PlayerVisibleInfo info;
    PACKET_DEFINE(info);
};

class EnterWorld_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(EnterWorld_Re, ENTER_WORLD_RE);
public:
	MapID    map_id;
	PlayerVisibleInfo self_info;

	PACKET_DEFINE(map_id, self_info);
};

class PlayerStartMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerStartMove, PLAYER_START_MOVE);
public:
	RoleID          roleid;
	A2DVECTOR_PACK  dest;
	uint16_t        speed;  // 百分数，单位厘米每秒
	uint8_t         mode;

	PACKET_DEFINE(roleid, dest, speed, mode);
};


///
/// ---- cmd: 5
///
class PlayerMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerMove, PLAYER_MOVE);
public:
	RoleID          roleid;
	A2DVECTOR_PACK  cur_pos;
	A2DVECTOR_PACK  dest;
	uint16_t        speed;  // 百分数，单位厘米每秒
	uint8_t         mode;

	PACKET_DEFINE(roleid, cur_pos, dest, speed, mode);
};

class PlayerStopMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerStopMove, PLAYER_STOP_MOVE);
public:
	RoleID          roleid;
	A2DVECTOR_PACK  pos;
	uint16_t        speed; // 百分数，单位厘米每秒
	uint8_t         mode;
	uint8_t         dir;   // 朝向

	PACKET_DEFINE(roleid, pos, speed, mode, dir);
};

class PlayerVisibleInfoChange : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerVisibleInfoChange, PLAYER_VISIBLE_INFO_CHANGE);
public:
	enum VISIBLE_MASK
	{
		VM_ROLE_CLS        = 0x0001,  // 角色职业变更
		VM_ROLE_GENDER     = 0x0002,  // 角色性别变更
		VM_WEAPON          = 0x0004,  // 角色武器变更
		VM_MOUNT           = 0x0008,  // 角色坐骑变更
	};

	int64_t roleid;
	int64_t visible_mask;
	std::vector<int32_t> visible_list; // visible_list里面字段的含义根据visible_mask来解析。
	                                   // visible_list元素个数和visible_mask被设置的二进制位个数是相等的。

	PACKET_DEFINE(roleid, visible_mask, visible_list);
};

class PlayerEnterView : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerEnterView, PLAYER_ENTER_VIEW);
public:
	PlayerVisibleInfo player_info;

	PACKET_DEFINE(player_info);
};

class PlayerLeaveView : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerLeaveView, PLAYER_LEAVE_VIEW);
public:
	RoleID roleid;

	PACKET_DEFINE(roleid);
};


///
/// ---- cmd: 10
///
class ObjectVisibleInfoNotify : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectVisibleInfoNotify, OBJECT_VISIBLE_INFO_NOTIFY);
public:
    ObjectVisibleInfo info;
	PACKET_DEFINE(info);
};

class ObjectVisibleInfoList : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectVisibleInfoList, OBJECT_VISIBLE_INFO_LIST);
public:
	std::vector<ObjectVisibleInfo> obj_info_list;

	PACKET_DEFINE(obj_info_list);
};

class PullPlayerBackToValidPos : public ProtoPacket
{
	DECLARE_PROTOPACKET(PullPlayerBackToValidPos, PULL_PLAYER_BACK_TO_VALID_POS);
public:
	A2DVECTOR_PACK  pos;
	uint16_t        move_seq;

	PACKET_DEFINE(pos, move_seq);
};

class EnterEventArea : public ProtoPacket
{
	DECLARE_PROTOPACKET(EnterEventArea, ENTER_EVENT_AREA);
public:
	PACKET_DEFINE();
};

class NotifyPlayerMoveProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(NotifyPlayerMoveProp, NOTIFY_PLAYER_MOVE_PROP);
public:
	float run_speed; // 奔跑速度 单位 m/s

	PACKET_DEFINE(run_speed);
};


///
/// ---- cmd: 15
///
class SelfItemDetail : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelfItemDetail, SELF_ITEM_DETAIL);
public:
    enum UpdateType
    {
        UT_NORMAL = 0, // 普通更新
        UT_REFINE,     // 精炼更新
    };

	int8_t where;
    int8_t uptype; // 对应上面的枚举UpdateType
	ItemDetail detail;
	PACKET_DEFINE(where, uptype, detail);
};

class SelfItemInfoList : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelfItemInfoList, SELF_ITEM_INFO_LIST);
public:
	struct ItemInfo
	{
		int32_t type;
		int16_t index;
		int32_t count;
		int32_t proc_type;
		int32_t expire_date;
		NESTED_DEFINE(type, index, count, proc_type, expire_date);
	};

	int8_t where;
	int16_t inv_cap;
	std::vector<ItemInfo> list;
	PACKET_DEFINE(where, inv_cap, list);
};

class SelfItemDetailList : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelfItemDetailList, SELF_ITEM_DETAIL_LIST);
public:
	int8_t where;
	int16_t inv_cap;
	std::vector<ItemDetail> list;
	PACKET_DEFINE(where, inv_cap, list);
};

class DropInvItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(DropInvItem, DROP_INV_ITEM);
public:
	int32_t type;
	int16_t index;
	int32_t count;
	PACKET_DEFINE(type, index, count);
};

class MoveInvItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(MoveInvItem, MOVE_INV_ITEM);
public:
	int8_t src_idx;
	int8_t dest_idx;
	int32_t count;
	PACKET_DEFINE(src_idx, dest_idx, count);
};


///
/// ---- cmd: 20
///
class SellInvItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(SellInvItem, SELL_INV_ITEM);
public:
    int8_t where;
	int16_t index;
	int32_t count;
	PACKET_DEFINE(where, index, count);
};

class ExchangeInvItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(ExchangeInvItem, EXCHANGE_INV_ITEM);
public:
	int16_t index1;
	int16_t index2;
	PACKET_DEFINE(index1, index2);
};

class EquipItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(EquipItem, EQUIP_ITEM);
public:
	int16_t idx_inv;
	int16_t idx_equip;
	PACKET_DEFINE(idx_inv, idx_equip);
};

class UndoEquip : public ProtoPacket
{
	DECLARE_PROTOPACKET(UndoEquip, UNDO_EQUIP);
public:
	int16_t idx_inv;//取下物品放在包裹中的位置
	int16_t idx_equip;//物品在装备栏的位置
	PACKET_DEFINE(idx_inv, idx_equip);
};

class UseItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(UseItem, USE_ITEM);
public:
	int8_t  where;  // 哪个包裹里
	int16_t index;  // 包裹里的位置
	int32_t type;   // 物品模板id
	int32_t count;  // 数量
	PACKET_DEFINE(where, index, type, count);
};


///
/// ---- cmd: 25
///
class RefineEquip_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(RefineEquip_Re, REFINE_EQUIP_RE);
public:
	int16_t where;
	ItemDetail detail;
	PACKET_DEFINE(where, detail);
};

class ObjectEnterView : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectEnterView, OBJECT_ENTER_VIEW);
public:
	ObjectVisibleInfo obj_info;
	PACKET_DEFINE(obj_info);
};

class ObjectLeaveView : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectLeaveView, OBJECT_LEAVE_VIEW);
public:
	ObjectID  obj_id;
	PACKET_DEFINE(obj_id);
};

class CoolDownData : public ProtoPacket
{
	DECLARE_PROTOPACKET(CoolDownData, COOL_DOWN_DATA);
public:
	std::vector<CDEntry> cd_vec;
	PACKET_DEFINE(cd_vec);
};

class SetCoolDown : public ProtoPacket
{
	DECLARE_PROTOPACKET(SetCoolDown, SET_COOL_DOWN);
public:
    CDEntry cd_ent;
	PACKET_DEFINE(cd_ent);
};


///
/// ---- cmd: 30
///
class BroadcastPlayerPullBack : public ProtoPacket
{
	DECLARE_PROTOPACKET(BroadcastPlayerPullBack, BROADCAST_PLAYER_PULLBACK);
public:
	RoleID         roleid;
	A2DVECTOR_PACK pos;

	PACKET_DEFINE(roleid, pos);
};


///
/// 战斗系统协议
///
class CombatPlayerJoinFail: public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerJoinFail, COMBAT_PLAYER_JOIN_FAIL);
public:
	int32_t combat_id;
	PACKET_DEFINE(combat_id);
};

class CombatSelectSkill_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatSelectSkill_Re, COMBAT_SELECT_SKILL_RE);
public:
	enum RESULT
	{
		ERR_SUCCESS,           // 选择成功
		ERR_NO_SELECT_ACCESS,  // 玩家攻击中，禁止选择技能
		ERR_NO_SKILL_ITEM,     // 无技能道具
		ERR_NO_SKILL_POWER,    // 无技能消耗
		ERR_SKILL_IS_COOLING,  // 技能冷却中
        ERR_GOLEM_EXIST,       // 魔偶已经存在
	};

	int16_t result;
	int32_t cur_skill;
	PACKET_DEFINE(result, cur_skill);
};

class CombatPVEStart : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPVEStart, COMBAT_PVE_START);
public:
	enum
	{
		COMBAT_NO_SNEAK_ATTACKED,
		PLAYER_BE_SNEAK_ATTACKED,
		MOB_BE_SNEAK_ATTACKED,
	};

	int32_t combat_id;           //战斗ID
	int32_t combat_scene_id;     //战斗场景ID
	int32_t wave_common_hint_id; //连续战斗时的常态提示
	int8_t  part_sneak_attacked; //被偷袭方(0:无偷袭;1:玩家被偷袭;2怪物被偷袭)
	int8_t  wave_total;          //连续战斗时共有多少波
	int8_t  new_combat;          //1:开启新战斗;0:上线继续战斗;
	std::vector<CombatMobInfo> mob_list;
	std::vector<CombatTeamNpcInfo> team_npc_list;
	std::vector<CombatPlayerInfo> teammate_list;

	PACKET_DEFINE(combat_id, combat_scene_id, wave_common_hint_id, part_sneak_attacked, wave_total, new_combat, mob_list, team_npc_list, teammate_list);
};

class CombatPVPStart : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPVPStart, COMBAT_PVP_START);
public:
	int32_t combat_id;           //战斗ID
	int32_t combat_scene_id;     //战斗场景ID
    int8_t  which_party;         //加入战斗的玩家属于哪一方,1:攻方;2:守方.
	std::vector<CombatPlayerInfo> attacker_list;
	std::vector<CombatPlayerInfo> defender_list;
	PACKET_DEFINE(combat_id, combat_scene_id, which_party, attacker_list, defender_list);
};


///
/// ---- cmd: 35
///
class CombatPVEEnd : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPVEEnd, COMBAT_PVE_END);
public:
	int8_t  result;//战斗结果:1:赢;2:输
	int32_t exp_gain;
	int32_t money_gain;
	std::vector<ItemEntry> items_task; // 普通掉落和全局掉落物品
	std::vector<ItemEntry> items_drop; // 普通掉落和全局掉落物品
	std::vector<ItemEntry> items_lottery; // 抽奖物品(特殊掉落)
	PACKET_DEFINE(result, exp_gain, money_gain, items_task, items_drop, items_lottery);
};

class CombatPlayerBaseProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerBaseProp, COMBAT_PLAYER_BASE_PROP);
public:
	int32_t unit_id;
	int32_t hp;//已经废弃
	int32_t mp;
	int32_t ep;
	PACKET_DEFINE(unit_id, hp, mp, ep);
};

class CombatSkillResult : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatSkillResult, COMBAT_SKILL_RESULT);
public:
	enum
	{
		ATTACKER_TYPE_INVALID,
		ATTACKER_TYPE_PLAYER,
		ATTACKER_TYPE_MOB,
		ATTACKER_TYPE_PET,
		ATTACKER_TYPE_GOLEM,
		ATTACKER_TYPE_BOSS,
	};

	int32_t combat_id;
	int32_t attacker;       // 攻击者的战斗对象ID, 宠物，魔偶攻击时，这个值为主人的战斗对象ID
	int8_t  attacker_type;  // 攻击者类型
	int8_t  pet_combat_pos; // 宠物在战场栏的位置，用来标识哪一只宠物施放技能，此字段只有在宠物攻击时有效
	std::vector<skill_result> skill_result_list;
	PACKET_DEFINE(combat_id, attacker, attacker_type, pet_combat_pos, skill_result_list);
};

class CombatBuffResult : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatBuffResult, COMBAT_BUFF_RESULT);
public:
	int32_t combat_id;
	std::vector<buff_entry> buff_list;
	PACKET_DEFINE(combat_id, buff_list);
};

class CombatAward : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatAward, COMBAT_AWARD);
public:
	std::vector<int32_t> items;     //物品奖励
	PACKET_DEFINE(items);
};


///
/// ---- cmd: 40
///
class CombatMobDead : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatMobDead, COMBAT_MOB_DEAD);
public:
	int32_t mob_unit_id;    //怪物unit-id
	int32_t killer_unit_id; //击杀者unit-id
	std::vector<ItemEntry> items_drop;
	PACKET_DEFINE(mob_unit_id, killer_unit_id, items_drop);
};

class CombatPlayerState : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerState, COMBAT_PLAYER_STATE);
public:
	int32_t unit_id; //玩家战场ID
	int8_t state;    //玩家状态
	PACKET_DEFINE(unit_id, state);
};

class CombatUnitVolatileProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatUnitVolatileProp, COMBAT_UNIT_VOLATILE_PROP);
public:
	int32_t unit_id;
	int32_t hp; // 已经废弃
	int32_t max_hp;
	PACKET_DEFINE(unit_id, hp, max_hp);
};

class CombatPlayerExtProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerExtProp, COMBAT_PLAYER_EXTPROP);
public:
	uint64_t prop_mask;
	std::vector<int32_t> props;//vector元素顺序和prop_mask中二进制位值一致
	PACKET_DEFINE(prop_mask, props);
};

class CombatPVEContinue : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPVEContinue, COMBAT_PVE_CONTINUE);
public:
	int8_t  wave_count;           // 当前战斗是第几波
	int32_t wave_common_hint_id;  // 连续战斗常态提示
	int32_t wave_start_hint_id;   // 连续战斗出场提示
	std::vector<CombatMobInfo> mob_list;
	PACKET_DEFINE(wave_count, wave_common_hint_id, wave_start_hint_id, mob_list);
};


///
/// ---- cmd: 45
///
class CombatPVPContinue : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPVPContinue, COMBAT_PVP_CONTINUE);
public:
	PACKET_DEFINE();
};

class PlayerLevelUp : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerLevelUp, PLAYER_LEVEL_UP);
public:
	RoleID  roleid;    //升级者roleid
	int16_t new_level; //升级后等级
	PACKET_DEFINE(roleid, new_level);
};

class PlayerBaseInfo : public ProtoPacket
{
	// 本协议只在上线时发送
	DECLARE_PROTOPACKET(PlayerBaseInfo, PLAYER_BASE_INFO);
public:
	int32_t hp;
	int32_t max_hp;
	int32_t mp;
	int32_t max_mp;
	int32_t ep;
	int32_t max_ep;
	int32_t exp;       //玩家经验
	int16_t level;     //玩家等级
	int32_t cat_exp;   //瞄类视觉经验
	int16_t cat_level; //瞄类视觉等级
	PACKET_DEFINE(hp, max_hp, mp, max_mp, ep, max_ep, exp, level, cat_exp, cat_level);
};

class PlayerExtendProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerExtendProp, PLAYER_EXTEND_PROP);
public:
	uint32_t prop_mask;//mask从低位到高位的编号分别对应vector的第1，2，...个元素
	std::vector<int32_t> props;//vector元素顺序和prop_mask中二进制位一致
	PACKET_DEFINE(prop_mask, props);
};

class PlayerGainExp : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerGainExp, PLAYER_GAIN_EXP);
public:
	int32_t exp_gain; //获得经验值
	int32_t exp_cur;  //当前经验值
	PACKET_DEFINE(exp_gain, exp_cur);
};


///
/// ---- cmd: 50
///
class ElsePlayerEnterCombat : public ProtoPacket
{
	DECLARE_PROTOPACKET(ElsePlayerEnterCombat, ELSE_PLAYER_ENTER_COMBAT);
public:
	int64_t roleid;//进入战场玩家的ROLEID
	int32_t combat_id;//战场ID
	PACKET_DEFINE(roleid, combat_id);
};

class ElsePlayerLeaveCombat : public ProtoPacket
{
	DECLARE_PROTOPACKET(ElsePlayerLeaveCombat, ELSE_PLAYER_LEAVE_COMBAT);
public:
	int64_t roleid;//离开战场玩家的ROLEID
	PACKET_DEFINE(roleid);
};

class ObjectMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectMove, OBJECT_MOVE);
public:
	ObjectID        obj_id;
	A2DVECTOR_PACK  dest;
	uint16_t        use_time;
	uint16_t        speed; // 百分数，单位厘米每秒
	uint8_t         move_mode;
	PACKET_DEFINE(obj_id, dest, use_time, speed, move_mode);
};

class ObjectStopMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectStopMove, OBJECT_STOP_MOVE);
public:
	ObjectID        obj_id;
	A2DVECTOR_PACK  pos;
	uint16_t        speed; // 百分数，单位厘米每秒
	uint8_t         dir;
	uint8_t         move_mode;
	PACKET_DEFINE(obj_id, pos, speed, dir, move_mode);
};

class SkillData : public ProtoPacket
{
	DECLARE_PROTOPACKET(SkillData, SKILL_DATA);
public:
	struct SkillTree
	{
		int32_t skill_tree_id;
		int8_t  skill_idx; // 技能位置
		int8_t  cur_level; // 技能基础等级
		int8_t  tmp_level; // 技能临时等级
		int8_t  is_active; // 是否被选中
		NESTED_DEFINE(skill_tree_id, skill_idx, cur_level, tmp_level, is_active);
	};
	std::vector<SkillTree> base_skill_tree_list;
	std::vector<SkillTree> extend_skill_tree_list;

	PACKET_DEFINE(base_skill_tree_list, extend_skill_tree_list);
};


///
/// ---- cmd: 55
///
class CombatBuffData : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatBuffData, COMBAT_BUFF_DATA);
public:
	std::vector<CombatBuffInfo> buff_list_del; //删除buff列表
	std::vector<CombatBuffInfo> buff_list_add; //新增buff列表
	PACKET_DEFINE(buff_list_del, buff_list_add);
};

class NpcGreeting : public ProtoPacket
{
	DECLARE_PROTOPACKET(NpcGreeting, NPC_GREETING);
public:
	ObjectID npc_id;
	PACKET_DEFINE(npc_id);
};

class QueryTeamMemberRe : public ProtoPacket
{
	DECLARE_PROTOPACKET(QueryTeamMemberRe, QUERY_TEAM_MEMBER_RE);
public:
	RoleID  member_roleid;
	int16_t level;
	int32_t weapon_id;
	int32_t combat_value;
	bool    is_in_combat;
	int8_t  cls; //职业
	PACKET_DEFINE(member_roleid, level, weapon_id, combat_value, is_in_combat, cls);
};

class GainPet : public ProtoPacket
{
	DECLARE_PROTOPACKET(GainPet, GAIN_PET);
public:
	PetEntry pet;
	PACKET_DEFINE(pet);
};

class LostPet : public ProtoPacket
{
	DECLARE_PROTOPACKET(LostPet, LOST_PET);
public:
	int16_t pet_item_inv_idx;
	PACKET_DEFINE(pet_item_inv_idx);
};


///
/// ---- cmd: 60
///
class PetGainExp : public ProtoPacket
{
	DECLARE_PROTOPACKET(PetGainExp, PET_GAIN_EXP);
public:
	int16_t pet_item_idx;
	int32_t pet_exp; // 宠物的当前经验值
	PACKET_DEFINE(pet_item_idx, pet_exp);
};

class PetLevelUp : public ProtoPacket
{
	DECLARE_PROTOPACKET(PetLevelUp, PET_LEVEL_UP);
public:
	int16_t pet_item_idx;
	int16_t pet_level;
	int32_t pet_exp;
	PACKET_DEFINE(pet_item_idx, pet_level, pet_exp);
};

class SetCombatPet_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(SetCombatPet_Re, SET_COMBAT_PET_RE);
public:
	enum
	{
		OP_PUSH,
		OP_POP,
	};

	int8_t  result;             // 操作结果：1:成功;0:失败
	int8_t  op_type;            // 操作类型：0:添加;1:删除
	int8_t  pet_combat_inv_pos; // 宠物在战斗栏的位置
	int16_t pet_item_inv_idx;   // 宠物在包裹栏的位置

	PACKET_DEFINE(result, op_type, pet_combat_inv_pos, pet_item_inv_idx);
};

class LevelUpBloodline_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(LevelUpBloodline_Re, LEVELUP_BLOODLINE_RE);
public:
	int8_t  result;           // 升级结果：1:成功;0:失败
	int16_t pet_item_inv_idx; // 宠物对应的物品在宠物包裹中的位置
	int16_t blevel;           // 升级后血脉等级

	PACKET_DEFINE(result, pet_item_inv_idx, blevel);
};

class LevelUpPetPowerCap_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(LevelUpPetPowerCap_Re, LEVELUP_PET_POWER_CAP_RE);
public:
	int8_t  result;            // 1:成功;0:失败
	std::vector<int32_t> list; // 多次升级提升的上限值列表
	int32_t pet_power_cap;     // 升级后宠物能量上限

	PACKET_DEFINE(result, list, pet_power_cap);
};


///
/// ---- cmd: 65
///
class CombatRoundEnd : public ProtoPacket
{
	//回合结束通知客户端
	//目前只有在能量耗尽不满足选中技能的施放条件时才发送
	DECLARE_PROTOPACKET(CombatRoundEnd, COMBAT_ROUND_END);
public:
	int32_t cur_skill;//玩家当前选中的技能ID
	PACKET_DEFINE(cur_skill);
};

class CombatPlayerJoin : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerJoin, COMBAT_PLAYER_JOIN);
public:
	int32_t combat_id;            //战场ID
	CombatPlayerInfo player_info; //新加入战斗的玩家信息
	PACKET_DEFINE(combat_id, player_info);
};

class GetOwnMoney : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetOwnMoney, GET_OWN_MONEY);
public:
	int64_t money;//上线包裹金钱数
	PACKET_DEFINE(money);
};

class GainMoney : public ProtoPacket
{
	DECLARE_PROTOPACKET(GainMoney, GAIN_MONEY);
public:
	int32_t amount;//获得多少金钱
	PACKET_DEFINE(amount);
};

class SpendMoney : public ProtoPacket
{
	DECLARE_PROTOPACKET(SpendMoney, SPEND_MONEY);
public:
	int32_t cost;//消耗多少金钱
	PACKET_DEFINE(cost);
};


///
/// ---- cmd: 70
///
class NotifySelfPos : public ProtoPacket
{
	DECLARE_PROTOPACKET(NotifySelfPos, NOTIFY_SELF_POS);
public:
	int32_t        map_id;
	A2DVECTOR_PACK pos;
	PACKET_DEFINE(map_id, pos);
};

class CombatPlayerGainGolem : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerGainGolem, COMBAT_PLAYER_GAIN_GOLEM);
public:
	CombatGolemInfo golem_info;
	PACKET_DEFINE(golem_info);
};

class CombatPlayerSwitchGolem : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerSwitchGolem, COMBAT_PLAYER_SWITCH_GOLEM);
public:
	CombatGolemInfo old_golem_info;
	CombatGolemInfo new_golem_info;
	PACKET_DEFINE(old_golem_info, new_golem_info);
};

class UpdateSkillTree : public ProtoPacket
{
	DECLARE_PROTOPACKET(UpdateSkillTree, UPDATE_SKILL_TREE);
public:
	int32_t skill_tree_id;
	int8_t  skill_idx; //技能位置
	int8_t  cur_level; //基础等级
	int8_t  tmp_level; //临时等级
	int8_t  is_active; //是否选中
	PACKET_DEFINE(skill_tree_id, skill_idx, cur_level, tmp_level, is_active);
};

class TaskNotifyClient : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskNotifyClient, TASK_NOTIFY_CLIENT);
public:
	uint16_t type;
	std::string databuf;
	PACKET_DEFINE(type, databuf);
};


///
/// ---- cmd: 75
///
class EnterRuleArea : public ProtoPacket
{
	DECLARE_PROTOPACKET(EnterRuleArea, ENTER_RULE_AREA);
public:
	int32_t elem_id;
	PACKET_DEFINE(elem_id);
};

class LeaveRuleArea : public ProtoPacket
{
	DECLARE_PROTOPACKET(LeaveRuleArea, LEAVE_RULE_AREA);
public:
	int32_t elem_id;
	PACKET_DEFINE(elem_id);
};

class TransferPrepare : public ProtoPacket
{
	DECLARE_PROTOPACKET(TransferPrepare, TRANSFER_PREPARE);
public:
    int32_t elem_id; // 大于0表示是传送点传送，小于等于0表示是其他方式传送
	PACKET_DEFINE(elem_id);
};

class PlayerDead : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerDead, PLAYER_DEAD);
public:
	RoleID roleid;
	PACKET_DEFINE(roleid);
};

class PlayerResurrect : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerResurrect, PLAYER_RESURRECT);
public:
	RoleID roleid;
	int32_t hp;
	PACKET_DEFINE(roleid, hp);
};


///
/// ---- cmd: 80
///
class JoinTeamReq : public ProtoPacket
{
	DECLARE_PROTOPACKET(JoinTeamReq, JOIN_TEAM_REQ);
public:
	int64_t requester;
	std::string first_name;
	std::string mid_name;
	std::string last_name;
	bool invite;

	PACKET_DEFINE(requester, first_name, mid_name, last_name, invite);
};

// 已经有队伍才会发这个协议
class BuddyJoinTeam : public ProtoPacket
{
	DECLARE_PROTOPACKET(BuddyJoinTeam, BUDDY_JOIN_TEAM);
public:
	int32_t buddy_id;       // 伙伴id，从1开始，0是无效
	int32_t pos_index;      // 从0开始
	int32_t buddy_templ_id; // template id
	PACKET_DEFINE(buddy_id, pos_index, buddy_templ_id);
};

class BuddyLeaveTeam : public ProtoPacket
{
	DECLARE_PROTOPACKET(BuddyLeaveTeam, BUDDY_LEAVE_TEAM);
public:
	int32_t buddy_id; // 0代表该位置为空，负数代表是玩家自己（自己离队则解散队伍），正数代表是伙伴
	PACKET_DEFINE(buddy_id);
};

class BuddyTeamInfo : public ProtoPacket
{
	DECLARE_PROTOPACKET(BuddyTeamInfo, BUDDY_TEAM_INFO);
public:
	BuddyMemberInfo pos1; 
	BuddyMemberInfo pos2;
	BuddyMemberInfo pos3;
	BuddyMemberInfo pos4;
	PACKET_DEFINE(pos1, pos2, pos3, pos4);
};

class ChangeBuddyTeamPos_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(ChangeBuddyTeamPos_Re, CHANGE_BUDDY_TEAM_POS_RE);
public:
    int8_t src_index;
	int8_t des_index;
	PACKET_DEFINE(src_index, des_index);
};


///
/// ---- cmd: 85
///
class GatherStart : public ProtoPacket
{
	DECLARE_PROTOPACKET(GatherStart, GATHER_START);
public:
	RoleID   role_id;	// 广播协议，发给周围玩家，不包含自己
	ObjectID matter_id; // 被采集对象 
	int32_t  use_time;	// 单位：ms 毫秒
	PACKET_DEFINE(role_id, matter_id, use_time);
};

class SelfGatherStart : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelfGatherStart, SELF_GATHER_START);
public:
	ObjectID matter_id;     // 被采集对象
	int32_t  use_time;	    // 单位：ms 毫秒
	int32_t  gather_seq_no; // 采集序号，C2G::GatherErasingResult里需要回传给服务器
	PACKET_DEFINE(matter_id, use_time, gather_seq_no);
};

class GatherStop : public ProtoPacket
{
	DECLARE_PROTOPACKET(GatherStop, GATHER_STOP);
public:
	enum
	{
		NORMAL_STOP       = 0,
		MINE_HAS_BEEN_ROB = 1, 
		GATHER_INTERRUPT  = 2,
		GATHER_FAILURE    = 3,
		NONE_GATHER_TIME  = 4,
	};

	RoleID   role_id; // 广播协议，包含自己
	ObjectID obj_id;  // 矿对象id
	int8_t   reason;  // 对应上面的enum，0表示正常结束，1表示矿已被别人抢走，
	                  // 2被打断（自身移动，或者被别人打断）,3采矿失败，4读条时间是零的矿
	PACKET_DEFINE(role_id, obj_id, reason);
};

class TeammateEnterCombatFail : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeammateEnterCombatFail, TEAMMATE_ENTER_COMBAT_FAIL);
public:
	RoleID teammate_roleid; //队友加入战斗失败
	PACKET_DEFINE(teammate_roleid);
};

class UpdateVisibleBuff : public ProtoPacket
{
	DECLARE_PROTOPACKET(UpdateVisibleBuff, UPDATE_VISIBLE_BUFF);
public:
	RoleID roleid; //广播协议，包含自己
	std::vector<PlayerVisibleState::BuffInfo> visible_buff_vec; // buff-id vec
	PACKET_DEFINE(roleid, visible_buff_vec);
};


///
/// ---- cmd: 90
///
class PlayerGainItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerGainItem, PLAYER_GAIN_ITEM);
public:
	int8_t  where;                // 放在哪个包裹
	int32_t item_type;            // 获得物品ID
	int16_t item_count;           // 获得物品个数
	int32_t expire_date;          // 物品过期时间
	int16_t last_slot_idx;        // 物品放在最后槽位的INDEX
	int16_t last_slot_amount;     // 放在最后槽位的物品个数
    int8_t  gain_mode;            // 对应GainItemMode枚举，代表物品是通过什么方式获得
	std::vector<int8_t> content;  // 物品动态属性
	uint16_t content_crc;         // 物品动态数据验证码
	PACKET_DEFINE(where, item_type, item_count, expire_date, last_slot_idx, last_slot_amount, gain_mode, content, content_crc);
};

class CatVisionGainExp : public ProtoPacket
{
	DECLARE_PROTOPACKET(CatVisionGainExp, CAT_VISION_GAIN_EXP);
public:
	int32_t exp_gain; //获得经验值
	int32_t exp_cur;  //当前经验值
	PACKET_DEFINE(exp_gain, exp_cur);
};

class CatVisionLevelUp : public ProtoPacket
{
	DECLARE_PROTOPACKET(CatVisionLevelUp, CAT_VISION_LEVEL_UP);
public:
	int8_t new_level;
	PACKET_DEFINE(new_level);
};

class OpenCatVision_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(OpenCatVision_Re, OPEN_CAT_VISION_RE);
public:
	int8_t result; //开启结果(1:成功;0:失败)
	PACKET_DEFINE(result);
};

class CloseCatVision_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(CloseCatVision_Re, CLOSE_CAT_VISION_RE);
public:
	int8_t result; //关闭结果(1:成功;0:失败)
	PACKET_DEFINE(result);
};


///
/// ---- cmd: 95
///
class CloseCatVision : public ProtoPacket
{
	DECLARE_PROTOPACKET(CloseCatVision, CLOSE_CAT_VISION);
public:
	PACKET_DEFINE();
};

class LearnSkill_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(LearnSkill_Re, LEARN_SKILL_RE);
public:
	int8_t skill_idx; //技能位置
	PACKET_DEFINE(skill_idx);
};

class SwitchSkill_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(SwitchSkill_Re, SWITCH_SKILL_RE);
public:
	int8_t skill_idx;
	PACKET_DEFINE(skill_idx);
};

class ElsePlayerExtProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(ElsePlayerExtProp, ELSE_PLAYER_EXTPROP);
public:
	int64_t else_player_roleid;
	std::vector<int32_t> props;
	PACKET_DEFINE(else_player_roleid, props);
};

class CombatUnitSpeak : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatUnitSpeak, COMBAT_UNIT_SPEAK);
public:
	int32_t unit_id;
	int16_t talk_id;
	int16_t talk_time;
	PACKET_DEFINE(unit_id, talk_id, talk_time);
};

class CombatSummonMob : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatSummonMob, COMBAT_SUMMON_MOB);
public:
    int8_t  mob_type; //怪物类型：具体定义见上面的CombatMobType枚举
    int8_t  mob_pos;  //怪物战斗位置
    int32_t mob_tid;  //怪物模板ID
    int32_t mob_hp;   //怪物血量
    int32_t unit_id;  //怪物在战斗中的ID
    PACKET_DEFINE(mob_type, mob_pos, mob_tid, mob_hp, unit_id);
};


///
/// ---- cmd: 100
///
class TaskData : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskData, TASK_DATA);
public:
	std::string active_task;
	std::string finish_task;
	std::string finish_time_task;
	std::string task_diary;
	std::string task_storage;

	PACKET_DEFINE(active_task, finish_task, finish_time_task, task_diary, task_storage);
};

class CombatMultiMobSpeak : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatMultiMobSpeak, COMBAT_MULTI_MOB_SPEAK);
public:
	int32_t mob_tid;
	int16_t talk_id;
	int16_t talk_time;
	PACKET_DEFINE(mob_tid, talk_id, talk_time);
};

class TransferCls : public ProtoPacket
{
	DECLARE_PROTOPACKET(TransferCls, TRANSFER_CLS);
public:
    // 该协议已作废
	int64_t roleid;
	int8_t dest_cls;
	PACKET_DEFINE(roleid, dest_cls);
};

class DramaGatherStart : public ProtoPacket
{
	DECLARE_PROTOPACKET(DramaGatherStart, DRAMA_GATHER_START);
public:
	int32_t  matter_tid;    // 矿的模板id
	int32_t  use_time;	    // 单位：ms 毫秒
	int32_t  gather_seq_no; // 采集序号，C2G::GatherErasingResult里需要回传给服务器
	PACKET_DEFINE(matter_tid, use_time, gather_seq_no);
};


///
/// ---- cmd: 105
///
class DramaGatherStop : public ProtoPacket
{
	DECLARE_PROTOPACKET(DramaGatherStop, DRAMA_GATHER_STOP);
public:
	int32_t  matter_tid; // 矿的模板id
	int8_t   reason;     // 对应GatherStop协议里的enum，0表示正常结束，1表示矿已被别人抢走，
	                     // 2被打断（自身移动，或者被别人打断）,3采矿失败，4读条时间是零的矿
	PACKET_DEFINE(matter_tid, reason);
};

class ObjectBWList : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectBWList, OBJECT_BW_LIST);
public:
	std::set<int32_t> black_list; // 模板id
	std::set<int32_t> white_list; // 模板id
	PACKET_DEFINE(black_list, white_list);
};

class ObjectBWListChange : public ProtoPacket
{
	DECLARE_PROTOPACKET(ObjectBWListChange, OBJECT_BW_LIST_CHANGE);
public:
	int32_t templ_id;  // 模板id
	int8_t  is_black;  // 0表示是白名单，非0表示黑名单
	int8_t  is_add;    // 0表示是删除，非0表示添加
	PACKET_DEFINE(templ_id, is_black, is_add);
};

class ElsePlayerEquipCRC : public ProtoPacket
{
	DECLARE_PROTOPACKET(ElsePlayerEquipCRC, ELSE_PLAYER_EQUIPCRC);
public:
	int64_t else_player_roleid;
	uint32_t equip_crc;
	PACKET_DEFINE(else_player_roleid, equip_crc);
};

class ElsePlayerEquipment : public ProtoPacket
{
	DECLARE_PROTOPACKET(ElsePlayerEquipment, ELSE_PLAYER_EQUIPMENT);
public:
	struct EquipInfo
	{
		int32_t type;
		int16_t index;
		int32_t proc_type;
		int32_t expire_date;
		std::vector<int8_t> content;
		NESTED_DEFINE(type, index, proc_type, expire_date, content);
	};

	int64_t else_player_roleid;
	std::vector<EquipInfo> equipment;
	PACKET_DEFINE(else_player_roleid, equipment);
};


///
/// ----cmd: 110
///
class ChangeName_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(ChangeName_Re, CHANGE_NAME_RE);
public:
	enum ErrorCode
	{
		NO_ERR = 0,
		TIMEOUT,
		NAME_EXIST,
		SVR_CHECK_FAIL,
		RPC_RETURN_ERR,
	};

	int8_t err_code;  // 对应上面的枚举
	int8_t name_type; // 定义与task里一样
	std::string name;
	PACKET_DEFINE(err_code, name_type, name);
};

class BroadcastChangeName : public ProtoPacket
{
	DECLARE_PROTOPACKET(BroadcastChangeName, BROADCAST_CHANGE_NAME);
public:
	int64_t role_id;
	int8_t  name_type; // 定义与task里一样
	std::string name;
	PACKET_DEFINE(role_id, name_type, name);
};

class GetStaticRoleInfo_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetStaticRoleInfo_Re, GET_STATIC_ROLE_INFO_RE);
public:	
	int64_t roleid;
	int32_t create_time;
	std::string first_name;
	std::string middle_name;
	std::string last_name;
	PACKET_DEFINE(roleid, create_time, first_name, middle_name, last_name);
};

class SelfStaticRoleInfo : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelfStaticRoleInfo, SELF_STATIC_ROLE_INFO);
public:
	int32_t create_time;
	std::string first_name;
	std::string middle_name;
	std::string last_name;
	PACKET_DEFINE(create_time, first_name, middle_name, last_name);
};

class MapKickoutCountdown : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapKickoutCountdown, MAP_KICKOUT_COUNTDOWN);
public:
	int32_t countdown_secs;
	PACKET_DEFINE(countdown_secs);
};


///
/// ---- cmd: 115
///
class GetInstanceData_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetInstanceData_Re, GET_INSTANCE_DATA_RE);
public:
	int32_t ins_templ_id;
    int8_t  is_cross_realm; // 是不是跨服副本，0表示不是跨服，1表示是跨服
	PACKET_DEFINE(ins_templ_id, is_cross_realm);
};

class ChatMsg : public ProtoPacket
{
	DECLARE_PROTOPACKET(ChatMsg, CHAT_MSG);
public:
	int8_t channel;
	int64_t sender;
	std::string sender_name;
	std::string msg;
	PACKET_DEFINE(channel, sender, sender_name, msg);
};

class MoveCombatPet_Re: public ProtoPacket
{
	DECLARE_PROTOPACKET(MoveCombatPet_Re, MOVE_COMBAT_PET_RE);
public:
	int8_t result;              // 1:成功;0:失败;
	int8_t src_combat_inv_pos;  // 源位置
	int8_t dest_combat_inv_pos; // 目标移动位置
	PACKET_DEFINE(result, src_combat_inv_pos, dest_combat_inv_pos);
};

class CombatUnitTurnFront : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatUnitTurnFront, COMBAT_UNIT_TURN_FRONT);
public:
	int32_t unit_id;
	PACKET_DEFINE(unit_id);
};

class QueryNpcZoneInfo_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(QueryNpcZoneInfo_Re, QUERY_NPC_ZONE_INFO_RE);
public:
	class Detail : public shared::copyable
	{
	public:
		ObjectID  obj_id; // 对象在服务器的真实id
        int32_t   tid;    // 模板id，templ_id
		NESTED_DEFINE(obj_id, tid);
	};

	int32_t elem_id;
	std::vector<Detail> npc_list;
	PACKET_DEFINE(elem_id, npc_list);
};

///
/// 120
///
class MallData : public ProtoPacket
{
	DECLARE_PROTOPACKET(MallData, MALL_DATA);
public:
	struct OrderEntry
	{
		int32_t goods_id;        //商品ID
		int32_t goods_limit_buy; //玩家还可以买几个
		NESTED_DEFINE(goods_id, goods_limit_buy);
	};

	struct LimitSaleGoodsEntry
	{
		int32_t goods_id;     //商品ID
		int32_t goods_remains;//商品剩余个数
		NESTED_DEFINE(goods_id, goods_remains);
	};

	int32_t cash_amount; //有多少元宝
	int16_t max_goods_id;//验证使用
	std::vector<OrderEntry> order_list;
	std::vector<LimitSaleGoodsEntry> limit_sale_goods_list;
	std::vector<int32_t/*goods-id*/> spec_class_goods_list;
	PACKET_DEFINE(cash_amount, max_goods_id, order_list, limit_sale_goods_list, spec_class_goods_list);
};

class MallGoodsDetail : public ProtoPacket
{
	DECLARE_PROTOPACKET(MallGoodsDetail, MALL_GOODS_DETAIL);
public:
	int32_t goods_id;        //商品ID(同上)
	int32_t goods_remains;   //商品剩余可售个数
	int32_t goods_limit_buy; //玩家还可以买几个
	PACKET_DEFINE(goods_id, goods_remains, goods_limit_buy);
};

class MallShopping_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(MallShopping_Re, MALL_SHOPPING_RE);
public:
	enum MALL_SHOPPING_ERR
	{
		MSR_SUCCESS,                // 购买成功
		MSR_NO_ENOUGH_CASH,         // 玩家身上的元宝不足
		MSR_NO_ENOUGH_GOODS,        // 商城的商品数量不足
		MSR_NOT_IN_SELL_TIME,       // 商品出售时间未到
		MSR_EXCEED_ITEM_PILE_LIMIT, // 购买数量超过物品堆叠上限
		MSR_EXCEED_MAX_BUY_COUNT,   // 购买数量超过商品限购上限
	};

	int8_t  result;           // 购买结果：0成功;>0失败
	int32_t goods_id;         // 购买商品的ID
	int32_t goods_count;      // 购买几个商品
	int32_t goods_remains;    // 商品剩余个数
	int32_t goods_limit_buy;  // 玩家还可以买几个
	PACKET_DEFINE(result, goods_id, goods_count, goods_remains, goods_limit_buy);
};

class GetOwnCash : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetOwnCash, GET_OWN_CASH);
public:
	int32_t cur_cash;
	PACKET_DEFINE(cur_cash);
};

class NpcServiceContent : public ProtoPacket
{
	DECLARE_PROTOPACKET(NpcServiceContent, NPC_SERVICE_CONTENT);
public:
	/**
	 * 部分服务的内容是动态的，所以玩家需要向服务器请求服务内容，
	 * 不同服务的内容是不同的，具体格式见下。
	 */
	struct shop_srv_content
	{
		struct goods_entry
		{
	 		int32_t goods_id;
	 		int32_t goods_remains;
			NESTED_DEFINE(goods_id, goods_remains);
		};

		int16_t max_goods_id; // 商品的最大ID(用来校验)
		int16_t dyn_goods_count; // 动态商品个数
		std::vector<goods_entry> dyn_goods_list; //动态商品内容
		NESTED_DEFINE(max_goods_id, dyn_goods_count, dyn_goods_list);
	};

    struct enhance_srv_content
    {
        int32_t count;
        int32_t reset_time;
        NESTED_DEFINE(count, reset_time);
    };

	int32_t npc_id;
	int32_t service_type;
	std::vector<int8_t> content;
	PACKET_DEFINE(npc_id, service_type, content);
};

///
/// 125
///
class ServerTime : public ProtoPacket
{
	DECLARE_PROTOPACKET(ServerTime, SERVER_TIME);
public:
	int32_t timestamp; //服务器的当前时间
	PACKET_DEFINE(timestamp);
};

class ShopShopping_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(ShopShopping_Re, SHOP_SHOPPING_RE);
public:
	enum SHOP_SHOPPING_ERR
	{
		ERR_SUCCESS,
		ERR_NO_OPEN,         // 商店暂不开放
		ERR_NO_ENOUGH_GOODS, // 商店商品不足
		ERR_GOODS_SELL_OUT,  // 商店商品已经卖完
	};

	int8_t  result;          // 购买结果：0成功;>0失败
	int32_t goods_id;        // 购买商品的ID
	int32_t goods_count;     // 购买了几个商品
	int32_t goods_remains;   // 商品剩余个数
	PACKET_DEFINE(result, goods_id, goods_count, goods_remains);
};

class QueryItemInfo_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(QueryItemInfo_Re, QUERY_ITEM_INFO_RE);
public:
	enum QUERY_ITEM_ERR
	{
		ERR_SUCCESS,
		ERR_ITEM_NOT_EXIST, // 查询的位置上不存在物品
		ERR_ITEM_NOT_MATCH,	// 查询位置上的物品模板不一致
	};

	int8_t result;
	ItemDetail detail;
	PACKET_DEFINE(result, detail);
};

class AnnounceNewMail : public ProtoPacket
{
	DECLARE_PROTOPACKET(AnnounceNewMail, ANNOUNCE_NEW_MAIL);
public:
	PACKET_DEFINE();
};

class GetMailList_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetMailList_Re, GET_MAIL_LIST_RE);
public:
	std::vector<Mail> mail_list;
	PACKET_DEFINE(mail_list);
};


///
/// ---- cmd: 130
///
class GetMailAttach_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetMailAttach_Re, GET_MAIL_ATTACH_RE);
public:
	MailAttach attach;
	PACKET_DEFINE(attach);
};

class DeleteMail_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(DeleteMail_Re, DELETE_MAIL_RE);
public:
	int64_t mail_id;
	PACKET_DEFINE(mail_id);
};

class TakeoffMailAttach_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(TakeoffMailAttach_Re, TAKEOFF_MAIL_ATTACH_RE);
public:
	int8_t result; // 1:成功;0:失败
	int64_t mail_id;
	PACKET_DEFINE(result, mail_id);
};

class SendMail_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(SendMail_Re, SEND_MAIL_RE);
public:
	int64_t mail_id;
	PACKET_DEFINE(mail_id);
};

class TeammemberLocalInsReply : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeammemberLocalInsReply, TEAMMEMBER_LOCAL_INS_REPLY);
public:
	// 该协议只会发给队长
	int32_t ins_group_id;  // 副本组id
	int8_t  agreement;     // 0表示不同意，1表示同意
	int64_t member_roleid; // 该队友的roleid
	std::vector<int32_t> ins_tid_vec; // 满足条件的副本模板id
	PACKET_DEFINE(ins_group_id, agreement, member_roleid, ins_tid_vec);
};


///
/// ---- cmd: 135
///
class TeammemberQuitLocalIns : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeammemberQuitLocalIns, TEAMMEMBER_QUIT_LOCAL_INS);
public:
	// 只有之前同意过的队员退出时才会产生这条协议
	int64_t member_roleid; // 队友roleid，如果id是队长则所有队员关闭副本界面
	int32_t ins_group_id;  
	PACKET_DEFINE(member_roleid, ins_group_id);
};

class TeammemberAgreeLocalIns : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeammemberAgreeLocalIns, TEAMMEMBER_AGREE_LOCAL_INS);
public:
	// 该协议只会发给队员
	int32_t ins_group_id;   // 副本组id
	int64_t member_roleid;  // 所有非队长的队员都会收到
	std::vector<int32_t> ins_tid_vec; // 满足条件的副本模板id
	PACKET_DEFINE(ins_group_id, member_roleid, ins_tid_vec);
};

class GainScore : public ProtoPacket
{
	DECLARE_PROTOPACKET(GainScore, GAIN_SCORE);
public:
	int32_t amount;
	PACKET_DEFINE(amount);
};

class SpendScore : public ProtoPacket
{
	DECLARE_PROTOPACKET(SpendScore, SPEND_SCORE);
public:
	int32_t cost;
	PACKET_DEFINE(cost);
};

class TeamLocalInsInvite : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeamLocalInsInvite, TEAM_LOCAL_INS_INVITE);
public:
	int32_t ins_group_id;
	PACKET_DEFINE(ins_group_id);
};


///
/// ---- cmd: 140
///
class MapTeamInfo : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamInfo, MAP_TEAM_INFO);
public:
	// 玩家第一次进入的时候会收到队伍信息
	int32_t teamid;
	int64_t leaderid;
	std::vector<MapTeamMemberInfo> members; // 始终是4个
	PACKET_DEFINE(teamid, leaderid, members);
};

class MapTeamQueryMemberRe : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamQueryMemberRe, MAP_TEAM_QUERY_MEMBER_RE);
public:
	// 查询副本队伍成员信息
	RoleID  member_roleid;
	int16_t level;
	int32_t weapon_id;
	int32_t combat_value;
	bool    is_in_combat;
	int8_t  cls; //职业
	PACKET_DEFINE(member_roleid, level, weapon_id, combat_value, is_in_combat, cls);
};

class MapTeamJoin : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamJoin, MAP_TEAM_JOIN);
public:
	// 新玩家进入副本队伍
	int8_t member_pos;
	MapTeamMemberInfo new_member;
	PACKET_DEFINE(member_pos, new_member);
};

class MapTeamLeave : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamLeave, MAP_TEAM_LEAVE);
public:
	// 玩家退出副本队伍（主动退出，即退出副本）
	int64_t leave_roleid;
	PACKET_DEFINE(leave_roleid);
};

class MapTeamStatusChange : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamStatusChange, MAP_TEAM_STATUS_CHANGE);
public:
	int64_t roleid;
	bool online;
	PACKET_DEFINE(roleid, online);
};


///
/// ---- cmd: 145
///
class MapTeamChangeLeader_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamChangeLeader_Re, MAP_TEAM_CHANGE_LEADER_RE);
public:
	int8_t error;  // 非0表示错误，错误时只会发给队长，对应G2C_error.h里的ChangeLeaderError枚举
	int64_t new_leader;
	PACKET_DEFINE(error, new_leader);
};

class MapTeamChangePos_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamChangePos_Re, MAP_TEAM_CHANGE_POS_RE);
public:
	int8_t src_index; // 从0开始,0~3
	int8_t des_index;
	PACKET_DEFINE(src_index, des_index);
};

class TransferGender : public ProtoPacket
{
	DECLARE_PROTOPACKET(TransferGender, TRANSFER_GENDER);
public:
    // 该协议已作废
	int64_t roleid;
	int8_t gender;
	PACKET_DEFINE(roleid, gender);
};

class CombatLearnSkill : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatLearnSkill, COMBAT_LEARN_SKILL);
public:
	int32_t skill_group_id;
	int16_t skill_group_level;
	int16_t time_learn; //msec
	PACKET_DEFINE(skill_group_id, skill_group_level, time_learn);
};

class PlayerEnterInsMap : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerEnterInsMap, PLAYER_ENTER_INS_MAP);
public:
	int32_t ins_templ_id;
	int32_t ins_create_time;
	PACKET_DEFINE(ins_templ_id, ins_create_time);
};


///
/// ---- cmd: 150
///
class InstanceEnd : public ProtoPacket
{
	DECLARE_PROTOPACKET(InstanceEnd, INSTANCE_END);
public:
	enum InsResult
	{
		IR_FAILURE = 1,
		IR_VICTORY,
	};

	enum AwardClass
	{
        AC_NONE   = 0, // 无奖励
		AC_NORMAL = 1, // 普通奖励
		AC_BRONZE,     // 铜牌奖励
		AC_SILVER,     // 银牌奖励
		AC_GOLD,       // 金牌奖励
		AC_SVR_RECORD, // 破服务器记录奖励
	};

	virtual void Release()
	{
		result     = IR_FAILURE;
		exp_gain   = 0;
		money_gain = 0;
        last_svr_record = 0;
		items_drop.clear();
		items_lottery.clear();
	}

	int32_t ins_templ_id;
	int32_t clear_time;   // 通关时间，单位：秒
	int8_t  result;       //战斗结果：对应枚举InsResult
	int8_t  award_class;  // 奖励级别：对应枚举AwardClass
	int32_t exp_gain;
	int32_t money_gain;
    int32_t last_svr_record; // 最近一次破服务器记录的时间，0是无效的
	std::vector<ItemEntry> items_drop;    // 普通掉落和全局掉落物品
	std::vector<ItemEntry> items_lottery; // 抽奖物品(特殊掉落)
	PACKET_DEFINE(ins_templ_id, clear_time, result, award_class, exp_gain, money_gain, items_drop, items_lottery);
};

class PlayerScore : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerScore, PLAYER_SCORE);
public:
    int32_t score_total;
	int32_t score_used;
	PACKET_DEFINE(score_total, score_used);
};

class SelfItemDetailListAll : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelfItemDetailListAll, SELF_ITEM_DETAIL_LIST_ALL);
public:
	struct Inventory
	{
		int16_t inv_cap;
		std::vector<ItemDetail> item_list;
		NESTED_DEFINE(inv_cap, item_list);
	};

	std::vector<Inventory> inv_list;
	PACKET_DEFINE(inv_list);
};

class PetPower : public ProtoPacket
{
	DECLARE_PROTOPACKET(PetPower, PET_POWER);
public:
	int32_t power;
	PACKET_DEFINE(power);
};

class PetData : public ProtoPacket
{
	DECLARE_PROTOPACKET(PetData, PET_DATA);
public:
	int32_t pet_power;
	int32_t pet_power_cap;
	std::vector<PetEntry> pet_list;
	PACKET_DEFINE(pet_power, pet_power_cap, pet_list);
};

///
/// ---- cmd: 155
///
class CombatPetAttack_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPetAttack_Re, COMBAT_PET_ATTACK_RE);
public:
	int8_t  result; // 1:成功;0:失败
	int8_t  combat_pet_inv_pos;
	PACKET_DEFINE(result, combat_pet_inv_pos);
};

class TalentData : public ProtoPacket
{
	DECLARE_PROTOPACKET(TalentData, TALENT_DATA);
public:
	struct TalentEntry
	{
		int32_t talent_id;  // 天赋ID
		int8_t  level;      // 正式等级
		int8_t  tmp_level;  // 临时等级
		NESTED_DEFINE(talent_id, level, tmp_level);
	};

	struct TalentGroup
	{
		int32_t talent_group_id;
		std::vector<TalentEntry> talent_list;
		NESTED_DEFINE(talent_group_id, talent_list);
	};
	std::vector<TalentGroup> talent_group_list;
	PACKET_DEFINE(talent_group_list);
};

class TalentDetail : public ProtoPacket
{
	DECLARE_PROTOPACKET(TalentDetail, TALENT_DETAIL);
public:
	int32_t talent_group_id; // 天赋组ID
	int32_t talent_id;       // 天赋ID
	int8_t  level;           // 正式等级
	int8_t  tmp_level;       // 临时等级
	PACKET_DEFINE(talent_group_id, talent_id, level, tmp_level);
};

class LevelUpTalent_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(LevelUpTalent_Re, LEVELUP_TALENT_RE);
public:
	int8_t  result;
	int32_t talent_group_id;
	int32_t talent_id;
	PACKET_DEFINE(result, talent_group_id, talent_id);
};

class OpenTalentGroup : public ProtoPacket
{
	DECLARE_PROTOPACKET(OpenTalentGroup, OPEN_TALENT_GROUP);
public:
	int32_t talent_group_id;
	PACKET_DEFINE(talent_group_id);
};

///
/// ---- cmd: 160
///
class CombatPetPower : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPetPower, COMBAT_PET_POWER);
public:
	int32_t pet_power; // 宠物能量的当前值
	PACKET_DEFINE(pet_power);
};

class FullPetPower_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(FullPetPower_Re, FULL_PET_POWER_RE);
public:
	int8_t  result;    // 1:成功;0:失败
	int32_t pet_power; // 宠物能量的当前值
	PACKET_DEFINE(result, pet_power);
};

class LevelUpCard_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(LevelUpCard_Re, LEVELUP_CARD_RE);
public:
	enum
	{
		LVLUP_TYPE_CARD,       // 消耗卡牌升级
		LVLUP_TYPE_CASH,      // 人民币直接升级, 不消耗卡牌
	};

    int8_t  result;                         // 1:成功；0:失败（钱不足）
	int8_t  lvlup_type;                     // 升级类型(卡牌或人民币)
	int16_t idx_main_card;                  // 主卡牌位置
    std::vector<int16_t> idx_vice_cards;    // 消耗的副卡牌位置

    PACKET_DEFINE(result, lvlup_type, idx_main_card, idx_vice_cards);
};

class AuctionItem_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionItem_Re, AUCTION_ITEM_RE);
public:
    int8_t  err_code; // 0表示成功，非零表示不成功（失败原因可以先不写）
    AuctionItemDetail detail;
    PACKET_DEFINE(err_code, detail);
};

class AuctionCancel_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionCancel_Re, AUCTION_CANCEL_RE);
public:
    enum
    {
        UNKNOWN    = -1, // 未知错误
        IS_SUCCESS = 0,  // 取消拍卖成功
        DB_BUSY,         // 数据库忙
        NO_AUCTION,      // 没有找到该拍卖的记录
        PARAM_ERR,       // 参数错误
        AUCTION_BID,     // 已经有人出价，不能取消拍卖
    };

    int64_t auction_id;
    int8_t  err_code; 
    int64_t bidder;
    int32_t cur_bid_price;
    PACKET_DEFINE(auction_id, err_code, bidder, cur_bid_price);
};


///
/// ---- cmd: 165
///
class AuctionBuyout_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionBuyout_Re, AUCTION_BUYOUT_RE);
public:
    enum 
    {
        UNKNOWN   = -1, // 未知错误
        IS_SUCCESS = 0, // 取消拍卖成功
        DB_BUSY,        // 数据库忙
        NO_AUCTION,     // 没有找到该拍卖的记录
        PARAM_ERR,      // 参数错误
    };

    int8_t  err_code;
    int64_t auction_id;
    PACKET_DEFINE(err_code, auction_id);
};

class AuctionBid_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionBid_Re, AUCTION_BID_RE);
public:
    enum 
    {
        UNKNOWN   = -1, // 未知错误
        IS_SUCCESS = 0, // 竞拍出价成功
        DB_BUSY,        // 数据库忙
        NO_AUCTION,     // 没有找到该拍卖的记录
        PRICE_ERR,      // 价格错误，已经有人出价比你高
        PARAM_ERR,      // 参数错误
    };

    int8_t  err_code;
    int64_t auction_id;
    int32_t cur_price;  // 当前出价
    int64_t cur_bidder; // 当前的出价者
    PACKET_DEFINE(err_code, auction_id, cur_price, cur_bidder);
};

class AuctionRevoke : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionRevoke, AUCTION_REVOKE);
public:
    // "我的拍卖"或"我的竞拍"里的拍卖已经撤销，两种可能情况：过期，已经被买走（这个拍卖确实没了）
    // 别人竞拍价更高，不会收到这个协议
    std::vector<int64_t> auctionid_list;
    PACKET_DEFINE(auctionid_list);
};

class PlayerLeaveInsMap : public ProtoPacket
{
    DECLARE_PROTOPACKET(PlayerLeaveInsMap, PLAYER_LEAVE_INS_MAP);
public:
    int32_t ins_templ_id;
    PACKET_DEFINE(ins_templ_id);
};

class CombatPlayerSelectSkill : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatPlayerSelectSkill, COMBAT_PLAYER_SELECT_SKILL);
public:
    int32_t unit_id;
    int32_t skill_id;
    PACKET_DEFINE(unit_id, skill_id);
};


///
/// ---- cmd: 170
///
class FriendData : public ProtoPacket
{
    DECLARE_PROTOPACKET(FriendData, FRIEND_DATA);
public:
    FriendList friend_list;
    FriendList enemy_list;
    FriendList black_list;
    PACKET_DEFINE(friend_list, enemy_list, black_list);
};

class AddFriendRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(AddFriendRe, ADD_FRIEND_RE);
public:
    int64_t roleid;
    int8_t flag;
    std::string name;
    PACKET_DEFINE(roleid, flag, name);
};

class DeleteFriendRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(DeleteFriendRe, DELETE_FRIEND_RE);
public:
    int64_t roleid;
    PACKET_DEFINE(roleid);
};

class TeamLeaderConvene : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeamLeaderConvene, TEAM_LEADER_CONVENE);
public:
    PACKET_DEFINE();
};

class CombatMobTransform : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatMobTransform, COMBAT_MOB_TRANSFORM);
public:
    int32_t unit_id; //哪一只怪物变身
    CombatMobInfo new_mob_info; //变身后的怪物信息
    PACKET_DEFINE(unit_id, new_mob_info);
};


///
/// ---- cmd: 175
///
class TeamCrossInsInvite : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeamCrossInsInvite, TEAM_CROSS_INS_INVITE);
public:
    int32_t ins_group_id;
    int32_t ins_templ_id;
    PACKET_DEFINE(ins_group_id, ins_templ_id);
};

class TeamCrossInsInviteReply : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeamCrossInsInviteReply, TEAM_CROSS_INS_INVITE_REPLY);
public:
    enum ReplyType 
    {
        RT_AGREE = 0, // 同意
        RT_DISAGREE,  // 不同意
        RT_NOT_MATCH, // 该队友不满足进入该副本的条件
    };

    // 发给队长的消息
    int64_t member_roleid;
    int32_t ins_group_id;
    int32_t ins_templ_id;
    int8_t  reply_type;
    PACKET_DEFINE(member_roleid, ins_group_id, ins_templ_id, reply_type);
};

class TeammemberAgreeCrossIns : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeammemberAgreeCrossIns, TEAMMEMBER_AGREE_CROSS_INS);
public:
    // 该协议只会发给队员
	int64_t member_roleid;  // 所有非队长的队员都会收到
	int32_t ins_group_id;   // 副本组id
	int32_t ins_templ_id;   // 满足条件的副本模板id
	PACKET_DEFINE(ins_group_id, member_roleid, ins_templ_id);
};

class TeamCrossInsRequestComplete : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeamCrossInsRequestComplete, TEAM_CROSS_INS_REQUEST_COMPLETE);
public:
    // 该协议表示跨服副本申请已经发送给报名服务器
    PACKET_DEFINE();
};

class CombatPVPEnd : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPVPEnd, COMBAT_PVP_END);
public:
	int8_t  result;//战斗结果:1:赢;2:输
	PACKET_DEFINE(result);
};


///
/// ---- cmd: 180
///
class TitleData : public ProtoPacket
{
    DECLARE_PROTOPACKET(TitleData, TITLE_DATA);
public:
    int32_t cur_title;
    std::vector<int32_t> title_list;
    PACKET_DEFINE(cur_title, title_list);
};

class GainTitle : public ProtoPacket
{
    DECLARE_PROTOPACKET(GainTitle, GAIN_TITLE);
public:
    int32_t title_id;
    PACKET_DEFINE(title_id);
};

class SwitchTitle_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(SwitchTitle_Re, SWITCH_TITLE_RE);
public:
    int32_t old_title;
    int32_t new_title;
    PACKET_DEFINE(old_title, new_title);
};

class BroadcastPlayerTitle : public ProtoPacket
{
    DECLARE_PROTOPACKET(BroadcastPlayerTitle, BROADCAST_PLAYER_TITLE);
public:
	RoleID  roleid;
    int32_t title_id;
    PACKET_DEFINE(roleid, title_id);
};

class DuelRequest : public ProtoPacket
{
    DECLARE_PROTOPACKET(DuelRequest, DUEL_REQUEST);
public:
    int64_t duel_roleid;  // 发起决斗的roleid，如果是组队则id是对方队长
    int8_t  is_team_duel; // 是不是队伍向该玩家发起决斗邀请
    PACKET_DEFINE(duel_roleid, is_team_duel);
};


///
/// ---- cmd: 185
///
class DuelRequest_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(DuelRequest_Re, DUEL_REQUEST_RE);
public:
    enum ResultType
    {
        RT_SUCCESS = 0, // 成功
        RT_NOT_AGREE,   // 对方不同意
        RT_LEADER_NOT_AROUND, // 对方的队长不在附近
        RT_TOO_FAR,     // 距离太远
    };

    int8_t  result; // 对应上面的ResultType枚举
    PACKET_DEFINE(result);
};

class TeammateDuelRequest : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeammateDuelRequest, TEAMMATE_DUEL_REQUEST);
public:
    int64_t duel_roleid;
    int64_t teammate_roleid;
    PACKET_DEFINE(duel_roleid, teammate_roleid);
};

class TeammateDuelRequest_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeammateDuelRequest_Re, TEAMMATE_DUEL_REQUEST_RE);
public:
    // 服务器给决斗发起的队员发
    int8_t agreement; // 0表示队长不同意，非0表示同意
    PACKET_DEFINE(agreement);
};

class DuelPrepare : public ProtoPacket
{
    DECLARE_PROTOPACKET(DuelPrepare, DUEL_PREPARE);
public:
    // 决斗准备，可以开始倒计时
    int8_t countdown_secs; // 单位秒
    std::vector<DuelMemberInfo> own_side;    // 己方成员，包括自己
    std::vector<DuelMemberInfo> enemy_side;  // 敌方成员
    PACKET_DEFINE(countdown_secs, own_side, enemy_side);
};

class ReputationData : public ProtoPacket
{
    DECLARE_PROTOPACKET(ReputationData, REPUTATION_DATA);
public:
    typedef std::map<int32_t, int32_t> ReputationMap;
    ReputationMap reputation;

    PACKET_DEFINE(reputation);
};


///
/// ---- cmd: 190
///
class OpenReputation : public ProtoPacket
{
    DECLARE_PROTOPACKET(OpenReputation, OPEN_REPUTATION);
public:
    int32_t reputation_id;

    PACKET_DEFINE(reputation_id);
};

class ModifyReputation : public ProtoPacket
{
    DECLARE_PROTOPACKET(ModifyReputation, MODIFY_REPUTATION);
public:
    int32_t reputation_id;
    int32_t reputation_delta;

    PACKET_DEFINE(reputation_id, reputation_delta);
};

class UIData : public ProtoPacket
{
    DECLARE_PROTOPACKET(UIData, UI_DATA);
public:
    std::string config;

    PACKET_DEFINE(config);
};

class PlayerPrimaryPropRF : public ProtoPacket
{
    // 上线第一次同步信息时使用
    DECLARE_PROTOPACKET(PlayerPrimaryPropRF, PLAYER_PRIMARY_PROP_RF);
public:
    struct Detail
    {
        int32_t cur_add_energy; // 当前已经充了多少能量
        int32_t add_points;     // 最终对应属性所加的点
        NESTED_DEFINE(cur_add_energy, add_points);
    };

    PPRFBriefInfo info;
    std::vector<Detail> detail; // 下标对应枚举PrimaryPropRFIndex
    PACKET_DEFINE(info, detail);
};

class QueryPrimaryPropRF_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryPrimaryPropRF_Re, QUERY_PRIMARY_PROP_RF_RE);
public:
    PPRFBriefInfo info;
    PACKET_DEFINE(info);
};


///
/// ---- cmd: 195
///
class PrimaryPropReinforce_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(PrimaryPropReinforce_Re, PRIMARY_PROP_REINFORCE_RE);
public:
    struct UpgradeInfo
    {
        int8_t  type; // 提升类型，对应模板primary_prop_reinforce_cfg.h里的枚举PrimaryPropRFUpgradeType
        int32_t add_points; // 每次升级后对应属性所加的点
        NESTED_DEFINE(type, add_points);
    };

    struct Detail
    {
        int32_t cur_energy; // 当前已经充了多少能量
        int32_t add_points; // 最终对应属性所加的点
        int32_t add_energy; // 本次共加了多少能量
        std::vector<UpgradeInfo> upgrade_info; // 每一项表示升级了一次
        NESTED_DEFINE(cur_energy, add_points, add_energy, upgrade_info);
    };

    PPRFBriefInfo info;
    std::vector<Detail>  detail; // 下标对应枚举PrimaryPropRFIndex
    PACKET_DEFINE(info, detail);
};

class BuyPrimaryPropRFEnergy_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(BuyPrimaryPropRFEnergy_Re, BUY_PRIMARY_PROP_RF_ENERGY_RE);
public:
    enum ErrorCode
    {
        EC_SUCCESS = 0, // 购买成功
        EC_NO_CASH,     // 没元宝
    };

    struct Detail
    {
        int32_t cur_energy; // 当前已经充了多少能量
        int32_t add_points; // 最终对应属性所加的点
        NESTED_DEFINE(cur_energy, add_points);
    };

    int8_t result;      // 对应上面的ErrorCode枚举
    int8_t sale_index;  // 买得是哪一档的能量
    std::vector<Detail> detail; // 下标对应枚举PrimaryPropRFIndex
    PACKET_DEFINE(result, sale_index, detail);
};

class AchieveData : public ProtoPacket
{
    DECLARE_PROTOPACKET(AchieveData, ACHIEVE_DATA);
public:
    std::string finish_achieve;
    std::string achieve_data;

    PACKET_DEFINE(finish_achieve, achieve_data);
};

class AchieveNotifyClient : public ProtoPacket
{
	DECLARE_PROTOPACKET(AchieveNotifyClient, ACHIEVE_NOTIFY_CLIENT);
public:
	uint16_t type;
	std::string databuf;
	PACKET_DEFINE(type, databuf);
};

class LoginData : public ProtoPacket
{
    DECLARE_PROTOPACKET(LoginData, LOGIN_DATA);
public:
    int32_t last_login_time;

    PACKET_DEFINE(last_login_time);
};

struct enhance_entry
{
    enhance_entry()
        : open_mode(0), slot_status(0), enhance_id(0)
    {
    }

    int8_t open_mode;
    int8_t slot_status;
    int32_t enhance_id;
    NESTED_DEFINE(open_mode, slot_status, enhance_id);
};
typedef std::vector<enhance_entry> EnhanceList;

class EnhanceData : public ProtoPacket
{
    DECLARE_PROTOPACKET(EnhanceData, ENHANCE_DATA);
public:
    EnhanceList enhance_list;
    PACKET_DEFINE(enhance_list);
};

class OpenEnhanceSlotRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(OpenEnhanceSlotRe, OPEN_ENHANCE_SLOT_RE);
public:
    int8_t slot_index;          // 为-1表示元宝不足
    enhance_entry new_slot;
    PACKET_DEFINE(slot_index, new_slot);
};

class ProtectEnhanceSlotRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(ProtectEnhanceSlotRe, PROTECT_ENHANCE_SLOT_RE);
public:
    int8_t slot_index;          // 为-1表示元宝不足
    PACKET_DEFINE(slot_index);
};

class UnProtectEnhanceSlotRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(UnProtectEnhanceSlotRe, UNPROTECT_ENHANCE_SLOT_RE);
public:
    int8_t slot_index;
    PACKET_DEFINE(slot_index);
};

class EnhanceRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(EnhanceRe, ENHANCE_RE);
public:
    enum ENHANCE_ERR
    {
        ERR_ENHANCE_SUCC,
        ERR_ENHANCE_NO_COUNT,
    };
    int32_t err;
    int8_t slot_index;
    int32_t enhance_id;
    int32_t count;
    PACKET_DEFINE(err, slot_index, enhance_id, count);
};

class StarData : public ProtoPacket
{
    DECLARE_PROTOPACKET(StarData, STAR_DATA);
public:
    typedef std::map<int8_t, int8_t> SparkMap;
    typedef std::map<int32_t, SparkMap> StarMap;
    StarMap star;
    PACKET_DEFINE(star);
};


///
/// ---- cmd: 205
///
class OpenStarRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(OpenStarRe, OPEN_STAR_RE);
public:
    int32_t star_id;
    PACKET_DEFINE(star_id);
};

class ActivateSparkRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(ActivateSparkRe, ACTIVATE_SPARK_RE);
public:
    int32_t star_id;            // 星盘ID
    int8_t spark_index;         // -1表示游戏币不足
    PACKET_DEFINE(star_id, spark_index);
};

class UseItemError : public ProtoPacket
{
    DECLARE_PROTOPACKET(UseItemError, USE_ITEM_ERROR);
public:
    int16_t error_num; // 对应G2C_types.h里的UseItemErrorCode枚举
    int32_t type;      // 物品模板id
    PACKET_DEFINE(error_num, type);
};

class PlayerEnterBGMap : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerEnterBGMap, PLAYER_ENTER_BG_MAP);
public:
	int32_t bg_templ_id;
	int32_t bg_create_time;
	PACKET_DEFINE(bg_templ_id, bg_create_time);
};


///
/// ---- cmd: 210
///
class PlayerLeaveBGMap : public ProtoPacket
{
    DECLARE_PROTOPACKET(PlayerLeaveBGMap, PLAYER_LEAVE_BG_MAP);
public:
    int32_t bg_templ_id;
    PACKET_DEFINE(bg_templ_id);
};

class GetBattleGroundData_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetBattleGroundData_Re, GET_BATTLEGROUND_DATA_RE);
public:
    int32_t bg_templ_id;
    int8_t  is_cross_realm; // 是不是跨服副本，0表示不是跨服，1表示是跨服
    PACKET_DEFINE(bg_templ_id, is_cross_realm);
};

class EquipCard_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(EquipCard_Re, EQUIP_CARD_RE);
public:
    int16_t card_index;     // 要镶嵌的卡牌在包裹中的位置
    int32_t star_id;        // 要嵌入的星辰ID
    PACKET_DEFINE(card_index, star_id);
};

class GainCard : public ProtoPacket
{
    DECLARE_PROTOPACKET(GainCard, GAIN_CARD);
public:
    CardEntry card;
	PACKET_DEFINE(card);
};

class LostCard : public ProtoPacket
{
	DECLARE_PROTOPACKET(LostCard, LOST_CARD);
public:
	int16_t item_inv_idx;
	PACKET_DEFINE(item_inv_idx);
};


///
/// ---- cmd: 215
///
class MapTeamJoinTeamRequest : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapTeamJoinTeamRequest, MAP_TEAM_JOIN_TEAM_REQUEST);
public:
    bool    invite; // true表示对方邀请，false表示对方申请加入队伍
    int64_t requester;
    std::string first_name;
    std::string mid_name;
    std::string last_name;
    PACKET_DEFINE(invite, requester, first_name, mid_name, last_name);
};

class InsPromptMessage : public ProtoPacket
{
    DECLARE_PROTOPACKET(InsPromptMessage, INS_PROMPT_MESSAGE);
public:
    MapPromptMessage info;
    PACKET_DEFINE(info);
};

class BGPromptMessage : public ProtoPacket
{
    DECLARE_PROTOPACKET(BGPromptMessage, BG_PROMPT_MESSAGE);
public:
    MapPromptMessage info;
    PACKET_DEFINE(info);
};

class GlobalCounterChange : public ProtoPacket
{
    DECLARE_PROTOPACKET(GlobalCounterChange, GLOBAL_COUNTER_CHANGE);
public:
    int32_t index; // 全局计数器index
    int32_t value; // 全局计数器的当前值
    PACKET_DEFINE(index, value);
};

class MapCounterChange : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapCounterChange, MAP_COUNTER_CHANGE);
public:
    int32_t map_id; // 地图号
    int32_t index;  // 地图计数器index
    int32_t value;  // 地图计数器的当前值，注意地图计数器每次换地图时都应该清除掉
                    // value肯定是一个大于0的值，小于0服务器不会通知
    PACKET_DEFINE(map_id, index, value);
};


///
/// ---- cmd: 220
///
class CombatBuffSync : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatBuffSync, COMBAT_BUFF_SYNC);
public:
	std::vector<CombatBuffInfo> buff_list; //buff列表
	PACKET_DEFINE(buff_list);
};

class UpdateTaskStorage_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(UpdateTaskStorage_Re, UPDATE_TASK_STORAGE_RE);
public:
    int8_t err; // 0表示成功，1表示钱不足
    int32_t storage_id;
    PACKET_DEFINE(err, storage_id);
};

class CombatMobEscape : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatMobEscape, COMBAT_MOB_ESCAPE);
public:
    int32_t unit_id; //哪一只怪物逃跑
    int32_t mob_pos; // 逃跑怪物所在位置
    PACKET_DEFINE(unit_id, mob_pos);
};

class PlayerCounterChange : public ProtoPacket
{
    DECLARE_PROTOPACKET(PlayerCounterChange, PLAYER_COUNTER_CHANGE);
public:
    enum ChangeType
    {
        CT_REGISTER,
        CT_UNREGISTER,
        CT_MODIFY,
    };

    int8_t  type;  // 对应上面的ChangeType枚举
    int32_t tid;   // 玩家计数器模板id
    int32_t value; // 玩家计数器的当前值
    PACKET_DEFINE(type, tid, value);
};

class PlayerCounterList : public ProtoPacket
{
    DECLARE_PROTOPACKET(PlayerCounterList, PLAYER_COUNTER_LIST);
public:
    struct Entry
    {
        int32_t tid;
        int32_t value;
        NESTED_DEFINE(tid, value);
    };

    std::vector<Entry> counter_list;
    PACKET_DEFINE(counter_list);
};


///
/// ---- cmd: 225
///
class NotifyObjectPos : public ProtoPacket
{
    DECLARE_PROTOPACKET(NotifyObjectPos, NOTIFY_OBJECT_POS);
public:
    ObjectID       obj_id;
    A2DVECTOR_PACK pos;
    uint8_t        dir;   // 朝向
    PACKET_DEFINE(obj_id, pos, dir);
};

class NPCFriendStatusChange : public ProtoPacket
{
    DECLARE_PROTOPACKET(NPCFriendStatusChange, NPC_FRIEND_STATUS_CHANGE);
public:
    FriendList friend_list;
    PACKET_DEFINE(friend_list);
};

class ShowMapCountDown : public ProtoPacket
{
    DECLARE_PROTOPACKET(ShowMapCountDown, SHOW_MAP_COUNTDOWN);
public:
    int32_t countdown; // 倒计时数，从X到0的倒计时
    int32_t screen_x;  // 显示在屏幕位置的x坐标
    int32_t screen_y;  // 显示在屏幕位置的y坐标
    PACKET_DEFINE(countdown, screen_x, screen_y);
};

class CombatSceneShake : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatSceneShake, COMBAT_SCENE_SHAKE);
public:
    int32_t x_amplitude;
    int32_t y_amplitude;
    int32_t shake_duration;
    int32_t shake_interval;
    PACKET_DEFINE(x_amplitude, y_amplitude, shake_duration, shake_interval);
};

class CombatUnitCurGolemProp : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatUnitCurGolemProp, COMBAT_UNIT_CUR_GOLEM_PROP);
public:
	int32_t unit_id;
    int32_t golem_id;
	int32_t mp;
	PACKET_DEFINE(unit_id, golem_id, mp);
};


///
/// ---- cmd: 230
///
class CombatUnitOtherGolemProp : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatUnitOtherGolemProp, COMBAT_UNIT_OTHER_GOLEM_PROP);
public:
    typedef std::map<int32_t/*golem_id*/, int32_t/*mp*/> GolemPowerMap;
    GolemPowerMap cur_power;
    GolemPowerMap deactive_power;
	PACKET_DEFINE(cur_power, deactive_power);
};

class PunchCardData : public ProtoPacket
{
    DECLARE_PROTOPACKET(PunchCardData, PUNCH_CARD_DATA);
public:
    int32_t history_punch;     // 所有签到次数
    int32_t history_award_got; // 历史奖励领取到哪一档，记录的是当前已经领取档次的累计次数
    int32_t re_punch_count;    // 可以帮好友补签的次数
    std::set<int32_t> monthly_punch;     // 当月那些天已经签到
    std::set<int32_t> monthly_award_got; // 当月已经领取的奖励，记录的是已经领取档次的累计次数
    PACKET_DEFINE(history_punch, history_award_got, re_punch_count, monthly_punch, monthly_award_got);
};

class PunchCardResult : public ProtoPacket
{
    DECLARE_PROTOPACKET(PunchCardResult, PUNCH_CARD_RESULT);
public:
    enum ResultType
    {
        RT_SUCCESS,         // 签到或补签成功
        RT_HAS_PUNCHED,     // 已经签到
        RT_FRIEND_REJECT,   // 好友拒绝补签
        RT_FRIEND_NO_COUNT, // 好友已经没有补签次数
    };

    int8_t  result;        // 签到结果，对应上面的枚举ResultType
    int32_t day_of_month;  // 签到的是哪天
    RoleID  friend_roleid; // 帮忙补签的好友id，当天自己签到则是0
    PACKET_DEFINE(result, day_of_month, friend_roleid);
};

class QueryInstanceRecord_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryInstanceRecord_Re, QUERY_INSTANCE_RECORD_RE);
public:
    int32_t ins_group_id; // 副本组id
    int32_t ins_tid;      // 被查询的副本模板id
    int32_t clear_time;   // 通关时间，单位秒，0表示该副本没有记录，-1表示正在冷却中
    std::vector<InsRecordPInfo> players;
    PACKET_DEFINE(ins_group_id, ins_tid, clear_time, players);
};

class QueryPunchCardData_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryPunchCardData_Re, QUERY_PUNCH_CARD_DATA_RE);
public:
    int32_t re_punch_count; // 可以帮好友补签的次数
    PACKET_DEFINE(re_punch_count);
};

///
/// ---- cmd: 235
///
class GainPunchCardAward_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(GainPunchCardAward_Re, GAIN_PUNCH_CARD_AWARD_RE);
public:
    enum ResultType
    {
        RT_SUCCESS,   // 领取奖励成功
        RT_GAINED,    // 已经领取
        RT_NOT_FOUND, // 没有找到对应的奖品级别
        RT_DATA_ERR,  // 数据错误
    };

    int8_t  result;      // 对应枚举ResultType
    int8_t  award_type;  // 对应枚举C2G::PunchCardAwardType
    int32_t cumulative_num; // 领取那个累计次数档次的奖品
    PACKET_DEFINE(result, award_type, cumulative_num);
};

class GeventData : public ProtoPacket
{
    DECLARE_PROTOPACKET(GeventData, GEVENT_DATA);
public:
    std::vector<GeventInfo> gevent_list;
    PACKET_DEFINE(gevent_list);
};

class JoinGevent_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(JoinGevent_Re, JOIN_GEVENT_RE);
public:
	enum RESULT
	{
        JOIN_GEVENT_SUCC,
        JOIN_GEVENT_GID,
        JOIN_GEVENT_ID,
        JOIN_GEVENT_TIME,
        JOIN_GEVENT_LEVEL,
        JOIN_GEVENT_NUM,
        JOIN_GEVENT_TASK,
        JOIN_GEVENT_MAP,
	};

    int8_t result;
    int32_t gevent_gid;
    int32_t gevent_id;
    PACKET_DEFINE(result, gevent_gid, gevent_id);
};

class ParticipationData : public ProtoPacket
{
    DECLARE_PROTOPACKET(ParticipationData, PARTICIPATION_DATA);
public:
    int32_t participation;
    std::string award_info;
    PACKET_DEFINE(participation, award_info);
};

class ParticipationChange : public ProtoPacket
{
    DECLARE_PROTOPACKET(ParticipationChange, PARTICIPATION_CHANGE);
public:
    int32_t new_value;
    PACKET_DEFINE(new_value);
};

///
/// ---- cmd: 240
///
class ObjectHateYou : public ProtoPacket
{
    DECLARE_PROTOPACKET(ObjectHateYou, OBJECT_HATE_YOU);
public:
    ObjectID obj_id;
    PACKET_DEFINE(obj_id);
};

class RePunchCardHelp : public ProtoPacket
{
    DECLARE_PROTOPACKET(RePunchCardHelp, RE_PUNCH_CARD_HELP);
public:
    RoleID  role_id;
    int32_t day_of_month;
    std::string first_name;
    std::string mid_name;
    std::string last_name;
    PACKET_DEFINE(role_id, day_of_month, first_name, mid_name, last_name);
};

class MountData : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountData, MOUNT_DATA);
public:
    int32_t mount_index;
    std::vector<int32_t> mount_equip_level;

    PACKET_DEFINE(mount_index, mount_equip_level);
};

class MountMount_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountMount_Re, MOUNT_MOUNT_RE);
public:
    int8_t result; // 0成功，1找不到对应的坐骑
    int32_t item_index;
    int32_t item_id;
    PACKET_DEFINE(result, item_index, item_id);
};

class MountExchange_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountExchange_Re, MOUNT_EXCHANGE_RE);
public:
    int8_t result; // 0成功，1兑换物品不足
    PACKET_DEFINE(result);
};


///
/// ---- cmd: 245
///
class MountEquipLevelUp_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountEquipLevelUp_Re, MOUNT_EQUIP_LEVELUP_RE);
public:
    int8_t result;      // 0成功，1条件不足
    int8_t equip_index; // 升级的骑具位置
    PACKET_DEFINE(result, equip_index);
};

class GetParticipationAward_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetParticipationAward_Re, GET_PARTICIPATION_AWARD_RE);
public:
    enum RESULT
    {
        GET_AWARD_SUCC,
        GET_AWARD_FULL,
    };
    int8_t award_index;
    int8_t result;
    PACKET_DEFINE(award_index, result);
};

class OpenMountEquip : public ProtoPacket
{
    DECLARE_PROTOPACKET(OpenMountEquip, OPEN_MOUNT_EQUIP);
public:
    int8_t equip_index;
    PACKET_DEFINE(equip_index);
};

class GainMount : public ProtoPacket
{
    DECLARE_PROTOPACKET(GainMount, GAIN_MOUNT);
public:
    int8_t mount_index;
    PACKET_DEFINE(mount_index);
};

class LostMount : public ProtoPacket
{
    DECLARE_PROTOPACKET(LostMount, LOST_MOUNT);
public:
    int8_t mount_index;
    PACKET_DEFINE(mount_index);
};


///
/// ---- cmd: 250
///
class CombatSkillSync : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatSkillSync, COMBAT_SKILL_SYNC);
public:
    int32_t cur_skillid;
    std::map<int32_t, int32_t> cooldown;
    PACKET_DEFINE(cur_skillid, cooldown);
};

class GeventDataChange : public ProtoPacket
{
    DECLARE_PROTOPACKET(GeventDataChange, GEVENT_DATA_CHANGE);
public:
    GeventInfo gevent_data;
    PACKET_DEFINE(gevent_data);
};

class ItemDisassemble_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(ItemDisassemble_Re, ITEM_DISASSEMBLE_RE);
public:
    // 成功分解普通包裹里的某个物品
    int16_t index;
    PACKET_DEFINE(index);
};

class GameServerVersion : public ProtoPacket
{
    DECLARE_PROTOPACKET(GameServerVersion, GAME_SERVER_VERSION);
public:
    std::string resource_version; // 资源文件的版本码
    PACKET_DEFINE(resource_version);
};

class RePunchCardHelp_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(RePunchCardHelp_Re, RE_PUNCH_CARD_HELP_RE);
public:
    int8_t is_success;  // 非零表示帮忙补签成功，0表示对方已经找别人帮忙补签
    std::string first_name;
    std::string middle_name;
    std::string last_name;
    PACKET_DEFINE(is_success, first_name, middle_name, last_name);
};


///
/// ---- cmd: 255
///
class BossChallengeData : public ProtoPacket
{
    DECLARE_PROTOPACKET(BossChallengeData, BOSS_CHALLENGE_DATA);
public:
    // BossMap记录对应Boss玩家的完成情况，value的最低位为1表示已经击败Boss，第二位为1表示挑战BOSS的奖励已经领取
    // monster_gid为0的表示挑战组的完成情况，value的第二位为1表示奖励已经领取，挑战组是否需要检查所有BOSS有没有战胜
    typedef std::map<int32_t/*monster_gid*/, int32_t> BossMap; 
    typedef std::map<int32_t/*challenge_id*/, BossMap> BossGroupMap;
    BossGroupMap boss;
    PACKET_DEFINE(boss);
};

class GetBossChallengeAward_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetBossChallengeAward_Re, GET_BOSS_CHALLENGE_AWARD_RE);
public:
	enum RESULT
	{
        GET_BOSS_AWARD_SUCC,        // 领取奖励成功
        GET_BOSS_AWARD_ALREADY,     // 奖励已经领取
        GET_BOSS_AWARD_NONE,        // 没有奖励
        GET_BOSS_AWARD_FULL,        // 包裹已满
	};

    int8_t result;
    int32_t challenge_id;
    int32_t monster_gid;
    PACKET_DEFINE(result, challenge_id, monster_gid);
};

class GetClearChallengeAward_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetClearChallengeAward_Re, GET_CLEAR_CHALLENGE_AWARD_RE);
public:
	enum RESULT
	{
        GET_CLEAR_AWARD_SUCC,        // 领取奖励成功
        GET_CLEAR_AWARD_ALREADY,     // 奖励已经领取
        GET_CLEAR_AWARD_NONE,        // 没有奖励
        GET_CLEAR_AWARD_FULL,        // 包裹已满
	};

    int8_t result;
    int32_t challenge_id;
    PACKET_DEFINE(result, challenge_id);
};

class MapTeamTidyPos : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapTeamTidyPos, MAP_TEAM_TIDY_POS);
public:
    std::vector<RoleID> members; // 调整后队员对应的位置，0表示这个位置是空的
    PACKET_DEFINE(members);
};

class BossChallenge_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(BossChallenge_Re, BOSS_CHALLENGE_RE);
public:
    int8_t result; // 战斗结果:1:赢;2:输（保持跟CombatPVEEnd的result一致）
    int32_t challenge_id;
    int32_t monster_gid;
    PACKET_DEFINE(result, challenge_id, monster_gid);
};


///
/// ---- cmd: 260
///
class QueryWorldBossRecord_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryWorldBossRecord_Re, QUERY_WORLD_BOSS_RECORD_RE);
public:
    WBDamageList list;
    PACKET_DEFINE(list);
};

class WBCombatEndRecord : public ProtoPacket
{
    // 世界BOSS战斗结束时给地图内的玩家广播该协议
    DECLARE_PROTOPACKET(WBCombatEndRecord, WB_COMBAT_END_RECORD);
public:
    WBDamageList list;
    PACKET_DEFINE(list);
};

class CombatWaitSelectSkill : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatWaitSelectSkill, COMBAT_WAIT_SELECT_SKILL);
public:
    int32_t skill_index;
    int32_t wait_msec;
    int32_t talk_id;
    PACKET_DEFINE(skill_index, wait_msec, talk_id);
};

class CombatWaitSelectPet : public ProtoPacket
{
    DECLARE_PROTOPACKET(CombatWaitSelectPet, COMBAT_WAIT_SELECT_PET);
public:
    int32_t pet_index;
    int32_t wait_msec;
    int32_t talk_id;
    PACKET_DEFINE(pet_index, wait_msec, talk_id);
};

class GainCash : public ProtoPacket
{
    DECLARE_PROTOPACKET(GainCash, GAIN_CASH);
public:
    int32_t amount; // 增加的元宝
    PACKET_DEFINE(amount);
};


///
/// ---- cmd: 265
///
class SpendCash : public ProtoPacket
{
    DECLARE_PROTOPACKET(SpendCash, SPEND_CASH);
public:
    int32_t cost; // 减少的元宝
    PACKET_DEFINE(cost);
};

class ArenaODBuyTicketRe : public ProtoPacket
{
    DECLARE_PROTOPACKET(ArenaODBuyTicketRe, ARENA_OD_BUY_TICKET_RE);
public:
    int8_t result;      // 0成功
    int32_t cur_buy_ticket;
    PACKET_DEFINE(result, cur_buy_ticket);
};

}; // namespace G2C

#endif // GAMED_CLIENT_PROTO_G2C_PROTO_H_
