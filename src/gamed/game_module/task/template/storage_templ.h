#ifndef TASK_STORAGE_TEMPL_H_
#define TASK_STORAGE_TEMPL_H_

#include "base_task_templ.h"
#include "basic_info.h"

namespace task
{

class StorageTempl : public BaseTempl
{
	DECLARE_SYS_TEMPL(StorageTempl, TEMPL_TYPE_STORAGE);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;
public:
	// 库任务基本属性
	int32_t interval;	// 刷新间隔，0表示以天为单位更新
	int32_t max_num;	// 刷新库任务时产生的任务数目
	bool refresh;		// 库中任务成功完成后是否递补
    int8_t quality;     // 付费刷新必定出现的最低品质
    int32_t cash;       // 付费刷新花费的元宝

	// 库任务对应的任务信息列表
	TaskInfoVec task_list;

    // 库任务对应的任务模板列表
    TaskVec templ_list;
};

} // namespace task

#endif // TASK_STORAGE_TEMPL_H_
