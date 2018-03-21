#ifndef GAMED_GS_TEMPLATE_DATATEMPL_TASK_ITEM_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_TASK_ITEM_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief: 任务物品模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class TaskItemTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(TaskItemTempl, TEMPL_TYPE_TASK_ITEM);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

protected:
	virtual void OnMarshal() { }
	virtual void OnUnmarshal() { }
	virtual bool OnCheckDataValidity() const { return true;	}
};

}; // namespace dataTempl


#endif // GAMED_GS_TEMPLATE_DATATEMPL_TASK_ITEM_TEMPL_H_
