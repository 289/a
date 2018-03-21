#ifndef TASK_TASK_DATA_H_
#define TASK_TASK_DATA_H_

#include "task_types.h"

#define DECLARE_TASKDATA(name, type) \
	DECLARE_PACKET_COMMON_TEMPLATE(name, type, TaskData)

#define TASKDATA_DEFINE PACKET_DEFINE

namespace task
{

enum TaskDataType
{
	TASK_DATA_ACTIVE,
	TASK_DATA_FINISH,
	TASK_DATA_FINISH_TIME,
	TASK_DATA_DIARY,
	TASK_DATA_STORAGE,
};

enum TaskState
{
	TASK_STATE_FINISH   = 0x01,	    // 是否完成
	TASK_STATE_SUCC     = 0x02,		// 是否成功
	TASK_STATE_GIVEUP   = 0x04,	    // 是否放弃
	TASK_STATE_SKIP     = 0x08,		// 是否跳过，用于脚本任务
	TASK_STATE_TALK     = 0x10,		// 是否在显示对话
	TASK_STATE_SCRIPT   = 0x20,		// 脚本是否执行完毕
};

class TaskEntry
{
public:
	TaskEntry();
	TaskEntry(const TaskEntry& rhs);

	TaskEntry& operator=(const TaskEntry& rhs);

    void FinishGoal();
    bool IsFinishGoal() const;
    void SetScriptEnd();
    bool IsScriptEnd() const;

	inline void SetSuccess();
	inline void ClearSuccess();
	inline bool IsSuccess() const;
	inline void SetFinish();
    inline void ClearFinish();
	inline bool IsFinish() const;
	inline void SetGiveUp();
	inline bool IsGiveUp() const;
	inline void SetSkip();
	inline bool IsSkip() const;
	inline void SetShowTalk();
	inline void ClearShowTalk();
	inline bool IsShowTalk() const;
public:
	TaskID id;
	int8_t state;
	int32_t deliver_time;
	int32_t finish_time;
	std::string data;    

    bool script_end;
	const TaskTempl* templ; // 加载后生成，不存盘

	NESTED_DEFINE(id, state, deliver_time, finish_time, data);
};
typedef std::map<TaskID, TaskEntry> EntryMap;

class TaskData : public shared::net::BasePacket
{
protected:
	TaskData(uint16_t type); 
	TaskData(const TaskData& rhs);
public:
	EntryMap task_list;
	TASKDATA_DEFINE(task_list);

	virtual void AddEntry(const TaskEntry& entry);
	virtual void Erase(TaskID id);
	virtual void LoadComplete(int32_t now);

	inline bool IsExist(TaskID id) const;
	inline TaskEntry* GetEntry(TaskID id);
};

class ActiveTask : public TaskData
{
	DECLARE_TASKDATA(ActiveTask, TASK_DATA_ACTIVE);
public:
	int32_t storage_num;

	inline int32_t GetStorageNum() const;
	virtual void AddEntry(const TaskEntry& entry);
	virtual void Erase(TaskID id);
	virtual void LoadComplete(int32_t now);
	void PreSave(int32_t now);
};

class FinishTask : public TaskData
{
	DECLARE_TASKDATA(FinishTask, TASK_DATA_FINISH);
};

class FinishTimeTask : public TaskData
{
	DECLARE_TASKDATA(FinishTimeTask, TASK_DATA_FINISH_TIME);
public:
    virtual void AddEntry(const TaskEntry& entry); 
};

class DiaryEntry
{
public:
	DiaryEntry();
	DiaryEntry(const TaskTempl* task, int8_t task_state, int32_t finish_time);

	TaskID id;
	int8_t state;
	int32_t time;

	const TaskTempl* templ; // 加载后生成，不存盘

	inline bool succ() const;
	const std::string& diary() const;

	NESTED_DEFINE(id, state, time);
};
typedef std::vector<DiaryEntry> DiaryVec;

class TaskDiary : public shared::net::BasePacket
{
	DECLARE_PACKET_COMMON_TEMPLATE(TaskDiary, TASK_DATA_DIARY, shared::net::BasePacket);
public:
	DiaryVec diary_list;
	TASKDATA_DEFINE(diary_list);

	void AddDiary(const TaskTempl* templ, int8_t state, int32_t time);
	void LoadComplete(int32_t now);
};

class StorageEntry
{
public:
	StorageEntry();

	int32_t time;
	TaskIDVec task_list;

	const StorageTempl* templ; // 加载后生成，不存盘

    int32_t GetUpdateInterval(int32_t now) const;

	NESTED_DEFINE(time, task_list);
};
typedef std::map<int32_t, StorageEntry> StorageMap;

class TaskStorage : public shared::net::BasePacket
{
	DECLARE_PACKET_COMMON_TEMPLATE(TaskStorage, TASK_DATA_STORAGE, shared::net::BasePacket);
public:
	StorageMap storage_list;
	TASKDATA_DEFINE(storage_list);

	void LoadComplete(int32_t now);
	StorageEntry& GetEntry(StorageID sid);
};

#include "task_data-inl.h"

} // namespace task

#endif // TASK_TASK_DATA_H_
