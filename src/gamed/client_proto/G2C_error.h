#ifndef GAMED_CLIENT_PROTO_G2C_ERROR_H_
#define GAMED_CLIENT_PROTO_G2C_ERROR_H_


namespace G2C {

///
/// 本文件错误码与客户端错误提示一一对应，原则上只增加不删除
///

enum ERR_CODE
{
// 0
	ERR_SUCCESS,
	ERR_FATAL_ERROR,                    // 致命错误
	ERR_CANNOT_USE_ITEM,                // 使用物品失败
	ERR_SELECT_SKILL_NO_ITEM,           // 选择技能失败(无技能道具)
	ERR_SELECT_SKILL_NO_POWER,          // 选择技能失败(无技能消耗)

// 5
	ERR_SELECT_SKILL_COOLING,           // 施放技能失败(技能冷却中)
	ERR_SELECT_SKILL_LIMIT,             // 选择技能失败(ATB已到,但是攻击目标未归位,此时禁止选择技能)
	ERR_SERVICE_UNAVILABLE,             // 服务不可用
	ERR_SERVICE_ERR_REQUEST,            // 错误的服务请求
	ERR_CANNOT_REGION_TRANSPORT,        // 不能进行区域传送

// 10
	ERR_CUR_STATE_CANNOT_TRANSPORT,     // 当前状态不能传送
	ERR_LONGJUMP_FAILURE,               // 跳转失败
	ERR_INV_FULL,                       // 包裹满
	ERR_DONOT_MATCH_INS_ENTER_COND,     // 不满足副本进入条件
	ERR_HAS_TEAM_WITH_NPC,              // 已经和NPC组队，不能和玩家组队

// 15
	ERR_OUT_OF_RANGE,                   // 超出有效范围，比如离矿、NPC太远
	ERR_MINE_HAS_BEEN_LOCKED,           // 矿物已被锁定
	ERR_DONOT_MATCH_INS_CREATE_COND,    // 不满足副本创建条件
	ERR_TEAM_INS_NOT_CREATE,            // 组队副本还没有创建，等待队长创建
	ERR_INS_NOT_EXIST,                  // 副本不存在

// 20
	ERR_ALREADY_HAS_INS,                // 玩家已经创建副本，不能再次创建或创建新的副本
	ERR_NO_MONEY,                       // 钱不足
	ERR_NO_CASH,                        // 元宝不足
	ERR_INS_GROUP_NOT_FOUND,            // 对应的副本组模板没有找到
	ERR_IS_NOT_TEAM_LEADER,             // 不是队长，一些操作只能由队长来执行

// 25
	ERR_TRANSFER_NOT_MEET_LEVEL,        // 传送不满足等级条件
	ERR_NO_SCORE,                       // 学分不足
	ERR_TRANSFER_NOT_MEET_COND,         // 条件不满足，无法传送
    ERR_NOT_IN_TEAM,                    // 已经不在队伍中
    ERR_TEAM_LEADER_CONVENE,            // 队长召集信息出错
    
// 30
    ERR_TEAM_LEADER_CHANGE,             // 队长已经更换
    ERR_THIS_MAP_CAN_NOT_EXE,           // 本地图不能执行该操作
    ERR_ITEM_CAN_NOT_AUCTION,           // 该物品不能拍卖
    ERR_CROSS_REALM_FORBID,             // 跨服禁止这种操作
    ERR_THIS_MAP_FORBID,                // 本地图禁止这种操作

// 35
    ERR_CAN_NOT_JOIN_IN_INS,            // 当前状态无法加入副本，在战斗中或已在副本地图
    ERR_TOO_FAR_FOR_DUEL,               // 距离太远无法决斗
    ERR_TEAMMATE_CAN_NOT_DUEL,          // 队友之间不能决斗
    ERR_CREATE_COMBAT_FAILURE,          // 创建战斗失败
    ERR_LEADER_TOO_FAR,                 // 本队队长太远请求无法发起

// 40
    ERR_LEADER_BUSY,                    // 本队队长很忙，无法响应请求
    ERR_DUEL_ALREADY_START,             // 对方已经开始决斗
    ERR_HAS_NOT_PRIMARY_PROP_RF,        // 没有可以强化的初阶属性
    ERR_DONOT_MATCH_BG_ENTER_COND,      // 不满足战场进入条件
    ERR_PPRF_CUR_ENERGY_FULL,           // 当前初阶属性强化能量已达上限

// 45
    ERR_SAME_CLS,                       // 转职的时候，转变的职业跟当前职业相同
    ERR_THE_PLAYER_HAS_TEAM,            // 对方已经在队伍中
    ERR_THE_PLAYER_HAS_NO_TEAM,         // 对方没有队伍
    ERR_ALREADY_IN_TEAM,                // 已经在队伍中
    ERR_ALREADY_NOT_IN_TEAM,            // 已经不再队伍中

// 50
    ERR_TEAM_INVITE_NOT_AGREE,          // 对方拒绝您的组队申请
    ERR_TEAM_APPLY_NOT_AGREE,           // 对方拒绝您的组队邀请
    ERR_THE_TEAM_IS_FULL,               // 对方队伍已满
    ERR_OUT_OF_SPECIFY_MAP_AREA,        // 不在指定的地图区域
    ERR_NOT_IN_SAME_SERVER,             // 不在同一个服务器无法执行该操作

// 55
    ERR_ALREADY_IN_COMBAT,              // 已经在战斗中
    ERR_JOIN_DUEL_COMBAT_FAIL,          // 加入决斗失败
    ERR_THE_PEER_IS_LEAVE,              // 对方已离开
    ERR_ITEM_CANNOT_DISASSEMBLE,        // 该物品无法分解
    ERR_INV_FULL_CANNOT_DISASSEMBLE,    // 包裹已满，无法分解物品

// 60
    ERR_DISASSEMBLE_FAILURE,            // 物品分解失败
    ERR_GAIN_ITEM_FAILURE,              // 获得物品失败
    ERR_CMD_PARAM_INVALID,              // 协议的参数有误
    ERR_WORLD_BOSS_NOT_FOUND,           // 没有找到对应的世界BOSS
    ERR_WORLD_BOSS_NO_RANKING,          // 该世界BOSS没有伤害排行

// 65
    ERR_WORLD_BOSS_DAILY_LIMIT,         // 超过世界BOSS每日开启次数

	ERR_MAX,
};


// 
// 换队长错误
//
enum ChangeLeaderError
{
	CLE_SUCC          = 0,
	CLE_NOT_LEADER    = 1,
	CLE_NOT_TEAMMATE  = 2,
	CLE_TEAM_NOTEXIST = 3,
};

}; // namespace G2C

#endif // GAMED_CLIENT_PROTO_G2C_ERROR_H_
