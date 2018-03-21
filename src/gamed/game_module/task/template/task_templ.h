#ifndef TASK_TASK_TEMPL_H_
#define TASK_TASK_TEMPL_H_

#include "base_task_templ.h"
#include "task_limit.h"
#include "talk_proc.h"
#include "path_info.h"
#include "task_premise.h"
#include "task_op.h"
#include "task_succ.h"
#include "task_fail.h"
#include "task_award.h"

namespace task
{

class TaskTempl : public BaseTempl
{
	DECLARE_SYS_TEMPL(TaskTempl, TEMPL_TYPE_TASK);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;

	inline bool can_giveup() const;
	inline bool can_seek() const;
	inline bool show_deliver_tips() const;
	inline bool show_complete_tips() const;
	inline bool hide_task() const;
	inline bool auto_deliver() const;
	inline bool can_redo_after_succ() const;
	inline bool can_redo_after_fail() const;
	inline bool record_result() const;
    inline bool limit_count() const;
	inline bool share_task() const;
	inline bool fail_trigger_parent_fail() const;
	inline bool succ_trigger_parent_succ() const;
	inline bool ui_task() const;
	inline bool cat_vision() const;
    inline bool cross_trans() const;
    inline bool close_npc_area() const;
    inline bool prior_track() const;
    inline bool mag_task() const;
public:
	// 基本信息
	std::string name;
	int8_t type;
    int32_t flag;
	int8_t finish_mode;	
    std::string track_err_msg;

	// 主任务属性
	int8_t tag1;
	int8_t tag2;
	int8_t quality;
	int8_t subtask_deliver_mode;	

	// 子任务属性
	int32_t deliver_prob;

	// NPC配置
	int32_t deliver_npc;
	int32_t finish_npc;

    // 对话配置
	TalkProcVec deliver_talk;
	TalkProcVec unqualified_talk;
	TalkProcVec unfinish_talk;
	TalkProcVec finish_talk;

	// 任务描述
	std::string desc;				// 任务描述
	std::string can_deliver_desc;	// 可接任务描述
	std::string hint_desc;			// 进行中快捷栏描述
	std::string hint_award_desc;	// 奖励快捷栏描述
	std::string succ_cond_desc;	    // 成功目标描述
	std::string succ_diary;			// 成功日记
	std::string fail_diary;			// 失败日记

    // 优先任务追踪
    PriorTrack track;

    // 物品快速使用提示
    ItemUseHint item_hint;

    // 顶层任务属性
    TaskLimit limit;

	// 任务寻径
	PathFinder path_finder;

	// 开启条件
	TaskPremise premise;

	// 开启操作
	TaskOp op;

	// 任务完成条件
	TaskSucc succ_cond;
	TaskFail fail_cond;

	// 奖励
	TaskAward succ_award;
	TaskAward fail_award;

	// 层次结构
	TaskID parent;
	TaskID next_sibling;
	TaskID first_child;

	// 以下部分不存盘，任务数据加载到内存后设置
	const TaskTempl* parent_templ;
	const TaskTempl* next_sibling_templ;
	const TaskTempl* first_child_templ;
	StorageID sid;
#ifdef CLIENT_SIDE
	std::string level;
#endif
};

#include "task_templ-inl.h"

} // namespace task

#endif // TASK_TASK_TEMPL_H_
