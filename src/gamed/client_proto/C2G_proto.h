#ifndef GAMED_CLIENT_PROTO_C2G_PROTO_H_
#define GAMED_CLIENT_PROTO_C2G_PROTO_H_

#include "shared/base/copyable.h"
#include "shared/net/packet/packet.h"
#include "types.h"


namespace C2G {

using namespace shared::net;
using namespace gamed;

typedef int32_t ObjectID;

///
/// cmd type
///

enum
{

//
// normal cmd type
//

// 0
    LOGOUT = C2G_CMD_LOWER_LIMIT,       // 登出
	START_MOVE,                         // 开始移动
	MOVE_CONTINUE,                      // 连续移动
	STOP_MOVE,                          // 停止移动
	MOVE_CHANGE,                        // 已不使用

// 5
	GET_ALL_DATA,                       // 客户端hostplayer初始化完毕，开始获取数据
	GET_ITEM_DATA,						// 获取指定物品数据
	GET_ITEM_LIST_BRIEF,				// 获取简要物品信息
	GET_ITEM_LIST_DETAIL,				// 获取详细物品数据
	DROP_INVENTORY_ITEM,				// 丢弃包裹物品

// 10
	MOVE_INVENTORY_ITEM,				// 移动包裹物品
	SELL_INVENTORY_ITEM,				// 出售包裹物品
	EXCHANGE_INVENTORY_ITEM,			// 交换包裹物品位置
	TIDY_INVENTORY,						// 整理包裹
	EQUIP_ITEM,							// 装备到装备栏

// 15
	UNDO_EQUIP,							// 卸载装备
	USE_ITEM,							// 使用物品
	REFINE_EQUIP,						// 装备精练
	COMBAT_PLAYER_JOIN,                 // 玩家加入战斗
	COMBAT_PLAYER_TRIGGER,              // 玩家触发战斗

// 20
	COMBAT_SELECT_SKILL,                // 玩家选择技能
	COMBAT_PET_ATTACK,                  // 宠物施放技能
	FULL_PET_POWER,                     // 一键提升宠物能量至上限值
	SERVICE_HELLO,                      // 和NPC say hello
    SERVICE_SERVE,                      // 请求NPC服务

// 25
	LEVELUP_PET_BLOODLINE,              // 升级宠物血脉
	LEVELUP_PET_POWER_CAP,              // 升级宠物能量上限值
	SET_COMBAT_PET,                     // 设置宠物出战或休战
	LEARN_SKILL,                        // 学习技能
	LEVELUP_TALENT,                     // 升级天赋

// 30
	TASK_NOTIFY,                        // 任务消息，任务的相关操作
	TRANSFER_PREPARE_FINISH,            // 客户端传送准备完毕
	SELECT_RESURRECT_POS,               // 玩家选择复活位置
	JOIN_TEAM,                          // 申请加入组队
	JOIN_TEAM_RES,                      // 是否同意加入队伍

// 35
	CHANGE_BUDDY_TEAM_POS,              // 伙伴组队调换位置
	GATHER_MATERIAL,                    // 采集
	GATHER_MINIGAME_RESULT,             // 小游戏开启方式的采集结果，成功失败都要发
	OPEN_CAT_VISION,                    // 开启瞄类视觉
	CLOSE_CAT_VISION,                   // 关闭瞄类视觉

// 40
	DRAMA_TRIGGER_COMBAT,               // 剧情触发战斗（客户端刷出来的怪）
	DRAMA_SERVICE_SERVE,                // 剧情npc服务（客户端刷出来的NPC）
	DRAMA_GATHER_MATERIAL,              // 剧情采矿
	DRAMA_GATHER_MINIGAME_RESULT,       // 剧情采矿小游戏开启方式的结果
	SWITCH_SKILL,                       // 转职后的切换技能

// 45
	GET_ELSE_PLAYER_EXTPROP,            // 请求其它玩家的16个属性
	CHAT_MSG,							// 聊天信息
	TASK_REGION_TRANSFER,               // 任务传送
	GET_ELSE_PLAYER_EQUIPCRC,           // 请求其它玩家的装备CRC
	GET_ELSE_PLAYER_EQUIPMENT,          // 请求其它玩家的装备数据

// 50
	TASK_CHANGE_NAME,                   // 玩家通过任务改名
	GET_STATIC_ROLE_INFO,               // 获取玩家的静态角色信息
	GET_INSTANCE_DATA,                  // 查询副本信息
	UI_TASK_REQUEST,                    // UI接任务请求
	QUERY_NPC_ZONE_INFO,                // 查询npc区域的信息

// 55
	OPEN_MALL,                          // 打开商城
	QUERY_MALL_GOODS_DETAIL,            // 获取指定商品的详细信息
	MALL_SHOPPING,                      // 商城购物
	SERVICE_GET_CONTENT,                // 获取服务内容
	QUERY_SERVER_TIME,                  // 获取服务器当前时间

// 60
	GET_MAIL_LIST,						// 获取邮件列表
	GET_MAIL_ATTACH,					// 查询邮件附件
	DELETE_MAIL,						// 删除邮件
	TAKEOFF_MAIL_ATTACH,				// 取走邮件附件
	SEND_MAIL,							// 发送邮件

// 65
    TEAM_LOCAL_INS_INVITE,              // 队长发起本服组队副本邀请
	LAUNCH_TEAM_LOCAL_INS,              // 队长发起进入组队本服副本
	QUIT_TEAM_LOCAL_INS_ROOM,           // 退出本服组队副本，界面点取消
	TEAM_CROSS_INS_REQUEST,             // 组队跨服副本请求
	QUIT_TEAM_CROSS_INS_ROOM,           // 退出跨服组队副本，界面点取消
	
// 70
	LAUNCH_SOLO_LOCAL_INS,              // 发起单人本服副本
	TEAM_LOCAL_INS_INVITE_RE,           // 组队本服副本邀请的回应，回应给队长
	TEAM_CROSS_INS_READY,               // 组队跨服副本，队员准备完毕
	PLAYER_QUIT_INSTANCE,               // 玩家主动退出副本
	MAP_TEAM_CHANGE_POS,                // 地图队伍换位置

// 75
	MAP_TEAM_CHANGE_LEADER,             // 地图队伍换队长
	MAP_TEAM_QUERY_MEMBER,              // 地图队伍查询队友详细信息
	TASK_TRANSFER_GENDER_CLS,			// 转职与变性
	TASK_INS_TRANSFER,                  // 任务传送进副本
	MOVE_COMBAT_PET,                    // 在战宠栏移动宠物

// 80
    AUCTION_ITEM,                       // 拍卖物品
    AUCTION_CANCEL,                     // 取消拍卖
    AUCTION_BUYOUT,                     // 一口价买入拍卖的物品
    AUCTION_BID,                        // 竞标拍卖的物品
    AUCTION_QUERY_LIST,                 // 打开拍卖行界面，查询具体类别里的所有物品

// 85
    AUCTION_QUERY_BID_PRICE,            // 查询-我的拍卖
    AUCTION_QUERY_DETAIL,               // 查询-我的竞拍
	LEVELUP_CARD,                       // 升级卡牌
    GET_SELF_AUCTION_ITEM,              // 获取玩家自己拍卖的物品
    GET_SELF_BID_ITEM,                  // 获取玩家参与竞拍的物品

// 90
    ADD_FRIEND,                         // 添加好友
    DELETE_FRIEND,                      // 删除好友
    QUERY_ROLEINFO,                     // 查询玩家的基本信息
    CONVENE_TEAMMATE,                   // 队长召集队友到身边
    CONVENE_RESPONSE,                   // 回应队长的召集

// 95
    TEAM_CROSS_INS_INVITE_RE,           // 跨服副本
    DUEL_REQUEST,                       // 发起决斗请求
    DUEL_REQUEST_RE,                    // 决斗请求的回应
    TEAMMATE_DUEL_REQUEST_RE,           // 回复队友的请求
    SWITCH_TITLE,                       // 切换称号

// 100
    QUERY_PRIMARY_PROP_RF,              // 打开界面时查询最新的初阶属性强化信息
    BUY_PRIMARY_PROP_RF_ENERGY,         // 使用元宝购买初阶属性强化能量
    PRIMARY_PROP_REINFORCE,             // 一键强化初阶属性
	ACHIEVE_NOTIFY,                     // 成就消息，成就的相关操作
    OPEN_ENHANCE_SLOT,                  // 开启附魔位

// 105
    PROTECT_ENHANCE_SLOT,               // 保护附魔位
    UNPROTECT_ENHANCE_SLOT,             // 解除附魔位保护
    ACTIVATE_SPARK,                     // 激活星火
    PLAYER_QUIT_BATTLEGROUND,           // 玩家退出战场
    GET_BATTLEGROUND_DATA,              // 进入战场后，查询战场的信息

// 110
    EQUIP_CARD,                         // 卡牌镶嵌
    MAP_TEAM_JOIN_TEAM,                 // 地图组队申请加入队伍（或邀请加入队伍）
    MAP_TEAM_LEAVE_TEAM,                // 地图组队离开队伍
    MAP_TEAM_KICKOUT_MEMBER,            // 地图组队队长踢人
    MAP_TEAM_JOIN_TEAM_RES,             // 地图组队收到邀请或申请入队后的结果

// 115
    UPDATE_TASK_STORAGE,                // 付费刷新库任务
    ARENA_OD_QUERY_DATA,                // 离线数据竞技场，玩家的数据（打开界面时发）
    ARENA_OD_INVITE_ASSIST,             // 离线数据竞技场，邀请助战
    ARENA_OD_ASSIST_SELECT,             // 离线数据竞技场，选择助战队友
    ARENA_OD_COMBAT_START,              // 离线数据竞技场，开始战斗

// 120
    PLAYER_PUNCH_CARD,                  // 签到或补签
    GAIN_PUNCH_CARD_AWARD,              // 领取签到奖励
    QUERY_INSTANCE_RECORD,              // 查询副本的服务器记录
    QUERY_PUNCH_CARD_DATA,              // 查询签到数据
    JOIN_GEVENT,                        // 加入活动

// 125
    RE_PUNCH_CARD_HELP_RE,              // 补签帮忙的回应（同不同意帮忙补签）
    ITEM_DISASSEMBLE,                   // 物品分解
    MOUNT_MOUNT,                        // 骑乘坐骑
    MOUNT_EXCHANGE,                     // 骑乘兑换
    MOUNT_EQUIP_LEVELUP,                // 骑具升级

// 130
    GET_PARTICIPATION_AWARD,            // 领取活跃度奖励
    BOSS_CHALLENGE,                     // 挑战界面BOSS
    GET_BOSS_CHALLENGE_AWARD,           // 领取战胜某一只BOSS的奖励
    GET_CLEAR_CHALLENGE_AWARD,          // 领取挑战组通关奖励
    QUERY_WORLD_BOSS_RECORD,            // 查询世界BOSS的排行记录

// 135
	TASK_BG_TRANSFER,                   // 任务传送进战场
    ARENA_OD_BUY_TICKET,                // 离线数据竞技场，购买挑战次数


// max: 7900
	MAX_CMD_NUM = C2G_CMD_UPPER_LIMIT,


//
// debug cmd type
// 

