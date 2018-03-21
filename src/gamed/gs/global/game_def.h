#ifndef GAMED_GS_GLOBAL_GAME_DEF_H_
#define GAMED_GS_GLOBAL_GAME_DEF_H_

#include <stdint.h>


namespace gamed {

	// 对以下定义的常量中，有逻辑关联的常量做一个检查
	bool CheckOptionsValidity();


	///
	/// 全局设置
	///
	static const int32_t  kMaxPlayerCount             = 8192;
	static const int32_t  kMaxNpcCount                = 164000; // 不可超过65536*16
	static const int32_t  kMaxMatterCount             = 65536;  // 动态物品不能超过2的16次方
	static const int32_t  kMaxAreaObjCount            = 8192;   // 不可超过8192
	static const int32_t  kMaxWorldManagerCount       = 16384;  // worldmanager的最多个数
	static const float    kDefaultSightRange          = 10.f;   // 玩家默认的视野距离（半径）
	static const int32_t  kWorldHeartbeatInterval     = 10;     // world的心跳频率，最好能被20整除，单位为tick数（默认每秒20个tick）
	static const int32_t  kMaxUnitSessions            = 8;      // Unit对象的session_list的最大长度
	static const int32_t  kSessionTimeout             = 15;     // 单个session可执行的最长时间，单位s
	static const float    kDefaultViewGridStep        = 6.0f;   // 默认视野网格的格子大小，单位m
	static const uint32_t kMaxNPCBWListSize           = 256;    // 黑名单、白名单的最大数量
    static const uint32_t kTeamMemberCount            = 4;      // 组队队伍成员的人数

	///
	/// 特殊模板id的定义（必须是1000以下，因为数据编辑器从1000开始分配）
	///  （1）增加特殊id时，需要在cpp里添上对应的检查，id不能重复
	///  （2）以下特殊tid需要与客户端的Share/network/protocol/gs/global/game_def.h 保持一致
	///
	static const int32_t kMapEffectsTemplID           = 10;     // 地图特效的templ id，特殊的模板id，数据编辑器里没有对应项
	static const int32_t kMapNpcZoneAnchorTemplID     = 11;     // 地图Npc区域的锚点templ id，用于通知客户端，该地图元素生效


	///
	/// 移动相关
	///
	static const int32_t kStdMoveUseTime              = 500;    // 移动的标准时间，单位ms
	static const int32_t kFastMoveSkipTime            = 150;    // 移动计算服务器、客户端之间用时差值的最大容忍值，单位ms
	static const int32_t kMinMoveUseTime              = 100;    // 移动耗时的最小时间，单位ms
	static const int32_t kMaxMoveUseTime              = 2000;   // 移动耗时的最大时间，单位ms
	static const float   kMaxMoveSpeed                = 20.f;   // 最大移动速度，单位米每秒
	static const float   kPlayerDefaultMoveSpeed      = 2.0f;   // 默认的移动速度，单位米每秒
	static const float   kNpcStrollUseMsecTime        = 1000;   // NPC漫步用时，单位ms
	static const float   kNpcFollowUseMsecTime        = 200;    // NPC跟随、追随用时，单位ms
	static const float   kNpcPatrolUseMsecTime        = 1000;   // NPC沿路径巡逻，走到某个目标的用时，单位ms
	static const float   kNpcStrollSpeed              = 2.0f;   // NPC漫步速度,为固定值，单位m/s
	static const float   kNpcDefaultFollowSpeed       = 3.0f;   // NPC跟随、追随的默认速度，单位m/s


	///
	/// 事件系统 相关
	///
	static const int32_t kAreaEvLuaEntityCount        = 16;     // 初始化多少个lua_State，多个state可以分摊锁竞争


	///
	/// player 相关
	///
	static const int     kMaxUIServicePerPlayer       = 128;    // 玩家身上最多能挂的服务数量
	static const float   kBroadCastEnterCombatRange   = 8.f;    // 给队友广播进入战斗消息的范围，可以把队友拉入战斗
	static const float   kBCEnterCombatRangeSquare    = kBroadCastEnterCombatRange * kBroadCastEnterCombatRange;
    static const float   kMaxPlayerDuelDistance       = 7.8f;    // 玩家最大的决斗距离
    static const float   kMaxPlayerDuelDisSquare      = kMaxPlayerDuelDistance * kMaxPlayerDuelDistance;


	///
	/// Npc、怪物相关
	///
	static const float   kMaxCombatDistance           = 2.f;    // 最大的有效战斗距离，即超过这个范围的两个object不能触发战斗
	static const float   kMaxCombatDistanceSquare     = kMaxCombatDistance * kMaxCombatDistance;
	static const float   kAggroNpcMaxCombatDis        = 1.f;    // 主动怪战斗距离，同时也是追击的最小距离，该值要小于kMaxCombatDistance
	static const float   kAggroNpcMaxCombatDisSquare  = kAggroNpcMaxCombatDis * kAggroNpcMaxCombatDis;
	static const float   kNpcMaxServiceDis            = 3.f;    // Npc提供Service的最大距离（半径）, 单位米
	static const float   kNpcMaxServiceDisSquare      = kNpcMaxServiceDis * kNpcMaxServiceDis;
	static const float   kMatterMaxGatherDis          = 3.f;    // Matter最大可采集距离
	static const float   kMatterMaxGatherDisSquare    = kMatterMaxGatherDis * kMatterMaxGatherDis;
	static const int     kNpcDefaultRebornInterval    = 10;     // 默认的重生时间（消失后刷新时间），单位s
	static const int     kMatterDefaultRebornInterval = 20;     // 默认重生时间（消失后刷新时间），单位s
	static const int     kMaxAggroListSize            = 16;     // 怪物仇恨列表大小
	static const float   kNpcDefaultAggroRange        = 2.f;    // 默认仇恨视野，所有npc的默认值，单位米
	static const int     kNpcDefaultAggroTime         = 60;     // 默认仇恨时间，所有npc的默认值，单位s
	static const int     kMaxServiceProviderPerNpc    = 32;     // 一个Npc最多能挂的服务数量

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GAME_DEF_H_
