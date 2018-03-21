#ifndef GAMED_GS_SCENE_EXT_WM_IMP_H_
#define GAMED_GS_SCENE_EXT_WM_IMP_H_

#include "shared/net/packet/bytebuffer.h"
#include "shared/net/packet/packet_util.h"

#include "gs/global/timer_lite.h"
#include "gs/global/msg_pack_def.h"

#include "world_man.h"


namespace gamed {

/**
 * @brief BaseWorldManImp
 *    （1）把一些功能地图的基础功能抽象出来，比如地图内组队、计时器、计数器等等
 *    （2）设计接口时注意线程安全问题
 *    （3）OnHeartbeat()和OnMessageHandler()跑在一个线程上
 */
class BaseWorldManImp : public WorldManagerImp
{
    static const int kMaxCounterCount  = 101; // 1到100的下标有效
	static const int kMaxClockCount    = 101; // 1到100的下标有效
	static const int kTeamMemberCount  = 4;
public:
    BaseWorldManImp(WorldManager& worldMan);
	virtual ~BaseWorldManImp();

    //
    // 子类继承注意显式调用一下几个函数
    //
	virtual bool OnInit();
	virtual void OnHeartbeat(); // 按秒心跳
	virtual int  OnMessageHandler(const MSG& msg);
	virtual bool OnInsertPlayer(Player* pplayer);
	virtual void OnPlayerLeaveMap(RoleID roleid);
    virtual void OnSetMapTeamInfo(void* buf, int len);

    int32_t GetCounter(int32_t index) const;
    bool    IsCounterLocked(int32_t index) const;

    // static function
    static bool CheckCounterIndex(int32_t index);


protected:
    typedef msgpack_map_team_info MapTeamInfo;
    typedef shared::net::ProtoPacket& PacketRef;

    ///
    /// 子类按需实现
    ///
    // auto create team
    virtual bool IsAutoMapTeam() const                         { return false; } // 是否需要自动组队
    virtual int  GetAutoMapTeamID(const Player* pplayer) const { return -1; }    // 返回队伍id，大于0为有效值
    virtual void NewMapTeamCreated(const MapTeamInfo& info)    { }
    // function
    virtual void OnModifyCounter(int32_t index, int32_t value) { }
    virtual void OnClockTimeUp(int32_t index)                  { }
    virtual void OnMonsterKilled(int32_t monster_tid, int32_t count)  { }
    virtual void OnPlayerGatherMine(int64_t roleid, int32_t mine_tid) { }
    virtual void OnPlayerQuitMap(int32_t member_count)         { } // 玩家主动离开地图 
    virtual void OnReachDestination(int32_t elem_id, const A2DVECTOR& pos) { }

    ///
    /// 子类可使用
    ///
    void    CopyPlayerInfo(const Player* pplayer, map_team_member_info& info);
    void    CopyPlayerInfo(const map_team_player_info& join_info, map_team_member_info& info);
    void    QueryPlayerInfoInTeam(); // 在子类地图running的时候调用
    void    GetAllTeamInfo(std::vector<MapTeamInfo>& info_list) const;
    // ---- thread safe ---
    void    SendPlayerMsg(int message, const XID& player, int64_t param = 0, const void* buf = NULL, size_t len = 0);
    void    SendObjectMsg(int message, const XID& obj, int64_t param = 0, const void* buf = NULL, size_t len = 0);
    void    SendPlayerPacket(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet);


private:
    struct CounterInfo
	{
		CounterInfo() : value(0), is_locked(false) { }
		int32_t value;
        bool    is_locked;
	};

