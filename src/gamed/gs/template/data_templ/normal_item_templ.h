#ifndef __GAMED_GS_TEMPLATE_DATA_TEMPL_NORMAL_ITEM_TEMPL_H__
#define __GAMED_GS_TEMPLATE_DATA_TEMPL_NORMAL_ITEM_TEMPL_H__

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief: 简单物品模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class NormalItemTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(NormalItemTempl, TEMPL_TYPE_NORMAL_ITEM);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

protected:
	virtual void OnMarshal() { }
	virtual void OnUnmarshal() { }
	virtual bool OnCheckDataValidity() const { return true;	}
};

}; // namespace dataTempl

#endif // __GAMED_GS_TEMPLATE_DATA_TEMPL_NORMAL_ITEM_TEMPL_H__