	// 21700
// 0
	DEBUG_COMMON_CMD = C2G_DEBUG_CMD_LOWER_LIMIT, // 通用Debug命令，可以传递8个参数，使用时需要在enum CommonDebugCmd里定义type
	DEBUG_GAIN_ITEM,					// 玩家包裹获得物品
	DEBUG_CLEAN_ALL_ITEM,               // 玩家清除指定栏位的全部物品
	DEBUG_CHANGE_PLAYER_POS,            // 改变玩家在地图的位置
    DEBUG_CHANGE_FIRST_NAME,            // 修改玩家前名

// 5


	DEBUG_CMD_END   = C2G_DEBUG_CMD_UPPER_LIMIT,
};


/**
 * @brief 通用Debug命令
 *    （1）定义的debug命令可以有8个参数，参数类型可以是int64_t或double（也可以int64_t和double混用）
 *    （2）参数含义由gs自行判断
 *    （3）注释解析：X1-i（代表参数1是一个int64_t型），X2-d（代表参数2是一个double型）
 */
enum CommonDebugCmd
{
// 0
	CDC_INVALID = 0,
	CDC_CLR_SERVICE_COOLDOWN,     // 清除player身上某项服务的冷却，参数：X1-i（服务号）
	CDC_OPEN_TALENT_GROUP,        // 开启天赋组，参数：X1-i(天赋组模板ID)
	CDC_CLOSE_TALENT_GROUP,       // 关闭天赋组，参数：X1-i(天赋组模板ID)
	CDC_OPEN_ENHANCE_SLOT,        // 开启附魔位，参数：X1-i(开启的方式)

// 5
	CDC_PROTECT_ENHANCE_SLOT,     // 保护附魔位，参数：X1-i(0取消保护，1保护)，X2-i(附魔位索引)
	CDC_DO_ENHANCE,               // 附魔，参数：X1-i(附魔组ID)
	CDC_GAIN_MONEY,               // 玩家获取金钱，参数：X1-i(金钱数量)
	CDC_GAIN_EXP,                 // 玩家获取经验，参数：X1-i(经验值)
	CDC_GAIN_ITEM,                // 玩家获得物品，参数：X1-i(物品ID)，X2-i(物品个数)

// 10
	CDC_CLEAN_INVENTROY,          // 玩家清空身上的物品列表，参数：X1-i(栏位标识,0表示包裹,1表示装备栏)
	CDC_CHANGE_PLAYER_POS,        // 改变玩家位置，参数：X1-i(地图id)，X2-d(X坐标)，X3-d(Y坐标)
	CDC_ACTIVATE_SPARK,           // 激活星火，  参数：X1-i(星盘id)
	CDC_DEL_ITEM,                 // 删除指定物品，参数：X1-i(物品ID)，X2-i(物品个数)
	CDC_TASK_CMD,				  // 任务调试命令，参数：X1-i(任务ID)，X2-i(0删除，1发放)	

// 15
	CDC_TRANSFER_CLS,             // 玩家转职，参数：X1-i(职业ID,不是职业模板ID)
	CDC_MODIFY_BW_LIST,           // 修改黑白名单，参数：X1-i(0白名单，1黑名单)，X2-i(0删除，1增加)，X3-i(npc或矿的模板id)
	CDC_GAIN_CAT_VISION_POWER,    // 获得瞄类视觉能量，参数：X1-i(瞄类视觉能量)
	CDC_ADD_CASH,                 // 充值(添加元宝)，参数：X1-i(元宝个数)
	CDC_OPEN_STAR,                // 开启星盘，参数：X1-i(星盘ID)

// 20
	CDC_CHANGE_GENDER,			  // 玩家变性，参数：X1-i(0男，1女)
	CDC_MAP_ELEM_ON_OFF,          // 玩家所在地图的地图元素开关，参数：X1-i(地图元素id)，X2-i(0关，1开)
	CDC_SEND_SYS_MAIL,			  // 发送邮件，参数：X1-i(金币数)，X2-i(物品ID)
	CDC_PLAYER_QUIT_INS,          // 玩家推出副本，无参数
	CDC_UPDATE_TASK_STORAGE,	  // 刷新指定库任务，参数：X1-i(库任务id)

// 25
	CDC_GAIN_SCORE,				  // 获取学分，参数：X1-i(获得的学分值)
	CDC_ENTER_INSTANCE,           // 进入副本，参数：X1-i(副本模板id)
    CDC_GAIN_TITLE,               // 获得称号，参数：X1-i(称号模板ID)
    CDC_OPEN_REPUTATION,          // 开启声望，参数：X1-i(声望ID)
    CDC_MODIFY_REPUTATION,        // 修改声望数值，参数：X1-i(声望ID)，X2-i(修改值)

// 30
    CDC_MODIFY_UI,                // 修改客户端UI配置
    CDC_ENTER_BATTLEGROUND,       // 进入战场，参数：X1-i(战场模板id)
    CDC_CLOSE_CUR_BG,             // 关闭当前玩家所在的战场
    CDC_PRINT_ACTIVE_TASK,        // 打印出当前身上的活跃任务
    CDC_PRINT_HIDE_ITEM,          // 打印所有隐藏物品

// 35
    CDC_ACHIEVE_CMD,              // 成就调试命令，参数：X1-i(成就ID)，X2-i(0删除，1完成)
    CDC_SPOT_MAPELEM_TELEPORT,    // 瞬移定点地图元素，参数：X1-i(地图元素ID)，X2-d(目标点x坐标)，X3-d(目标点y坐标) X4-i(朝向0~7方向)
    CDC_SPOT_MONSTER_MOVE,        // 指定定点怪移动到某个点，参数：X1-i(地图元素ID)，X2-d(目标点x坐标)，X3-d(目标点y坐标)
    CDC_RESET_PRIMARY_PROP_RF,    // 重置（清空）初阶属性强化
    CDC_NPC_FRIEND,               // 设置NPC好友，参数：X1-i(NPC模板ID)，X2-i(0添加，1删除，2上线，3下线)

// 40
    CDC_PET_EXP,                  // 获取宠物经验，参数：X1-i(经验值)
    CDC_CLOSE_CUR_INS,            // 关闭当前副本
    CDC_RESET_PUNCH_CARD,         // 清空所有签到数据
    CDC_OPEN_MOUNT_EQUIP,         // 开启骑具，参数：X1-i(开启的位置，只有0-3有效)
    CDC_SET_GEVENT_NUM,           // 设置活动完成次数，参数：X1-i(活动组ID)，X2-i(次数)

// 45
    CDC_SET_PARTICIPATION,        // 设置活跃度，参数：X1-i(活跃度值，大于等于0)
    CDC_RESET_PARTI_AWARD,        // 清空活跃度奖励领取
    CDC_CLOSE_STAR,               // 关闭星辰，参数：X1-i(星辰ID，0清空所有)
    CDC_PRINT_FINISH_TASK,        // 打印出当前身上的已完成任务
    CDC_ADD_PPRF_CUR_ENERGY,      // 增满初阶属性强化的当前能量

// 50
    CDC_CLEAR_BOSS_CHALLENGE,     // 清除BOSS挑战奖励, 参数：X1-i(挑战组ID)
    CDC_PLAYER_SUICIDE,           // 玩家自杀
    CDC_RANDOM_TEST_A,            // 测试随机函数，参数：X1-d(概率值0到1之间的小数), X2-i(随机次数)
    CDC_RANDOM_TEST_B,            // 测试随机函数，参数：X1-i(概率值0到10000之间的整数), X2-i(随机次数)


