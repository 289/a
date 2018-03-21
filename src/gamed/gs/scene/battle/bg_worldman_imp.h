#ifndef GAMED_GS_SCENE_BATTLE_BG_WORLDMAN_IMP_H_
#define GAMED_GS_SCENE_BATTLE_BG_WORLDMAN_IMP_H_

#include "gs/scene/base_wm_imp.h"

#include "bg_script_if.h"


namespace gamed {

class BattleGroundLite;

/**
 * @brief BGWorldManImp
 *    （1）设计接口时注意线程安全问题
 *    （2）OnHeartbeat()和OnMessageHandler()跑在一个线程上
 */
class BGWorldManImp : public BaseWorldManImp
{
    // 友元类只能访问protected及public函数
    friend class BGPveWorldBoss;

    static const int kMaxRemainTime    = 3*60; // 单位秒，小于玩家最小人数时，多长时间后关闭
	static const int kKickoutCloseTime = 60;   // 单位s
public:
	BGWorldManImp(WorldManager& worldMan);
	virtual ~BGWorldManImp();

    virtual bool OnInit();
	virtual void OnHeartbeat(); // 按秒心跳
	virtual int  OnMessageHandler(const MSG& msg);

    virtual bool OnInsertPlayer(Player* pplayer);
	virtual void OnPlayerEnterMap(RoleID roleid);
	virtual void OnPlayerLeaveMap(RoleID roleid);
    virtual bool OnCheckPlayerCountLimit(world::BGEnterType enter_type) const;


protected:
    // auto create team
    virtual bool IsAutoMapTeam() const;
    virtual int  GetAutoMapTeamID(const Player* pplayer) const;
    virtual void NewMapTeamCreated(const MapTeamInfo& info);
    // function
	virtual void OnModifyCounter(int32_t index, int32_t value);
    virtual void OnClockTimeUp(int32_t index);
    virtual void OnMonsterKilled(int32_t monster_tid, int32_t count);
    virtual void OnPlayerGatherMine(int64_t roleid, int32_t mine_tid);
    virtual void OnPlayerQuitMap(int32_t member_count); // 玩家主动离开地图 
    virtual void OnReachDestination(int32_t elem_id, const A2DVECTOR& pos);


protected:
	virtual void OnReady();
	virtual void OnRunning();
	virtual void OnClose();
	virtual void OnRemove();


protected:
	enum BGStates
	{
		BS_INVALID,
		BS_READY,
		BS_RUNNING,
		BS_CLOSING,
	};

    enum BGResult
	{
		BGR_UNDONE = 0,
        BGR_ALL_PLAYER_QUIT,
        BGR_SYS_TASK_CLOSE,
        BGR_SYS_SCRIPT_CLOSE,
        BGR_SYS_WORLD_BOSS_DEAD,
	};

    void SetBattleGroundFinish(BGResult res);


private:
    void    DriveMachine();
	void    UpdateStatus(BGStates state);
	void    SysCloseBattleGround(const MSG& msg);
    void    SendNotifyBGStatus(int status);
    void    SendNotifyBGPlayerCount();
    void    HandleBGResult();

    inline bool bg_is_running() const;


private:
	world::battleground_info bg_info_;  // 只有init的时候会设置，战场生命周期内不会变化
	BGStates   state_;

    int32_t    running_endtime_;   // RUNNING的结束时间，肯定比max_endtime_小
	int32_t    closing_endtime_;   // CLOSING状态的结束时间
	int32_t    max_endtime_;       // 最长结束时间，绝对时间

	int32_t    player_count_;      // 战场内的玩家数量，刷新结束时间用
    int32_t    player_min_count_;  // 玩家最小数量，小于这个值战场关闭
    int32_t    player_max_count_;  // 玩家最大数量，大于这个值是不允许玩家进入
	int32_t    bg_type_;           // 战场类型：对应BattleGroundTempl::BGType枚举
    int32_t    supplement_time_;   // 战场允许补人时间，绝对时间
    bool       is_auto_map_team_;  // 战场是否支持预组队

    // 战场结果
	BGResult   bg_result_;

    // 战场脚本接口
	BGScriptIf bg_script_if_;

    // 用于不同战场的imp
    BattleGroundLite* bg_lite_;
};

///
/// inline func
///
inline bool BGWorldManImp::bg_is_running() const
{
    return state_ == BS_RUNNING || state_ == BS_READY;
}


/**
 * @brief 不同类型战场的多态基类
 */
class BattleGroundLite
{
public:
    BattleGroundLite(BGWorldManImp& worldManImp)
        : worldman_imp_(worldManImp)
    { }
    virtual ~BattleGroundLite() { }

    virtual bool OnInit()                         { return true; }
	virtual int  OnMessageHandler(const MSG& msg) { return -1; }
	virtual void OnHeartbeat()                    { } // 按秒心跳

protected:
	BGWorldManImp& worldman_imp_;
};

} // namespace gamed

#endif // GAMED_GS_SCENE_BATTLE_BG_WORLDMAN_IMP_H_
