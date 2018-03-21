#ifndef GAMED_GS_SUBSYS_PLAYER_TASK_H_
#define GAMED_GS_SUBSYS_PLAYER_TASK_H_

#include "game_module/task/include/task_data.h"

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player任务子系统
 */
class PlayerTask : public PlayerSubSystem
{
public:
	PlayerTask(Player& player);
	virtual ~PlayerTask();

	bool SaveToDB(common::PlayerTaskData* pData);
	bool LoadFromDB(const common::PlayerTaskData& data);
	
	virtual void OnRelease();
	virtual void OnLeaveWorld();
	virtual void OnHeartbeat(time_t cur_time);
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();
	
	inline task::ActiveTask* GetActiveTask();
	inline task::FinishTask* GetFinishTask();
	inline task::FinishTimeTask* GetFinishTimeTask();
	inline task::TaskDiary* GetTaskDiary();
	inline task::TaskStorage* GetTaskStorage();

	bool HasActiveTask(int32_t task_id);
	bool HasFinishTask(int32_t task_id);

	void KillMonster(int32_t monster_id, int32_t monster_num, std::vector<ItemEntry>& items);
	void PlayerGetTaskData();
    void StartRecvWorldTask();
    void SubscribeGlobalCounter(int32_t index, bool is_subscribe);
    bool GetGlobalCounter(int32_t index, int32_t& value) const;
    void ModifyGlobalCounter(int32_t index, int8_t op, int32_t delta);
    void SubscribeMapCounter(int32_t world_id, int32_t index, bool is_subscribe);
    void ClearMapCounterList(); // 离开地图是调用
    bool CombatFail(int32_t taskid); // 返回true表示该任务检查战斗失败
    bool IsTaskFinish(int32_t taskid) const;

	int  DeliverTask(int32_t taskid); // 返回0表示接任务成功
	void DeliverAward(int32_t task_id, int32_t choice);
	int  CanDeliverTask(int32_t taskid) const;

	bool MiniGameEnd(int32_t game_tid);


protected:
	//
	// CMD处理函数
	//
	void CMDHandler_TaskNotify(const C2G::TaskNotify&);
	void CMDHandler_TaskRegionTransfer(const C2G::TaskRegionTransfer&);
	void CMDHandler_TaskChangeName(const C2G::TaskChangeName&);
	void CMDHandler_TaskTransferGenderCls(const C2G::TaskTransferGenderCls&);
	void CMDHandler_TaskInsTransfer(const C2G::TaskInsTransfer&);
	void CMDHandler_UITaskRequest(const C2G::UITaskRequest&);
	void CMDHandler_UpdateTaskStorage(const C2G::UpdateTaskStorage&);
	void CMDHandler_TaskBGTransfer(const C2G::TaskBGTransfer&);

    //
    // MSG处理函数
    //
	int  MSGHandler_WorldDeliverTask(const MSG&);
    int  MSGHandler_GlobalCounterChange(const MSG&);
    int  MSGHandler_MapCounterChange(const MSG&);


private:
	bool CheckChangeNameParam(int32_t task_id, int8_t name_type, const std::string& name);
	bool CheckGenderClsParam(int32_t task_id, int8_t gender, int32_t cls);
    void MapCounterSubscribe(int32_t index, bool is_subscribe);
    void SendGlobalCounterChange(int32_t index, int32_t value);
	void SendChangeNameError();
    void FlushWaitingTasks();


private:
	task::ActiveTask     active_task_;
	task::FinishTask     finish_task_;
	task::FinishTimeTask finish_time_task_;
	task::TaskDiary		 task_diary_;
	task::TaskStorage	 task_storage_;

    // 没有getalldata前不能用直接发任务，先保存下来
	bool     is_getalldata_complete_;
    typedef std::vector<int32_t> WaitingTaskVec;
	WaitingTaskVec waiting_task_;

    // 订阅全局计数器，当该计数器变化时通知客户端
    typedef std::set<int32_t/*index*/> CounterSet;
    CounterSet    gcounter_set_;
    CounterSet    map_counter_set_;
};

///
/// inline func
///
inline task::ActiveTask* PlayerTask::GetActiveTask()
{
	return &active_task_;
}

inline task::FinishTask* PlayerTask::GetFinishTask()
{
	return &finish_task_;
}

inline task::FinishTimeTask* PlayerTask::GetFinishTimeTask()
{
	return &finish_time_task_;
}

inline task::TaskDiary* PlayerTask::GetTaskDiary()
{
	return &task_diary_;
}

inline task::TaskStorage* PlayerTask::GetTaskStorage()
{
	return &task_storage_;
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_TASK_H_