	CDC_MAX
};


///
/// proto_packet define
///
//---- cmd: 0
class PlayerLogout : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerLogout, LOGOUT);
public:
	uint8_t logout_type;

	PACKET_DEFINE(logout_type);
};

class StartMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(StartMove, START_MOVE);
public:
	A2DVECTOR_PACK dest;
	uint16_t    speed;  // 百分数，单位厘米每秒
	uint8_t     mode;

	PACKET_DEFINE(dest, speed, mode);
};

class MoveContinue : public ProtoPacket
{
	DECLARE_PROTOPACKET(MoveContinue, MOVE_CONTINUE);
public: 
	A2DVECTOR_PACK cur_pos;
	A2DVECTOR_PACK dest;
	uint16_t    use_time;
	uint16_t    speed;  // 百分数，单位厘米每秒
	uint8_t     mode;
	uint16_t    cmd_seq;

	PACKET_DEFINE(cur_pos, dest, use_time, speed, mode, cmd_seq);
};

class StopMove : public ProtoPacket
{
	DECLARE_PROTOPACKET(StopMove, STOP_MOVE);
public:
	A2DVECTOR_PACK pos;
	uint16_t    speed;  // 百分数，单位厘米每秒
	uint8_t     mode;
	uint8_t     dir;
	uint16_t    use_time;
	uint16_t    cmd_seq;

	PACKET_DEFINE(pos, speed, mode, dir, use_time, cmd_seq);
};

class MoveChange : public ProtoPacket, public shared::copyable
{
	DECLARE_PROTOPACKET(MoveChange, MOVE_CHANGE);
public:
	A2DVECTOR_PACK dest;
	uint8_t dir;

	PACKET_DEFINE(dest, dir);
};


// ---- cmd: 5
class GetAllData : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetAllData, GET_ALL_DATA);
public:
	int8_t detail_inv;
	int8_t detail_equip;
	int8_t detail_task;

	PACKET_DEFINE(detail_inv, detail_equip, detail_task);
};

class GetItemData : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetItemData, GET_ITEM_DATA);
public:
	int8_t where;
	int16_t index;
	PACKET_DEFINE(where, index);
};

class GetItemListBrief : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetItemListBrief, GET_ITEM_LIST_BRIEF);
public:
	int8_t where;
	PACKET_DEFINE(where);
};

class GetItemListDetail : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetItemListDetail, GET_ITEM_LIST_DETAIL);
public:
	int8_t where;
	PACKET_DEFINE(where);
};

class DropInventoryItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(DropInventoryItem, DROP_INVENTORY_ITEM);
public:
	int16_t index;
	int32_t count;
	PACKET_DEFINE(index, count);
};

// ---- cmd: 10
class MoveInventoryItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(MoveInventoryItem, MOVE_INVENTORY_ITEM);
public:
	int16_t src_idx;
	int16_t dst_idx;
	int32_t count;
	PACKET_DEFINE(src_idx, dst_idx, count);
};

class SellInventoryItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(SellInventoryItem, SELL_INVENTORY_ITEM);
public:
    int8_t where;
	int16_t index;
	int32_t count;
	PACKET_DEFINE(where, index, count);
};

class ExchangeInventoryItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(ExchangeInventoryItem, EXCHANGE_INVENTORY_ITEM);
public:
	int16_t index1;
	int16_t index2;
	PACKET_DEFINE(index1, index2);
};

class TidyInventory : public ProtoPacket
{
	DECLARE_PROTOPACKET(TidyInventory, TIDY_INVENTORY);
public:
	int8_t where;
	PACKET_DEFINE(where);
};

class EquipItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(EquipItem, EQUIP_ITEM);
public:
	int16_t idx_inv;
	int16_t idx_equip;
	PACKET_DEFINE(idx_inv, idx_equip);
};


///
/// ---- cmd: 15
///
class UndoEquip : public ProtoPacket
{
	DECLARE_PROTOPACKET(UndoEquip, UNDO_EQUIP);
public:
	int16_t idx_equip;
	PACKET_DEFINE(idx_equip);
};

class UseItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(UseItem, USE_ITEM);
public:
	int8_t where;
	int16_t index;
	PACKET_DEFINE(where, index);
};

class RefineEquip : public ProtoPacket
{
	DECLARE_PROTOPACKET(RefineEquip, REFINE_EQUIP);
public:
	int8_t  where;
	int16_t index;
	int8_t  level;
	PACKET_DEFINE(where, index, level);
};

