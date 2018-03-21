#ifndef TASK_TASK_MSG_H_
#define TASK_TASK_MSG_H_

#include "task_types.h"

#define DECLARE_NOTIFY(name, type) \
	DECLARE_PACKET_COMMON_TEMPLATE(name, type, TaskNotifyBase)

#define NOTIFY_DEFINE PACKET_DEFINE

namespace task
{

enum TaskNotifyType
{
	TASK_SVR_NOTIFY_ERR,			// 错误码
	TASK_SVR_NOTIFY_NEW,			// 新任务发放
	TASK_SVR_NOTIFY_FINISHED,		// 处于得到奖励状态
	TASK_SVR_NOTIFY_COMPLETE,		// 任务完毕
	TASK_SVR_NOTIFY_MONSTER_KILLED, // 杀怪数量
    TASK_SVR_NOTIFY_MODIFY,         // 任务数据变化
	TASK_SVR_NOTIFY_STORAGE_RES,	// 返回库任务
    TASK_SVR_NOTIFY_LIMIT_RES,      // 返回限制次数任务的当前数量

	TASK_CLT_NOTIFY_AUTO_DELV,		// 检查任务自动发放
	TASK_CLT_NOTIFY_CHECK_FINISH,	// 检查任务完成
	TASK_CLT_NOTIFY_CHECK_AWARD,	// 检查任务自动发放奖励
	TASK_CLT_NOTIFY_GIVE_UP,		// 放弃任务
	TASK_CLT_NOTIFY_SCRIPT_END,		// 剧情脚本执行完毕
	TASK_CLT_NOTIFY_GUIDE_END,		// 新手指引执行完成
	TASK_CLT_NOTIFY_MINIGAME_END,	// 小游戏完成
	TASK_CLT_NOTIFY_STORAGE_REQ,	// 请求获取库任务列表
    TASK_CLT_NOTIFY_LIMIT_REQ,      // 检查限制次数任务达到清除时间点
    TASK_CLT_NOTIFY_REVIVE,         // 检查玩家死亡失败任务
    TASK_CLT_NOTIFY_LOGIN,          // 检查玩家下线失败任务
    TASK_CLT_NOTIFY_UITASK,         // 检查UI任务是否可以接取
};

class TaskNotifyBase : public shared::net::BasePacket
{
public:
	TaskNotifyBase(uint16_t type);
	virtual ~TaskNotifyBase();

	virtual void Marshal();
	virtual void Unmarshal();
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	int32_t id;
};

class TaskNotifyErr : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyErr, TASK_SVR_NOTIFY_ERR);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	int32_t err;
};

class TaskNotifyNew : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyNew, TASK_SVR_NOTIFY_NEW);
};

class TaskNotifyFinish : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyFinish, TASK_SVR_NOTIFY_FINISHED);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	bool succ;
};

class TaskNotifyComplete : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyComplete, TASK_SVR_NOTIFY_COMPLETE);
};

class TaskNotifyMonsterKilled : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyMonsterKilled, TASK_SVR_NOTIFY_MONSTER_KILLED);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	int32_t monster_index;
	int32_t monster_count;
};

class TaskNotifyModify : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyModify, TASK_SVR_NOTIFY_MODIFY);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
};

class TaskNotifyStorageRes : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyStorageRes, TASK_SVR_NOTIFY_STORAGE_RES);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	bool override;
	TaskIDVec task_list;
};

class TaskNotifyLimitRes : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyLimitRes, TASK_SVR_NOTIFY_LIMIT_RES);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
    int32_t num;
};

class TaskNotifyAutoDeliver : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyAutoDeliver, TASK_CLT_NOTIFY_AUTO_DELV);
};

class TaskNotifyCheckFinish : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyCheckFinish, TASK_CLT_NOTIFY_CHECK_FINISH);
};

class TaskNotifyCheckAward : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyCheckAward, TASK_CLT_NOTIFY_CHECK_AWARD);
};

class TaskNotifyGiveUp : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyGiveUp, TASK_CLT_NOTIFY_GIVE_UP);
};

class TaskNotifyScriptEnd : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyScriptEnd, TASK_CLT_NOTIFY_SCRIPT_END);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	bool skip;
};

class TaskNotifyGuideEnd : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyGuideEnd, TASK_CLT_NOTIFY_GUIDE_END);
};

class TaskNotifyMiniGameEnd : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyMiniGameEnd, TASK_CLT_NOTIFY_MINIGAME_END);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
public:
	bool succ;
};

class TaskNotifyStorageReq : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyStorageReq, TASK_CLT_NOTIFY_STORAGE_REQ);
};

class TaskNotifyLimitReq : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyLimitReq, TASK_CLT_NOTIFY_LIMIT_REQ);
};

class TaskNotifyRevive : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyRevive, TASK_CLT_NOTIFY_REVIVE);
};

class TaskNotifyLogin : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyLogin, TASK_CLT_NOTIFY_LOGIN);
};

class TaskNotifyUITask : public TaskNotifyBase
{
	DECLARE_NOTIFY(TaskNotifyUITask, TASK_CLT_NOTIFY_UITASK);
};

} // namespace task

#endif // TASK_TASK_MSG_H_
