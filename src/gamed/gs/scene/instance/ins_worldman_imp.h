#ifndef GAMED_GS_SCENE_INSTANCE_INS_WORLDMAN_IMP_H_
#define GAMED_GS_SCENE_INSTANCE_INS_WORLDMAN_IMP_H_

#include "gs/global/timer_lite.h"
#include "gs/scene/world_man.h"
#include "gs/scene/base_wm_imp.h"
#include "gs/global/msg_pack_def.h"

#include "ins_script_if.h"


namespace gamed {

struct msg_ins_finish_result;

/**
 * @brief InsWorldManImp
 *    （1）设计接口时注意线程安全问题
 *    （2）OnHeartbeat()和OnMessageHandler()跑在一个线程上
 */
class InsWorldManImp : public BaseWorldManImp
{
	static const int kKickoutCloseTime = 60 + 30; // 单位s
public:
	InsWorldManImp(WorldManager& worldMan);
	virtual ~InsWorldManImp();

	virtual bool OnInit();
	virtual void OnHeartbeat(); // 按秒心跳
	virtual int  OnMessageHandler(const MSG& msg);

	virtual bool OnInsertPlayer(Player* pplayer);
	virtual void OnPlayerEnterMap(RoleID roleid);
	virtual void OnPlayerLeaveMap(RoleID roleid);


protected:
	virtual void OnReady();
	virtual void OnRunning();
	virtual void OnClose();
	virtual void OnRemove();


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


private:
	enum InstanceStates
	{
		IS_INVALID,
		IS_READY,
		IS_RUNNING,
		IS_CLOSING,
	};

	enum InstanceResult
	{
		IR_UNDONE = 0,
		IR_KEY_MONSTER_ALL_KILLED, // player win
		IR_TASK_SET_P_SUCCESS,     // player win
		IR_TASK_SET_P_FAILURE,     // player fail
		IR_SCRIPT_SET_P_SUCCESS,   // player win
		IR_SCRIPT_SET_P_FAILURE,   // player fail
        IR_ALL_PLAYER_QUIT,        // player fail
	};

	
private:
	void    DriveMachine();
	void    UpdateStatus(InstanceStates state);
	void    RefreshInstanceResult();
	
	bool    IsPlayersVictory() const;
	void    SysCloseInstance(const MSG& msg);
	void    CalcInsResult();
	void    BreakServerRecord(const MapTeamInfo& info, int32_t masterid, int32_t ins_tid, int32_t clear_time);
	void    SendInsResult(RoleID roleid, const msg_ins_finish_result& param);
   
    inline bool ins_is_running() const;


private:
	world::instance_info ins_info_;  // 只有init的时候会设置，副本生命周期内不会变化
	InstanceStates       state_;

	int32_t    running_endtime_; // RUNNING的结束时间，肯定比max_endtime_小
	int32_t    closing_endtime_; // CLOSING状态的结束时间
	int32_t    max_endtime_;     // 最长结束时间
	int32_t    remain_secs_;     // 副本里没有玩家时，副本保留多长时间

	int32_t    player_count_;    // 副本内的玩家数量，刷新结束时间用
	int32_t    instance_type_;   // 副本类型：对应InstanceTempl::InstanceType枚举
    int32_t    ins_team_id_;     // 副本地图组队id

	// 副本结果
	InstanceResult    instance_result_;

	bool              has_key_monster_;
	std::set<int32_t> key_monster_set_;

	// 副本脚本接口
	InsScriptIf       ins_script_if_;
};

///
/// inline func
///
inline bool InsWorldManImp::ins_is_running() const
{
	return state_ == IS_RUNNING || state_ == IS_READY;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_INSTANCE_INS_WORLDMAN_IMP_H_