class CombatPlayerJoin : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerJoin, COMBAT_PLAYER_JOIN);
public:
	int32_t combat_id;
	RoleID  roleid_to_join; // 表明要加入谁的战斗，不能是自己的roleid
	PACKET_DEFINE(combat_id, roleid_to_join);
};

class CombatPlayerTrigger : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPlayerTrigger, COMBAT_PLAYER_TRIGGER);
public:
	int32_t object_id;//怪物大世界ID,非怪物模板ID
	PACKET_DEFINE(object_id);
};


///
/// ---- cmd: 20
///
class CombatSelectSkill : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatSelectSkill, COMBAT_SELECT_SKILL);
public:
	int32_t skill_id;
    bool player_select;
	PACKET_DEFINE(skill_id, player_select);
};

class CombatPetAttack : public ProtoPacket
{
	DECLARE_PROTOPACKET(CombatPetAttack, COMBAT_PET_ATTACK);
public:
	int16_t combat_pet_inv_pos;
	PACKET_DEFINE(combat_pet_inv_pos);
};

class FullPetPower : public ProtoPacket
{
	DECLARE_PROTOPACKET(FullPetPower, FULL_PET_POWER);
public:
	PACKET_DEFINE();
};

class ServiceHello : public ProtoPacket
{
	DECLARE_PROTOPACKET(ServiceHello, SERVICE_HELLO);
public:
	ObjectID obj_id;	
	PACKET_DEFINE(obj_id);
};

class ServiceServe : public ProtoPacket
{
	DECLARE_PROTOPACKET(ServiceServe, SERVICE_SERVE);
public:
	uint8_t     is_ui_srv;   // 0 表示是npc服务，非0表示是UI服务
	int32_t     service_type;
	std::string content;
	PACKET_DEFINE(is_ui_srv, service_type, content);
};


///
/// ---- cmd: 25
///
class LevelUpPetBloodline : public ProtoPacket
{
	DECLARE_PROTOPACKET(LevelUpPetBloodline, LEVELUP_PET_BLOODLINE);
public:
	int16_t pet_item_inv_idx;    // 宠物对应的物品在宠物包裹中的位置
	int32_t inc_prob_item_id;    // 提升升级概率的道具ID
	int32_t inc_prob_item_count; // 提升升级概率的道具个数
	PACKET_DEFINE(pet_item_inv_idx, inc_prob_item_id, inc_prob_item_count);
};

class LevelUpPetPowerCap : public ProtoPacket
{
	DECLARE_PROTOPACKET(LevelUpPetPowerCap, LEVELUP_PET_POWER_CAP);
public:
	int16_t lvlup_times;
	PACKET_DEFINE(lvlup_times);
};

class SetCombatPet : public ProtoPacket
{
	DECLARE_PROTOPACKET(SetCombatPet, SET_COMBAT_PET);
public:
	enum
	{
		OP_PUSH, // 往战宠栏添加宠物
		OP_POP,  // 从战宠栏拖出宠物
	};

	int8_t  op_type;            // 标记操作类型(添加or删除战宠)
	int8_t  pet_combat_inv_pos; // 宠物在战斗栏的位置
	int16_t pet_item_inv_idx;   // 宠物在宠物栏的位置
	PACKET_DEFINE(op_type, pet_combat_inv_pos, pet_item_inv_idx);
};

class LearnSkill : public ProtoPacket
{
	DECLARE_PROTOPACKET(LearnSkill, LEARN_SKILL);
public:
	int32_t skill_tree_id;
	int8_t  skill_idx;
	PACKET_DEFINE(skill_tree_id, skill_idx);
};

class LevelUpTalent : public ProtoPacket
{
	DECLARE_PROTOPACKET(LevelUpTalent, LEVELUP_TALENT);
public:
	int32_t talent_group_id;
	int32_t talent_id;
	PACKET_DEFINE(talent_group_id, talent_id);
};


//
// ---- cmd: 30
// 
class TaskNotify : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskNotify, TASK_NOTIFY);
public:
	uint16_t type;
	std::string databuf;
	PACKET_DEFINE(type, databuf);
};

class TransferPrepareFinish : public ProtoPacket
{
	DECLARE_PROTOPACKET(TransferPrepareFinish, TRANSFER_PREPARE_FINISH);
public:
	PACKET_DEFINE();
};

class SelectResurrectPos : public ProtoPacket
{
	DECLARE_PROTOPACKET(SelectResurrectPos, SELECT_RESURRECT_POS);
public:
	enum
	{
		ORIGINAL_POS  = 0, //原地复活
		RESURRECT_POS = 1, //复活点复活
	};

	int8_t pos;
	PACKET_DEFINE(pos);
};

class JoinTeam : public ProtoPacket
{
	DECLARE_PROTOPACKET(JoinTeam, JOIN_TEAM);
public:
	RoleID other_roleid;

	PACKET_DEFINE(other_roleid);
};

class JoinTeamRes : public ProtoPacket
{
	DECLARE_PROTOPACKET(JoinTeamRes, JOIN_TEAM_RES);
public:
	bool invite;
	bool accept;
	RoleID requester;
	PACKET_DEFINE(invite, accept, requester);
};


//
// ---- cmd: 35
// 
class ChangeBuddyTeamPos : public ProtoPacket
{
	DECLARE_PROTOPACKET(ChangeBuddyTeamPos, CHANGE_BUDDY_TEAM_POS);
public:
	int8_t src_index; // 从0开始
	int8_t des_index;
	PACKET_DEFINE(src_index, des_index);
};

class GatherMaterial : public ProtoPacket
{
	DECLARE_PROTOPACKET(GatherMaterial, GATHER_MATERIAL);
public:
	ObjectID obj_id;
	PACKET_DEFINE(obj_id);
};

class GatherMiniGameResult : public ProtoPacket
{
	DECLARE_PROTOPACKET(GatherMiniGameResult, GATHER_MINIGAME_RESULT);
public:
	ObjectID obj_id;        // 采集对象
	int8_t   is_success;    // 0表示失败，其他表示成功
	int32_t  gather_seq_no; // 采集序号
	PACKET_DEFINE(obj_id, is_success, gather_seq_no);
};

class OpenCatVision : public ProtoPacket
{
	DECLARE_PROTOPACKET(OpenCatVision, OPEN_CAT_VISION);
public:
	PACKET_DEFINE();
};

class CloseCatVision : public ProtoPacket
{
	DECLARE_PROTOPACKET(CloseCatVision, CLOSE_CAT_VISION);
public:
	PACKET_DEFINE();
};


///
/// ---- cmd: 40
///
class DramaTriggerCombat : public ProtoPacket
{
	DECLARE_PROTOPACKET(DramaTriggerCombat, DRAMA_TRIGGER_COMBAT);
public:
	int32_t task_id;      // 剧情触发战斗,任务刷出来的怪
	int32_t monster_tid;  // monster templ_id
	int32_t monster_group_id; // 怪物组id
	PACKET_DEFINE(task_id, monster_tid, monster_group_id);
};

class DramaServiceServe : public ProtoPacket
{
	DECLARE_PROTOPACKET(DramaServiceServe, DRAMA_SERVICE_SERVE);
public:
	int32_t task_id;      // 剧情npc服务,任务刷出来的npc
	int32_t npc_tid;      // service_npc templ_id
	int32_t service_type; // 服务类型
	std::string content;  // 服务参数
	PACKET_DEFINE(task_id, npc_tid, service_type, content);
};

class DramaGatherMaterial : public ProtoPacket
{
	DECLARE_PROTOPACKET(DramaGatherMaterial, DRAMA_GATHER_MATERIAL);
public:
	int32_t  task_id;       // 剧情采集，任务刷出来的矿
	int32_t  mine_tid;      // 矿模板id 
	PACKET_DEFINE(task_id, mine_tid);
};