	struct ClockInfo
	{
		ClockInfo() : times(0), seconds(0) { }
		int32_t times;
		int32_t seconds;
	};
	

private:
	void    ModifyCounter(int32_t op_type, int32_t index, int32_t value);
	void    CheckClockTimeup(time_t now);
    void    ActivateClock(int32_t index, int32_t times, int32_t secs);
    void    DeactivateClock(int32_t index);
    void    SetClockNextTime(time_t now, int64_t nextTime);
    void    MonsterKilled(int32_t monster_tid, int32_t count);
    void    PlayerGatherMine(int64_t roleid, int32_t mine_tid);
    void    PlayerQuitMap(const XID& player);
    void    DeliverTaskToAll(int32_t task_id);
    void    MapPromptMessage(const MSG& msg);
    void    ShowMapCountDown(const MSG& msg);
    void    MapCounterSubscribe(RoleID roleid, int32_t index, bool is_subscribe);
    void    NotifyPlayerCounterChange(int32_t index, int32_t value);
    void    NotifyMapCounterChange(RoleID roleid, int32_t index, int32_t value);
    void    HandleSpotMapElemTeleport(const MSG& msg);
    void    HandleSpotMonsterMove(const MSG& msg);
    void    HandleSpotMonsterSpeed(const MSG& msg);
    void    HandleReachDestination(const MSG& msg);
    void    BroadcastToAllPlayers(PacketRef packet);
    void    GetAllTeamInfoReal(std::vector<MapTeamInfo>& info_list) const;
    void    LockCounter(int32_t index, int32_t value);
    void    UnlockCounter(int32_t index);
    //
    // 地图内组队
    // （1）发给队员的组队MSG，都要进循序消息队列
	// （2）组队相关函数都是线程不安全的，需要外部加锁
    //
    void    QueryMapTeamInfo(const XID& player);
    void    SyncMapTeamInfo(Player* pplayer, MapTeamInfo* pinfo);
    void    SyncMapTeamInfo(RoleID roleid, MapTeamInfo* pinfo);
    void    PlayerJoinTeam(const Player* pplayer, MapTeamInfo* pinfo, size_t pos);
    void    PlayerJoinTeam(const map_team_player_info& info, MapTeamInfo* team_info, size_t pos);
    void    PlayerQuitTeam(const XID& player);
    void    PlayerChangePos(const MSG& msg);
    void    PlayerChangeLeader(const MSG& msg);
    void    ChangeLeader(RoleID newleader, MapTeamInfo* pinfo);
    bool    PlayerEnterMapTeam(Player* pplayer);
    void    PlayerLeaveMapTeam(RoleID playerid);
    void    PlayerOffline(RoleID playerid);
    void    PlayerKickoutMember(RoleID leader, RoleID kicked_roleid);
    void    ResetQueryPInfoTimeout();
	void    UpdatePlayerInfo(const XID& player, int32_t combat_value);
    void    PlayerApplyForJoinTeam(const MSG& msg);
    void    SendMemberJoinTeam(const MapTeamInfo* team_info, size_t pos, RoleID except_id);
    MapTeamInfo* CreateNewTeam(size_t pos, const Player* pplayer);
    MapTeamInfo* CreateNewTeam(size_t pos, const map_team_player_info& info);
    MapTeamInfo* CreateNewTeam(const MapTeamInfo& info);
    void    DismissTeam(int32_t team_id);
    void    TidyMapTeamPos(MapTeamInfo* team_info, RoleID except_id);

    // **** thread unsafe ****
    void    SendMemberMsg(const MapTeamInfo* team_info, int message, int64_t param = 0, 
                          const void* buf = NULL, size_t len = 0, RoleID except_id = 0);
    void    SendPlayerError(RoleID roleid, int err_no);

	inline bool    check_team_index(int index) const;
    inline int     get_clock_index(const ClockInfo& clock) const;
	inline int32_t get_next_map_team_id();


private:
    int32_t    cur_team_id_;
    int32_t    query_pinfo_timeout_;  // 查询地图内玩家信息，包括战斗力等

    // 计数器
	shared::net::FixedArray<CounterInfo, kMaxCounterCount> counter_list_;

	// 计时器
	shared::net::FixedArray<ClockInfo, kMaxClockCount> clock_list_;

	// clock timer
	int32_t               check_clock_timeup_;
	TimerLite<ClockInfo>  clock_timer_;
	std::vector<TimerLite<ClockInfo>::Entry> expired_vec_;

    // map team
    typedef std::map<int32_t/*team_id*/, MapTeamInfo> MapTeamInfoMap;
	mutable MutexLock mutex_team_;
	MapTeamInfoMap    team_info_map_;

    // counter subscribe
    typedef std::set<RoleID> RoleIDSet;
    typedef std::map<int32_t/*index*/, RoleIDSet> CounterSubscribeMap;
    CounterSubscribeMap counter_subscribe_map_;
};

///
/// inline function
///
inline bool BaseWorldManImp::check_team_index(int index) const
{   
	if (index < 0 || index >= kTeamMemberCount)
		return false;
	return true;
}

inline int BaseWorldManImp::get_clock_index(const ClockInfo& clock) const
{
	int index = &clock - &clock_list_[0];
	ASSERT(index >= 0 && index < kMaxClockCount);
	return index;
}

inline int32_t BaseWorldManImp::get_next_map_team_id()
{
    return ++cur_team_id_;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_EXT_WM_IMP_H_
