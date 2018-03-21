#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_COUNTER_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_COUNTER_H_

#include "base_datatempl.h"


namespace dataTempl
{

/*
 * @brief: 全局计数器
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *        （1）本模板只是一个标记没有任何数据
 */
class GlobalCounter : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(GlobalCounter, TEMPL_TYPE_GLOBAL_COUNTER);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

protected:
	virtual void OnMarshal() { }
	virtual void OnUnmarshal() { }

	virtual bool OnCheckDataValidity() const
	{
		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_COUNTER_H_