class DramaGatherMiniGameResult : public ProtoPacket
{
	DECLARE_PROTOPACKET(DramaGatherMiniGameResult, DRAMA_GATHER_MINIGAME_RESULT);
public:
	int32_t  task_id;       // 剧情采集，任务刷出来的檫图矿
	int32_t  mine_tid;      // 矿模板id
	int8_t   is_success;    // 0表示失败，其他表示成功
	int32_t  gather_seq_no; // 采集序号
	PACKET_DEFINE(task_id, mine_tid, is_success, gather_seq_no);
};

class SwitchSkill : public ProtoPacket
{
	DECLARE_PROTOPACKET(SwitchSkill, SWITCH_SKILL);
public:
	int8_t skill_idx;
	PACKET_DEFINE(skill_idx);
};


///
/// ---- cmd:45
///
class GetElsePlayerExtProp : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetElsePlayerExtProp, GET_ELSE_PLAYER_EXTPROP);
public:
	int64_t else_player_roleid;
	PACKET_DEFINE(else_player_roleid);
};

class ChatMsg : public ProtoPacket
{
	DECLARE_PROTOPACKET(ChatMsg, CHAT_MSG);
public:
	enum
	{
		CHANNEL_REGION,
		CHANNEL_WORLD,
		CHANNEL_TEAM,
        CHANNEL_PRIVATE,
	};

	int8_t channel;
	int64_t receiver;
	std::string msg;
	PACKET_DEFINE(channel, receiver, msg);
};

class TaskRegionTransfer : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskRegionTransfer, TASK_REGION_TRANSFER);
public:
	int32_t task_id;
	int32_t map_id;
	float   pos_x;
	float   pos_y;
	PACKET_DEFINE(task_id, map_id, pos_x, pos_y);
};

class GetElsePlayerEquipCRC: public ProtoPacket
{
	DECLARE_PROTOPACKET(GetElsePlayerEquipCRC, GET_ELSE_PLAYER_EQUIPCRC);
public:
	int64_t else_player_roleid;
	PACKET_DEFINE(else_player_roleid);
};

class GetElsePlayerEquipment : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetElsePlayerEquipment, GET_ELSE_PLAYER_EQUIPMENT);
public:
	int64_t else_player_roleid;
	PACKET_DEFINE(else_player_roleid);
};


///
/// ---- cmd:50
///
class TaskChangeName : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskChangeName, TASK_CHANGE_NAME);
public:
	int32_t task_id;
	int8_t  name_type; // 定义与task里一样
	std::string name;
	PACKET_DEFINE(task_id, name_type, name);
};

class GetStaticRoleInfo : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetStaticRoleInfo, GET_STATIC_ROLE_INFO);
public:
	int64_t roleid;
	PACKET_DEFINE(roleid);
};

class GetInstanceData : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetInstanceData, GET_INSTANCE_DATA);
public:
	PACKET_DEFINE();
};

class UITaskRequest : public ProtoPacket
{
	DECLARE_PROTOPACKET(UITaskRequest, UI_TASK_REQUEST);
public:
	int32_t task_id;
	PACKET_DEFINE(task_id);
};

class QueryNpcZoneInfo : public ProtoPacket
{
	DECLARE_PROTOPACKET(QueryNpcZoneInfo, QUERY_NPC_ZONE_INFO);
public:
	int32_t elem_id;
	PACKET_DEFINE(elem_id);
};


///
/// ---- cmd: 55
///
class OpenMall : public ProtoPacket
{
	DECLARE_PROTOPACKET(OpenMall, OPEN_MALL);
public:
	PACKET_DEFINE();
};

class QueryMallGoodsDetail : public ProtoPacket
{
	DECLARE_PROTOPACKET(QueryMallGoodsDetail, QUERY_MALL_GOODS_DETAIL);
public:
	int32_t goods_id;
	PACKET_DEFINE(goods_id);
};

class MallShopping : public ProtoPacket
{
	DECLARE_PROTOPACKET(MallShopping, MALL_SHOPPING);
public:
	int32_t goods_id;
	int32_t goods_count;
	PACKET_DEFINE(goods_id, goods_count);
};

class ServiceGetContent : public ProtoPacket
{
	DECLARE_PROTOPACKET(ServiceGetContent, SERVICE_GET_CONTENT);
public:
	int32_t service_type;
	PACKET_DEFINE(service_type);
};

class QueryServerTime : public ProtoPacket
{
	DECLARE_PROTOPACKET(QueryServerTime, QUERY_SERVER_TIME);
public:
	PACKET_DEFINE();
};


///
/// ---- cmd: 60
///
class GetMailList : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetMailList, GET_MAIL_LIST);
public:
	int64_t max_mailid;
	PACKET_DEFINE(max_mailid);
};

class GetMailAttach : public ProtoPacket
{
	DECLARE_PROTOPACKET(GetMailAttach, GET_MAIL_ATTACH);
public:
	int64_t mail_id;
	PACKET_DEFINE(mail_id);
};

class DeleteMail : public ProtoPacket
{
	DECLARE_PROTOPACKET(DeleteMail, DELETE_MAIL);
public:
	int64_t mail_id;
	PACKET_DEFINE(mail_id);
};

class TakeoffMailAttach : public ProtoPacket
{
	DECLARE_PROTOPACKET(TakeoffMailAttach, TAKEOFF_MAIL_ATTACH);
public:
	int64_t mail_id;
	PACKET_DEFINE(mail_id);
};

class SendMail : public ProtoPacket
{
	DECLARE_PROTOPACKET(SendMail, SEND_MAIL);
public:
	struct ItemInfo
	{
		int32_t type;
		int16_t index;
		int32_t count;
		NESTED_DEFINE(type, index, count)
	};
	typedef std::vector<ItemInfo> ItemInfoVec;

	int64_t sender;
	int64_t receiver;
	int32_t attach_score;
	std::string name;
	std::string title;
	std::string content;
	ItemInfoVec attach_item;
	PACKET_DEFINE(sender, receiver, attach_score, name, title, content, attach_item);
};


///
/// ---- cmd: 65
///
class TeamLocalInsInvite : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeamLocalInsInvite, TEAM_LOCAL_INS_INVITE);
public:
	int32_t ins_group_id; // 副本组id
	PACKET_DEFINE(ins_group_id);
};

class LaunchTeamLocalIns : public ProtoPacket
{
	DECLARE_PROTOPACKET(LaunchTeamLocalIns, LAUNCH_TEAM_LOCAL_INS);
public:
	int32_t ins_group_id; // 本服副本组id
	int32_t ins_templ_id; // 本服副本模板id
	PACKET_DEFINE(ins_group_id, ins_templ_id);
};

// 本服组队副本界面点取消
class QuitTeamLocalInsRoom : public ProtoPacket
{
	DECLARE_PROTOPACKET(QuitTeamLocalInsRoom, QUIT_TEAM_LOCAL_INS_ROOM);
public:
	int32_t ins_group_id; // 本服副本组模板
	PACKET_DEFINE(ins_group_id);
};

class TeamCrossInsRequest : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeamCrossInsRequest, TEAM_CROSS_INS_REQUEST);
public:
	int32_t ins_group_id; // 跨服副本组id
	int32_t ins_templ_id; // 跨服副本模板id
    int8_t  solo_request; // 非0表示是单人请求，0表示是组队请求，必须是队长发起
	PACKET_DEFINE(ins_group_id, ins_templ_id, solo_request);
};

// 取消跨服匹配,界面点取消
class QuitTeamCrossInsRoom : public ProtoPacket
{
	DECLARE_PROTOPACKET(QuitTeamCrossInsRoom, QUIT_TEAM_CROSS_INS_ROOM);
public:
	int32_t ins_group_id; // 跨服副本组模板
	PACKET_DEFINE(ins_group_id);
};


///
/// ---- cmd: 70
///
class LaunchSoloLocalIns : public ProtoPacket
{
	DECLARE_PROTOPACKET(LaunchSoloLocalIns, LAUNCH_SOLO_LOCAL_INS);
public:
	int32_t ins_group_id; // 单人本服副本组
	int32_t ins_templ_id; // 单人本服副本模板id
	PACKET_DEFINE(ins_group_id, ins_templ_id);
};

class TeamLocalInsInvite_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeamLocalInsInvite_Re, TEAM_LOCAL_INS_INVITE_RE);
public:
	bool    agreement;
	int32_t ins_group_id;
	std::vector<int32_t> ins_tid_vec;
	PACKET_DEFINE(agreement, ins_group_id, ins_tid_vec);
};

class TeamCrossInsReady : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeamCrossInsReady, TEAM_CROSS_INS_READY);
public:
	// 房间内满四个人开始倒计时，时间到发该协议
	int32_t ins_group_id; // 副本组
	int32_t ins_templ_id; // 副本模板
	PACKET_DEFINE(ins_group_id, ins_templ_id);
};

class PlayerQuitInstance : public ProtoPacket
{
	DECLARE_PROTOPACKET(PlayerQuitInstance, PLAYER_QUIT_INSTANCE);
public:
	// 玩家主动退出副本
	PACKET_DEFINE();
};

class MapTeamChangePos : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamChangePos, MAP_TEAM_CHANGE_POS);
public:
	int8_t src_index; // 从0开始
	int8_t des_index;
	PACKET_DEFINE(src_index, des_index);
};


///
/// ---- cmd: 75
///
class MapTeamChangeLeader : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamChangeLeader, MAP_TEAM_CHANGE_LEADER);
public:
	int64_t new_leader;
	PACKET_DEFINE(new_leader);
};

class MapTeamQueryMember : public ProtoPacket
{
	DECLARE_PROTOPACKET(MapTeamQueryMember, MAP_TEAM_QUERY_MEMBER);
public:
	PACKET_DEFINE();
};

class TaskTransferGenderCls : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskTransferGenderCls, TASK_TRANSFER_GENDER_CLS);
public:
	int32_t taskid; // 任务
	int32_t gender; // 性别
	int32_t cls;	// 职业
	PACKET_DEFINE(taskid, gender, cls);
};

class TaskInsTransfer : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskInsTransfer, TASK_INS_TRANSFER);
public:
	int32_t taskid; 
	int32_t ins_templ_id;
	PACKET_DEFINE(taskid, ins_templ_id);
};

class MoveCombatPet : public ProtoPacket
{
	DECLARE_PROTOPACKET(MoveCombatPet, MOVE_COMBAT_PET);
public:
	//把源位置上的宠物移动到目标位置
	int8_t src_combat_inv_pos;  // 源位置
	int8_t dest_combat_inv_pos; // 目标移动位置
	PACKET_DEFINE(src_combat_inv_pos, dest_combat_inv_pos);
};


///
/// ---- cmd: 80
///
enum AuctionCurrencyType
{
    ACT_SCORE, // 学分
    ACT_CASH,  // 元宝
};

class AuctionItem : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionItem, AUCTION_ITEM);
public:
    int8_t  where;
    int16_t index;
    int32_t item_id;
    int32_t item_count;
    int8_t  currency_type; // 0表示学分，1表示元宝. 对应上面的AuctionCurrencyType
    int32_t buyout_price;
    int32_t bid_price;
    PACKET_DEFINE(where, index, item_id, item_count, currency_type, buyout_price, bid_price);
};

class AuctionCancel : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionCancel, AUCTION_CANCEL);
public:
    int64_t auction_id;
    PACKET_DEFINE(auction_id);
};

class AuctionBuyout : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionBuyout, AUCTION_BUYOUT);
public:
    int64_t auction_id;
    int8_t  currency_type; // 0表示学分，1表示元宝. 对应上面的AuctionCurrencyType
    int32_t price;
    PACKET_DEFINE(auction_id, currency_type, price);
};

class AuctionBid : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionBid, AUCTION_BID);
public:
    int64_t auction_id;
    int8_t  currency_type; // 0表示学分，1表示元宝. 对应上面的AuctionCurrencyType
    int32_t price;
    PACKET_DEFINE(auction_id, currency_type, price);
};

class AuctionQueryList : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionQueryList, AUCTION_QUERY_LIST);
public:
    int32_t category;
    int8_t currency_type;
    PACKET_DEFINE(category, currency_type);
};


///
/// ---- cmd: 85
///
class AuctionQueryBidPrice : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionQueryBidPrice, AUCTION_QUERY_BID_PRICE);
public:
    // 用于查询"我的拍卖"或"我的竞拍"里最新的竞价
    std::vector<int64_t> auctionid_list;
    PACKET_DEFINE(auctionid_list);
};

class AuctionQueryDetail : public ProtoPacket
{
    DECLARE_PROTOPACKET(AuctionQueryDetail, AUCTION_QUERY_DETAIL);
public:
    // 用于查询"我的拍卖"或"我的竞拍"里物品的所有信息，比如刚登陆时
    std::vector<int64_t> auctionid_list;
    PACKET_DEFINE(auctionid_list);
};

class LevelUpCard : public ProtoPacket
{
    DECLARE_PROTOPACKET(LevelUpCard, LEVELUP_CARD);
public:
	enum
	{
		LVLUP_TYPE_CARD,       // 消耗卡牌升级，可能还消耗金币
		LVLUP_TYPE_CASH,      // 人民币直接升级, 不消耗卡牌
	};

	int8_t  lvlup_type;                     // 升级类型(卡牌或人民币)
	int16_t idx_main_card;                  // 主卡牌位置
    std::vector<int16_t> idx_vice_cards;    // 消耗的副卡牌位置

    PACKET_DEFINE(lvlup_type, idx_main_card, idx_vice_cards);
};

class GetSelfAuctionItem : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetSelfAuctionItem, GET_SELF_AUCTION_ITEM);
public:
    PACKET_DEFINE();
};

class GetSelfBidItem : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetSelfBidItem, GET_SELF_BID_ITEM);
public:
    PACKET_DEFINE();
};


///
/// ---- cmd: 90
///
class AddFriend : public ProtoPacket
{
    DECLARE_PROTOPACKET(AddFriend, ADD_FRIEND);
public:
    int64_t roleid;
    int8_t flag;
    std::string name;
    PACKET_DEFINE(roleid, flag, name);
};

class DeleteFriend : public ProtoPacket
{
    DECLARE_PROTOPACKET(DeleteFriend, DELETE_FRIEND);
public:
    int64_t roleid;
    PACKET_DEFINE(roleid);
};

class QueryRoleInfo : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryRoleInfo, QUERY_ROLEINFO);
public:
    std::vector<int64_t> query_list;
    PACKET_DEFINE(query_list);
};

class ConveneTeammate : public ProtoPacket
{
    DECLARE_PROTOPACKET(ConveneTeammate, CONVENE_TEAMMATE);
public:
    // 队长发起召集队友，将队友传送到身边
    PACKET_DEFINE();
};

class ConveneResponse : public ProtoPacket
{
    DECLARE_PROTOPACKET(ConveneResponse, CONVENE_RESPONSE);
public:
    int8_t agree; // 0表示不同意传送，1表示同意传送
    PACKET_DEFINE(agree);
};


///
/// ---- cmd: 95
///
class TeamCrossInsInvite_Re : public ProtoPacket
{
	DECLARE_PROTOPACKET(TeamCrossInsInvite_Re, TEAM_CROSS_INS_INVITE_RE);
public:
	bool    agreement;
	int32_t ins_group_id;
	int32_t ins_templ_id;
	PACKET_DEFINE(agreement, ins_group_id, ins_templ_id);
};

class DuelRequest : public ProtoPacket
{
    DECLARE_PROTOPACKET(DuelRequest, DUEL_REQUEST);
public:
    // 决斗对象的id
    int64_t duel_roleid;
    PACKET_DEFINE(duel_roleid);
};

class DuelRequest_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(DuelRequest_Re, DUEL_REQUEST_RE);
public:
    // 接受方的回应
    int64_t requester_roleid;
    int8_t  agreement;
    PACKET_DEFINE(requester_roleid, agreement);
};

class TeammateDuelRequest_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(TeammateDuelRequest_Re, TEAMMATE_DUEL_REQUEST_RE);
public:
    // 队长发给服务器的
    int8_t  agreement;   // 0表示不同意，非0表示同意
    int64_t duel_roleid;
    int64_t teammate_roleid;
    PACKET_DEFINE(agreement, duel_roleid, teammate_roleid);
};

class SwitchTitle : public ProtoPacket
{
    DECLARE_PROTOPACKET(SwitchTitle, SWITCH_TITLE);
public:
    int32_t old_title;
    int32_t new_title;
    PACKET_DEFINE(old_title, new_title);
};

class AchieveNotify : public ProtoPacket
{
	DECLARE_PROTOPACKET(AchieveNotify, ACHIEVE_NOTIFY);
public:
	uint16_t type;
	std::string databuf;
	PACKET_DEFINE(type, databuf);
};


///
/// ---- cmd: 100
///
class QueryPrimaryPropRF : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryPrimaryPropRF, QUERY_PRIMARY_PROP_RF);
public:
    // 打开界面时发送
    PACKET_DEFINE();
};

class BuyPrimaryPropRFEnergy : public ProtoPacket
{
    DECLARE_PROTOPACKET(BuyPrimaryPropRFEnergy, BUY_PRIMARY_PROP_RF_ENERGY);
public:
    // 使用元宝购买能量时发送
    int8_t index; // 选择是模板里第几档
    PACKET_DEFINE(index);
};

class PrimaryPropReinforce : public ProtoPacket
{
    DECLARE_PROTOPACKET(PrimaryPropReinforce, PRIMARY_PROP_REINFORCE);
public:
    // 一键强化
    PACKET_DEFINE();
};

class OpenEnhanceSlot : public ProtoPacket
{
    DECLARE_PROTOPACKET(OpenEnhanceSlot, OPEN_ENHANCE_SLOT);
public:
    PACKET_DEFINE();
};


///
/// ---- cmd: 105
///
class ProtectEnhanceSlot : public ProtoPacket
{
    DECLARE_PROTOPACKET(ProtectEnhanceSlot, PROTECT_ENHANCE_SLOT);
public:
    int8_t slot_index;
    PACKET_DEFINE(slot_index);
};

class UnProtectEnhanceSlot : public ProtoPacket
{
    DECLARE_PROTOPACKET(UnProtectEnhanceSlot, UNPROTECT_ENHANCE_SLOT);
public:
    int8_t slot_index;
    PACKET_DEFINE(slot_index);
};

class ActivateSpark : public ProtoPacket
{
    DECLARE_PROTOPACKET(ActivateSpark, ACTIVATE_SPARK);
public:
    int32_t star_id;
    PACKET_DEFINE(star_id);
};

class PlayerQuitBattleGround : public ProtoPacket
{
    DECLARE_PROTOPACKET(PlayerQuitBattleGround, PLAYER_QUIT_BATTLEGROUND);
public:
	// 玩家主动退出战场
    PACKET_DEFINE();
};

class GetBattleGroundData : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetBattleGroundData, GET_BATTLEGROUND_DATA);
public:
    // 进入战场后，客户端主动获取战场信息
    PACKET_DEFINE();
};


///
/// ---- cmd: 110
///
class EquipCard : public ProtoPacket
{
    DECLARE_PROTOPACKET(EquipCard, EQUIP_CARD);
public:
    int16_t card_index;     // 要镶嵌的卡牌在包裹中的位置
    int32_t star_id;        // 要嵌入的星辰ID
    PACKET_DEFINE(card_index, star_id);
};

class MapTeamJoinTeam : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapTeamJoinTeam, MAP_TEAM_JOIN_TEAM);
public:
    int64_t other_roleid;
    PACKET_DEFINE(other_roleid);
};

class MapTeamLeaveTeam : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapTeamLeaveTeam, MAP_TEAM_LEAVE_TEAM);
public:
    PACKET_DEFINE();
};

class MapTeamKickoutMember : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapTeamKickoutMember, MAP_TEAM_KICKOUT_MEMBER);
public:
    int64_t kicked_roleid;
    PACKET_DEFINE(kicked_roleid);
};

class MapTeamJoinTeamRes : public ProtoPacket
{
    DECLARE_PROTOPACKET(MapTeamJoinTeamRes, MAP_TEAM_JOIN_TEAM_RES);
public:
    bool    invite;
    int8_t  accept;
    int64_t requester;
    PACKET_DEFINE(invite, accept, requester);
};


///
/// ---- cmd: 115
///
class UpdateTaskStorage : public ProtoPacket
{
    DECLARE_PROTOPACKET(UpdateTaskStorage, UPDATE_TASK_STORAGE);
public:
    int32_t storage_id;
    PACKET_DEFINE(storage_id);
};

class ArenaODQueryData : public ProtoPacket
{
    // OD - offline data
    DECLARE_PROTOPACKET(ArenaODQueryData, ARENA_OD_QUERY_DATA);
public:
    PACKET_DEFINE();
};

class ArenaODInviteAssist : public ProtoPacket
{
    // OD - offline data
    DECLARE_PROTOPACKET(ArenaODInviteAssist, ARENA_OD_INVITE_ASSIST);
public:
    // 离线数据竞技场，邀请助战
    PACKET_DEFINE();
};

class ArenaODAssistSelect : public ProtoPacket
{
    // OD - offline data
    DECLARE_PROTOPACKET(ArenaODAssistSelect, ARENA_OD_ASSIST_SELECT);
public:
    std::vector<int64_t> assists; // 选择的助战队友
    PACKET_DEFINE(assists);
};

class ArenaODCombatStart : public ProtoPacket
{
    // OD - offline data
    DECLARE_PROTOPACKET(ArenaODCombatStart, ARENA_OD_COMBAT_START);
public:
    PACKET_DEFINE();
};


///
/// ---- cmd: 120
///
class PlayerPunchCard : public ProtoPacket
{
    DECLARE_PROTOPACKET(PlayerPunchCard, PLAYER_PUNCH_CARD);
public:
    int32_t day_of_month;  // 签到或补签当月的哪天[1,31]
    RoleID  friend_roleid; // 邀请好友帮忙补签，当天签到不需要填该值
    PACKET_DEFINE(day_of_month, friend_roleid);
};

// 签到的奖励类型
enum PunchCardAwardType
{
    PCAT_MONTHLY = 0, // 每月奖励
    PCAT_HISTORY,     // 历史累计奖励
};

class GainPunchCardAward : public ProtoPacket
{
    DECLARE_PROTOPACKET(GainPunchCardAward, GAIN_PUNCH_CARD_AWARD);
public:
    int8_t  award_type;     // 对应枚举PunchCardAwardType
    int32_t cumulative_num; // 领取那个累计次数档次的奖品
    PACKET_DEFINE(award_type, cumulative_num);
};

class QueryInstanceRecord : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryInstanceRecord, QUERY_INSTANCE_RECORD);
public:
    int32_t ins_group_id; // 副本组id
    int32_t ins_tid;      // 对应的副本模板id
    PACKET_DEFINE(ins_group_id, ins_tid);
};

class QueryPunchCardData : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryPunchCardData, QUERY_PUNCH_CARD_DATA);
public:
    PACKET_DEFINE();
};

class JoinGevent : public ProtoPacket
{
    DECLARE_PROTOPACKET(JoinGevent, JOIN_GEVENT);
public:
    int32_t gevent_gid;     // 活动组id
    PACKET_DEFINE(gevent_gid);
};


///
/// ---- cmd: 125
///
class RePunchCardHelp_Re : public ProtoPacket
{
    DECLARE_PROTOPACKET(RePunchCardHelp_Re, RE_PUNCH_CARD_HELP_RE);
public:
    int8_t  agreement;  // 0表示不同意，1表示同意
    RoleID  requester;
    int32_t day_of_month;
    PACKET_DEFINE(agreement, requester, day_of_month);
};

class ItemDisassemble : public ProtoPacket
{
    DECLARE_PROTOPACKET(ItemDisassemble, ITEM_DISASSEMBLE);
public:
    // 物品分解，只能对普通包裹里的装备或物品进行
	int16_t index;
    int32_t item_id;
    PACKET_DEFINE(index, item_id);
};

class MountMount : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountMount, MOUNT_MOUNT);
public:
	int16_t item_index;
    int32_t item_id;
    PACKET_DEFINE(item_index, item_id);
};

class MountExchange : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountExchange, MOUNT_EXCHANGE);
public:
    int8_t config_index;
    PACKET_DEFINE(config_index);
};

class MountEquipLevelUp : public ProtoPacket
{
    DECLARE_PROTOPACKET(MountEquipLevelUp, MOUNT_EQUIP_LEVELUP);
public:
    int8_t equip_index;
    PACKET_DEFINE(equip_index);
};

class GetParticipationAward : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetParticipationAward, GET_PARTICIPATION_AWARD);
public:
    int8_t award_index;
    PACKET_DEFINE(award_index);
};

class BossChallenge : public ProtoPacket
{
    DECLARE_PROTOPACKET(BossChallenge, BOSS_CHALLENGE);
public:
    int32_t challenge_id;
    int32_t monster_gid;
    PACKET_DEFINE(challenge_id, monster_gid);
};

class GetBossChallengeAward : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetBossChallengeAward, GET_BOSS_CHALLENGE_AWARD);
public:
    int32_t challenge_id;
    int32_t monster_gid;
    PACKET_DEFINE(challenge_id, monster_gid);
};

class GetClearChallengeAward : public ProtoPacket
{
    DECLARE_PROTOPACKET(GetClearChallengeAward, GET_CLEAR_CHALLENGE_AWARD);
public:
    int32_t challenge_id;
    PACKET_DEFINE(challenge_id);
};

class QueryWorldBossRecord : public ProtoPacket
{
    DECLARE_PROTOPACKET(QueryWorldBossRecord, QUERY_WORLD_BOSS_RECORD);
public:
    int32_t monster_tid;
    PACKET_DEFINE(monster_tid);
};

class TaskBGTransfer : public ProtoPacket
{
	DECLARE_PROTOPACKET(TaskBGTransfer, TASK_BG_TRANSFER);
public:
	int32_t taskid; 
	int32_t bg_templ_id;
	PACKET_DEFINE(taskid, bg_templ_id);
};

class ArenaODBuyTicket : public ProtoPacket
{
    // OD - offline data
    DECLARE_PROTOPACKET(ArenaODBuyTicket, ARENA_OD_BUY_TICKET);
public:
    int32_t ticket_count;
    PACKET_DEFINE(ticket_count);
};


///
/// 从这里开始定义debug-cmd
///
class DebugCommonCmd : public ProtoPacket
{
	DECLARE_PROTOPACKET(DebugCommonCmd, DEBUG_COMMON_CMD);

	struct param_value
	{
		enum 
		{
			VAR_IS_INT    = 1,
			VAR_IS_DOUBLE = 2,
		};

		union arithmeticType
		{
			int64_t i_var;
			double  d_var;
		};

		void Pack(shared::net::ByteBuffer& buf)
		{
			buf << var_type;
			if (var_type == VAR_IS_INT)
				buf << value.i_var;
			else if (var_type == VAR_IS_DOUBLE)
				buf << value.d_var;
			else
				assert(false);
		}

		void UnPack(shared::net::ByteBuffer& buf)
		{
			buf >> var_type;
			if (var_type == VAR_IS_INT)
				buf >> value.i_var;
			else if (var_type == VAR_IS_DOUBLE)
				buf >> value.d_var;
			else
				assert(false);
		}

		param_value() : var_type(0) { }

		uint8_t        var_type;
		arithmeticType value;
	};
	
	static const uint32_t kMaxParamCount = 8;

protected:
	mutable std::vector<param_value> param_list;

public:
	int32_t type;
	
	void SetParam(int64_t iparam)
	{
		if (param_list.size() >= kMaxParamCount)
			return;

		param_value tmp;
		tmp.var_type    = param_value::VAR_IS_INT;
		tmp.value.i_var = iparam;
		param_list.push_back(tmp);
	}

	void SetParam(double dparam)
	{
		if (param_list.size() >= kMaxParamCount)
			return;

		param_value tmp;
		tmp.var_type    = param_value::VAR_IS_DOUBLE;
		tmp.value.d_var = dparam;
		param_list.push_back(tmp);
	}

	static bool PopParam(const DebugCommonCmd& cmd, int64_t& iparam)
	{
		if (cmd.param_list.empty())
			return false;

		param_value tmpvalue = cmd.param_list.front();
		if (tmpvalue.var_type != param_value::VAR_IS_INT)
			return false;

		iparam = tmpvalue.value.i_var;
		cmd.param_list.erase(cmd.param_list.begin());
		return true;
	}

	static bool PopParam(const DebugCommonCmd& cmd, double& dparam)
	{
		if (cmd.param_list.empty())
			return false;

		param_value tmpvalue = cmd.param_list.front();
		if (tmpvalue.var_type == param_value::VAR_IS_DOUBLE)
        {
		    dparam = tmpvalue.value.d_var;
        }
        else if (tmpvalue.var_type == param_value::VAR_IS_INT)
        {
            dparam = tmpvalue.value.i_var;
        }
        else
        {
			return false;
        }

		cmd.param_list.erase(cmd.param_list.begin());
		return true;
	}

	PACKET_DEFINE(type, param_list);
};

class DebugGainItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(DebugGainItem, DEBUG_GAIN_ITEM);
public:
	int32_t item_type;
	int32_t item_count;
	PACKET_DEFINE(item_type, item_count);
};

class DebugCleanAllItem : public ProtoPacket
{
	DECLARE_PROTOPACKET(DebugCleanAllItem, DEBUG_CLEAN_ALL_ITEM);
public:
	int8_t where;
	PACKET_DEFINE(where);
};

class DebugChangePlayerPos : public ProtoPacket
{
	DECLARE_PROTOPACKET(DebugChangePlayerPos, DEBUG_CHANGE_PLAYER_POS);
public:
	int32_t world_id;
	float x,y;
	PACKET_DEFINE(world_id, x, y);
};

class DebugChangeFirstName : public ProtoPacket
{
    DECLARE_PROTOPACKET(DebugChangeFirstName, DEBUG_CHANGE_FIRST_NAME);
public:
    std::string first_name;
    PACKET_DEFINE(first_name);
};

}; // namespace C2G

#endif // GAMED_CLIENT_PROTO_C2G_PROTO_H_
